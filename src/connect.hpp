#ifndef CONNECT_HPP_
#define CONNECT_HPP_


#include <atomic>
#include <memory>
#include <queue>
#include "logger.hpp"
#include "storage.hpp"


class SendConnect final
{
private:
    int sock_;
    const std::atomic_bool& done_;

    /**
     * @brief Sends passed buffer via non-blocking socket.
    **/
    bool try_send_buffer(const uint8_t* buf, std::size_t len);

    /**
     * @brief Generate header of a message and sends.
     *     Currently, header is equal to the length of a message.
    **/
    bool try_send_header(const std::string& msg);

    /**
     * @brief Sends char-by-char body of a message.
    **/
    bool try_send_body(const std::string& msg);

public:
    SendConnect(int sock, const std::atomic_bool& done);

    /**
     * @brief Send a message.
     *
     * @return True upon success, otherwise False.
    **/
    bool try_send_message(const Message& msg);
};


class RecvConnect final
{
private:
    int sock_;
    const std::atomic_bool& done_;

    /**
     * @brief Receives exactly @b len number of raw bytes.
     *
     * @note Socket shall be configured as non-blocking.
    **/
    bool try_recv_buffer(std::unique_ptr<uint8_t*>& buf, std::size_t len);

    /**
     * @brief Receives length of an incoming text message.
    **/
    std::optional<std::size_t> recv_maybe_header();

    /**
     * @brief Receives body part of a packet.
    **/
    std::optional<Message> recv_maybe_body(std::size_t len);

public:
    RecvConnect(int sock, const std::atomic_bool& done);

    /**
     * @brief Main loop of the RecvConnect receives header and message itself,
     *     stores the message to in-message storage.
    **/
    std::optional<Message> recv_maybe_message();
};


#endif
