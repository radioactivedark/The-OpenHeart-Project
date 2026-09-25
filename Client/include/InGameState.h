#ifndef _INGAMESTATE_H_
#define _INGAMESTATE_H_

#include <GameState.h>
#include <ControlManager.h>
#include <ChatManager.h>
#include <MapManager.h>
#include "RakSleep.h"
#include <OptionsManager.h>
#include <GamePanelManager.h>

#define NUM_STATUSFLAGS 2

class InGameState : public GameState, public ChatListener
{
protected:
    InfoManager *mInfoMgr;
    ControlManager *mControlMgr;
    ChatManager *mChatMgr;
    GridManager *mGridMgr;
    MapManager *mMapMgr;
    NameTagManager *mNameTagMgr;
    FriendsManager *mFriendsMgr;
    PartyManager *mPartyMgr;
    GroupManager *mGroupMgr;
    OptionsManager *mOptionsMgr;
    MovieMaker *mMovieMaker;
    MiniMapManager *mMiniMapMgr;
    GamePanelManager *mGamePanelMgr;
    GuiPanel *mEscPanel;
    PlayerData mPlayerData;
    bool mLoaded;
    String mToken;
    unsigned int mCharID;
    String mUsername;
    unsigned char mTokenRetryAttempts;
    Real mArrivalYaw;
    bool mFirstEntrance;
    bool mMapTransition;

    bool mGhostMode;
    String mGhostToken;
    String mGhostMap;
    Vector3 mGhostPosition;

    Real mSendUpdateTimer;
    Real mSendUpdateTimer2;
    Real mResendUpdateTimer;
    Vector3 mLastSentPosition;
    Real mLastSentYaw;
    Vector2 mLastSentHeadDir;
    unsigned char mLastSentAction;
    bool mLastSentFlags[NUM_STATUSFLAGS];
public:
    InGameState()
    {
        mInfoMgr = InfoManager::getSingletonPtr();
        mControlMgr = ControlManager::getSingletonPtr();
        mChatMgr = ChatManager::getSingletonPtr();
        mGridMgr = GridManager::getSingletonPtr();
        mMapMgr = MapManager::getSingletonPtr();
        mNameTagMgr = NameTagManager::getSingletonPtr();
        mFriendsMgr = FriendsManager::getSingletonPtr();
        mPartyMgr = PartyManager::getSingletonPtr();
        mGroupMgr = GroupManager::getSingletonPtr();
        mOptionsMgr = OptionsManager::getSingletonPtr();
        mMovieMaker = MovieMaker::getSingletonPtr();
        mMiniMapMgr = MiniMapManager::getSingletonPtr();
        mGamePanelMgr = GamePanelManager::getSingletonPtr();
        mPlayerData = PlayerData();
        mArrivalYaw = 0;
        mCharID = 0;
        mUsername = "";
        mFirstEntrance = false;
        mMapTransition = false;
        mGhostMode = false;
        mGhostToken = "";
        mGhostMap = "";
    }
    ~InGameState()
    {
        exit();
    }
    void enter()
    {
        init();
        mGui->hideCursorOnRightClick(true);

        mCollMgr->setListener(this);
        mNameTagMgr->init();
        mFriendsMgr->init();
        mPartyMgr->init();
        mPartyMgr->setInfoCaller(mInfoMgr);
        mGroupMgr->init();
        mGroupMgr->setInfoCaller(mInfoMgr);
        mMovieMaker->init();
        mInfoMgr->init();
        mInfoMgr->setCharID(mCharID);
        mOptionsMgr->init(mSceneMgr);
        mEscPanel = mGui->addPanel("EscScreen",true);
        mEscPanel->show(false);
        //mGui->addPanel("DebugScreen");

		mLoaded = false;
		mNetMgr->clearListeners();
		mNetMgr->addListener(this);
		mNetMgr->addListener(mFriendsMgr);
		mNetMgr->addListener(mPartyMgr);
		mNetMgr->addListener(mGroupMgr);
		mNetMgr->addListener(mInfoMgr);
		// mNetMgr->addListener(mMiniMapMgr); // keep commented for now

		if (!mSplashScreenMgr->isVisible())mSplashScreenMgr->show(true);
		mNetMgr->connectMainServer();

        mSendUpdateTimer = 0;
        mSendUpdateTimer2 = 0;
        mResendUpdateTimer = 0;
        mTokenRetryAttempts = 0;
    }
    void exit()
    {
        if(!mMapTransition)
        {
            mGroupMgr->clearAll();
            mPartyMgr->clearAll();
            mFriendsMgr->clearAll();
            mGamePanelMgr->reset();
        }
        mOptionsMgr->reset();
        mNetMgr->trueShutdown();
        mMiniMapMgr->reset();
        mMovieMaker->clear();
        mGroupMgr->clear();
        mPartyMgr->clear();
        mFriendsMgr->clear();
        mNameTagMgr->clear();
        mChatMgr->reset();
        mControlMgr->reset();
        mInfoMgr->reset();
        mMapMgr->reset();
        uninit();
    }
    void update(const Real &timeElapsed)
    {
        mNetMgr->update(timeElapsed);
        mScreenFXMgr->update(timeElapsed);
        if(mLoaded)
        {
            mNameTagMgr->update(timeElapsed);
            mControlMgr->update(timeElapsed);
            mUnitMgr->update(timeElapsed);
            mControlMgr->updateCamera();
            mCameraMgr->update(timeElapsed);
            mGridMgr->update(mCameraMgr->getPosition());
            mSkyMgr->update(timeElapsed);
            mWaterMgr->update(mCameraMgr->getPosition());
            mSoundMgr->update(mCameraMgr->getPosition(),mCameraMgr->getDirection());
            mMovieMaker->update(timeElapsed);
            mInfoMgr->update(timeElapsed);
            mMiniMapMgr->update(timeElapsed);

            //mGui->getPanelByName("DebugScreen")->getComponentByName("DebugScreen/Debug")->setCaption(mMovieMaker->mDebug);
            if(!mMovieMaker->isPlaying() && !isGhostMode())sendUpdate(timeElapsed);
        }

        mRoot->renderOneFrame();
    }
    void sendUpdate(const Real &timeElapsed)
    {
        mSendUpdateTimer += timeElapsed;
        mSendUpdateTimer2 += timeElapsed;
        mResendUpdateTimer += timeElapsed;

        Unit *player = mControlMgr->getPlayer();

        //Periodic resend to upkeep data synchronization
        bool doResend = false;
        if(mResendUpdateTimer>30.0f)
        {
            mResendUpdateTimer = 0;
            doResend = true;
        }

        if(doResend || mSendUpdateTimer>(player->isRunningModifier()?0.5f:1.50f))
        {

            //Position
            Vector3 position = player->getPosition();
            if(!player->mAntiGravity)position.y = player->getGroundHeight();
            if(doResend || position!=mLastSentPosition)
            {
                mNetMgr->sendCharPosition(position);
                mLastSentPosition = position;
            }
            //Status flags
            const bool flags[NUM_STATUSFLAGS] = {player->mAntiGravity,player->mIsSwimming};
            bool changed = false;
            for(int i=0;i<NUM_STATUSFLAGS;i++)
            {
                if(mLastSentFlags[i]!=flags[i])
                {
                    mLastSentFlags[i] = flags[i];
                    changed = true;
                }
            }
            if(doResend || changed)mNetMgr->sendCharStatusFlags(flags[0],flags[1]);

            mSendUpdateTimer = 0;
        }
        if(doResend || mSendUpdateTimer2>2.00f)
        {

            //Yaw
            const Real yaw = player->getYaw();
            if(doResend || yaw!=mLastSentYaw)
            {
                mNetMgr->sendCharYaw((unsigned char)(yaw/360*255));
                mLastSentYaw = yaw;
            }
            //Head dir
            const Real ewState = player->getHeadEWState();
            const Real nsState = player->getHeadNSState();
            if(doResend || ewState!=mLastSentHeadDir.x || nsState!=mLastSentHeadDir.y)
            {
                mNetMgr->sendCharHeadDir(player->convertHeadEWState(ewState),player->convertHeadNSState(nsState));
                mLastSentHeadDir = Vector2(ewState,nsState);
            }
            //Action
            if(player->isSpecialAction())
            {
                const unsigned char action = player->getAction();
                if(doResend || action!=mLastSentAction)
                {
                    mNetMgr->sendCharAction(action);
                    mLastSentAction = action;
                }
            }

            mSendUpdateTimer2 = 0;
        }
    }
    void setPlayerData(PlayerData &data)
    {
        mPlayerData = data;
    }
    void setToken(const String &token, const unsigned int &charID=0)
    {
        mToken = token;
        mCharID = charID;
    }
    void setUsername(const String &username)
    {
        mUsername = username;
    }
    void setArrivalYaw(const Real &yaw)
    {
        mArrivalYaw = yaw;
    }
    void setFirstEntrance()
    {
        mFirstEntrance = true;
    }
    void setGhostMode(const String &token, const String &ghostMap, const Vector3 &ghostPos)
    {
        mGhostMode = (token!="");
        mGhostToken = token;
        mGhostMap = ghostMap;
        mGhostPosition = ghostPos;
    }
    const bool keyPressed(const char &index)
    {
        using namespace KeyboardIndex;
        if(mMovieMaker->isPlaying())
        {
            if(index==KB_OPTIONS)mMovieMaker->stopMovie();
            return true;
        }
        switch(index)
        {
            case KB_OPTIONS:
                if(mLoaded)
                {
                    if(mEscPanel->isVisible())mEscPanel->show(false);
                    else
                    {
                        if(!mGui->isVisible())mGui->showAll(true);
                        mEscPanel->show(true);
                        mGui->setActivePanel(mEscPanel);
                    }
                }
                else changeState = true;
                break;
            case KB_RETURN:
                mChatMgr->enterPressed();
                break;
            case KB_PRINTSCREEN:
                mChatMgr->systemMessage("Saved screenshot.");
                break;
            case KB_TOGGLEUI:
                if(mGui->isVisible())mGui->showAll(false);
                else mGui->showAll(true);
                break;
            default: break;
        }
        mControlMgr->keyPressed(index);
        mGamePanelMgr->keyPressed(index);
        mOptionsMgr->keyPressed(index);
        return true;
    }
    const bool keyReleased(const char &index)
    {
        mControlMgr->keyReleased(index);
        return true;
    }
    bool buttonClicked(GuiComponent *button)
    {
        if(button->nameIs("EscScreen/Options"))
        {
            mEscPanel->show(false);
            mOptionsMgr->show(true);
        }
        else if(button->nameIs("EscScreen/Quit"))
        {
            changeState = true;
        }
        else if(StringUtil::startsWith(button->getName(),"chat"))mChatMgr->buttonClicked(button);
        else if(StringUtil::startsWith(button->getName(),"movie"))mMovieMaker->buttonClicked(button);
        else if(StringUtil::startsWith(button->getName(),"action"))mControlMgr->buttonClicked(button);
        else if(StringUtil::startsWith(button->getName(),"friends"))mFriendsMgr->buttonClicked(button);
        else if(StringUtil::startsWith(button->getName(),"party"))mPartyMgr->buttonClicked(button);
        else if(StringUtil::startsWith(button->getName(),"group"))mGroupMgr->buttonClicked(button);
        else if(StringUtil::startsWith(button->getName(),"info"))mInfoMgr->buttonClicked(button);
        else if(StringUtil::startsWith(button->getName(),"ingamebuttons"))mGamePanelMgr->buttonClicked(button);
        mOptionsMgr->buttonClicked(button);
        return true;
    }
    bool mouseMoved(const OIS::MouseEvent &arg)
    {
        if(mMovieMaker->isPlaying())return true;

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
        if(StringUtil::startsWith(component->getName(),"chat"))mChatMgr->changeEvent(component,event);
        else if(StringUtil::startsWith(component->getName(),"movie"))mMovieMaker->changeEvent(component,event);
        else if(StringUtil::startsWith(component->getName(),"action"))mControlMgr->changeEvent(component,event);
        else if(StringUtil::startsWith(component->getName(),"friends"))mFriendsMgr->changeEvent(component,event);
        else if(StringUtil::startsWith(component->getName(),"party"))mPartyMgr->changeEvent(component,event);
        else if(StringUtil::startsWith(component->getName(),"group"))mGroupMgr->changeEvent(component,event);
        else if(StringUtil::startsWith(component->getName(),"info"))mInfoMgr->changeEvent(component,event);
        else if(StringUtil::startsWith(component->getName(),"minimap"))mMiniMapMgr->changeEvent(component,event);
        else if(StringUtil::startsWith(component->getName(),"options"))mOptionsMgr->changeEvent(component,event);
        return true;
    }
    bool comboBoxEvent(const String &name)
    {
        if(StringUtil::startsWith(name,"chat"))mChatMgr->comboBoxEvent(name);
        else if(StringUtil::startsWith(name,"friends"))mFriendsMgr->comboBoxEvent(name);
        else if(StringUtil::startsWith(name,"party"))mPartyMgr->comboBoxEvent(name);
        else if(StringUtil::startsWith(name,"group"))mGroupMgr->comboBoxEvent(name);
        return true;
    }
    bool alertBoxEvent(const String &name, const bool &flag)
    {
        if(name=="InGame/ConnectFail")
        {
            changeState = true;
        }
        else if(name=="InGame/ConnectRetry")
        {
            if(flag)mNetMgr->connectMapServer();
            else changeState = true;
        }
        else if(StringUtil::startsWith(name,"friends"))mFriendsMgr->alertBoxEvent(name,flag);
        else if(StringUtil::startsWith(name,"party"))mPartyMgr->alertBoxEvent(name,flag);
        else if(StringUtil::startsWith(name,"group"))mGroupMgr->alertBoxEvent(name,flag);
        else if(StringUtil::startsWith(name,"info"))mInfoMgr->alertBoxEvent(name,flag);
        else if(StringUtil::startsWith(name,"movie"))mMovieMaker->alertBoxEvent(name,flag);
        else if(StringUtil::startsWith(name,"options"))mOptionsMgr->alertBoxEvent(name,flag);
        return true;
    }
    void mapChange(const GateInfo &mapInfo)
    {
        doMapChange(mapInfo);
    }
    void nearGate(const String &name)
    {
        String displayName = "";
        if(mMapMgr->getMapSeed(name,&displayName)!="")mScreenFXMgr->showMapPeek(true,displayName);
    }
    void mainServerReply(const unsigned char &reply)
    {
        switch(reply)
        {
            case NetworkManager::SERVER_CONNECT_SUCCESS:
                //mGui->showAlertBox("Request accepted, loading data...","InGame/Loading",false,false);
                //First connection
                if(mFirstEntrance)mNetMgr->sendConnectMain(mToken,mCharID);
                //Mapchange connection
                else mNetMgr->sendConnectMain(mToken,0,isGhostMode()?mGhostMap:"");
                break;
            case NetworkManager::SERVER_CONNECT_FAIL:
                mGui->showAlertBox("Failed to connect to main server.","InGame/ConnectFail");
                break;
            case NetworkManager::SERVER_CONNECT_FULL:
                mGui->showAlertBox("Server is full.","InGame/ConnectFail");
                break;
            case NetworkManager::SERVER_CONNECT_MISMATCH:
                mGui->showAlertBox("Please download latest client.","InGame/ConnectFail");
                break;
            case NetworkManager::SERVER_CONNECT_LOST:
                mGui->showAlertBox("Connection Lost.","InGame/ConnectFail");
                break;
            case NetworkManager::SERVER_TOKEN_FAIL:
                if(mTokenRetryAttempts>=3)mGui->showAlertBox("Bad token.","InGame/ConnectFail");
                else
                {
                    mTokenRetryAttempts++;
                    RakSleep(300);
                    mNetMgr->sendConnectMain(mToken,0,isGhostMode()?mGhostMap:"");
                }
                break;
            default: break;
        }
    }
    void mainServerMapReply(const String &map, const Real &dayTime, const Real &weatherTime)
    {
        mSkyMgr->setDayTime(dayTime);
        mSkyMgr->setWeatherTime(weatherTime);
        mMapMgr->init(mSceneMgr,map);
        mNetMgr->doMapTransition();
    }
    void mapServerReply(const unsigned char &reply)
    {
        switch(reply)
        {
            case NetworkManager::SERVER_CONNECT_SUCCESS:
                mTokenRetryAttempts = 0;
                mNetMgr->sendConnectMap(mToken,mFirstEntrance,isGhostMode());
                break;
            case NetworkManager::SERVER_CONNECT_FAIL:
                if(mTokenRetryAttempts>=3)mGui->showAlertBox("Failed to connect to map server. Retry?","InGame/ConnectRetry",true);
                else
                {
                    mTokenRetryAttempts++;
                    RakSleep(300);
                    mNetMgr->doMapTransition();
                }
                break;
            case NetworkManager::SERVER_CONNECT_FULL:
                mGui->showAlertBox("Map server is full.","InGame/ConnectFail");
                break;
            case NetworkManager::SERVER_CONNECT_MISMATCH:
                mGui->showAlertBox("Please download latest client.","InGame/ConnectFail");
                break;
            case NetworkManager::SERVER_CONNECT_LOST:
                mChatMgr->systemMessage("Connection Lost from map server.");
                mGui->showAlertBox("Reconnecting...","InGame/Reconnect",false,false);
                mUnitMgr->destroyAllExceptPlayer();
                mNetMgr->doMapTransition();
                break;
            case NetworkManager::SERVER_DISCONNECTED:
                mChatMgr->systemMessage("Disconnected fom map server.");
                doMapChange(GateInfo());
                break;
            case NetworkManager::SERVER_TOKEN_FAIL:
                if(mTokenRetryAttempts>=3)mGui->showAlertBox("Bad token.","InGame/ConnectFail");
                else
                {
                    mTokenRetryAttempts++;
                    RakSleep(300);
                    mNetMgr->sendConnectMap(mToken,mFirstEntrance,isGhostMode());
                }
                break;
            default: break;
        }
    }
    void mapServerInfo(const Vector3 &position)
    {
        if(mGui->getAlertBoxName()=="InGame/Loading"||mGui->getAlertBoxName()=="InGame/Reconnect")mGui->hideAlertBox();
        if(!mLoaded)
        {
            Unit *unit = mUnitMgr->createUnit(mPlayerData,mUsername);
            unit->mCharID = mCharID;
            //unit->mUsername = mUsername;
            unit->setPosition(isGhostMode()?mGhostPosition:position);
            unit->setYaw(mArrivalYaw,true);
            mCameraMgr->yaw(mArrivalYaw);
            mUnitMgr->initUnitPosition(unit);
            mUnitMgr->setPlayer(unit);
            //mNameTagMgr->createNameTag(unit,mUsername,0.9f,1.0f);
            mControlMgr->init(unit,mSceneMgr,true);
            mNetMgr->addListener(SkillsManager::getSingletonPtr());
            unit->addListener(SkillsManager::getSingletonPtr());
            mControlMgr->updateCameraScale();
            mChatMgr->init();
            mChatMgr->setInputName(mUsername,mPlayerData.mName);
            mChatMgr->addListener(this);
            if(mControlMgr->getActionsManager())
            {
                mChatMgr->addListener(mControlMgr->getActionsManager());
                mControlMgr->getActionsManager()->setPlayerDataEmotePointer(&mPlayerData.mEmote);
            }
            mMovieMaker->setPlayer(unit);
            mMiniMapMgr->init();
            mMiniMapMgr->setPlayer(unit);
            mGamePanelMgr->init();
            mGroupMgr->setListener(mGamePanelMgr);
            mPartyMgr->setListener(mGamePanelMgr);
            mFriendsMgr->setListener(mGamePanelMgr);
            mChatMgr->setGamePanelListener(mGamePanelMgr);
            mInfoMgr->setOwnInfo(mUsername,mPlayerData.mName);
            mInfoMgr->setPlayer(unit);
            mSplashScreenMgr->show(false);

            if(isGhostMode())
            {
                unit->setFullMaterial("Collision/Highlight");
                mChatMgr->setMuteSelfOnLocal(true);
                mNetMgr->sendGhostMapChange(false);
            }

            mChatMgr->systemMessage(" - "+mMapMgr->getMapName()+" - ");
            mScreenFXMgr->showMapTitle(true,mMapMgr->getMapName());
        }
        mLoaded = true;
        mFirstEntrance = false;
    }
    void charInfo(const unsigned int &charID, PlayerData &data, const Vector3 &position, const unsigned int &userID, const String &username)
    {
        //Silently destroy old clones
        Unit *clone = mUnitMgr->getUnitByCharID(charID);
        if(clone)
        {
            mMiniMapMgr->removeUnitMarker(clone);
            mUnitMgr->destroyUnit(clone);
        }

        //No other characters in default map
        if(mMapMgr->isDefaultMap())return;

        Unit *unit = mUnitMgr->createUnit(data,username);
        unit->mCharID = charID;
        unit->mUserID = userID;
        unit->mIsLocal = false;
        unit->mApplySlopeForces = false;
        unit->setPosition(position);
        mUnitMgr->initUnitPosition(unit);
        unit->addListener(mControlMgr);
        unit->addListener(SkillsManager::getSingletonPtr());
        if(mFriendsMgr->isBlocked(userID))unit->setInvisible(true);
        else
        {
            if(mFriendsMgr->isFriend(userID))mMiniMapMgr->addUnitMarker(unit);
            //Announce arrival only if there were no clones
            if(!clone && !mChatMgr->isHideSystemChat())mChatMgr->systemMessage(mChatMgr->getSystemChatName(username,data.mName)+" has arrived.",CHAT_SYSTEM,charID);
        }
    }
    void charExit(const unsigned int &charID, const bool &isProper, const bool &isMapChange)
    {
        Unit *unit = mUnitMgr->getUnitByCharID(charID);
        if(unit)
        {
            if(!mChatMgr->isHideSystemChat())mChatMgr->systemMessage(mChatMgr->getSystemChatName(unit->mUsername,unit->mName)+(isProper?(isMapChange?" has left.":" has disconnected."):" has lost connection."));
            mMiniMapMgr->removeUnitMarker(unit);
            mUnitMgr->destroyUnit(unit);
        }
    }
    void charPositionUpdate(const unsigned int &charID, const Vector3 &position)
    {
        Unit *unit = mUnitMgr->getUnitByCharID(charID);
        if(unit)
        {
            if(unit->mAntiGravity)
            {
                if(unit->getPosition()!=position)unit->setMoveTarget(position);
            }
            else
            {
                if(unit->getPosition().x!=position.x || unit->getPosition().z!=position.z)unit->setMoveTarget(position);
                unit->setRemoteGroundHeight(position.y);
            }
        }
    }
    void chatEvent(const unsigned int &charID, const String &caption, const unsigned char &channel, const unsigned int &userID, const String &whisperer="", const bool &whisperFail=false, const bool &isOffline=false, const bool &doLipSync=true)
    {
        if(channel==CHAT_SYSTEM)mChatMgr->systemMessage(caption);
        else if(channel==CHAT_MOD)mChatMgr->systemMessage(caption,CHAT_MOD);
        else if(channel==CHAT_MODWHISPER)mChatMgr->pushChatLine(caption,whisperer,CHAT_MODWHISPER,0,true);
        else
        {
            if((channel==CHAT_GROUP || channel==CHAT_PARTY || channel==CHAT_WHISPER) && mFriendsMgr->isBlocked(userID) && !mGhostMode)return;
            if(channel==CHAT_GROUP)mChatMgr->pushChatLine(caption,mGroupMgr->getGroupMemberNameByCharID(charID),channel,charID);
            else if(channel==CHAT_PARTY)mChatMgr->pushChatLine(caption,mPartyMgr->getPartyMemberNameByCharID(charID),channel,charID);
            else if(channel==CHAT_WHISPER)
            {
                if(isOffline)mChatMgr->systemMessage(whisperer+" is offline.");
                else if(whisperFail)mChatMgr->systemMessage(whisperer+" is not found.");
                else mChatMgr->pushChatLine(caption,whisperer,channel,charID,true);
            }
            else
            {
                Unit *unit = mUnitMgr->getUnitByCharID(charID);
                if(unit && mFriendsMgr->isBlocked(unit->mUserID) && !mGhostMode)return;
                if(!unit)
                {
                    if(!mMapMgr->isDefaultMap())mChatMgr->pushChatLine(caption,"???",channel,charID);
                    return;
                }
                if(channel==CHAT_GENERAL)mChatMgr->pushChatLine(caption,mChatMgr->getGeneralChatName(unit->mUsername,unit->mName),channel,charID);
                else
                {
                    mChatMgr->pushChatLine(caption,mChatMgr->getLocalChatName(unit->mUsername,unit->mName),channel,charID);

                    if(doLipSync)unit->setLipSyncSpeech(caption);
                }
            }
        }
    }
    void charYawUpdate(const unsigned int &charID, const unsigned char &yaw)
    {
        Unit *unit = mUnitMgr->getUnitByCharID(charID);
        if(unit)unit->setYaw((Real(yaw)/255)*360);
    }
    void charHeadDirUpdate(const unsigned int &charID, const unsigned char &ew, const unsigned char &ns)
    {
        Unit *unit = mUnitMgr->getUnitByCharID(charID);
        if(unit)
        {
            unit->setHeadEWState(unit->convertHeadEWState(ew));
            unit->setHeadNSState(unit->convertHeadNSState(ns));
        }
    }
    void charActionUpdate(const unsigned int &charID, const unsigned char &action)
    {
        Unit *unit = mUnitMgr->getUnitByCharID(charID);
        if(unit)
        {
            if(action!=Unit::ACT_IDLE || unit->isSpecialAction())unit->setAction(action);
        }
    }
    void charStatusFlagsUpdate(const unsigned int &charID, const bool &antiGravity, const bool &swimming)
    {
        Unit *unit = mUnitMgr->getUnitByCharID(charID);
        if(unit)
        {
            unit->mAntiGravity = antiGravity;
            unit->mIsSwimming = swimming;
        }
    }
    void charMoveModifierEvent(const unsigned int &charID, const unsigned char &moveModifier)
    {
        Unit *unit = mUnitMgr->getUnitByCharID(charID);
        if(unit)unit->setMovementModifier(moveModifier);
    }
    void charEmoteEvent(const unsigned int &charID, const unsigned char &emote)
    {
        Unit *unit = mUnitMgr->getUnitByCharID(charID);
        if(unit)unit->setEmote(emote);
    }
    void charJumpEvent(const unsigned int &charID, const bool &flag)
    {
        Unit *unit = mUnitMgr->getUnitByCharID(charID);
        if(unit)
        {
            if(flag)unit->startJump();
            else unit->stopJump();
        }
    }
    void movieEvent(const unsigned int &charID, const String &movie, const Real &camYaw)
    {
        Unit *unit = mUnitMgr->getUnitByCharID(charID);
        if(unit)mMovieMaker->pushMovie(movie,unit,camYaw);
    }
    void findCharEvent(const String &name, const String &mapName)
    {
        if(mapName=="")
        {
            mChatMgr->systemMessage(name+" is offline.");
            return;
        }
        String displayName = "";
        mMapMgr->getMapSeed(mapName,&displayName);
        mChatMgr->systemMessage(name+" is in "+(displayName==""?mapName:displayName)+".");
    }
    void doMapChange(const GateInfo &mapInfo, const bool &isGoHome=false)
    {
        if(mapInfo.mMap!="")
        {
            //Check that map exists
            const String mapSeed = mMapMgr->getMapSeed(mapInfo.mMap,0,isGoHome);
            if(mapSeed=="")
            {
                mGui->showAlertBox("Cannot find Map file.");
                return;
            }
            changeState = true;
            replaceState = true;
            InGameState *state = new InGameState();
            state->setPlayerData(mPlayerData);
            state->setToken(mToken,mCharID);
            state->setUsername(mUsername);
            if(isGhostMode())state->setGhostMode(mGhostToken,mapInfo.mMap+mapSeed,mapInfo.mPosition);
            else mNetMgr->sendMapChange(mapInfo.mMap+mapSeed,mapInfo.mPosition);
            state->setArrivalYaw(mapInfo.mYaw);
            mNextState = state;
            mMapTransition = true;
        }
        //Map reset
        else
        {
            changeState = true;
            replaceState = true;
            InGameState *state = new InGameState();
            state->setPlayerData(mPlayerData);
            state->setToken(mToken,mCharID);
            state->setUsername(mUsername);
            state->setGhostMode(mGhostToken,mGhostMap,mGhostPosition);
            mNextState = state;
            mMapTransition = true;
        }
    }
    const bool isGhostMode()
    {
        return (mGhostMode && mGhostToken=="aRxPuT");
    }
    void modEvent(const bool &bIsSender, const String &command)
    {
        if(StringUtil::startsWith(command,COMMAND_HELP))
        {
            const StringVector part = StringUtil::split(command,"\n");
            for(StringVector::const_iterator it=part.begin(); it!=part.end(); it++)
            {
                const String line = *it;
                if(it!=part.begin())mChatMgr->systemMessage(line);
            }
        }
        else if(StringUtil::startsWith(command,COMMAND_NOSUCHUSER))
        {
            const StringVector part = StringUtil::split(command," ",1);
            if(part.size()>1)mChatMgr->systemMessage("No user by the name "+part[1]+".");
        }
        else if(command==COMMAND_GHOST)
        {
            if(isGhostMode())
            {
                mGhostMode = false;
                mGhostToken = "";
                doMapChange(GateInfo());
                mNetMgr->sendGhostMapChange(true);
            }
            else
            {
                mGhostMode = true;
                mGhostToken = "aRxPuT";
                mNetMgr->sendGhostMapChange(false);
                mChatMgr->setMuteSelfOnLocal(true);

                //Unnecessary effects
                Unit *player = mControlMgr->getPlayer();
                if(player)
                {
                    player->setFullMaterial("Collision/Highlight");
                    Unit *unit = mUnitMgr->createUnit(mPlayerData);
                    unit->mUsername = mUsername;
                    unit->mCharID = mCharID;
                    unit->setPosition(player->getPosition());
                    unit->setYaw(player->getYaw(),true);
                    unit->setHeadEWState(player->getHeadEWState(),true);
                    unit->setHeadNSState(player->getHeadNSState(),true);
                    unit->setEmote(player->getEmote());
                    unit->setAction(player->getAction(),true);
                    mUnitMgr->initUnitPosition(unit);
                    mNameTagMgr->createNameTag(unit,mUsername,0.9f,1.0f);
                }
            }
        }
        else if(StringUtil::startsWith(command,COMMAND_KICK))
        {
            const StringVector section = StringUtil::split(command,";",1);
            if(section.size()>0)
            {
                const StringVector part = StringUtil::split(section[0]," ",1);
                if(part.size()>1)
                {
                    const String username = part[1];
                    if(!bIsSender)
                    {
                        mGui->showAlertBox("Kicked by administrator: " + (section.size()>1?section[1]:""));
                        mNetMgr->shutdown();
                    }
                    else mChatMgr->systemMessage("Kicked "+username+": " + (section.size()>1?section[1]:""));
                }
            }
        }
        else if(StringUtil::startsWith(command,COMMAND_BAN))
        {
            const StringVector section = StringUtil::split(command,";",1);
            if(section.size()>0)
            {
                const StringVector part = StringUtil::split(section[0]," ");
                if(part.size()>3)
                {
                    String username = part[1];
                    for(int i=2;i<(int)part.size()-2;i++)username += " " + part[i];
                    if(!bIsSender)
                    {
                        mGui->showAlertBox("Banned by administrator for "+part[part.size()-2]+" "+part[part.size()-1]+": "+(section.size()>1?section[1]:""));
                        mNetMgr->shutdown();
                    }
                    else mChatMgr->systemMessage("Banned "+username+" for "+part[part.size()-2]+" "+part[part.size()-1]+": "+(section.size()>1?section[1]:""));
                }
            }
        }
        else if(StringUtil::startsWith(command,COMMAND_UNBAN))
        {
            const StringVector part = StringUtil::split(command," ",1);
            if(part.size()>1)mChatMgr->systemMessage("Unbanned "+part[1]+".");
        }
    }
    const bool chatLineClicked(const ClickableLine &line)
    {
        return false;
    }
    const bool chatCommand(const String &command)
    {
        if(command==COMMAND_HELP
            ||command==COMMAND_GHOST
            ||StringUtil::startsWith(command,COMMAND_KICK)
            ||StringUtil::startsWith(command,COMMAND_BAN)
            ||StringUtil::startsWith(command,COMMAND_UNBAN)
            ||StringUtil::startsWith(command,COMMAND_GODSPEAK))
        {
            mNetMgr->sendMod(command);
        }
        return false;
    }
    const bool chatLocal(const String &caption)
    {
        return false;
    }
    void duplicateLoginEvent(const String &token)
    {
        if(token==mToken)
        {
            mGui->showAlertBox("Logged out by another user.");
            mNetMgr->shutdown();
        }
    }
    void goHomeEvent(const String &home, const Vector3 &position)
    {
        GateInfo info;
        info.mMap = home;
        info.mPosition = position;
        doMapChange(info,true);
    }
};

#endif

