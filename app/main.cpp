#include "proj\pch.h"

#include "server\gameserver.h"

int main()
{
    GameServer server;
    if (server.Init()) {
        server.Run();
    }
    server.Cleanup();

    return 1;
}