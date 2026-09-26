#ifndef _APPLICATION_H_
#define _APPLICATION_H_

using namespace Ogre;

#if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
#include <CoreFoundation/CoreFoundation.h>

// This function will locate the path to our application on OS X,
// unlike windows you can not rely on the curent working directory
// for locating your configuration files and resources.
std::string macBundlePath()
{
    char path[1024];
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    assert(mainBundle);

    CFURLRef mainBundleURL = CFBundleCopyBundleURL(mainBundle);
    assert(mainBundleURL);

    CFStringRef cfStringRef = CFURLCopyFileSystemPath( mainBundleURL, kCFURLPOSIXPathStyle);
    assert(cfStringRef);

    CFStringGetCString(cfStringRef, path, 1024, kCFStringEncodingASCII);

    CFRelease(mainBundleURL);
    CFRelease(cfStringRef);

    return std::string(path);
}
#endif

#include <LogoState.h>
#include <GuiHandler.h>
#include <MacAddress.h>

#define AppTitle "FeralHeart"

class Application : public WindowEventListener
{
private:
    bool running;
    String resourcePath;

    Root *root;
	RenderWindow *window;
	GuiHandler *mGuiHandler;

	Real timeSinceLastFrame;
	Timer timer;
    unsigned long lastTime;

    std::vector<GameState*> mStates;
    GameState *mGameState;

    bool mProperShutdown;

public:
    Application()
    {
        mProperShutdown = false;
        bool previouslySuccessful = false;
        const bool showConfigDialog = shouldShowConfigDialog(previouslySuccessful);
        setShutdownSuccessful(false, previouslySuccessful);
        //Create default ogre.cfg if required
        OptionsManager::checkAndcreateDefaultVideoConfig(showConfigDialog && !previouslySuccessful);

        String macAddress = "";
        String resourcePath = "";
        #if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
        resourcePath = macBundlePath() + "/Contents/Resources/";
        #endif

        //Initialize Ogre
        root = OGRE_NEW Root("",resourcePath + "ogre.cfg",resourcePath + "feralheart.log");

        //Load appropriate plugins
        #if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        HWND hWnd = GetConsoleWindow();
        ShowWindow( hWnd, SW_HIDE );
        root->loadPlugin("Plugin_CgProgramManager");
        root->loadPlugin("Plugin_OctreeSceneManager");
        root->loadPlugin("Plugin_ParticleFX");
        if(isRenderSystemOpenGL(resourcePath + "ogre.cfg"))root->loadPlugin("RenderSystem_GL");
        else root->loadPlugin("RenderSystem_Direct3D9");
        macAddress = getMACaddress();
        #else
        #define STRINGIFY(x) #x
        #define EXPAND(x) STRINGIFY(x)
        root->loadPlugin(EXPAND(OGRE_PLUGINDIR) "Plugin_CgProgramManager");
        root->loadPlugin(EXPAND(OGRE_PLUGINDIR) "Plugin_OctreeSceneManager");
        root->loadPlugin(EXPAND(OGRE_PLUGINDIR) "Plugin_ParticleFX");
        if(isRenderSystemOpenGL(resourcePath + "ogre.cfg"))root->loadPlugin(EXPAND(OGRE_PLUGINDIR) "RenderSystem_GL");
        else root->loadPlugin(EXPAND(OGRE_PLUGINDIR) "RenderSystem_Direct3D9");
        #endif

        //Show dialog if config is absent
        bool usedConfigDialog = false;
        if(showConfigDialog)
        {
            if(root->showConfigDialog())usedConfigDialog = true;
            else
            {
                setShutdownSuccessful(false, false);
                OGRE_DELETE root;
                root = 0;
                return;
            }
            //Fill ogre.cfg blanks
            OptionsManager::checkAndcreateDefaultVideoConfig(false,true);
        }

        if(!usedConfigDialog)root->restoreConfig();
        window = root->initialise(true, AppTitle);
        loadResources(resourcePath);
        loadMapResources();
        srand(time(NULL));

        //To fix overlay caption bug since ogre 1.7
        FontManager *pFontMgr = FontManager::getSingletonPtr();
        pFontMgr->getByName("Tahoma")->load();
        pFontMgr->getByName("TempusSans")->load();
        pFontMgr->getByName("BlueHighway")->load();
        pFontMgr->getByName("FadeFont")->load();

        new LoggerManager();
        new LoadingBarManager();
        new ZipManager();
        new CapabilitiesManager(root);
        new KeyboardManager();
        new SplashScreenManager();
        new ScreenEffectsManager();
        new CollisionManager();
        new WaterManager();
        new CameraManager();
        new SoundManager();
        mGuiHandler = new GuiHandler(window);
        new GridManager();
        new LightManager();
        new SkyManager();
        new UnitManager();
        new CharCreateManager();
        new EmoteManager();
        new MarkingManager();
        new LipSyncManager();
        new NameTagManager();
        new WorldObjectManager();
        new MapManager();
        new NetworkManager(macAddress);
        new ChatManager();
        new FriendsManager();
        new PartyManager();
        new GroupManager();
        new InfoManager();
        new ControlManager();
        new MovieMaker();
        new ActionsManager();
        new ItemsManager();
        new SkillsManager();
        new MiniMapManager();
        new GamePanelManager();
        OptionsManager *optionsMgr = new OptionsManager();

        WindowEventUtilities::addWindowEventListener(window, this);
        mGuiHandler->init(optionsMgr->getFullScreen(),optionsMgr->getMouseSensitivityPtr());
        loadScripts(resourcePath);

        mGameState = 0;
    }

    ~Application()
    {
        if(!root)return;

        unload();

        const bool restartRequired = OptionsManager::getSingletonPtr()->isRestartRequired();
        delete OptionsManager::getSingletonPtr();
        delete GamePanelManager::getSingletonPtr();
        delete MiniMapManager::getSingletonPtr();
        delete SkillsManager::getSingletonPtr();
        delete ItemsManager::getSingletonPtr();
        delete ActionsManager::getSingletonPtr();
        delete MovieMaker::getSingletonPtr();
        delete ControlManager::getSingletonPtr();
        delete InfoManager::getSingletonPtr();
        delete GroupManager::getSingletonPtr();
        delete PartyManager::getSingletonPtr();
        delete FriendsManager::getSingletonPtr();
        delete ChatManager::getSingletonPtr();
        delete NetworkManager::getSingletonPtr();
        delete MapManager::getSingletonPtr();
        delete WorldObjectManager::getSingletonPtr();
        delete NameTagManager::getSingletonPtr();
        delete LipSyncManager::getSingletonPtr();
        delete MarkingManager::getSingletonPtr();
        delete EmoteManager::getSingletonPtr();
        delete CharCreateManager::getSingletonPtr();
        delete UnitManager::getSingletonPtr();
        delete SkyManager::getSingletonPtr();
        delete LightManager::getSingletonPtr();
        delete GridManager::getSingletonPtr();
        delete mGuiHandler;
        WindowEventUtilities::removeWindowEventListener(window, this);
        delete SoundManager::getSingletonPtr();
        delete CameraManager::getSingletonPtr();
        delete WaterManager::getSingletonPtr();
        delete CollisionManager::getSingletonPtr();
        delete ScreenEffectsManager::getSingletonPtr();
        delete SplashScreenManager::getSingletonPtr();
        delete KeyboardManager::getSingletonPtr();
        delete CapabilitiesManager::getSingletonPtr();
        ZipManager::getSingletonPtr()->getTempFilenames();
        delete LoadingBarManager::getSingletonPtr();
        delete LoggerManager::getSingletonPtr();

        //Shut down Ogre
        OGRE_DELETE root;

        delete ZipManager::getSingletonPtr();

        if(mProperShutdown)setShutdownSuccessful(true,!restartRequired);
    }

    const bool load()
    {
        if(!root)return false;

        //Init gamestate
        pushState(new LogoState());

        lastTime = timer.getMilliseconds();

        return true;
    }

    void unload()
    {
        mGuiHandler->exitProcedure();
        while(!mStates.empty())
        {
            GameState *state = mStates.back();
            mStates.pop_back();
            delete state;
        }
    }

    void run()
    {
        //Main loop
        while(running)
        {
            //Handle windows events
            WindowEventUtilities::messagePump();
            if(!running)break;

            //Update frame
            updateTimer();
            /*const Real tTimeThreshold = 0.1;
            while(timeSinceLastFrame>tTimeThreshold)
            {
                const Real tempTime = timeSinceLastFrame;
                timeSinceLastFrame = tTimeThreshold;
                render();
                timeSinceLastFrame = tempTime-tTimeThreshold;
            }*/
            if(timeSinceLastFrame>=0)
            {
                mGuiHandler->update(timeSinceLastFrame);
                //mGuiHandler->updateStats();
                mGameState->update(timeSinceLastFrame);
                //mGuiHandler->updateStats();
                if(mGameState->changeState)
                {
                    if(mGameState->mNextState)pushState(mGameState->mNextState,mGameState->replaceState);
                    else popState();
                }
            }
        }

        mProperShutdown = true;
    }

    void updateTimer()
    {
        const unsigned long currentTime = timer.getMilliseconds();

        //Calculate the amount of time passed since the last frame
        timeSinceLastFrame = (currentTime - lastTime) * 0.001f;
        if(timeSinceLastFrame >= 0.001f)lastTime = currentTime;
        else timeSinceLastFrame = 0;
    }

    void pushState(GameState *state, const bool &replace=false)
    {
        if(replace)popState(false);
        else if(mGameState)
        {
            mGuiHandler->exitProcedure();
            mGameState->exit();
        }
        mStates.push_back(state);
        mGameState = state;
        mGameState->enter();
        mGuiHandler->enterProcedure();
        mGuiHandler->setListeners(mGameState,mGameState,mGameState,mGameState);
        running = true;
    }
    void popState(const bool &enterPrevious=true)
    {
        if(!mStates.empty())
        {
            mGuiHandler->exitProcedure();
            GameState *state = mStates.back();
            mStates.pop_back();
            delete state;
            mGameState = (mStates.empty()?0:mStates.back());
            if(mGameState)
            {
                if(enterPrevious)
                {
                    mGameState->enter();
                    mGuiHandler->enterProcedure();
                    mGuiHandler->setListeners(mGameState,mGameState,mGameState,mGameState);
                }
            }
            else running = false;
        }
    }
    void loadResources(const String &resourcePath)
    {
        // Load resource paths from config file
        ConfigFile cf;
        cf.load(resourcePath + "resources.cfg");

        // Go through all sections & settings in the file
        ConfigFile::SectionIterator seci = cf.getSectionIterator();

        String secName, typeName, archName;
        while (seci.hasMoreElements())
        {
            secName = seci.peekNextKey();
            ConfigFile::SettingsMultiMap *settings = seci.getNext();
            ConfigFile::SettingsMultiMap::iterator i;
            for (i = settings->begin(); i != settings->end(); ++i)
            {
                typeName = i->first;
                archName = i->second;
                #if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
                // OS X does not set the working directory relative to the app,
                // In order to make things portable on OS X we need to provide
                // the loading with it's own bundle path location
                ResourceGroupManager::getSingleton().addResourceLocation(
                    String(macBundlePath() + "/" + archName), typeName, secName);
                #else
                ResourceGroupManager::getSingleton().addResourceLocation(
                    archName, typeName, secName);
                #endif
            }
        }
        ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
    }
    void loadMapResources()
    {
        const StringVector list = SaveFile::findResourceNames("Terrains","*.map");
        for(StringVector::const_iterator i = list.begin(); i!=list.end(); i++)
        {
            String mapName = *i;
            mapName.erase((int)mapName.length()-4,4);
            #if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
            ResourceGroupManager::getSingleton().addResourceLocation(String(macBundlePath()+"/"+MAPMAKER_DIR+mapName),"FileSystem","Terrains/"+mapName);
            #else
            ResourceGroupManager::getSingleton().addResourceLocation(MAPMAKER_DIR+mapName,"FileSystem","Terrains/"+mapName);
            #endif
            ResourceGroupManager::getSingleton().initialiseResourceGroup("Terrains/"+mapName);
        }
    }
    void loadScripts(const String &resourcePath)
    {
        EmoteManager::getSingletonPtr()->load(resourcePath+"emotes.cfg");
        MarkingManager::getSingletonPtr()->load(resourcePath+"markings.cfg");
        MarkingManager::getSingletonPtr()->initEyeMaterials();
        SkyManager::getSingletonPtr()->loadWeathers();
        MovieMaker::getSingletonPtr()->loadCamMovements(resourcePath+"movieCamMovements.cfg");
        //MovieMaker::getSingletonPtr()->loadActions(resourcePath+"movieActions.cfg");
        ItemsManager::getSingletonPtr()->load(resourcePath+"items.cfg");
    }
    const bool isRenderSystemOpenGL(const String &filename)
    {
        std::ifstream inFile(filename.c_str());
        if(!inFile.good())return true;
        char buffer[64] = "";
        inFile.getline(buffer,64);
        inFile.close();
        if(StringUtil::endsWith(String(buffer),"opengl rendering subsystem",true))return true;
        return false;
    }
    const bool shouldShowConfigDialog(bool &previouslySuccessful)
    {
        const String filename = resourcePath + "config.good";
        std::ifstream inFile(filename.c_str());
        if(!inFile.good())return true;
        const char ch = inFile.get();
        previouslySuccessful = (inFile.get() == '1');
        inFile.close();
        return (ch == '0');
    }
    void setShutdownSuccessful(const bool &flag, const bool &previouslySuccessful=false)
    {
        const String filename = resourcePath + "config.good";
        std::ofstream outFile(filename.c_str());
        outFile.put(flag ? '1' : '0');
        outFile.put(previouslySuccessful ? '1' : '0');
        outFile.close();
    }
    bool windowClosing(RenderWindow *rw)
    {
        running = false;
        unload();
        mProperShutdown = true;
        return true;
    }
};

#endif
