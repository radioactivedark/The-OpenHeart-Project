#ifndef _CHARCREATESTATE_H_
#define _CHARCREATESTATE_H_

#include <MapTestState.h>
#include <GuiTextHandler.h>
#include <MapManager.h>
#include <CharCreateManager.h>

class CharCreateState : public GameState
{
protected:
    CharCreateManager *mCharCreateMgr;
    GridManager *mGridMgr;
    MapManager *mMapMgr;
    bool mOfflineMode;
    bool mPresetMakerMode;
    bool *mIsCreating;
    bool *mHasDeleted;
    bool mEditMode;
    bool mWasTesting;
    bool mIsExportedPreset;
    PlayerData mTestData;
    unsigned int mCharID;
    String mUsername;
    PlayerData *mReturnedPlayerData;
public:
    CharCreateState()
    {
        mCharCreateMgr = CharCreateManager::getSingletonPtr();
        mGridMgr = GridManager::getSingletonPtr();
        mMapMgr = MapManager::getSingletonPtr();
        mOfflineMode = false;
        mPresetMakerMode = false;
        mIsCreating = 0;
        mHasDeleted = 0;
        mEditMode = false;
        mWasTesting = false;
        mIsExportedPreset = false;
        mTestData = PlayerData();
        mCharID = 0;
        mUsername = "";
        mReturnedPlayerData = 0;
    }
    ~CharCreateState()
    {
        exit();
    }
    void enter()
    {
        init();

        mSkyMgr->setDayTime(1000);
        mSkyMgr->setWeatherTime(0);
        mSkyMgr->freezeTime(true);
        mMapMgr->init(mSceneMgr,"fhPodium",false);
        mCharCreateMgr->setPresetMakerMode(mPresetMakerMode);
        mCharCreateMgr->setUsername(mUsername);
        if(mWasTesting)
        {
            mCharCreateMgr->setEditMode(true,&mTestData);
            mWasTesting = false;
            mCharCreateMgr->mIsExportedPreset = mIsExportedPreset;
        }
        mCharCreateMgr->init();
        //mGui->addPanel("DebugScreen");

        mNetMgr->addListener(this);
    }
    void exit()
    {
        mCharCreateMgr->reset();
        mMapMgr->reset();
        uninit();
    }
    void update(const Real &timeElapsed)
    {
        if(mCharCreateMgr->popNeedsRender())mRoot->renderOneFrame();
        mCharCreateMgr->update(timeElapsed);
        mGridMgr->update(mCameraMgr->getPosition());
        mSkyMgr->update(timeElapsed);
        mWaterMgr->update(mCameraMgr->getPosition());
        if(!mOfflineMode)mNetMgr->update(timeElapsed);

        mRoot->renderOneFrame();
    }
    void setPresetMakerMode(const bool &flag)
    {
        mPresetMakerMode = flag;
        mOfflineMode = flag;
    }
    void setOfflineMode(const bool &flag)
    {
        mOfflineMode = flag;
    }
    void setCreatingPtr(bool *ptr)
    {
        mIsCreating = ptr;
    }
    void setDeletingPtr(bool *ptr)
    {
        mHasDeleted = ptr;
    }
    void setEditMode(const bool &flag, const unsigned int &charID)
    {
        mEditMode = flag;
        if(flag)mCharID = charID;
    }
    void setUsername(const String &username)
    {
        mUsername = username;
    }
    void setReturnedPlayerDataPtr(PlayerData *ptr)
    {
        mReturnedPlayerData = ptr;
    }
    const bool keyPressed(const char &index)
    {
        using namespace KeyboardIndex;
        switch(index)
        {
            case KB_OPTIONS:
                changeState = true;
                break;
            default: break;
        }
        return true;
    }
    const bool keyReleased(const char &index)
    {
        return true;
    }
    bool buttonClicked(GuiComponent *button)
    {
        if(button->nameIs("CharCreateScreen/Cancel"))changeState = true;
        else if(button->nameIs("CharCreateScreen/Finish"))
        {
            if(mPresetMakerMode)
            {
                changeState = true;
                mWasTesting = true;
                mIsExportedPreset = mCharCreateMgr->mIsExportedPreset;
                MapTestState *state = new MapTestState();
                mTestData = mCharCreateMgr->getPlayerData();
                mTestData.mName = (mIsExportedPreset ? mUsername : "");
                state->setPlayerData(mTestData);
                MapTestData mapData;
                mapData.mStartPos = Vector3(2500,0,2500);
                state->setMapTestData(&mapData);
                mNextState = state;
            }
            else if(mCharCreateMgr->validateName())
            {
                changeState = true;
                if(mIsCreating)*mIsCreating = true;
                if(mOfflineMode)
                {
                    if(mReturnedPlayerData)mReturnedPlayerData->copyData(mCharCreateMgr->getPlayerData());
                }
                else
                {
                    mOfflineMode = true;
                    mNetMgr->sendCharCreate(mCharCreateMgr->getPlayerData(),mEditMode,mCharID);
                }
            }
        }
        else if(button->nameIs("CharCreateScreen/Delete"))
        {
            mGui->showAlertBox("Delete character?","CharCreate/Delete",true);
        }
        else mCharCreateMgr->buttonClicked(button);
        return true;
    }
    bool changeEvent(GuiComponent *component, const unsigned char &event)
    {
        mCharCreateMgr->changeEvent(component);
        return true;
    }
    bool mouseMoved(const OIS::MouseEvent &arg)
    {
        if(arg.state.buttonDown(OIS::MB_Left))
        {
            mCharCreateMgr->zoom(0.0025f*arg.state.Y.rel);
        }
        return true;
    }
    bool mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
    {
        if(id==OIS::MB_Left)mCharCreateMgr->doScroll(true);
        return true;
    }
    bool mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
    {
        if(id==OIS::MB_Left)mCharCreateMgr->doScroll(false);
        return true;
    }
    bool alertBoxEvent(const String &name, const bool &flag)
    {
        if(name=="CharCreate/Delete")
        {
            if(flag)
            {
                changeState = true;
                if(mHasDeleted)*mHasDeleted = true;
                if(!mOfflineMode)
                {
                    mOfflineMode = true;
                    mNetMgr->sendCharDelete(mCharID);
                }
            }
        }
        return true;
    }
    void frontServerReply(const unsigned char &reply)
    {
        switch(reply)
        {
            case NetworkManager::SERVER_CONNECT_LOST:
                mGui->showAlertBox("Connection Lost.");
                changeState = true;
                break;
            default: break;
        }
    }
};

#endif

