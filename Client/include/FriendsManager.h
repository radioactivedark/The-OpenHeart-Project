#ifndef _FRIENDSMANAGER_H_
#define _FRIENDSMANAGER_H_

#include <GamePanelListener.h>
#include <ChatManager.h>

struct FriendInfo
{
    unsigned int mUserID;
    String mUsername;
    bool mIsOnline;
    FriendInfo(const unsigned int &userID, const String &username, const bool &isOnline=false)
    {
        mUserID = userID;
        mUsername = username;
        mIsOnline = isOnline;
    }
};

struct BlockInfo
{
    unsigned int mUserID;
    String mUsername;
    BlockInfo(const unsigned int &userID, const String &username)
    {
        mUserID = userID;
        mUsername = username;
    }
};

class FriendsManager : public Singleton<FriendsManager>, public NetworkListener
{
private:
    Gui *mGui;
    ChatManager *mChatMgr;
    UnitManager *mUnitMgr;
    NetworkManager *mNetMgr;
    std::vector<FriendInfo*> mFriends;
    std::vector<BlockInfo*> mRequests;
    std::vector<BlockInfo*> mBlocks;
    std::vector<unsigned int> mBlockedBy;
    GuiPanel *mPanel;
    GuiHighlightList *mFriendOnlineList;
    GuiHighlightList *mFriendOfflineList;
    GuiHighlightList *mFriendRequestList;
    GuiHighlightList *mBlockList;
    GuiTextField *mFriendField;
    GuiTextField *mBlockField;
    GuiButton *mRequestYesButton;
    GuiButton *mRequestNoButton;
    GuiTab *mRequestTab;
    std::vector<String> mFriendOnlineComboList;
    std::vector<String> mFriendOfflineComboList;
    std::vector<String> mBlockComboList;
    String mRemoveName;

    GamePanelListener *mListener;
public:
    FriendsManager()
    {
        mGui = Gui::getSingletonPtr();
        mChatMgr = ChatManager::getSingletonPtr();
        mUnitMgr = UnitManager::getSingletonPtr();
        mNetMgr = NetworkManager::getSingletonPtr();
        clearAll();

        mFriendOnlineComboList.push_back("Whisper");
        mFriendOnlineComboList.push_back("Find");
        mFriendOnlineComboList.push_back("Remove");

        mFriendOfflineComboList.push_back("Remove");

        mBlockComboList.push_back("Remove");
    }
    ~FriendsManager()
    {
        clearAll();
    }
    static FriendsManager* getSingletonPtr();
    static FriendsManager& getSingleton();
    void init()
    {
        mPanel = mGui->addPanel("FriendsScreen");
        mPanel->show(false);

        mFriendOnlineList = mPanel->getHighlightList("FriendsScreen/FriendOnline");
        mFriendOnlineList->setAllowReclick(true);
        mFriendOnlineList->update(0);
        mFriendOfflineList = mPanel->getHighlightList("FriendsScreen/FriendOffline");
        mFriendOfflineList->setAllowReclick(true);
        for(std::vector<FriendInfo*>::iterator it=mFriends.begin(); it!=mFriends.end(); it++)
        {
            FriendInfo *info = *it;
            if(info->mIsOnline)mFriendOnlineList->pushLine(info->mUsername);
            else mFriendOfflineList->pushLine(info->mUsername);
        }
        mFriendOnlineList->update(0);
        mFriendOfflineList->update(0);
        mFriendRequestList = mPanel->getHighlightList("FriendsScreen/FriendRequest");
        mFriendRequestList->setAllowReclick(true);
        for(std::vector<BlockInfo*>::iterator it=mRequests.begin(); it!=mRequests.end(); it++)
        {
            BlockInfo *info = *it;
            mFriendRequestList->pushLine(info->mUsername);
        }
        mFriendRequestList->update(0);
        mBlockList = mPanel->getHighlightList("FriendsScreen/Block");
        mBlockList->setAllowReclick(true);
        for(std::vector<BlockInfo*>::iterator it=mBlocks.begin(); it!=mBlocks.end(); it++)
        {
            BlockInfo *info = *it;
            mBlockList->pushLine(info->mUsername);
        }
        mBlockList->update(0);

        mFriendField = mPanel->getTextField("FriendsScreen/FriendName");
        mFriendField->mMaxLength = 32;
        mBlockField = mPanel->getTextField("FriendsScreen/BlockName");
        mBlockField->mMaxLength = 32;
        mRequestYesButton = mPanel->getButton("FriendsScreen/RequestYes");
        mRequestNoButton = mPanel->getButton("FriendsScreen/RequestNo");
        mRequestTab = mPanel->getTab("FriendsScreen/Page2");
        updateRequestPanel();
    }
    void clear()
    {
        mRemoveName = "";
        mListener = 0;
    }
    void clearAll()
    {
        clear();
        while(!mFriends.empty())
        {
            FriendInfo *info = mFriends.back();
            mFriends.pop_back();
            delete info;
        }
        while(!mRequests.empty())
        {
            BlockInfo *info = mRequests.back();
            mRequests.pop_back();
            delete info;
        }
        while(!mBlocks.empty())
        {
            BlockInfo *info = mBlocks.back();
            mBlocks.pop_back();
            delete info;
        }
        mBlockedBy.clear();
    }
    void setListener(GamePanelListener *listener)
    {
        mListener = listener;
        if(!mRequests.empty())mListener->highlightButton(GamePanelListener::GAMEPANEL_FRIENDS,true);
    }
    void updateRequestPanel()
    {
        //Show/hide request buttons
        const String selectedName = mFriendRequestList->getSelection();
        const bool hasSelection = (selectedName!="");
        mRequestYesButton->show(hasSelection);
        mRequestNoButton->show(hasSelection);

        //Update request tab
        /*const unsigned short numRequests = (unsigned short)mRequests.size();
        if(numRequests>0)mRequestTab->setCaption("Requests ("+StringConverter::toString(numRequests)+")");
        else mRequestTab->setCaption("Requests");*/
    }
    GuiPanel* getPanel()
    {
        return mPanel;
    }
    void buttonClicked(GuiComponent *button)
    {
        if(button==mRequestYesButton || button==mRequestNoButton)
        {
            const String selectedName = mFriendRequestList->getSelection();
            if(selectedName!="")
            {
                for(std::vector<BlockInfo*>::iterator it=mRequests.begin(); it!=mRequests.end(); it++)
                {
                    BlockInfo *info = *it;
                    if(info->mUsername==selectedName)
                    {
                        //Send acceptance
                        if(button==mRequestYesButton)mChatMgr->systemMessage("Accepted "+selectedName+" as Friend.");
                        mNetMgr->sendAcceptFriend(info->mUserID,button==mRequestYesButton);
                        mFriendRequestList->eraseLine(selectedName);
                        mFriendRequestList->clearSelection();
                        mFriendRequestList->update(0);
                        mRequests.erase(it);
                        delete info;
                        break;
                    }
                }
            }
            updateRequestPanel();
        }
        else if(button->nameIs("FriendsScreen/FriendAdd"))
        {
            const String name = mFriendField->getCaption();
            if(name!="")
            {
                mNetMgr->sendAddFriend(name,false);
                mFriendField->setCaption("");
            }
        }
        else if(button->nameIs("FriendsScreen/BlockAdd"))
        {
            const String name = mBlockField->getCaption();
            if(name!="")
            {
                mNetMgr->sendAddFriend(name,true);
                mBlockField->setCaption("");
            }
        }
    }
    void changeEvent(GuiComponent *component, const unsigned char &event)
    {
        if(component==mFriendRequestList)updateRequestPanel();
        else if(component==mFriendOnlineList)mGui->showComboBox("Friends/Online",mFriendOnlineComboList);
        else if(component==mFriendOfflineList)mGui->showComboBox("Friends/Offline",mFriendOfflineComboList);
        else if(component==mBlockList)mGui->showComboBox("Friends/Block",mBlockComboList);
    }
    void comboBoxEvent(const String &name)
    {
        const unsigned short selection = mGui->getComboBoxList()->getSelectedLine();
        if(name=="Friends/Online")
        {
            //Whisper
            if(selection==0)
            {
                mChatMgr->setChannel(CHAT_WHISPER);
                mChatMgr->setWhisperTarget(mFriendOnlineList->getSelection());
            }
            //Find
            else if(selection==1)
            {
                const String username = mFriendOnlineList->getSelection();
                for(std::vector<FriendInfo*>::iterator it=mFriends.begin(); it!=mFriends.end(); it++)
                {
                    FriendInfo *info = *it;
                    if(info->mUsername==username)
                    {
                        mNetMgr->sendFindChar(info->mUserID,true);
                        break;
                    }
                }
            }
            //Remove
            else if(selection==2)
            {
                mRemoveName = mFriendOnlineList->getSelection();
                mGui->showAlertBox("Remove "+mRemoveName+" from Friends list?","Friends/FriendRemove",true);
            }
        }
        else if(name=="Friends/Offline")
        {
            if(selection==0)
            {
                mRemoveName = mFriendOfflineList->getSelection();
                mGui->showAlertBox("Remove "+mRemoveName+" from Friends list?","Friends/FriendRemove",true);
            }
        }
        else if(name=="Friends/Block")
        {
            if(selection==0)
            {
                mRemoveName = mBlockList->getSelection();
                mGui->showAlertBox("Remove "+mRemoveName+" from Block list?","Friends/BlockRemove",true);
            }
        }
    }
    void alertBoxEvent(const String &name, const bool &flag)
    {
        if(name=="Friends/FriendRemove" || name=="Friends/BlockRemove")
        {
            if(flag)
            {
                const bool isBlock = (name=="Friends/BlockRemove");
                const unsigned int userID = isBlock ? getBlockUserIDByUsername(mRemoveName) : getFriendUserIDByUsername(mRemoveName);
                if(userID!=0)
                {
                    mNetMgr->sendRemoveFriend(userID,isBlock);
                    if(isBlock)blockRemove(userID);
                    else friendRemove(userID);
                }
            }
        }
    }
    void addFriendEvent(const String &username, const bool &isBlock, const bool &success)
    {
        if(success)
        {
            if(isBlock)mChatMgr->systemMessage("Blocked "+username+".");
            else mChatMgr->systemMessage("Sent Friend request to "+username+".");
        }
        else mChatMgr->systemMessage("No user by the name "+username+".");
    }
    void friendListInfo(const unsigned int &userID, const String &username, const bool &isOnline)
    {
        FriendInfo *info = new FriendInfo(userID,username,isOnline);
        mFriends.push_back(info);
        if(isOnline)
        {
            mFriendOnlineList->pushLine(username);
            mFriendOnlineList->update(0);
            mFriendOnlineList->clearSelection();
        }
        else
        {
            mFriendOfflineList->pushLine(username);
            mFriendOfflineList->update(0);
            mFriendOfflineList->clearSelection();
        }
    }
    void requestListInfo(const unsigned int &userID, const String &username)
    {
        BlockInfo *info = new BlockInfo(userID,username);
        mRequests.push_back(info);
        mFriendRequestList->pushLine(username);
        mFriendRequestList->clearSelection();
        mFriendRequestList->update(0);
        mFriendRequestList->clearSelection();
        updateRequestPanel();

        if(mListener && !mPanel->isVisible())mListener->highlightButton(GamePanelListener::GAMEPANEL_FRIENDS,true,true);
    }
    void blockListInfo(const unsigned int &userID, const String &username, const bool &isBlockedBy)
    {
        if(isBlockedBy)
        {
            mBlockedBy.push_back(userID);
        }
        else
        {
            BlockInfo *info = new BlockInfo(userID,username);
            mBlocks.push_back(info);
            mBlockList->pushLine(username);
            mBlockList->update(0);
            mBlockList->clearSelection();
        }
        Unit *unit = mUnitMgr->getUnitByUserID(userID);
        if(unit)unit->setInvisible(true);
    }
    void charOnlineStatusEvent(const unsigned int &userID, const unsigned int &charID, const bool &isOnline)
    {
        friendListUpdate(userID,isOnline);

    }
    void friendListUpdate(const unsigned int &userID, const bool &isOnline)
    {
        for(std::vector<FriendInfo*>::iterator it=mFriends.begin(); it!=mFriends.end(); it++)
        {
            FriendInfo *info = *it;
            if(info->mUserID==userID)
            {
                if(info->mIsOnline==isOnline)return;
                info->mIsOnline = isOnline;
                if(isOnline)
                {
                    mFriendOfflineList->eraseLine(info->mUsername);
                    mFriendOnlineList->pushLine(info->mUsername);
                }
                else
                {
                    mFriendOnlineList->eraseLine(info->mUsername);
                    mFriendOfflineList->pushLine(info->mUsername);
                }
                mFriendOnlineList->update(0);
                mFriendOnlineList->clearSelection();
                mFriendOfflineList->update(0);
                mFriendOfflineList->clearSelection();
                return;
            }
        }
    }
    void friendRemove(const unsigned int &userID)
    {
        for(std::vector<FriendInfo*>::iterator it=mFriends.begin(); it!=mFriends.end(); it++)
        {
            FriendInfo *info = *it;
            if(info->mUserID==userID)
            {
                if(info->mIsOnline)
                {
                    mFriendOnlineList->eraseLine(info->mUsername);
                    mFriendOnlineList->update(0);
                    mFriendOnlineList->clearSelection();
                }
                else
                {
                    mFriendOfflineList->eraseLine(info->mUsername);
                    mFriendOfflineList->update(0);
                    mFriendOfflineList->clearSelection();
                }
                mFriends.erase(it);
                delete info;
                return;
            }
        }
    }
    void blockRemove(const unsigned int &userID)
    {
        Unit *unit = mUnitMgr->getUnitByUserID(userID);
        if(unit)unit->setInvisible(false);
        for(std::vector<BlockInfo*>::iterator it=mBlocks.begin(); it!=mBlocks.end(); it++)
        {
            BlockInfo *info = *it;
            if(info->mUserID==userID)
            {
                mBlockList->eraseLine(info->mUsername);
                mBlockList->update(0);
                mBlockList->clearSelection();
                mBlocks.erase(it);
                delete info;
                return;
            }
        }
    }
    void blockedByRemove(const unsigned int &userID)
    {
        Unit *unit = mUnitMgr->getUnitByUserID(userID);
        if(unit)unit->setInvisible(false);
        for(std::vector<unsigned int>::iterator it=mBlockedBy.begin(); it!=mBlockedBy.end(); it++)
        {
            const unsigned int id = *it;
            if(id==userID)
            {
                mBlockedBy.erase(it);
                return;
            }
        }
    }
    const bool isBlocked(const unsigned int &userID)
    {
        for(std::vector<unsigned int>::iterator it=mBlockedBy.begin(); it!=mBlockedBy.end(); it++)
        {
            const unsigned int id = *it;
            if(id==userID)return true;
        }
        for(std::vector<BlockInfo*>::iterator it=mBlocks.begin(); it!=mBlocks.end(); it++)
        {
            BlockInfo *info = *it;
            if(info->mUserID==userID)return true;
        }
        return false;
    }
    const bool isFriend(const unsigned int &userID)
    {
        for(std::vector<FriendInfo*>::iterator it=mFriends.begin(); it!=mFriends.end(); it++)
        {
            FriendInfo *info = *it;
            if(info->mUserID==userID)return true;
        }
        return false;
    }
    const unsigned int getFriendUserIDByUsername(const String &username)
    {
        for(std::vector<FriendInfo*>::iterator it=mFriends.begin(); it!=mFriends.end(); it++)
        {
            FriendInfo *info = *it;
            if(info->mUsername==username)return info->mUserID;
        }
        return 0;
    }
    const unsigned int getBlockUserIDByUsername(const String &username)
    {
        for(std::vector<BlockInfo*>::iterator it=mBlocks.begin(); it!=mBlocks.end(); it++)
        {
            BlockInfo *info = *it;
            if(info->mUsername==username)return info->mUserID;
        }
        return 0;
    }
};

template<> FriendsManager* Singleton<FriendsManager>::ms_Singleton = 0;

FriendsManager* FriendsManager::getSingletonPtr()
{
	return ms_Singleton;
}

FriendsManager& FriendsManager::getSingleton()
{
	assert(ms_Singleton);
	return *ms_Singleton;
}

#endif
