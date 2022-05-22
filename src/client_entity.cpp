#include <cstring>
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "client_entity.hpp"
#include "client_session.hpp"
#include "utility.hpp"


auto Client::init(const ClientArgsParser& args) -> void
{
    name_ = args.get_value("name");

    if (!is_user_name_valid(name_)) {
        throw std::invalid_argument("User name is malformed.");
    }

    sock_ = create_new_socket();
    allow_socket_reuse(*sock_);

    sockaddr_in server_addr {
        .sin_family = AF_INET,
        .sin_port = htons(parse_port(args.get_value("port"))),
        .sin_addr = in_addr{htonl(INADDR_ANY)},
        .sin_zero = {  }
    };

    // try to convert host address into binary form
    if (inet_pton(AF_INET, args.get_value("host").c_str(), &server_addr.sin_addr) <= 0) {
        throw std::invalid_argument("Host address is not convertible into binary form.");
    }

    // try to connect server
    if (connect(*sock_, (sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        throw std::runtime_error("The connection with host is refused.");
    }

    // unblock AFTER connect!
    set_socket_non_blocking(*sock_);

    std::cout
        << "Client has established connection as "
        << name_
        << " on socket "
        << *sock_
        << ", host "
        << args.get_value("host")
        << ", port "
        << args.get_value("port")
        << "."
        << std::endl;
}

auto Client::loop() -> void
{
    ClientSession session(*sock_, std::move(name_));
    session.serve();
}

Client::~Client()
{
    std::cout
        << "Client goes down..."
        << std::endl;
}
