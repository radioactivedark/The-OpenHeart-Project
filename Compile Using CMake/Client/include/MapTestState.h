#ifndef _MAPTESTSTATE_H_
#define _MAPTESTSTATE_H_

#include <GameState.h>
#include <ControlManager.h>
#include <MapManager.h>
#include <OptionsManager.h>

struct MapTestData
{
    String mMapName;
    bool mObjectMakerTest;
    Vector3 mStartPos;
    bool mShowCollisions;
    MapTestData()
    {
        mMapName = "";
        mObjectMakerTest = false;
        mStartPos = Vector3::ZERO;
        mShowCollisions = false;
    }
    void setData(MapTestData *data)
    {
        mMapName = data->mMapName;
        mObjectMakerTest = data->mObjectMakerTest;
        mStartPos = data->mStartPos;
        mShowCollisions = data->mShowCollisions;
    }
};

class MapTestState : public GameState
{
protected:
    ControlManager *mControlMgr;
    GridManager *mGridMgr;
    MapManager *mMapMgr;
    MiniMapManager *mMiniMapMgr;
    OptionsManager *mOptionsMgr;
    GuiPanel *mEscPanel;
    PlayerData mPlayerData;
    MapTestData mMapTestData;
public:
    MapTestState()
    {
        mControlMgr = ControlManager::getSingletonPtr();
        mGridMgr = GridManager::getSingletonPtr();
        mMapMgr = MapManager::getSingletonPtr();
        mMiniMapMgr = MiniMapManager::getSingletonPtr();
        mOptionsMgr = OptionsManager::getSingletonPtr();
        mPlayerData = PlayerData();
        mMapTestData = MapTestData();
    }
    ~MapTestState()
    {
        exit();
    }
    void enter()
    {
        init();
        mGui->hideCursorOnRightClick(true);

        mCollMgr->showCollisions(mMapTestData.mShowCollisions);
        mCollMgr->setListener(this);
        mSkyMgr->setDayTime(1000);
        mSkyMgr->setWeatherTime(0);
        mMapMgr->init(mSceneMgr,mMapTestData.mMapName,true,false);
        if(mMapTestData.mObjectMakerTest)mMapMgr->loadTempObject();
        Unit *unit = mUnitMgr->createUnit(mPlayerData,mPlayerData.mName);
        if(mPlayerData.mPreset && mPlayerData.mName=="")unit->setPreset(mPlayerData.mPreset);
        unit->setPosition(mMapTestData.mObjectMakerTest ? Vector3(2500,0,2700) : mMapTestData.mStartPos);
        if(mMapTestData.mObjectMakerTest)
        {
            unit->yaw(180);
            mCameraMgr->yaw(180);
        }
        mUnitMgr->initUnitPosition(unit);
        mUnitMgr->setPlayer(unit);
        mControlMgr->init(unit,mSceneMgr);
        mControlMgr->updateCameraScale();
        mMiniMapMgr->init();
        mMiniMapMgr->setPlayer(unit);
        mOptionsMgr->init(mSceneMgr);
        mEscPanel = mGui->addPanel("EscScreen",true);
        mEscPanel->show(false);
        mGui->addPanel("DebugScreen");
    }
    void exit()
    {
        mOptionsMgr->reset();
        mMiniMapMgr->reset();
        mControlMgr->reset();
        mMapMgr->reset();
        uninit();
    }
    void update(const Real &timeElapsed)
    {
        mControlMgr->update(timeElapsed);
        mUnitMgr->update(timeElapsed);
        mControlMgr->updateCamera();
        mCameraMgr->update(timeElapsed);
        mGridMgr->update(mCameraMgr->getPosition());
        mSkyMgr->update(timeElapsed);
        mWaterMgr->update(mCameraMgr->getPosition());
        mSoundMgr->update(mCameraMgr->getPosition(),mCameraMgr->getDirection());
        mMiniMapMgr->update(timeElapsed);
        //mGui->getPanelByName("DebugScreen")->getComponentByName("DebugScreen/Debug")->setCaption(mCollMgr->mDebug);

        mRoot->renderOneFrame();
    }
    void mapChange(const GateInfo &mapInfo)
    {
        mGui->getPanelByName("DebugScreen")->getComponentByName("DebugScreen/Debug")->setCaption(mapInfo.mMap+" "+StringConverter::toString(mapInfo.mPosition)+";"+StringConverter::toString(mapInfo.mYaw));
    }
    void setPlayerData(PlayerData &data)
    {
        mPlayerData = data;
    }
    void setMapTestData(MapTestData *data)
    {
        mMapTestData.setData(data);
    }
    const bool keyPressed(const char &index)
    {
        using namespace KeyboardIndex;
        switch(index)
        {
            case KB_OPTIONS:
                if(mEscPanel->isVisible())mEscPanel->show(false);
                else
                {
                    if(!mGui->isVisible())mGui->showAll(true);
                    mEscPanel->show(true);
                    mGui->setActivePanel(mEscPanel);
                }
                break;
            case KB_TOGGLEUI:
                if(mGui->isVisible())mGui->showAll(false);
                else mGui->showAll(true);
                break;
            default: break;
        }
        mControlMgr->keyPressed(index);
        mOptionsMgr->keyPressed(index);
        return true;
    }
    const bool keyReleased(const char &index)
    {
        mControlMgr->keyReleased(index);
        return true;
    }
    const bool keyPressed(const OIS::KeyEvent &arg)
    {
        using namespace OIS;
        switch(arg.key)
        {
            case KC_LBRACKET:
                mSkyMgr->setFastForward(false);
                break;
            case KC_RBRACKET:
                mSkyMgr->setFastForward(true);
                break;
            default: break;
        }
        return true;
    }
    bool buttonClicked(GuiComponent *button)
    {
        if(button->nameIs("EscScreen/Options"))
        {
            mEscPanel->show(false);
            mOptionsMgr->show(true);
        }
        else if(button->nameIs("EscScreen/Quit"))changeState = true;
        mOptionsMgr->buttonClicked(button);
        return true;
    }
    bool mouseMoved(const OIS::MouseEvent &arg)
    {
        if(arg.state.buttonDown(OIS::MB_Right))
        {
            mCameraMgr->yaw(-0.5f*arg.state.X.rel*mOptionsMgr->getMouseSensitivity());
            mCameraMgr->pitch(0.5f*arg.state.Y.rel*mOptionsMgr->getMouseSensitivity());
        }
        else if(arg.state.buttonDown(OIS::MB_Middle) && !mCameraMgr->isFreeCamera())mCameraMgr->zoom(0.005f*arg.state.Y.rel);

        return true;
    }
    bool mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
    {
        if(id==OIS::MB_Left)mControlMgr->mouseLeftDown();
        return true;
    }
    bool changeEvent(GuiComponent *component, const unsigned char &event)
    {
        mMiniMapMgr->changeEvent(component,event);
        mOptionsMgr->changeEvent(component,event);
        return true;
    }
    bool alertBoxEvent(const String &name, const bool &flag)
    {
        mOptionsMgr->alertBoxEvent(name,flag);
        return true;
    }
};

#endif

