#ifndef SERVER_ENTITY_HPP_
#define SERVER_ENTITY_HPP_

#include "args.hpp"
#include "entity.hpp"
#include "logger.hpp"
#include "storage.hpp"


/**
 * @file
 *
 * This header file declares Server entity.
**/


/**
 * @brief Server class.
**/
class Server final : public Entity {
private:
    Logger<std::string> logger_;

public:
    Server();

    /**
     * @brief Initializes Server instance.
    **/
    void init(const ServerArgsParser& args);

    /**
     * @brief Main Server endless loop accepting incoming connections.
     *     Connections are served by ServerSession instances.
    **/
    void loop() override;

    ~Server();
};

#endif
