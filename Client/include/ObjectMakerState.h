#ifndef _OBJECTMAKERSTATE_H_
#define _OBJECTMAKERSTATE_H_

#include <MapTestState.h>
#include <ControlManager.h>
#include <ObjectMaker.h>
#include <MapManager.h>

class ObjectMakerState : public GameState
{
protected:
    ControlManager *mControlMgr;
    GridManager *mGridMgr;
    MapManager *mMapMgr;
    ObjectMaker *mObjectMaker;
public:
    ObjectMakerState()
    {
        mControlMgr = ControlManager::getSingletonPtr();
        mGridMgr = GridManager::getSingletonPtr();
        mMapMgr = MapManager::getSingletonPtr();
        mObjectMaker = new ObjectMaker();
    }
    ~ObjectMakerState()
    {
        exit();
        delete mObjectMaker;
    }
    void enter()
    {
        init();
        mGui->hideCursorOnRightClick(true);

        mSkyMgr->setDayTime(1000);
        mSkyMgr->setWeatherTime(0);
        mSkyMgr->freezeTime(true);
        mMapMgr->init(mSceneMgr,"");
        mCameraMgr->setPosition(Vector3(2540,40,2700));
        mCameraMgr->yaw(180);
        mControlMgr->init(0,0);

        mGui->addPanel("DebugScreen");

        mObjectMaker->init(mSceneMgr);
    }
    void exit()
    {
        mObjectMaker->clearScene();
        mControlMgr->reset();
        mMapMgr->reset();
        uninit();
    }
    void update(const Real &timeElapsed)
    {
        mObjectMaker->update(timeElapsed);
        mControlMgr->update(timeElapsed);
        mCameraMgr->update(timeElapsed);
        mGridMgr->update(mCameraMgr->getPosition());
        mSkyMgr->update(timeElapsed);
        //mGui->getPanelByName("DebugScreen")->getComponentByName("DebugScreen/Debug")->setCaption(mCollMgr->mDebug);

        mRoot->renderOneFrame();
    }
    const bool keyPressed(const char &index)
    {
        using namespace KeyboardIndex;
        switch(index)
        {
            case KB_OPTIONS:
                mGui->showAlertBox("Exit Object Maker?","ObjectMaker/Exit",true);
                break;
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
        mObjectMaker->keyPressed(arg);
        return true;
    }
    const bool keyReleased(const OIS::KeyEvent &arg)
    {
        mObjectMaker->keyReleased(arg);
        return true;
    }
    bool buttonClicked(GuiComponent *button)
    {
        if(button->nameIs("ObjectMakerScreen/Test"))
        {
            mObjectMaker->saveTemp();
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
            mapData.mObjectMakerTest = true;
            mapData.mShowCollisions = mObjectMaker->mShowCollisions;
            state->setMapTestData(&mapData);
            mNextState = state;
        }
        else mObjectMaker->buttonClicked(button);
        return true;
    }
    bool buttonHeld(GuiComponent *button)
    {
        mObjectMaker->buttonHeld(button);
        return true;
    }
    bool buttonReleased(GuiComponent *button)
    {
        mObjectMaker->buttonReleased(button);
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
        if(id==OIS::MB_Left)mObjectMaker->doMousePicking(mGui->getCursorX(),mGui->getCursorY());
        return true;
    }
    bool changeEvent(GuiComponent *component, const unsigned char &event)
    {
        mObjectMaker->changeEvent(component,event);
        return true;
    }
    bool alertBoxEvent(const String &name, const bool &flag)
    {
        if(name=="ObjectMaker/Exit")
        {
            if(flag)changeState = true;
        }
        else mObjectMaker->alertBoxEvent(name,flag);
        return true;
    }
};

#endif
