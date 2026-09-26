#ifndef _GROUPMANAGER_H_
#define _GROUPMANAGER_H_

#include <InfoCaller.h>
#include <GamePanelListener.h>

struct GroupInfo
{
    unsigned int mGroupID;
    String mGroupName;
    GroupInfo(const unsigned int &groupID, const String &groupname)
    {
        mGroupID = groupID;
        mGroupName = groupname;
    }
};
struct GroupMemberInfo
{
    unsigned int mCharID;
    String mCharName;
    unsigned char mRank;
    String mTitle;
    bool mIsOnline;
    GroupMemberInfo(const unsigned int &charID, const String &charname)
    {
        mCharID = charID;
        mCharName = charname;
        mRank = 255;
        mTitle = "";
        mIsOnline = false;
    }
};

class GroupManager : public Singleton<GroupManager>, public NetworkListener, public GuiPanelListener
{
private:
    Gui *mGui;
    NetworkManager *mNetMgr;
    ChatManager *mChatMgr;
    InfoCaller *mInfoCaller;
    GuiPanel *mPanel;
    GuiHighlightList *mMyGroupList;
    OverlayElement *mNoGroupLabel;
    GuiTextField *mGroupField;
    OverlayElement *mRequestLabel;
    GuiHighlightList *mRequestList;
    GuiButton *mCreateButton;
    GuiButton *mBioButton;
    GuiButton *mLeaveButton;
    GuiTextField *mFilterGroupField;
    GuiHighlightList *mViewGroupList;
    String mGroupSearchCache;

    GuiPanel *mBioPanel;
    GuiTextComponent *mBioNameField;
    GuiTextArea *mBioArea;

    GuiPanel *mViewGroupPanel;
    GuiTextComponent *mViewGroupField;
    GuiHighlightList *mViewGroupMembersList;
    GuiButton *mSearchButton;

    String mGroupName;
    bool mIsLeader;
    String mGroupBioCache;
    //bool mRequestedViewGroups;
    unsigned int mSelectionID;

    std::vector<GroupMemberInfo*> mGroupMembers;
    std::vector<GroupInfo*> mInvites;
    std::vector<GroupInfo*> mRequests;
    std::vector<GroupInfo*> mViewGroups;
    std::vector<GroupMemberInfo*> mViewGroupMembers;

    std::vector<String> mMyGroupComboList;
    std::vector<String> mViewGroupComboList;
    std::vector<String> mRequestComboList;

    GamePanelListener *mListener;
public:
    GroupManager()
    {
        mGui = Gui::getSingletonPtr();
        mNetMgr = NetworkManager::getSingletonPtr();
        mChatMgr = ChatManager::getSingletonPtr();
        mInfoCaller = 0;
        clearAll();

        mMyGroupComboList.push_back("View Info");
        mMyGroupComboList.push_back("Find");

        mViewGroupComboList.push_back("View Members");
        mViewGroupComboList.push_back("View Bio");

        mRequestComboList.push_back("Accept");
        mRequestComboList.push_back("Reject");
    }
    ~GroupManager()
    {
        clearAll();
    }
    static GroupManager* getSingletonPtr();
    static GroupManager& getSingleton();
    void init()
    {
        mPanel = mGui->addPanel("GroupScreen");
        mPanel->show(false);
        mPanel->setListener(this);

        //My Group
        mNoGroupLabel = OverlayManager::getSingleton().getOverlayElement("GroupScreen/NoGroupLabel");
        mNoGroupLabel->hide();
        mMyGroupList = mPanel->getHighlightList("GroupScreen/Members");
        mMyGroupList->setAllowReclick(true);
        mMyGroupList->show(false);

        //Actions
        mGroupField = mPanel->getTextField("GroupScreen/GroupName");
        mGroupField->mClickable = false;
        mGroupField->mMaxLength = 32;
        mRequestLabel = OverlayManager::getSingleton().getOverlayElement("GroupScreen/RequestLabel");
        mRequestLabel->hide();
        mRequestList = mPanel->getHighlightList("GroupScreen/Request");
        mRequestList->show(false);
        mRequestList->setAllowReclick(true);
        mCreateButton = mPanel->getButton("GroupScreen/GroupCreate");
        mCreateButton->show(false);
        mBioButton = mPanel->getButton("GroupScreen/GroupBio");
        mBioButton->show(false);
        mLeaveButton = mPanel->getButton("GroupScreen/GroupLeave");
        mLeaveButton->show(false);

        //View Groups
        mFilterGroupField = mPanel->getTextField("GroupScreen/FilterGroupName");
        mFilterGroupField->mMaxLength = 32;
        mViewGroupList = mPanel->getHighlightList("GroupScreen/ViewGroups");
        mViewGroupList->setAllowReclick(true);
        mViewGroupList->show(false);
        mSearchButton = mPanel->getButton("GroupScreen/GroupSearch");
        mGroupSearchCache = "";

        //Bio
        mBioPanel = mGui->addPanel("GroupBioScreen");
        mBioPanel->show(false);
        mBioNameField = mBioPanel->getTextComponent("GroupBioScreen/GroupName");
        mBioNameField->mClickable = false;
        mBioArea = mBioPanel->getTextArea("GroupBioScreen/Bio");
        mBioArea->mClickable = false;
        mBioArea->mMaxLength = 1024;
        mBioArea->mAllowNewLine = true;

        //View Group Popup
        mViewGroupPanel = mGui->addPanel("GroupViewScreen");
        mViewGroupPanel->show(false);
        mViewGroupField = mViewGroupPanel->getTextComponent("GroupViewScreen/GroupName");
        mViewGroupField->mClickable = false;
        mViewGroupMembersList = mViewGroupPanel->getHighlightList("GroupViewScreen/Members");
        mViewGroupMembersList->setAllowReclick(true);

        updateGroupPanel();
    }
    void setInfoCaller(InfoCaller *caller)
    {
        mInfoCaller = caller;
    }
    void clear()
    {
        mListener = 0;
        mInfoCaller = 0;
        //mRequestedViewGroups = false;
        mSelectionID = 0;
    }
    void clearAll()
    {
        clear();
        clearMyGroupMembers();
        while(!mInvites.empty())
        {
            GroupInfo *info = mInvites.back();
            mInvites.pop_back();
            delete info;
        }
        while(!mRequests.empty())
        {
            GroupInfo *info = mRequests.back();
            mRequests.pop_back();
            delete info;
        }
        clearViewGroups();
        clearViewGroupMembers();
        mGroupName = "";
        mIsLeader = false;
        mGroupBioCache = "";
    }
    void clearMyGroupMembers()
    {
        while(!mGroupMembers.empty())
        {
            GroupMemberInfo *info = mGroupMembers.back();
            mGroupMembers.pop_back();
            delete info;
        }
    }
    void clearViewGroups(const bool &clearList=false)
    {
        while(!mViewGroups.empty())
        {
            GroupInfo *info = mViewGroups.back();
            mViewGroups.pop_back();
            delete info;
        }
        if(clearList)
        {
            mViewGroupList->clear();
            mViewGroupList->clearSelection();
            mViewGroupList->update(0);
        }
    }
    void clearViewGroupMembers(const bool &clearList=false)
    {
        while(!mViewGroupMembers.empty())
        {
            GroupMemberInfo *info = mViewGroupMembers.back();
            mViewGroupMembers.pop_back();
            delete info;
        }
        if(clearList)
        {
            mViewGroupMembersList->clear();
            mViewGroupMembersList->clearSelection();
            mViewGroupMembersList->update(0);
        }
    }
    GuiPanel* getPanel()
    {
        return mPanel;
    }
    void setListener(GamePanelListener *listener)
    {
        mListener = listener;
        if(!mRequests.empty())mListener->highlightButton(GamePanelListener::GAMEPANEL_GROUP,true);
    }
    void buttonClicked(GuiComponent *button)
    {
        if(button==mCreateButton)
        {
            const String groupname = mGroupField->getCaption();
            if(mGroupName=="")mGui->showAlertBox("Create "+groupname+"?","Group/Create",true);
            else if(mGroupName!=groupname && mIsLeader)mGui->showAlertBox("Rename group to "+groupname+"?","Group/Create",true);
        }
        else if(button==mBioButton)
        {
            if(!mBioPanel->isVisible() || mBioNameField->getCaption()!=mGroupName)
            {
                mBioPanel->show(true);
                mGui->setActivePanel(mBioPanel);
                mBioNameField->setCaption(mGroupName);
                mBioArea->setCaption("");
                if(mGroupName!="")
                {
                    mBioArea->mClickable = mIsLeader;
                    mNetMgr->sendGroupBioRequest(0);
                }
            }
        }
        else if(button==mLeaveButton)mGui->showAlertBox("Leave "+mGroupName+(mIsLeader?"? It will be disbanded.":"?"),"Group/Leave",true);
        else if(button==mSearchButton)searchGroup();
    }
    void changeEvent(GuiComponent *component, const unsigned char &event)
    {
        if(component==mMyGroupList)mGui->showComboBox("Group/MyGroup",mMyGroupComboList);
        else if(component==mRequestList)mGui->showComboBox("Group/Request",mRequestComboList);
        else if(component==mFilterGroupField)
        {
            if(event==ChangeListener::EVENT_KEYPRESSED)searchGroup();
        }
        else if(component==mViewGroupList)mGui->showComboBox("Group/ViewGroup",mViewGroupComboList);
        else if(component==mBioArea)
        {
            if(mGroupName!="" && mIsLeader)
            {
                const string bio = mBioArea->getCaption();
                if(mGroupBioCache!=bio)
                {
                    mGroupBioCache = bio;
                    mNetMgr->sendGroupBioUpdate(bio);
                }
            }
        }
        /*
        else if(component->nameIs("GroupScreen/Page4"))
        {
            if(!mRequestedViewGroups)
            {
                mRequestedViewGroups = true;
                clearViewGroups(true);
                mNetMgr->sendViewGroupRequest(false);
            }
        }
        */
    }
    void comboBoxEvent(const String &name)
    {
        const unsigned short selection = mGui->getComboBoxList()->getSelectedLine();
        if(name=="Group/MyGroup")
        {
            const unsigned short line = mMyGroupList->getSelectedLine();
            GroupMemberInfo *member = 0;
            if(line<(int)mGroupMembers.size())member = mGroupMembers[line];
            if(!member)return;

            //View Info
            if(selection==0)
            {
                if(mInfoCaller)mInfoCaller->viewInfo(0,member->mCharID);
            }
            //Find
            else if(selection==1)mNetMgr->sendFindChar(member->mCharID,false);
            //Kick
            else if(selection==2)
            {
                mSelectionID = member->mCharID;
                mGui->showAlertBox("Kick "+member->mCharName+" from group?","Group/Kick",true);
            }
        }
        else if(name=="Group/Request")
        {
            const unsigned short line = mRequestList->getSelectedLine();
            GroupInfo *info = 0;
            if(mGroupName=="")
            {
                if(line<(int)mInvites.size())info = mInvites[line];
            }
            else if(mIsLeader)
            {
                if(line<(int)mRequests.size())info = mRequests[line];
            }
            else return;
            if(!info)return;

            //Accept
            if(selection==0)
            {
                mSelectionID = info->mGroupID;
                if(mIsLeader)mGui->showAlertBox("Accept "+info->mGroupName+" into "+mGroupName+"?","Group/AcceptRequest",true);
                else mGui->showAlertBox("Join "+info->mGroupName+"?","Group/AcceptRequest",true);
            }
            //Reject
            else if(selection==1)
            {
                mNetMgr->sendAcceptGroupRequest(info->mGroupID,false,mIsLeader);
                if(mIsLeader)
                {
                    mChatMgr->systemMessage("Rejected request from "+info->mGroupName+".",CHAT_GROUP);
                    deleteRequestInfo(info->mGroupID);
                }
                else
                {
                    mChatMgr->systemMessage("Rejected invitation from "+info->mGroupName+".",CHAT_GROUP);
                    deleteInviteInfo(info->mGroupID);
                }
                updateRequestList();
            }
        }
        else if(name=="Group/ViewGroup")
        {
            const String groupname = mViewGroupList->getSelection();
            unsigned int groupID = 0;
            for(std::vector<GroupInfo*>::iterator it=mViewGroups.begin(); it!=mViewGroups.end(); it++)
            {
                GroupInfo *info = *it;
                if(info->mGroupName==groupname)
                {
                    groupID = info->mGroupID;
                    break;
                }
            }
            if(groupID==0)return;
            //View members
            if(selection==0)
            {
                mViewGroupPanel->show(true);
                mGui->setActivePanel(mViewGroupPanel);
                mViewGroupField->setCaption(groupname);
                clearViewGroupMembers(true);
                mNetMgr->sendViewGroupRequest(true,groupID);
            }
            //View bio
            else if(selection==1)
            {
                viewGroupBio(groupID,groupname);
            }
            //Join request
            else if(selection==2)
            {
                mSelectionID = groupID;
                mGui->showAlertBox("Send join request to "+groupname+"?","Group/SendRequest",true);
            }
        }
    }
    void alertBoxEvent(const String &name, const bool &flag)
    {
        if(name=="Group/Create")
        {
            if(flag)
            {
                const String groupname = mGroupField->getCaption();
                if(groupname=="")mGui->showAlertBox("Enter a group name.");
                else mNetMgr->sendCreateGroup(groupname);
            }
            else if(mGroupName!="")mGroupField->setCaption(mGroupName);
        }
        else if(name=="Group/Leave")
        {
            if(flag)mNetMgr->sendLeaveGroup(0,false);
        }
        else if(name=="Group/Kick")
        {
            if(flag && mIsLeader)mNetMgr->sendLeaveGroup(mSelectionID,true);
        }
        else if(name=="Group/SendRequest")
        {
            if(flag && mGroupName=="")mNetMgr->sendGroupRequest(mSelectionID,true);
        }
        else if(name=="Group/AcceptRequest")
        {
            if(flag && (mGroupName==""||mIsLeader))
            {
                mNetMgr->sendAcceptGroupRequest(mSelectionID,true,mIsLeader);
                if(mIsLeader)deleteRequestInfo(mSelectionID);
                else deleteInviteInfo(mSelectionID);
                updateRequestList();
            }
        }
    }
    const bool guiPanelEvent(GuiPanel *panel, const unsigned char &event)
    {
        if(panel==mPanel && event==GuiPanelListener::GUIPANEL_CLOSED)
        {
            //mRequestedViewGroups = false;
            mSelectionID = 0;
        }
        return false;
    }
    const String getGroupMemberNameByCharID(const unsigned int &charID)
    {
        for(std::vector<GroupMemberInfo*>::iterator it=mGroupMembers.begin(); it!=mGroupMembers.end(); it++)
        {
            GroupMemberInfo *info = *it;
            if(info->mCharID==charID)return info->mCharName;
        }
        return "";
    }
    void searchGroup()
    {
        const String filter = mFilterGroupField->getCaption();
        if(filter == mGroupSearchCache)return;
        mGroupSearchCache = filter;
        if(filter.length() >= 3)
        {
            mNetMgr->sendViewGroupRequest(false,0,filter);
            clearViewGroups(true);
            mViewGroupList->show(false);
        }
        else mGui->showAlertBox("Search query is too short!");
    }
    void updateGroupPanel()
    {
        //No group
        if(mGroupName=="")
        {
            mNoGroupLabel->show();
            mMyGroupList->show(false);
            mRequestLabel->setCaption("Invitations:");
            mRequestLabel->show();
            mRequestList->show(true);
            mGroupField->mClickable = true;
            mGroupField->setCaption("");
            mCreateButton->show(true);
            mCreateButton->setCaption("Create");
            mBioButton->show(false);
            mLeaveButton->show(false);
            if(mViewGroupComboList.size()<3)mViewGroupComboList.push_back("Join Request");
            updateRequestList();
        }
        //Has group
        else
        {
            if(mIsLeader)
            {
                mRequestLabel->setCaption("Requests:");
                mRequestLabel->show();
                mRequestList->show(true);
                mGroupField->mClickable = true;
                mCreateButton->show(true);
                mCreateButton->setCaption("Rename");
                if(mMyGroupComboList.size()<3)mMyGroupComboList.push_back("Kick");
            }
            else
            {
                mCreateButton->show(false);
                mRequestLabel->hide();
                mRequestList->show(false);
                if(mMyGroupComboList.size()>=3)mMyGroupComboList.pop_back();
            }
            if(mViewGroupComboList.size()>=3)mViewGroupComboList.pop_back();
            mNoGroupLabel->hide();
            mMyGroupList->show(true);
            mGroupField->setCaption(mGroupName);
            mBioButton->show(true);
            mLeaveButton->show(true);
            updateMyGroupList();
            updateRequestList();
        }
    }
    void updateMyGroupList()
    {
        mMyGroupList->clear();
        for(std::vector<GroupMemberInfo*>::iterator it=mGroupMembers.begin(); it!=mGroupMembers.end(); it++)
        {
            GroupMemberInfo *info = *it;
            mMyGroupList->pushLine((info->mIsOnline?"+ ":"- ") + info->mCharName + (info->mTitle==""?"":("  ["+info->mTitle+"]")) );
        }
        mMyGroupList->clearSelection();
        mMyGroupList->update(0);
    }
    void updateViewGroupsList()
    {
        mViewGroupList->clear();

        for(std::vector<GroupInfo*>::iterator it=mViewGroups.begin(); it!=mViewGroups.end(); it++)
        {
            GroupInfo *info = *it;
            mViewGroupList->pushLine(info->mGroupName);
        }

        mViewGroupList->clearSelection();
        mViewGroupList->update(0);
        mViewGroupList->show(!mViewGroups.empty());
    }
    void updateViewGroupMembersList()
    {
        mViewGroupMembersList->clear();

        for(std::vector<GroupMemberInfo*>::iterator it=mViewGroupMembers.begin(); it!=mViewGroupMembers.end(); it++)
        {
            GroupMemberInfo *info = *it;
            mViewGroupMembersList->pushLine(info->mCharName + (info->mTitle==""?"":("  ["+info->mTitle+"]")) );
        }

        mViewGroupMembersList->clearSelection();
        mViewGroupMembersList->update(0);
    }
    void updateRequestList()
    {
        mRequestList->clear();
        //Requests to join group
        if(mIsLeader)
        {
            for(std::vector<GroupInfo*>::iterator it=mRequests.begin(); it!=mRequests.end(); it++)
            {
                GroupInfo *info = *it;
                mRequestList->pushLine(info->mGroupName);
            }
        }
        //Invitations to join groups
        else if(mGroupName=="")
        {
            for(std::vector<GroupInfo*>::iterator it=mInvites.begin(); it!=mInvites.end(); it++)
            {
                GroupInfo *info = *it;
                mRequestList->pushLine(info->mGroupName);
            }
        }
        mRequestList->clearSelection();
        mRequestList->update(0);
    }
    void deleteRequestInfo(const unsigned int &charID)
    {
        for(std::vector<GroupInfo*>::iterator it=mRequests.begin(); it!=mRequests.end(); it++)
        {
            GroupInfo *info = *it;
            if(info->mGroupID==charID)
            {
                mRequests.erase(it);
                delete info;
                return;
            }
        }
    }
    void deleteInviteInfo(const unsigned int &groupID)
    {
        for(std::vector<GroupInfo*>::iterator it=mInvites.begin(); it!=mInvites.end(); it++)
        {
            GroupInfo *info = *it;
            if(info->mGroupID==groupID)
            {
                mInvites.erase(it);
                delete info;
                return;
            }
        }
    }
    void myGroupInfo(const String &groupname, const bool &isLeader)
    {
        mGroupName = groupname;
        mIsLeader = isLeader;
        updateGroupPanel();
        if(mInfoCaller)mInfoCaller->setOwnGroup(groupname);
    }
    void myGroupMemberInfo(const unsigned int &charID, const String &charname, const unsigned char &rank, const String &title, const bool &isOnline, const bool &announce)
    {
        GroupMemberInfo *member = new GroupMemberInfo(charID,charname);
        member->mRank = rank;
        member->mTitle = title;
        member->mIsOnline = isOnline;
        //Insert in sorted order by rank
        bool inserted = false;
        for(std::vector<GroupMemberInfo*>::iterator it=mGroupMembers.begin(); it!=mGroupMembers.end(); it++)
        {
            GroupMemberInfo *info = *it;
            if(rank<=info->mRank)
            {
                inserted = true;
                mGroupMembers.insert(it,member);
                break;
            }
        }
        if(!inserted)mGroupMembers.push_back(member);

        if(mMyGroupList->isVisible())updateMyGroupList();
        if(announce)mChatMgr->systemMessage(charname+" has joined "+mGroupName+".",CHAT_GROUP);
    }
    void myGroupEvent(const bool &isCreate, const bool &success)
    {
        if(success)mChatMgr->systemMessage(isCreate?"Group created successfully.":"Group renamed successfully.",CHAT_GROUP);
        else
        {
            mChatMgr->systemMessage("Group name has been taken.",CHAT_GROUP);
            if(!isCreate)mGroupField->setCaption(mGroupName);
        }
    }
    void myGroupLeaveEvent(const unsigned int &charID, const bool &isKick, const bool &isDisband)
    {
        if(charID==0)
        {
            mChatMgr->systemMessage(isKick?"You have been kicked from the group.":"You have left the group.",CHAT_GROUP);
            myGroupInfo("",false);
            clearMyGroupMembers();
            if(mMyGroupList->isVisible())updateMyGroupList();
        }
        else
        {
            const String charname = getGroupMemberNameByCharID(charID);
            if(charname!="")mChatMgr->systemMessage(charname+(isKick?" has been kicked from the group.":" has left the group."),CHAT_GROUP);
            for(std::vector<GroupMemberInfo*>::iterator it=mGroupMembers.begin(); it!=mGroupMembers.end(); it++)
            {
                GroupMemberInfo *info = *it;
                if(info->mCharID==charID)
                {
                    mGroupMembers.erase(it);
                    break;
                }
            }
            updateMyGroupList();
        }
        if(isDisband)
        {
            mChatMgr->systemMessage("The group has been disbanded.",CHAT_GROUP);
            if(charID!=0)
            {
                myGroupInfo("",false);
                clearMyGroupMembers();
                if(mMyGroupList->isVisible())updateMyGroupList();
            }
        }
    }
    void viewGroupInfo(const unsigned int &groupID, const String &groupname)
    {
        GroupInfo *info = new GroupInfo(groupID,groupname);
        mViewGroups.push_back(info);
        updateViewGroupsList();
    }
    void viewGroupMembersInfo(const unsigned int &charID, const String &charname, const unsigned char &rank, const String &title)
    {
        GroupMemberInfo *member = new GroupMemberInfo(charID,charname);
        member->mRank = rank;
        member->mTitle = title;
        //Insert in sorted order by rank
        bool inserted = false;
        for(std::vector<GroupMemberInfo*>::iterator it=mViewGroupMembers.begin(); it!=mViewGroupMembers.end(); it++)
        {
            GroupMemberInfo *info = *it;
            if(rank<=info->mRank)
            {
                inserted = true;
                mViewGroupMembers.insert(it,member);
                break;
            }
        }
        if(!inserted)mViewGroupMembers.push_back(member);

        if(mViewGroupMembersList->isVisible())updateViewGroupMembersList();
    }
    void groupRequestInfo(const unsigned int &charID, const String &charname, const bool &isRequest)
    {
        GroupInfo *info = new GroupInfo(charID,charname);
        if(isRequest)mRequests.push_back(info);
        else mInvites.push_back(info);

        if(mRequestList->isVisible())updateRequestList();

        if(mListener && !mPanel->isVisible())mListener->highlightButton(GamePanelListener::GAMEPANEL_GROUP,true,true);
    }
    void groupBio(const String &bio)
    {
        mBioArea->setCaption(bio);
        if(mBioNameField->getName()==mGroupName)mGroupBioCache = bio;
    }
    void charOnlineStatusEvent(const unsigned int &userID, const unsigned int &charID, const bool &isOnline)
    {
        if(mInfoCaller && mInfoCaller->getOwnCharID()==charID)return;
        for(std::vector<GroupMemberInfo*>::iterator it=mGroupMembers.begin(); it!=mGroupMembers.end(); it++)
        {
            GroupMemberInfo *info = *it;
            if(info->mCharID==charID)
            {
                info->mIsOnline = isOnline;
                mChatMgr->systemMessage(info->mCharName + (isOnline?" is online.":" is offline."),CHAT_GROUP);
                updateMyGroupList();
                break;
            }
        }
    }
    void viewGroupBio(const unsigned int &groupID, const String &groupname)
    {
        mBioPanel->show(true);
        mGui->setActivePanel(mBioPanel);
        mBioNameField->setCaption(groupname);
        mBioArea->mClickable = false;
        mBioArea->setCaption("");
        mNetMgr->sendGroupBioRequest(groupID);
    }
    const bool isLeader()
    {
        return (mIsLeader && mGroupName!="");
    }
    /*
    const bool match(String caption, String pattern)
    {
        if(caption.length()<=0 || pattern.length()<=0)return false;
        StringUtil::toLowerCase(caption);
        StringUtil::toLowerCase(pattern);

        String::const_iterator pit = pattern.begin();
        for(String::const_iterator it=caption.begin(); it!=caption.end(); it++)
        {
            const char c = *it;
            const char p = *pit;
            if(c==p)
            {
                pit++;
                if(pit==pattern.end())return true;
            }
            else pit = pattern.begin();
        }

        return false;
    }
    */
};

template<> GroupManager* Singleton<GroupManager>::ms_Singleton = 0;

GroupManager* GroupManager::getSingletonPtr()
{
	return ms_Singleton;
}

GroupManager& GroupManager::getSingleton()
{
	assert(ms_Singleton);
	return *ms_Singleton;
}

#endif
