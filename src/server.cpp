#include "server_entity.hpp"


int main(int argc, char *argv[])
{
    main_entity<ServerArgsParser, Server>(argc, argv);
    return 0;
}
