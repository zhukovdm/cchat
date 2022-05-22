#include <iostream>
#include <sstream>
#include "client_gui.hpp"
#include "client_session.hpp"
#include "message.hpp"


constexpr int64_t GUI_STORAGE_RATE = 100;


auto recv_gui_message(std::atomic_bool& done, PendingDeque& recv_gui) -> std::optional<Message>
{
    std::optional<Message> result;

    while (!done.load() && !result.has_value()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(GUI_STORAGE_RATE));
        result = recv_gui.maybe_pop();
    }

    return result;
}


ClientSession::ClientSession(int sock, std::string&& name)
    : Session(sock), name_(std::move(name)), send_gui_(), recv_gui_()
{
}

auto ClientSession::command_help() -> void
{
    std::vector<std::string> messages {
        "help: shows all commands with short description.",
        "hist # user_name: receive up to 10 last messages with the user.",
        "chat user_name: opens chat with a user, user could be offline.",
        "     Enter <$> to escape chat.",
        "pend: shows users with messages waiting to be delivered.",
        "quit: exits the program."
    };

    for (auto&& message : messages) { send_gui_.push_back(message); }
}

auto ClientSession::recv_sequence() -> void
{
    bool bit;
    std::optional<Message> msg;

    do {
        msg = recv_with_maybe_fail();
        bit = msg.has_value() && (*msg != TERMINATION_SYMBOL);
        if (bit) { send_gui_.push_back(*msg); }
    } while (bit);
}

auto ClientSession::serve() -> void
{
    std::vector<std::thread> services;
    Panel panel(
        (std::ostringstream()
            << "cchat as "
            << name_
        ).str()
    );

    while (!done_.load()) {
        switch (mode_)
        {
        case ClientMode::LOG_IN:
        {
            send_with_maybe_fail(name_);
            auto resp = recv_with_maybe_fail();

            if (done_.load() || (*resp != name_)) {
                std::cout
                    << "Log in as "
                    << name_
                    << " cannot be performed."
                    << std::endl;
                done_.store(true);
            }

            else {
                services.emplace_back([&]() {
                    Gui gui(panel, recv_gui_, send_gui_);
                    gui.loop(done_);
                });
            }

            mode_ = ClientMode::COMMAND;
        }
        break;
        case ClientMode::COMMAND:
        {
            // Here message will always have a value!!!
            auto maybe_msg = recv_gui_message(done_, recv_gui_);
            send_gui_.push_back(*maybe_msg);

            switch (parse_command(*maybe_msg))
            {
            case Command::HELP:
            {
                command_help();
            }
            break;
            case Command::PEND:
            case Command::HIST:
            {
                send_with_maybe_fail(*maybe_msg);
                recv_sequence();
            }
            break;
            case Command::QUIT:
            {
                send_with_maybe_fail(*maybe_msg);
                done_.store(true);
            }
            break;
            case Command::CHAT:
            {
                auto chat_opponent = parse_chat_command(*maybe_msg);
                send_with_maybe_fail(*maybe_msg);
                auto resp = recv_with_maybe_fail();

                if (resp.has_value() && (*resp == chat_opponent)) {
                    panel.store(panel.load() + " with " + (*resp));
                }

                mode_ = ClientMode::CHAT;
            }
            break;
            case Command::BAD:
            {
                send_gui_.push_back("Entered command is not recognized.");
            }
            break;
            default:
            {
                done_.store(true);
            }
            break;
            }
        }
        break;
        case ClientMode::CHAT:
        {
            std::atomic_bool chat_done(false);

            // receive messages
            std::thread t([&]() {
                while (!chat_done.load()) {
                    auto msg = RecvConnect(sock_, chat_done).recv_maybe_message();
                    if (!chat_done.load() && msg.has_value()) {
                        send_gui_.push_back(std::move(*msg));
                    }
                    chat_done.store(chat_done.load() || !msg.has_value());
                }
            });

            // send messages
            while (!chat_done.load()) {
                auto msg = recv_gui_message(chat_done, recv_gui_);
                if (msg.has_value()) {
                    if (*msg != END_OF_CHAT_SYMBOL) {
                        *msg = "[" + name_ + "] " + *msg;
                    }
                    send_gui_.push_back(*msg);
                    chat_done.store(!SendConnect(sock_, chat_done).try_send_message(*msg) || (*msg == END_OF_CHAT_SYMBOL));
                }
            }

            t.join();
            panel.store("cchat as " + name_);
            mode_ = ClientMode::COMMAND;
        }
        break;
        default:
        {
            done_.store(true);
        }
        break;
        }
    }

    for (auto&& service : services) { if (service.joinable()) { service.join(); } };
}

ClientSession::~ClientSession()
{
}
