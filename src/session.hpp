#ifndef SESSION_HPP_
#define SESSION_HPP_


/**
 * @file
 *
 * This header file declares abstract class Session.
**/
#include <atomic>
#include <optional>
#include "connect.hpp"
#include "storage.hpp"


constexpr char TERMINATION_SYMBOL[] = "$";
constexpr char END_OF_CHAT_SYMBOL[] = "<$>";


/**
 * @brief Client mode is used on both Client and Server sides
 *     to implement state machine.
**/
enum class ClientMode
{
    LOG_IN,
    COMMAND,
    CHAT
};


/**
 * @brief Base class for both ClientSession and ServerSession.
 
 * @note On contrary, Client allocates only one socket upon start, which is released
 *     by the Client instance!
**/
class Session
{
protected:
    int sock_;
    ClientMode mode_;
    std::atomic_bool done_;

    Session(int sock);

    /**
     * @brief Tries to @b send a message. If sending fails, bit @b done_ is set.
    **/
    void send_with_maybe_fail(const Message& msg);

    /**
     * @brief Tries to @b recv a message. If receiving fails, bit @b done_ is set.
    **/
    std::optional<Message> recv_with_maybe_fail();

public:

    /**
     * @brief Abstract method.
    **/
    virtual void serve() = 0;

    Session(Session&&) = delete;
    Session(const Session&) = delete;
    Session& operator=(Session&&) = delete;
    Session& operator=(const Session&) = delete;
    virtual ~Session() {}
};

inline Session::Session(int sock)
    : sock_(sock), mode_(ClientMode::LOG_IN), done_(false)
{
}

inline auto Session::send_with_maybe_fail(const Message& msg) -> void
{
    done_.store(!SendConnect(sock_, done_).try_send_message(msg));
}

inline auto Session::recv_with_maybe_fail() -> std::optional<Message>
{
    auto maybe_result = RecvConnect(sock_, done_).recv_maybe_message();
    done_.store(!maybe_result.has_value());
    return maybe_result;
}

#endif
