#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <sys/socket.h>
#include <fcntl.h>
#include "utility.hpp"


auto split_string(const std::string& input) -> std::vector<std::string>
{
    std::string token;
    std::vector<std::string> result;
    std::unordered_set<char> seps{ ' ', '\t', '\r' };

    for (std::size_t i = 0; i < input.size(); ++i) {

        //separator found
        if (seps.contains(input[i])) {
            if (token.size() > 0) {
                result.emplace_back(std::move(token));
            }
        } else {
            token.push_back(input[i]);
        }
    }

    if (token.size() > 0) {
        result.emplace_back(std::move(token));
    }

    return result;
}


auto is_user_name_valid(const std::string& name) -> bool
{
    return name.size() > 0
        && std::all_of(name.begin(), name.end(), [](char c) { return std::isalnum(c); });
}


auto parse_port(const std::string& word) -> uint16_t
{
    // POSIX sockets use 16-bit long addresses.
    unsigned long MAX_PORT = 65535;

    for (std::size_t i = 0; i < word.size(); ++i) {
        if (!std::isdigit(word[i])) {
            std::invalid_argument("Port number is not properly formatted.");
        }
    }

    auto port = std::strtoul(word.c_str(), nullptr, 10);

    if (port == 0 || port > MAX_PORT) {
        std::invalid_argument("Port number is outside the allowed bounds.");
    }

    return static_cast<uint16_t>(port);
}


auto create_new_socket() -> int
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        throw std::runtime_error("Not possible to create new socket.");
    }
    return sock;
}


auto allow_socket_reuse(int sock) -> void
{
    int val = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) == -1) {
        throw std::runtime_error("Socket cannot be properly configured.");
    }
}


auto set_socket_non_blocking(int sock) -> void
{
    if (fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK) == -1) {
        throw std::runtime_error("Socket cannot be properly configured.");
    }
}
