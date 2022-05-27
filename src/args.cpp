#include <exception>
#include <iostream>
#include "args.hpp"


auto ArgsParser::parse_specific(int argc, char **argv, int optc, const struct option optv[]) -> void
{
    opterr = 0; // avoid reporting unrecognized options.

    int idx, res;

    // all options are required, "abc"
    std::string config;
    for (int i = 0; i < optc; ++i) {
        config.push_back(optv[i].val);
    }

    // parse and copy found values
    while ((res = getopt_long(argc, argv, config.c_str(), optv, &idx)) >= 0) {
        for (int i = 0; i < optc; ++i) {
            if (optv[i].val == res) {

                // check option without value
                if (!optarg) {
                    throw std::invalid_argument("Invalid input arguments, consult user manual.");
                }

                opts_[optv[i].name] = optarg;
                break;
            }
        }
    }

    // check all options are found
    for (int i = 0; i < optc; ++i) {
        if (opts_.find(optv[i].name) == opts_.end()) {
            throw std::invalid_argument("Invalid input arguments, consult user manual.");
        }
    }
}

auto ArgsParser::get_value(const std::string& opt) const -> std::string
{
    auto&& kv = opts_.find(opt);

    if (kv == opts_.end()) {
        throw std::invalid_argument("Value for " + opt + " does not exist.");
    }

    return kv->second;
}


auto ClientArgsParser::parse(int argc, char **argv) -> void
{
    const struct option optv[] {
        { .name="name", .has_arg=required_argument, .flag=nullptr, .val=(int)'n' },
        { .name="host", .has_arg=required_argument, .flag=nullptr, .val=(int)'h' },
        { .name="port", .has_arg=required_argument, .flag=nullptr, .val=(int)'p' },
        {0, 0, 0, 0}
    };

    parse_specific(argc, argv, 3, optv);
}


auto ServerArgsParser::parse(int argc, char **argv) -> void
{
    const struct option optv[] {
        { .name="port", .has_arg=required_argument, .flag=nullptr, .val=(int)'p' },
        { 0, 0, 0, 0 }
    };

    parse_specific(argc, argv, 1, optv);
}
