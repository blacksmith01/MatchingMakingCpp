#include "proj/pch.h"

#include "gameserver.h"

using namespace myproj;

int main()
{
    {
        GameServer server;
        if (server.Init()) {
            server.Run();
        }
        server.Cleanup();
    }
    return 1;
}