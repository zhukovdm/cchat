#ifndef CLIENT_ENTITY_HPP_
#define CLIENT_ENTITY_HPP_


/**
 * @file
 *
 * This header file declares Client entity.
**/
#include <string>
#include "args.hpp"
#include "entity.hpp"


/**
 * @brief Client class definition.
**/
class Client final : public Entity {
private:
    std::string name_;

public:

    /**
     * @brief Initializes Client instance.
    **/
    void init(const ClientArgsParser& args);

    /**
     * @brief Main Client loop.
    **/
    void loop() override;

    ~Client();
};


#endif
