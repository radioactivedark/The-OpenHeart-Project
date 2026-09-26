#ifndef _CHARSELECTSTATE_H_
#define _CHARSELECTSTATE_H_

#include <InGameState.h>
#include <CharCreateState.h>

class CharSelectState : public GameState
{
protected:
    GridManager *mGridMgr;
    MapManager *mMapMgr;
    GuiButton *mSelectButton[2];
    OverlayElement *mNameTag;
    GuiButton *mStartButton;
    GuiButton *mEditButton;
    GuiButton *mCreateButton;
    Unit *mPlayer;
    PlayerData mPlayerData;
    unsigned char mNumCharacters;
    unsigned int mCharID;
    String mUsername;
    char mSelection;
    bool mIsCreating;
    bool mHasDeleted;
    bool mIsEditing;
public:
    CharSelectState()
    {
        mGridMgr = GridManager::getSingletonPtr();
        mMapMgr = MapManager::getSingletonPtr();
        mSelection = 0;
        mIsCreating = false;
        mHasDeleted = false;
        mIsEditing = false;
        mUsername = "";
    }
    ~CharSelectState()
    {
        exit();
    }
    void enter()
    {
        init();

        mSkyMgr->setDayTime(745);
        mSkyMgr->setWeatherTime(0);
        mSkyMgr->freezeTime(true);
        mMapMgr->init(mSceneMgr,"fhPodium",false);
        GuiPanel *panel = mGui->addPanel("CharSelectScreen",true);
        for(int i=0;i<2;i++)mSelectButton[i] = panel->getButton("CharSelectScreen/Select"+StringConverter::toString(i));
        mSelectButton[0]->setOwnMaterials("GuiMat/Button2LUp","GuiMat/Button2LOver","GuiMat/Button2LUp");
        mSelectButton[1]->setOwnMaterials("GuiMat/Button2RUp","GuiMat/Button2ROver","GuiMat/Button2RUp");
        mNameTag = OverlayManager::getSingletonPtr()->getOverlayElement("CharSelectScreen/Name");
        mStartButton = panel->getButton("CharSelectScreen/Start");
        mEditButton = panel->getButton("CharSelectScreen/Edit");
        mCreateButton = panel->getButton("CharSelectScreen/Create");

        for(int i=0;i<2;i++)mSelectButton[i]->show(false);
        mStartButton->show(false);
        mEditButton->show(false);
        mCreateButton->show(false);
        mNameTag->hide();

        mCameraMgr->setPosition(Vector3(1540,112,1540));
        mCameraMgr->yaw(225);
        mCameraMgr->pitch(5);
        mPlayer = 0;
        mPlayerData.mMesh.push_back(1); //body
        mPlayerData.mMesh.push_back(1); //tail
        mPlayerData.mMesh.push_back(0); //mane
        mPlayerData.mMesh.push_back(1); //ears
        mPlayerData.mMesh.push_back(0); //tuft
        mPlayerData.mMat.push_back(1); //eyes
        mPlayerData.mMat.push_back(0); //body markings
        mPlayerData.mMat.push_back(0); //head markings
        mPlayerData.mMat.push_back(0); //tail markings

        mNumCharacters = 0;
        mCharID = 0;
        mNetMgr->addListener(this);

        //Deleted character
        if(mHasDeleted)
        {
            mIsEditing = false;
            mHasDeleted = false;
            mSelection = 0;
        }

        //Created character, waiting for server reply
        if(mIsCreating)
        {
            if(mIsEditing)mGui->showAlertBox("Updating...","CharSelect/Creating",false,false);
            else mGui->showAlertBox("Creating...","CharSelect/Creating",false,false);
        }
        else
        {
            mIsEditing = false;
            mGui->showAlertBox("Loading...","CharSelect/Loading",false,false);
            mNetMgr->sendCharSelect(mSelection);
        }
    }
    void exit()
    {
        mMapMgr->reset();
        uninit();
    }
    void update(const Real &timeElapsed)
    {
        //mCameraMgr->update(timeElapsed);
        mGridMgr->update(mCameraMgr->getPosition());
        mSkyMgr->update(timeElapsed);
        mWaterMgr->update(mCameraMgr->getPosition());
        mNetMgr->update(timeElapsed);
        if(mPlayer)mPlayer->update(timeElapsed);
        //mGui->getPanelByName("DebugScreen")->getComponentByName("DebugScreen/Debug")->setCaption(mNetMgr->mDebug);

        mRoot->renderOneFrame();
    }
    void setUsername(const String &username)
    {
        mUsername = username;
    }
    const bool keyPressed(const char &index)
    {
        using namespace KeyboardIndex;
        switch(index)
        {
            case KB_OPTIONS:
                changeState = true;
                mNetMgr->shutdown();
                break;
            default: break;
        }
        return true;
    }
    bool buttonClicked(GuiComponent *button)
    {
        if(button->nameIs("CharSelectScreen/Back"))
        {
            changeState = true;
            mNetMgr->shutdown();
        }
        else if(button==mStartButton)
        {
            //mGui->showAlertBox("Starting...","CharSelect/Start",false,false);
            mSplashScreenMgr->show(true);
            mNetMgr->sendCharStart(mCharID);
        }
        else if(button==mCreateButton)
        {
            changeState = true;
            CharCreateState *state = new CharCreateState();
            state->setCreatingPtr(&mIsCreating);
            state->setUsername(mUsername);
            mNextState = state;
        }
        else if(button==mEditButton)
        {
            mIsEditing = true;
            changeState = true;
            CharCreateState *state = new CharCreateState();
            state->setCreatingPtr(&mIsCreating);
            state->setDeletingPtr(&mHasDeleted);
            state->setEditMode(true,mCharID);
            state->setUsername(mUsername);
            CharCreateManager::getSingletonPtr()->setEditMode(true,&mPlayerData);
            mNextState = state;
        }
        else if(button==mSelectButton[1])
        {
            if(mNumCharacters>1)
            {
                mSelection++;
                if(mSelection>=mNumCharacters)mSelection = 0;
                mGui->showAlertBox("Loading...","CharSelect/Loading",false,false);
                mNetMgr->sendCharSelect(mSelection);
            }
        }
        else if(button==mSelectButton[0])
        {
            if(mNumCharacters>1)
            {
                mSelection--;
                if(mSelection<0)mSelection = mNumCharacters-1;
                mGui->showAlertBox("Loading...","CharSelect/Loading",false,false);
                mNetMgr->sendCharSelect(mSelection);
            }
        }
        return true;
    }
    void charSelectReply(const char &selection, PlayerData *data, const unsigned char &numCharacters, const unsigned int &charID)
    {
        if(selection!=mSelection)return;
        if(mGui->getAlertBoxName()=="CharSelect/Loading")mGui->hideAlertBox();
        if(data)
        {
            mPlayerData = *data;

            for(int i=0;i<2;i++)mSelectButton[i]->show(numCharacters>1);
            mStartButton->show(true);
            mEditButton->show(true);
            if(numCharacters<MAX_CHARACTERS)mCreateButton->show(true);
            mNameTag->setCaption(data->mName);
            mNameTag->show();
            mNumCharacters = numCharacters;
            mCharID = charID;

            if(mPlayer)mUnitMgr->destroyUnit(mPlayer);
            mPlayer = mUnitMgr->createUnit(mPlayerData,mUsername);
            mPlayer->setPosition(Vector3(1510,102,1510));
            mPlayer->setGroundHeight(102);
        }
        else mCreateButton->show(true);
    }
    void charCreateReply(const bool &success, const char &selection)
    {
        if(success && !mIsEditing)mSelection = selection;
        mIsCreating = false;
        mIsEditing = false;
        mNetMgr->sendCharSelect(mSelection);
        mGui->showAlertBox("Loading...","CharSelect/Loading",false,false);
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
    void frontServerToken(const String &token)
    {
        mNetMgr->shutdown();
        changeState = true;
        replaceState = true;
        InGameState *state = new InGameState();
        state->setPlayerData(mPlayerData);
        state->setToken(token,mCharID);
        state->setUsername(mUsername);
        state->setFirstEntrance();
        mNextState = state;
    }
};

#endif


