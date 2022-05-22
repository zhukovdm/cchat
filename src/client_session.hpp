#ifndef CLIENT_SESSION_HPP_
#define CLIENT_SESSION_HPP_


/**
 * @file
 *
 * This header file declares ClientSession class.
**/
#include "logger.hpp"
#include "session.hpp"


class ClientSession final : public Session
{
private:
    std::string name_;
    PendingDeque send_gui_;
    PendingDeque recv_gui_;

    /**
     * @brief Send help messages to user Gui.
    **/
    void command_help();

    /**
     * @brief Receives list of users with pending messages on the Server.
     *     End of sequences is reported by the Server using special token.
    **/
    void recv_sequence();

public:
    ClientSession(int sock, std::string&& name);

    /**
     * @brief Serves session till session is done.
    **/
    void serve() override;

    /**
     * @brief ClientSession destructor @b shall @b not close the socket!
    **/
    ~ClientSession();
};


#endif
