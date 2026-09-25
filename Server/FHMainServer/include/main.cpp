#ifndef _MAIN_CPP_
#define _MAIN_CPP_

#include <ServerManager.h>

int main()
{
    ServerManager mServer;
    if(mServer.init())mServer.run();
    mServer.shutdown();
}

#endif
