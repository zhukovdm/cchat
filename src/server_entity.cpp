#include <cstring>
#include <iostream>
#include <sstream>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "server_session.hpp"
#include "server_entity.hpp"
#include "utility.hpp"


constexpr int BACKLOG = 1; // max length of the accepting queue


Server::Server()
    : Entity(), logger_(&std::cout)
{
}

auto Server::init(const ServerArgsParser& args) -> void
{
    sock_ = create_new_socket();
    allow_socket_reuse(*sock_);
    // server socket blocks on accept!

    sockaddr_in addr {
        .sin_family = AF_INET,
        .sin_port = htons(parse_port(args.get_value("port"))),
        .sin_addr = in_addr{htonl(INADDR_ANY)},
        .sin_zero = {  }
    };

    // assign a name to the socket
    if (bind(*sock_, (sockaddr *)&addr, sizeof(addr)) == -1) {
        throw std::runtime_error("Socket cannot be bound.");
    }

    // mark socket as accepting connections
    if (listen(*sock_, BACKLOG) == -1) {
        throw std::runtime_error("Socket cannot listen and accept.");
    }

    std::cout
        << "Server listens at socket "
        << *sock_
        << ", port "
        << args.get_value("port")
        << "."
        << std::endl;
}

auto Server::loop() -> void
{
    UserMap users;
    HistoryMap history;
    std::atomic_bool done(false);
    std::vector<std::thread> services;

    services.emplace_back([&]() { logger_.loop(done); });

    while (!done.load()) {
        sockaddr_in peer_addr;
        socklen_t peer_addr_len = sizeof(peer_addr);

        auto new_sock = accept(*sock_, (sockaddr *)&peer_addr, &peer_addr_len);

        // new socket shall be configured as non-blocking
        try {
            set_socket_non_blocking(new_sock);
        } catch (...) { close(new_sock); new_sock = -1; }

        // broken connection
        if (new_sock == -1) {
            logger_.log(
                (std::ostringstream()
                    << "Error "
                    << errno
                    << " upon accepting new socket."
                ).str()
            );
        }

        // confirmed connection
        else {
            std::string addr;

            // decypher IP address
            {
                char buf[INET_ADDRSTRLEN];
                inet_ntop(peer_addr.sin_family, &peer_addr, buf, INET_ADDRSTRLEN);
                addr = buf;
            }

            // create new thread for new connection
            std::thread thread([&, new_sock = new_sock, addr = addr, port = peer_addr.sin_port]() {
                ServerSession conn(new_sock, users, history, logger_);

                logger_.log(
                    (std::ostringstream()
                        << "New connection from peer "
                        << addr
                        << " port "
                        << ntohs(port)
                        << '.'
                    ).str()
                );

                conn.serve();

                logger_.log(
                    (std::ostringstream()
                        << "Closing connection with peer "
                        << addr
                        << " port "
                        << ntohs(port)
                        << '.'
                    ).str()
                );
            });

            thread.detach();
        }
    }

    for (auto&& service : services) {
        if (service.joinable()) {
            service.join();
        }
    }
}

Server::~Server()
{
    std::cout
        << "Server goes down..."
        << std::endl;
}
