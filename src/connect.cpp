#include <memory>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "connect.hpp"


/**
 * @brief Checks if socket error is allowed upon send returned -1.
 *
 * @note @b errno set to a value diff. than @b EWOULDBLOCK or @b EAGAIN .
**/
auto is_unrecoverable_error() -> bool
{
    return errno != EWOULDBLOCK && errno != EAGAIN;
}


/**
 * @brief Checks if blocking error appeared.
 *
 * @note Blocking error is either @b EWOULDBLOCK or @b EAGAIN .
**/
auto is_blocking_error() -> bool
{
    return errno == EWOULDBLOCK || errno == EAGAIN;
}


SendConnect::SendConnect(int sock, const std::atomic_bool& done)
    : sock_(sock), done_(done)
{
}

auto SendConnect::try_send_buffer(const uint8_t* buf, std::size_t len) -> bool
{
    constexpr int64_t SEND_RECOVERY_TIMEOUT = 50;
    std::size_t idx = 0;

    while (!done_.load() && idx < len) {
        auto cnt = send(sock_, buf, len - idx, 0);

        // nothing is sent and errno is unrecoverable
        if (cnt == -1 && is_unrecoverable_error()) { break; }

        // sent no bytes or bytes less than expected, cnt is either -1 or >=0
        if ((cnt == -1 || (static_cast<size_t>(cnt) < (len - idx))) && is_blocking_error()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(SEND_RECOVERY_TIMEOUT));
        }

        if (cnt > 0) { idx += cnt; buf += cnt; }
    }

    return idx == len;
}

auto SendConnect::try_send_header(const std::string& msg) -> bool
{
    auto len = sizeof(uint32_t);
    auto hdr = static_cast<uint32_t>(msg.size());

    uint32_t buf[1] { htonl(hdr) }; // host-to-network byte order!

    return
        try_send_buffer(reinterpret_cast<const uint8_t*>(buf), len);
}

auto SendConnect::try_send_body(const std::string& msg) -> bool
{
    return
        try_send_buffer(reinterpret_cast<const uint8_t*>(msg.c_str()), msg.size());
}

auto SendConnect::try_send_message(const Message& msg) -> bool
{
    auto result = true;

    if (result) { result = try_send_header(msg); }
    if (result) { result = try_send_body(msg); }

    return result;
}


RecvConnect::RecvConnect(int sock, const std::atomic_bool& done)
    : sock_(sock), done_(done)
{
}

auto RecvConnect::try_recv_buffer(std::unique_ptr<uint8_t*>& buf, std::size_t len) -> bool
{
    constexpr int64_t RECV_RECOVERY_TIMEOUT = 50;
    std::size_t idx = 0;

    while (!done_.load() && idx < len) {
        auto cnt = recv(sock_, *buf + idx, len - idx, 0);

        // nothing is received and error is unrecoverable
        if (cnt == -1 && is_unrecoverable_error()) { break; }

        // received no bytes or bytes less than expected, cnt is either -1 or >=0
        if ((cnt == -1 || (static_cast<size_t>(cnt) < (len - idx))) && is_blocking_error()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(RECV_RECOVERY_TIMEOUT));
        }

        if (cnt > 0) { idx += cnt; }
    }

    return idx == len;
}

auto RecvConnect::recv_maybe_header() -> std::optional<std::size_t>
{
    std::optional<std::size_t> header;

    auto len = sizeof(uint32_t);
    auto hdr = std::make_unique<uint8_t*>(new uint8_t[len]);

    if (try_recv_buffer(hdr, len)) {
        auto size = ntohl(reinterpret_cast<uint32_t *>(*hdr)[0]); // network-to-host byte order!
        if (size != 0) {
            header = static_cast<std::size_t>(size);
        }
    }

    return header;
}

auto RecvConnect::recv_maybe_body(std::size_t len) -> std::optional<Message>
{
    std::optional<Message> result;
    auto hdr = std::make_unique<uint8_t*>(new uint8_t[len]);

    if (try_recv_buffer(hdr, len)) {
        result.emplace();
        for (std::size_t i = 0; i < len; ++i) {
            result->push_back(static_cast<char>((*hdr)[i]));
        }
    }

    return result;
}

auto RecvConnect::recv_maybe_message() -> std::optional<Message>
{
    std::optional<Message> result;

    auto header = recv_maybe_header();
    if (header.has_value()) { result = recv_maybe_body(*header); }

    return result;
}
