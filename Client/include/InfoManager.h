#ifndef _INFOMANAGER_H_
#define _INFOMANAGER_H_

#include <InfoCaller.h>
#include <FriendsManager.h>
#include <PartyManager.h>
#include <GroupManager.h>
#include <MapManager.h>

class InfoManager : public Singleton<InfoManager>, public InfoCaller, public NetworkListener, public ChatListener
{
private:
    Gui *mGui;
    NetworkManager *mNetMgr;
    ChatManager *mChatMgr;
    PartyManager *mPartyMgr;
    GroupManager *mGroupMgr;
    GuiPanel *mPanel;
    GuiTextComponent *mUsernameField;
    GuiTextComponent *mCharnameField;
    GuiTextComponent *mGroupField;
    GuiTextArea *mBioArea;
    GuiTextComponent *mHomeField;
    String mBioCache;
    bool mOwnBioRequested;
    unsigned int mCharID;
    Unit *mPlayer;
    bool mHomeRequested;

    GuiMultiPanel *mViewPanel;
    GuiTab *mViewInfoTab;
    GuiTextComponent *mViewUsernameField;
    GuiTextComponent *mViewCharnameField;
    GuiTextComponent *mViewGroupField;
    GuiButton *mGroupViewButton;
    GuiButton *mGroupInviteButton;
    GuiTextArea *mViewBioArea;
    unsigned int mViewUserID;
    unsigned int mViewCharID;
    unsigned int mViewGroupID;
    bool mViewBioRequested;

    GuiPanel *mReportPanel;
    GuiTextComponent *mReportField;
    GuiTextArea *mReportArea;
    Real mReportTimer;

    unsigned short mChatClickedLine;
    Real mChatDoubleClickTimer;
public:
    InfoManager()
    {
        mGui = Gui::getSingletonPtr();
        mNetMgr = NetworkManager::getSingletonPtr();
        mChatMgr = ChatManager::getSingletonPtr();
        mPartyMgr = PartyManager::getSingletonPtr();
        mGroupMgr = GroupManager::getSingletonPtr();
        reset();
        mBioCache = "";
        mOwnBioRequested = false;
        mHomeRequested = false;
        mCharID = 0;
        mReportTimer = 0;
    }
    ~InfoManager()
    {
    }
    static InfoManager* getSingletonPtr();
    static InfoManager& getSingleton();
    void init()
    {
        mChatMgr->addListener(this);

        mPanel = mGui->addPanel("InfoScreen");
        mPanel->show(false);
        mUsernameField = mPanel->getTextComponent("InfoScreen/Username");
        mUsernameField->mClickable = false;
        mCharnameField = mPanel->getTextComponent("InfoScreen/Charname");
        mCharnameField->mClickable = false;
        mGroupField = mPanel->getTextComponent("InfoScreen/Group");
        mGroupField->mClickable = false;
        mBioArea = mPanel->getTextArea("InfoScreen/Bio");
        mBioArea->mMaxLength = 1024;
        mBioArea->mAllowNewLine = true;
        mHomeField = mPanel->getTextComponent("InfoScreen/Home");
        mHomeField->mClickable = false;

        mViewPanel = mGui->addMultiPanel("InfoViewScreen");
        mViewPanel->show(false);
        mViewInfoTab = mViewPanel->getTab("InfoViewScreen/Page1");
        mViewUsernameField = mViewPanel->getTextComponent("InfoViewScreen/Username");
        mViewUsernameField->mClickable = true;
        mViewCharnameField = mViewPanel->getTextComponent("InfoViewScreen/Charname");
        mViewCharnameField->mClickable = true;
        mViewGroupField = mViewPanel->getTextComponent("InfoViewScreen/Group");
        mViewGroupField->mClickable = true;
        mGroupViewButton = mViewPanel->getButton("InfoViewScreen/GroupView");
        mGroupInviteButton = mViewPanel->getButton("InfoViewScreen/GroupInvite");
        mViewBioArea = mViewPanel->getTextArea("InfoViewScreen/Bio");
        mViewBioArea->mClickable = false;

        mReportPanel = mGui->addPanel("InfoReportScreen");
        mReportPanel->show(false);
        mReportField = mReportPanel->getTextComponent("InfoReportScreen/Username");
        mReportField->mClickable = false;
        mReportArea = mReportPanel->getTextArea("InfoReportScreen/Message");
        mReportArea->mMaxLength = 256;
        mReportArea->mAllowNewLine = true;
    }
    void reset()
    {
        mOwnBioRequested = false;
        mViewUserID = 0;
        mViewCharID = 0;
        mViewGroupID = 0;
        mViewBioRequested = false;
        mChatClickedLine = 0;
        mChatDoubleClickTimer = 0;
        mPlayer = 0;
        mHomeRequested = false;
    }
    GuiPanel* getPanel()
    {
        return mPanel;
    }
    void update(const Real &timeElapsed)
    {
        if(mReportTimer>0)mReportTimer -= timeElapsed;
        if(mChatDoubleClickTimer>0)mChatDoubleClickTimer -= timeElapsed;
    }
    void updateBio()
    {
        const string bio = mBioArea->getCaption();
        if(mBioCache==bio)return;
        mBioCache = bio;
        mNetMgr->sendCharBio(bio);
    }
    void changeEvent(GuiComponent *component, const unsigned char &event)
    {
        if(component==mBioArea)updateBio();
        else if(component==mViewUsernameField && event==ChangeListener::EVENT_MOUSEPRESSED)
        {
            mChatMgr->appendInputArea(mViewUsernameField->getCaption());
        }
        else if(component==mViewCharnameField && event==ChangeListener::EVENT_MOUSEPRESSED)
        {
            mChatMgr->appendInputArea(mViewCharnameField->getCaption());
        }
        else if(component==mViewGroupField && event==ChangeListener::EVENT_MOUSEPRESSED)
        {
            mChatMgr->appendInputArea(mViewGroupField->getCaption());
        }
        else if(component->isTab() && component->nameIs("InfoScreen/Page2"))
        {
            if(!mOwnBioRequested)
            {
                mNetMgr->sendCharBioRequest(0,true);
                mOwnBioRequested = true;
            }
        }
        else if(component->isTab() && component->nameIs("InfoViewScreen/Page2"))
        {
            if(!mViewBioRequested)
            {
                mNetMgr->sendCharBioRequest(mViewCharID,true);
                mViewBioRequested = true;
            }
        }
        else if(component->isTab() && component->nameIs("InfoScreen/Page3"))
        {
            if(!mHomeRequested)
            {
                mNetMgr->sendHome(true,false);
                mHomeRequested = true;
            }
        }
    }
    void buttonClicked(GuiComponent *button)
    {
        if(button==mGroupInviteButton)mGui->showAlertBox("Send group invitation to "+mViewCharnameField->getCaption()+"?","InfoView/GroupInvite",true);
        else if(button==mGroupViewButton)mGroupMgr->viewGroupBio(mViewGroupID,mViewGroupField->getCaption());
        else if(button->nameIs("InfoViewScreen/Whisper"))
        {
            mChatMgr->setChannel(CHAT_WHISPER);
            mChatMgr->setWhisperTarget(mViewUsernameField->getCaption());
        }
        else if(button->nameIs("InfoViewScreen/Party"))
        {
            const String username = mViewUsernameField->getCaption();
            mGui->showAlertBox(mPartyMgr->hasParty() ? "Invite "+username+" to party?" : "Join "+username+"'s party?","InfoView/Party",true);
        }
        else if(button->nameIs("InfoViewScreen/Friend"))
        {
            mNetMgr->sendAddFriend(mViewUsernameField->getCaption(),false);
        }
        else if(button->nameIs("InfoViewScreen/Block"))
        {
            mGui->showAlertBox("Block "+mViewUsernameField->getCaption()+"?","InfoView/BlockAdd",true);
        }
        else if(button->nameIs("InfoViewScreen/Report"))
        {
            if(mReportTimer>0)mGui->showAlertBox("Can only report once every 60 seconds.");
            else
            {
                mReportPanel->show(true);
                mReportField->setCaption(mViewUsernameField->getCaption());
                mReportArea->setCaption("");
            }
        }
        else if(button->nameIs("InfoReportScreen/Report"))
        {
            mGui->showAlertBox("Your report has been submitted to the admin.");
            mReportPanel->show(false);
            if(mReportTimer<=0)mNetMgr->sendReport(mViewUserID,mReportArea->getCaption());
            mReportTimer = 60;
        }
        else if(button->nameIs("InfoScreen/SetHome"))
        {
            mGui->showAlertBox("Set home to "+MapManager::getSingletonPtr()->getMapName()+"?","Info/SetHome",true);
        }
        else if(button->nameIs("InfoScreen/GoHome"))
        {
            mGui->showAlertBox("Return to "+mHomeField->getCaption()+"?","Info/GoHome",true);
        }
        else if(button->nameIs("InfoScreen/ResetHome"))
        {
            mGui->showAlertBox("Reset home position?","Info/ResetHome",true);
        }
    }
    void alertBoxEvent(const String &name, const bool &flag)
    {
        if(name=="InfoView/BlockAdd")
        {
            if(flag)mNetMgr->sendAddFriend(mViewUsernameField->getCaption(),true);
        }
        else if(name=="InfoView/GroupInvite")
        {
            if(flag && mViewCharID)mNetMgr->sendGroupRequest(mViewCharID,false);
        }
        else if(name=="Info/SetHome")
        {
            if(flag && mPlayer)mNetMgr->sendHome(false,false,false,mPlayer->getPosition());
        }
        else if(name=="Info/GoHome")
        {
            if(flag)mNetMgr->sendHome(false,true);
        }
        else if(name=="Info/ResetHome")
        {
            if(flag && mPlayer)mNetMgr->sendHome(false,false,true);
        }
        else if(name=="InfoView/Party")
        {
            if(flag && mViewCharID)mNetMgr->sendPartyRequest(mViewCharID,false);
        }
    }
    void viewInfo(const unsigned int &userID, const unsigned int &charID, const String &username="", const String &charname="")
    {
        if(mCharID==charID)
        {
            mPanel->show(true);
            return;
        }
        mViewUserID = userID;
        mViewCharID = charID;
        mViewGroupID = 0;
        mViewUsernameField->setCaption(username);
        mViewCharnameField->setCaption(charname);
        mViewGroupField->setCaption("");
        mViewPanel->selectTab(mViewInfoTab);
        mViewPanel->show(true);
        mGui->setActivePanel(mViewPanel);
        mViewBioRequested = false;
        mViewBioArea->setCaption("");
        mNetMgr->sendCharBioRequest(charID,false,userID==0);
        mGroupViewButton->show(false);
        mGroupInviteButton->show(false);
    }
    void charBioInfoEvent(const String &username, const String &charname, const String &group, const unsigned int &groupID, const unsigned int &userID)
    {
        mViewUsernameField->setCaption(username);
        mViewCharnameField->setCaption(charname);
        mViewGroupField->setCaption(group);
        mViewGroupID = groupID;
        if(groupID==0)
        {
            if(mGroupMgr->isLeader())mGroupInviteButton->show(true);
        }
        else mGroupViewButton->show(true);
        if(userID)mViewUserID = userID;
    }
    void charBioEvent(const bool &isOwn, const String &bio)
    {
        if(isOwn)
        {
            mBioCache = bio;
            mBioArea->setCaption(mBioCache);
        }
        else mViewBioArea->setCaption(bio);
    }
    void setCharID(const unsigned int &charID)
    {
        if(mCharID==charID)return;
        mCharID = charID;
        mOwnBioRequested = false;
        mBioArea->setCaption("");
    }
    void setPlayer(Unit *unit)
    {
        mPlayer = unit;
    }
    void setOwnInfo(const String &username, const String &charname)
    {
        mUsernameField->setCaption(username);
        mCharnameField->setCaption(charname);
    }
    void setOwnGroup(const String &groupname)
    {
        mGroupField->setCaption(groupname);
    }
    const unsigned int getOwnCharID()
    {
        return mCharID;
    }
    const bool chatLineClicked(const ClickableLine &line)
    {
        if(line.mCharID==0)return false;

        //Double clicked
        if(mChatDoubleClickTimer>0 && mChatClickedLine==line.mIndex)
        {
            mChatDoubleClickTimer = 0;
            mChatClickedLine = 0;
            viewInfo(0,line.mCharID);
        }
        else
        {
            mChatDoubleClickTimer = 0.5f;
            mChatClickedLine = line.mIndex;
        }
        return true;
    }
    const bool chatCommand(const String &command){return false;}
    const bool chatLocal(const String &caption){return false;}
    void homeInfo(const String &home)
    {
        String mapName = "";
        MapManager::getSingletonPtr()->getMapSeed(home,&mapName);
        mHomeField->setCaption(mapName);
    }
};

template<> InfoManager* Singleton<InfoManager>::ms_Singleton = 0;

InfoManager* InfoManager::getSingletonPtr()
{
	return ms_Singleton;
}

InfoManager& InfoManager::getSingleton()
{
	assert(ms_Singleton);
	return *ms_Singleton;
}

#endif
