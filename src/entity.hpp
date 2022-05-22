#ifndef ENTITY_HPP_
#define ENTITY_HPP_


/**
 * @file
 *
 * This header file defines the most basic network primitive Entity.
**/
#include <iostream>
#include <optional>
#include <unistd.h>


/**
 * @brief Template function for both Client and Server, shall be called from
 * the @b main function.
**/
template <typename Parser, typename Entity>
inline void main_entity(int argc, char *argv[])
{
    try {
        Parser parser;
        parser.parse(argc, argv);

        Entity entity;
        entity.init(parser);
        entity.loop();
    }

    catch (std::exception& ex) {
        std::cout << "Unhandled exception. " << ex.what() << std::endl;
    }
}


class Entity {
protected:
    std::optional<int> sock_;

public:

    /**
     * @brief Main Entity loop.
    **/
    virtual void loop() = 0;

    /**
     * @brief Destroys @b maybe socket associated with the entity.
    **/
    virtual ~Entity()
    {
        if (sock_.has_value()) { close(*sock_); }
    }
};


#endif
