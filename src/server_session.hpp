#ifndef SERVER_SESSION_HPP_
#define SERVER_SESSION_HPP_


/**
 * @file
 *
 * This header file declares object ServerSession.
**/


#include "logger.hpp"
#include "session.hpp"
#include "storage.hpp"


/**
 * @brief Server potentially accommodates more than one socket during
 *     its lifetime. Server instance releases socket allocated upon
 *     start, and ServerSession releases accepted sockets.
**/
class ServerSession final : public Session
{
private:
    UserMap& users_;
    HistoryMap& history_;
    Logger<std::string>& logger_;

    UserPair get_ordered_pair(const UserId& u1, const UserId& u2);
    bool try_log_in(const UserId& user_name);

public:
    ServerSession(int sock, UserMap& users, HistoryMap& history, Logger<std::string>& logger);

    /**
     * @brief Server does not initiate
    **/
    void serve() override;

    /**
     * @brief ServerSession destructor @b shall close the socket!
    **/
    ~ServerSession();
};


#endif
