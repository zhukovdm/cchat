#include "client_entity.hpp"


int main(int argc, char *argv[])
{
    main_entity<ClientArgsParser, Client>(argc, argv);
    return 0;
}
