#include <sstream>
#include <thread>
#include "connect.hpp"
#include "message.hpp"
#include "server_session.hpp"
#include "utility.hpp"


ServerSession::ServerSession(int sock, UserMap& users, HistoryMap& history, Logger<std::string>& logger)
    : Session(sock), users_(users), history_(history), logger_(logger)
{
}

auto ServerSession::get_ordered_pair(const UserId& u1, const UserId& u2) -> UserPair
{
    auto min = (u1 < u2) ? u1 : u2;
    auto max = (u1 > u2) ? u1 : u2;
    return { min, max };
}

auto ServerSession::try_log_in(const UserId& user_name) -> bool
{
    return is_user_name_valid(user_name)
        && users_.observe(user_name).try_acquire(sock_);
}

auto ServerSession::serve() -> void
{
    std::optional<UserId> maybe_user;
    UserId chat_opponent;

    while (!done_.load()) {
        switch (mode_)
        {
        case ClientMode::LOG_IN:
        {
            maybe_user = recv_with_maybe_fail();

            if (!done_.load()) {
                auto succ = try_log_in(*maybe_user);
                std::string suffix = (succ)
                    ? ("")
                    : (TERMINATION_SYMBOL);
                send_with_maybe_fail(*maybe_user + suffix);
                done_.store(!succ);
            }
            mode_ = ClientMode::COMMAND;
        }
        break;
        case ClientMode::COMMAND:
        {
            auto maybe_command = recv_with_maybe_fail();

            if (!done_.load()) {
                auto c = parse_command(*maybe_command);
                switch (c)
                {
                break;
                case Command::PEND:
                {
                    auto&& pending = users_.observe(*maybe_user).get_pending();
                    for (auto&& opponent : pending.keys()) {
                        if (!pending.observe(opponent).empty()) {
                            send_with_maybe_fail(opponent);
                        }
                    }
                    send_with_maybe_fail(TERMINATION_SYMBOL);
                }
                break;
                case Command::QUIT:
                {
                    done_.store(true);
                }
                break;
                case Command::CHAT:
                {
                    chat_opponent = parse_chat_command(*maybe_command);
                    send_with_maybe_fail(chat_opponent);
                    mode_ = ClientMode::CHAT;
                }
                break;
                case Command::HIST:
                {
                    auto [n, opponent] = parse_hist_command(*maybe_command);
                    auto hist = history_.observe(get_ordered_pair(*maybe_user, opponent)).get_last_n(n);
                    for (const auto& h : hist) { send_with_maybe_fail(h); }
                    send_with_maybe_fail(TERMINATION_SYMBOL);
                }
                break;
                case Command::BAD:
                case Command::HELP:
                default:
                {
                    logger_.log(
                        (std::ostringstream()
                            << "Bad Command received on socket "
                            << sock_
                            << ", internal Session error."
                        ).str()
                    );
                    done_.store(true);
                }
                break;
                }
            }
        }
        break;
        case ClientMode::CHAT:
        {
            constexpr int64_t CHAT_RATE = 50;
            logger_.log("Chat " + *maybe_user + " -> " + chat_opponent + " started.");

            // receive pendings for the opponent
            std::atomic_bool recv_done(false);
            std::thread t([&]() {
                auto&& pendings = users_.observe(chat_opponent).get_pending().observe(*maybe_user);

                while (!done_.load() && !recv_done.load()) {
                    auto msg = RecvConnect(sock_, recv_done).recv_maybe_message();
                    recv_done.store(!msg.has_value() || (msg.has_value() && (*msg == END_OF_CHAT_SYMBOL)));

                    if (!recv_done.load()) {
                        pendings.push_back(std::move(*msg));
                        std::this_thread::sleep_for(std::chrono::milliseconds(CHAT_RATE));
                    }
                }
            });

            // send opponent's pendings
            auto&& pending = users_.observe(*maybe_user).get_pending().observe(chat_opponent);
            auto&& history = history_.observe(get_ordered_pair(*maybe_user, chat_opponent));
            while (!done_.load() && !recv_done.load()) {
                auto msg = pending.maybe_pop();
                if (msg.has_value()) {
                    send_with_maybe_fail(*msg);
                    if (!done_.load()) { history.push_back(std::move(*msg)); }
                }

                if (done_.load()) { pending.push_front(std::move(*msg)); }

                std::this_thread::sleep_for(std::chrono::milliseconds(CHAT_RATE));
            }

            recv_done.store(true);
            if (t.joinable()) { t.join(); }
            mode_ = ClientMode::COMMAND;
            logger_.log("Chat " + *maybe_user + " -> " + chat_opponent + " ended.");
        }
        break;
        default:
        {
            logger_.log(
                (std::ostringstream()
                    << "Bad ClientMode on socket "
                    << sock_
                    << ", internal Session error."
                ).str()
            );
            done_.store(true);
        }
        break;
        }
    }

    if (maybe_user.has_value()) { users_.observe(*maybe_user).release(sock_); }

    logger_.log(
        (std::ostringstream()
            << "Socket "
            << sock_
            << " done in ClientMode "
            << static_cast<int>(mode_)
            << "."
        ).str()
    );
}

ServerSession::~ServerSession()
{
    close(sock_);
}
