#ifndef _SKYMAKERSTATE_H_
#define _SKYMAKERSTATE_H_

#include <GameState.h>
#include <ControlManager.h>
#include <SkyMaker.h>

class SkyMakerState : public GameState
{
protected:
    ControlManager *mControlMgr;
    MapManager *mMapMgr;
    SkyMaker *mSkyMaker;
public:
    SkyMakerState()
    {
        mControlMgr = ControlManager::getSingletonPtr();
        mMapMgr = MapManager::getSingletonPtr();
        mSkyMaker = new SkyMaker;
    }
    ~SkyMakerState()
    {
        exit();
        delete mSkyMaker;
    }
    void enter()
    {
        init();
        mGui->hideCursorOnRightClick(true);

        mMapMgr->init(mSceneMgr,"WaterMill");
        mCameraMgr->setPosition(Vector3(4855,146,5346));
        mControlMgr->init(0,0);
        mGui->addPanel("DebugScreen");

        mSkyMaker->init(mSceneMgr);
    }
    void exit()
    {
        mSkyMaker->clear();
        mControlMgr->reset();
        mMapMgr->reset();
        uninit();
    }
    void update(const Real &timeElapsed)
    {
        mControlMgr->update(timeElapsed);
        mCameraMgr->update(timeElapsed);
        mSkyMgr->update(timeElapsed);
        mWaterMgr->update(mCameraMgr->getPosition());
        mSkyMaker->update(timeElapsed);
        //mGui->getPanelByName("DebugScreen")->getComponentByName("DebugScreen/Debug")->setCaption(mSkyMgr->getWeatherManager()->mDebug);

        mRoot->renderOneFrame();
    }
    const bool keyPressed(const char &index)
    {
        using namespace KeyboardIndex;
        switch(index)
        {
            case KB_OPTIONS: mGui->showAlertBox("Exit Sky Maker?","SkyMaker/Exit",true); break;
            default: break;
        }
        mControlMgr->keyPressed(index);
        return true;
    }
    const bool keyReleased(const char &index)
    {
        mControlMgr->keyReleased(index);
        return true;
    }
    bool buttonClicked(GuiComponent *button)
    {
        mSkyMaker->buttonClicked(button);
        return true;
    }
    bool mouseMoved(const OIS::MouseEvent &arg)
    {
        if(arg.state.buttonDown(OIS::MB_Right))
        {
            mCameraMgr->yaw(-0.75f*arg.state.X.rel);
            mCameraMgr->pitch(0.75f*arg.state.Y.rel);
        }
        else if(arg.state.buttonDown(OIS::MB_Middle) && !mCameraMgr->isFreeCamera())mCameraMgr->zoom(0.005f*arg.state.Y.rel);

        return true;
    }
    bool changeEvent(GuiComponent *component, const unsigned char &event)
    {
        mSkyMaker->changeEvent(component,event);
        return true;
    }
    bool alertBoxEvent(const String &name, const bool &flag)
    {
        if(name=="SkyMaker/Exit")
        {
            if(flag)changeState = true;
        }
        else mSkyMaker->alertBoxEvent(name,flag);
        return true;
    }
};

#endif

