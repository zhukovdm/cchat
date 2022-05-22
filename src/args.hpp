#ifndef ARGS_HPP_
#define ARGS_HPP_


/**
 * @file
 *
 * This header file declares primitives necessary for parsing command-line
 * arguments.
**/
#include <map>
#include <string>
#include <getopt.h>


/**
 * @brief Base class for both client and server argument parsers.
**/
class ArgsParser
{
protected:
    std::map<std::string, std::string> opts_;

    /**
     * @brief Runs configuration on actual arguments.
    **/
    void parse_specific(int argc, char **argv, int optc, const struct option optv[]);

public:

    /**
     * @brief Try get value from key-value storage.
     *     Throws @b std::invalid_argument on non-existent option.
    **/
    std::string get_value(const std::string& opt) const;

    /**
     * @brief Parses command-line arguments into a structure.
     *     Throws @b std::invalid_argument on unsuccessful parsing.
    **/
    virtual void parse(int argc, char **argv) = 0;

    virtual ~ArgsParser() {}
};


class ClientArgsParser final : public ArgsParser
{
public:

    /**
     * @brief Client-specific parse recognizes user --name, --host, and --port.
    **/
    void parse(int argc, char **argv) override;
};


class ServerArgsParser final : public ArgsParser
{
public:
    /**
     * @brief Server-specific parse recognizes --port.
    **/
    void parse(int argc, char **argv) override;
};


#endif
