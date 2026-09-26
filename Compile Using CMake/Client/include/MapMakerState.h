#ifndef _MAPMAKERSTATE_H_
#define _MAPMAKERSTATE_H_

#include <GameState.h>
#include <ControlManager.h>
#include <MapManager.h>
#include <MapMaker.h>
#include <MapTestState.h>

class MapMakerState : public GameState
{
protected:
    ControlManager *mControlMgr;
    GridManager *mGridMgr;
    MapMaker *mMapMaker;
public:
    MapMakerState()
    {
        mControlMgr = ControlManager::getSingletonPtr();
        mGridMgr = GridManager::getSingletonPtr();
        mMapMaker = new MapMaker();
    }
    ~MapMakerState()
    {
        exit();
        delete mMapMaker;
    }
    void enter()
    {
        init();
        mGui->hideCursorOnRightClick(true);

        mSkyMgr->setDayTime(1000);
        mSkyMgr->setWeatherTime(0);
        mSkyMgr->freezeTime(true);
        mCameraMgr->setPosition(Vector3(2500,120,2500));
        mControlMgr->init(0,0);
        mGui->addPanel("DebugScreen");

        mMapMaker->init(mSceneMgr);
    }
    void exit()
    {
        mMapMaker->deleteData();
        mControlMgr->reset();
        uninit();
    }
    void update(const Real &timeElapsed)
    {
        mMapMaker->update(timeElapsed);
        mControlMgr->update(timeElapsed);
        mCameraMgr->update(timeElapsed);
        mGridMgr->update(mCameraMgr->getPosition());
        mSkyMgr->update(timeElapsed);
        mWaterMgr->update(mCameraMgr->getPosition());
        //mGui->getPanelByName("DebugScreen")->getComponentByName("DebugScreen/Debug")->setCaption(mSkyMgr->getWeatherManager()->mDebug);

        mRoot->renderOneFrame();
    }
    const bool keyPressed(const char &index)
    {
        using namespace KeyboardIndex;
        switch(index)
        {
            case KB_OPTIONS: mGui->showAlertBox("Exit Map Maker?","MapMaker/Exit",true); break;
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
    const bool keyPressed(const OIS::KeyEvent &arg)
    {
        mMapMaker->keyPressed(arg);
        return true;
    }
    const bool keyReleased(const OIS::KeyEvent &arg)
    {
        mMapMaker->keyReleased(arg);
        return true;
    }
    bool buttonClicked(GuiComponent *button)
    {
        if(button->nameIs("MapMakerScreen/Test"))
        {
            mMapMaker->saveTemp();
            changeState = true;
            MapTestState *state = new MapTestState();

            PlayerData playerData;
            playerData.mMesh.push_back(1); //body
            playerData.mMesh.push_back(1); //tail
            playerData.mMesh.push_back(0); //mane
            playerData.mMesh.push_back(1); //ears
            playerData.mMesh.push_back(0); //tuft
            playerData.mMat.push_back(0); //eyes
            playerData.mMat.push_back(0); //body markings
            playerData.mMat.push_back(0); //head markings
            playerData.mMat.push_back(0); //tail markings
            CharCreateManager::getSingletonPtr()->setDefaultColours(&playerData);
            state->setPlayerData(playerData);

            MapTestData mapData;
            mapData.mMapName = "temp";
            mapData.mStartPos = mCameraMgr->getPosition();
            mapData.mShowCollisions = mMapMaker->mShowCollisions;
            state->setMapTestData(&mapData);
            mNextState = state;
        }
        else mMapMaker->buttonClicked(button);
        return true;
    }
    bool buttonHeld(GuiComponent *button)
    {
        mMapMaker->buttonHeld(button);
        return true;
    }
    bool buttonReleased(GuiComponent *button)
    {
        mMapMaker->buttonReleased(button);
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
    bool mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
    {
        if(id==OIS::MB_Left)mMapMaker->mouseLeftDown();
        return true;
    }
    bool changeEvent(GuiComponent *component, const unsigned char &event)
    {
        mMapMaker->changeEvent(component,event);
        return true;
    }
    bool alertBoxEvent(const String &name, const bool &flag)
    {
        if(name=="MapMaker/Exit")
        {
            if(flag)changeState = true;
        }
        else mMapMaker->alertBoxEvent(name,flag);
        return true;
    }
};

#endif
