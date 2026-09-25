#ifndef _GAMESTATE_H_
#define _GAMESTATE_H_

#include <KeyboardManager.h>
#include <ButtonListener.h>
#include <ChangeListener.h>
#include <Gui.h>
#include <SplashScreenManager.h>
#include <ScreenEffectsManager.h>
#include <CameraManager.h>
#include <SoundManager.h>
#include <UnitManager.h>
#include <SkyManager.h>
#include <WaterManager.h>
#include <NetworkManager.h>

class GameState : public KeyboardListener, public OIS::MouseListener, public ButtonListener, public ChangeListener, public NetworkListener, public CollisionListener
{
protected:
    Root *mRoot;
	SceneManager *mSceneMgr;
	Gui *mGui;
	LoadingBarManager *mLoadingBarMgr;
	SplashScreenManager *mSplashScreenMgr;
	ScreenEffectsManager *mScreenFXMgr;
	CollisionManager *mCollMgr;
	CameraManager *mCameraMgr;
	SoundManager *mSoundMgr;
    LightManager *mLightMgr;
    SkyManager *mSkyMgr;
    WaterManager *mWaterMgr;
	UnitManager *mUnitMgr;
	NetworkManager *mNetMgr;
public:
    bool changeState;
	bool replaceState;
	GameState *mNextState;
    GameState()
    {
        changeState = false;
        replaceState = false;
        mNextState = 0;

        mRoot = Root::getSingletonPtr();
        mSceneMgr = 0;
        mGui = Gui::getSingletonPtr();
        mLoadingBarMgr = LoadingBarManager::getSingletonPtr();
        mSplashScreenMgr = SplashScreenManager::getSingletonPtr();
        mScreenFXMgr = ScreenEffectsManager::getSingletonPtr();
        mCollMgr = CollisionManager::getSingletonPtr();
        mCameraMgr = CameraManager::getSingletonPtr();
        mSoundMgr = SoundManager::getSingletonPtr();
        mLightMgr = LightManager::getSingletonPtr();
        mSkyMgr = SkyManager::getSingletonPtr();
        mWaterMgr = WaterManager::getSingletonPtr();
        mUnitMgr = UnitManager::getSingletonPtr();
        mNetMgr = NetworkManager::getSingletonPtr();
    }
    virtual ~GameState()
    {
        exit();
    }
    virtual void enter()
    {
        init();
    }
    void init()
    {
        RenderWindow *window = mRoot->getAutoCreatedWindow();
        mSceneMgr = mRoot->createSceneManager(ST_EXTERIOR_CLOSE);
        mSceneMgr->setShadowTechnique(SHADOWTYPE_NONE);
        mSceneMgr->setShadowColour(ColourValue(0.5f,0.5f,0.5f));
        Entity::setDefaultQueryFlags(0);
        const ColourValue fadeColour(192.0f/255, 229.0f/255, 253.0f/255);
        mSceneMgr->setFog( FOG_LINEAR, fadeColour, FOG_RATIO, FOG_START, FOG_END);

        Camera *camera = mSceneMgr->createCamera("MainCamera");
        Viewport *viewport = window->addViewport(camera);
        viewport->setBackgroundColour(fadeColour);
        camera->setAspectRatio(Real(viewport->getActualWidth()) / Real(viewport->getActualHeight()));
        camera->setNearClipDistance(1.0f);
        camera->setFarClipDistance(26000.0f);

        mLoadingBarMgr->init();
        mCollMgr->init(mSceneMgr);
        mCameraMgr->init(camera,mSceneMgr);
        mLightMgr->init(mSceneMgr);
        mSkyMgr->init(mSceneMgr,viewport);
        mWaterMgr->init(mSceneMgr,camera);
        mUnitMgr->init(mSceneMgr);
    }
    virtual void exit()
    {
        uninit();
    }
    void uninit()
    {
        changeState = false;
        replaceState = false;
        mNextState = 0;

        if(!mSceneMgr)return;
        mNetMgr->clearListeners();
        mUnitMgr->reset();
        mSkyMgr->reset();
        mLightMgr->clear();
        mSoundMgr->clear();
        mCameraMgr->reset();
        mWaterMgr->clear();
        mCollMgr->reset();
        mGui->clear();
        mScreenFXMgr->reset();

        mSceneMgr->clearScene();
        mRoot->getAutoCreatedWindow()->removeAllViewports();
        mSceneMgr->destroyAllCameras();
        mRoot->destroySceneManager(mSceneMgr);
        mSceneMgr = 0;
    }
    virtual void update(const Real &timeElapsed)
    {
    }
    virtual const bool keyPressed(const char &index)
    {
        return true;
    }
    virtual const bool keyReleased(const char &index)
    {
        return true;
    }
    virtual const bool keyPressed(const OIS::KeyEvent &arg)
    {
        return true;
    }
    virtual const bool keyReleased(const OIS::KeyEvent &arg)
    {
        return true;
    }
    virtual bool mouseMoved(const OIS::MouseEvent &arg)
    {
        return true;
    }
    virtual bool mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
    {
        return true;
    }
    virtual bool mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
    {
        return true;
    }
    virtual bool buttonClicked(GuiComponent *button)
    {
        return true;
    }
    virtual bool buttonHeld(GuiComponent *button)
    {
        return true;
    }
    virtual bool buttonReleased(GuiComponent *button)
    {
        return true;
    }
    virtual bool changeEvent(GuiComponent *component, const unsigned char &event)
    {
        return true;
    }
    virtual bool comboBoxEvent(const String &name)
    {
        return true;
    }
    virtual bool alertBoxEvent(const String &name, const bool &flag)
    {
        return true;
    }
};

#endif
