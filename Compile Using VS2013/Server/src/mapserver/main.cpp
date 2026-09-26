#ifndef _MAIN_CPP_
#define _MAIN_CPP_

#include <ServerManager.h>
#include <signal.h>

int main(int argc, const char* argv[])
{
    ServerManager mServer(argc>1?atoi(argv[1]):MAP_SERVER_PORT, argc>2?argv[2]:"");
    if(mServer.init())mServer.run();
    mServer.shutdown();
}

#endif
