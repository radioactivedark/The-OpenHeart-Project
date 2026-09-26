#define _WIN32_WINNT 0x0500

//Include windows/Ogre/OIS headers
#include <Ogre.h>
#include <OIS.h>
#include <Application.h>

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#include <windows.h>
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR strCmdLine, INT nCmdShow)
#else
int main(int argc, char *argv[])
#endif
{
    try
    {
        Application app;
        if(app.load())app.run();
    }
    catch( Ogre::Exception& e )
    {
        #if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
        #else
        std::cerr << "An exception has occured: " << e.getFullDescription().c_str() << std::endl;
        #endif
    }

	return 0;
}
