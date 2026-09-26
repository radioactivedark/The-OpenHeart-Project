#ifndef _PARTYMANAGER_H_
#define _PARTYMANAGER_H_

#include <InfoCaller.h>
#include <GamePanelListener.h>

struct PartyMemberInfo
{
    unsigned int mCharID;
    String mCharName;
    PartyMemberInfo(const unsigned int &charID, const String &charname)
    {
        mCharID = charID;
        mCharName = charname;
    }
};

class PartyManager : public Singleton<PartyManager>, public NetworkListener
{
private:
    Gui *mGui;
    NetworkManager *mNetMgr;
    ChatManager *mChatMgr;
    InfoCaller *mInfoCaller;
    GuiPanel *mPanel;
    GuiHighlightList *mPartyList;
    OverlayElement *mNoPartyLabel;

    OverlayElement *mRequestLabel;
    GuiHighlightList *mRequestList;

    GuiButton *mLeaveButton;
    GuiButton *mIgnoreAllButton;
    unsigned int mSelectionID;

    std::vector<PartyMemberInfo*> mPartyMembers;
    std::vector<PartyMemberInfo*> mRequests;

    std::vector<String> mPartyComboList;
    std::vector<String> mRequestComboList;

    GamePanelListener *mListener;
public:
    PartyManager()
    {
        mGui = Gui::getSingletonPtr();
        mNetMgr = NetworkManager::getSingletonPtr();
        mChatMgr = ChatManager::getSingletonPtr();
        mInfoCaller = 0;
        clearAll();

        mPartyComboList.push_back("View Info");
        mPartyComboList.push_back("Find");

        mRequestComboList.push_back("Accept");
        mRequestComboList.push_back("Reject");
    }
    ~PartyManager()
    {
        clearAll();
    }
    static PartyManager* getSingletonPtr();
    static PartyManager& getSingleton();
    void init()
    {
        mPanel = mGui->addPanel("PartyScreen");
        mPanel->show(false);

        //My Party
        mNoPartyLabel = OverlayManager::getSingleton().getOverlayElement("PartyScreen/NoPartyLabel");
        mNoPartyLabel->show();
        mPartyList = mPanel->getHighlightList("PartyScreen/Party");
        mPartyList->setAllowReclick(true);
        mPartyList->show(false);
        mLeaveButton = mPanel->getButton("PartyScreen/PartyLeave");
        mLeaveButton->show(false);

        //Party requests
        mRequestList = mPanel->getHighlightList("PartyScreen/Request");
        mRequestList->setAllowReclick(true);
        mIgnoreAllButton = mPanel->getButton("PartyScreen/IgnoreAll");
        mIgnoreAllButton->show(false);

        //updatePartyPanel();
    }
    void setInfoCaller(InfoCaller *caller)
    {
        mInfoCaller = caller;
    }
    void clear()
    {
        mListener = 0;
        mInfoCaller = 0;
        mSelectionID = 0;
    }
    void clearAll()
    {
        clear();
        clearPartyMembers();
        clearRequests();
    }
    void clearPartyMembers()
    {
        while(!mPartyMembers.empty())
        {
            PartyMemberInfo *info = mPartyMembers.back();
            mPartyMembers.pop_back();
            delete info;
        }
    }
    void clearRequests()
    {
        while(!mRequests.empty())
        {
            PartyMemberInfo *info = mRequests.back();
            mRequests.pop_back();
            delete info;
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
        if(button==mLeaveButton)mGui->showAlertBox("Leave party?","Party/Leave",true);
        else if(button==mIgnoreAllButton)
        {
            clearRequests();
            updateRequestList();
        }
    }
    void changeEvent(GuiComponent *component, const unsigned char &event)
    {
        if(component==mPartyList)mGui->showComboBox("Party/MyParty",mPartyComboList);
        else if(component==mRequestList)mGui->showComboBox("Party/Request",mRequestComboList);
    }
    void comboBoxEvent(const String &name)
    {
        const unsigned short selection = mGui->getComboBoxList()->getSelectedLine();
        if(name=="Party/MyParty")
        {
            const unsigned short line = mPartyList->getSelectedLine();
            PartyMemberInfo *member = 0;
            if(line<(int)mPartyMembers.size())member = mPartyMembers[line];
            if(!member)return;

            //View Info
            if(selection==0)
            {
                if(mInfoCaller)mInfoCaller->viewInfo(0,member->mCharID);
            }
            //Find
            else if(selection==1)mNetMgr->sendFindChar(member->mCharID,false);
        }
        else if(name=="Party/Request")
        {
            const unsigned short line = mRequestList->getSelectedLine();
            PartyMemberInfo *info = 0;
            if(line<(int)mRequests.size())info = mRequests[line];
            if(!info)return;

            //Accept
            if(selection==0)
            {
                mSelectionID = info->mCharID;
                if(hasParty())mGui->showAlertBox("Accept "+info->mCharName+" into party?","Party/AcceptRequest",true);
                else mGui->showAlertBox("Join "+info->mCharName+"'s party?","Party/AcceptRequest",true);
            }
            //Reject
            else if(selection==1)
            {
                mChatMgr->systemMessage("Rejected party request from "+info->mCharName+".",CHAT_PARTY);
                deleteRequestInfo(info->mCharID);
                updateRequestList();
            }
        }
    }
    void alertBoxEvent(const String &name, const bool &flag)
    {
        if(name=="Party/Leave")
        {
            if(flag && hasParty())mNetMgr->sendLeaveParty();
        }
        else if(name=="Party/AcceptRequest")
        {
            if(flag)
            {
                mNetMgr->sendPartyRequest(mSelectionID,true);
                deleteRequestInfo(mSelectionID);
                updateRequestList();
            }
        }
    }
    const String getPartyMemberNameByCharID(const unsigned int &charID)
    {
        for(std::vector<PartyMemberInfo*>::iterator it=mPartyMembers.begin(); it!=mPartyMembers.end(); it++)
        {
            PartyMemberInfo *info = *it;
            if(info->mCharID==charID)return info->mCharName;
        }
        return "";
    }
    const bool hasPartyMember(const unsigned int &charID)
    {
        for(std::vector<PartyMemberInfo*>::iterator it=mPartyMembers.begin(); it!=mPartyMembers.end(); it++)
        {
            PartyMemberInfo *info = *it;
            if(info->mCharID==charID)return true;
        }
        return false;
    }
    const bool hasPartyRequest(const unsigned int &charID)
    {
        for(std::vector<PartyMemberInfo*>::iterator it=mRequests.begin(); it!=mRequests.end(); it++)
        {
            PartyMemberInfo *info = *it;
            if(info->mCharID==charID)return true;
        }
        return false;
    }
    void updatePartyPanel()
    {
        //No party
        if(!hasParty())
        {
            mNoPartyLabel->show();
            mPartyList->show(false);
            mLeaveButton->show(false);
            updateRequestList();
        }
        //Has party
        else
        {
            mNoPartyLabel->hide();
            mPartyList->show(true);
            mLeaveButton->show(true);
            updatePartyList();
            updateRequestList();
        }
    }
    void updateRequestList()
    {
        mRequestList->clear();
        for(std::vector<PartyMemberInfo*>::iterator it=mRequests.begin(); it!=mRequests.end(); it++)
        {
            PartyMemberInfo *info = *it;
            mRequestList->pushLine(info->mCharName);
        }
        mRequestList->clearSelection();
        mRequestList->update(0);
        mIgnoreAllButton->show(!mRequests.empty());
    }
    void deleteRequestInfo(const unsigned int &charID)
    {
        for(std::vector<PartyMemberInfo*>::iterator it=mRequests.begin(); it!=mRequests.end(); it++)
        {
            PartyMemberInfo *info = *it;
            if(info->mCharID==charID)
            {
                mRequests.erase(it);
                delete info;
                return;
            }
        }
    }
    void updatePartyList()
    {
        mPartyList->clear();
        for(std::vector<PartyMemberInfo*>::iterator it=mPartyMembers.begin(); it!=mPartyMembers.end(); it++)
        {
            PartyMemberInfo *info = *it;
            mPartyList->pushLine(info->mCharName);
        }
        mPartyList->clearSelection();
        mPartyList->update(0);
    }
    const bool hasParty()
    {
        return (!mPartyMembers.empty());
    }
    void charOnlineStatusEvent(const unsigned int &userID, const unsigned int &charID, const bool &isOnline)
    {
        if(isOnline || (mInfoCaller && mInfoCaller->getOwnCharID()==charID))return;
        for(std::vector<PartyMemberInfo*>::iterator it=mPartyMembers.begin(); it!=mPartyMembers.end(); it++)
        {
            PartyMemberInfo *info = *it;
            if(info->mCharID==charID)
            {
                mChatMgr->systemMessage(info->mCharName + " has went offline.",CHAT_PARTY);
                mPartyMembers.erase(it);
                if(!checkPartyDisband() && mPartyList->isVisible())updatePartyList();
                break;
            }
        }
    }
    void partyMemberInfo(const unsigned int &charID, const String &charname, const bool &announce)
    {
        if(hasPartyMember(charID))return;

        //Announce party joinage if previously had no party
        bool newParty = false;
        if(!hasParty())
        {
            mChatMgr->systemMessage("You have joined the party.",CHAT_PARTY);
            newParty = true;
        }

        PartyMemberInfo *member = new PartyMemberInfo(charID,charname);
        mPartyMembers.push_back(member);

        if(newParty)
        {
            updatePartyPanel();
        }
        else if(mPartyList->isVisible())updatePartyList();
        if(announce)mChatMgr->systemMessage(charname+" has joined the party.",CHAT_PARTY);
    }
    void partyLeaveEvent(const unsigned int &charID)
    {
        if(charID==mInfoCaller->getOwnCharID())
        {
            mChatMgr->systemMessage("You have left the party.",CHAT_PARTY);
            clearPartyMembers();
            updatePartyPanel();
            return;
        }

        for(std::vector<PartyMemberInfo*>::iterator it=mPartyMembers.begin(); it!=mPartyMembers.end(); it++)
        {
            PartyMemberInfo *info = *it;
            if(info->mCharID==charID)
            {
                mChatMgr->systemMessage(info->mCharName+" has left the party.",CHAT_PARTY);
                mPartyMembers.erase(it);
                break;
            }
        }

        if(!checkPartyDisband() && mPartyList->isVisible())updatePartyList();
    }
    const bool checkPartyDisband()
    {
        if(mPartyMembers.empty())
        {
            mChatMgr->systemMessage("The party has been disbanded.",CHAT_PARTY);
            updatePartyPanel();
            return true;
        }
        return false;
    }
    void partyRequestEvent(const unsigned int &charID, const String &charname, const bool &isFail)
    {
        if(isFail)
        {
            mChatMgr->systemMessage(charname+" is already in a party.",CHAT_PARTY);
            return;
        }
        if(hasPartyRequest(charID) || hasPartyMember(charID))return;
        //Add to request list
        PartyMemberInfo *member = new PartyMemberInfo(charID,charname);
        mRequests.push_back(member);
        if(mRequestList->isVisible())updateRequestList();
        if(mListener && !mPanel->isVisible())mListener->highlightButton(GamePanelListener::GAMEPANEL_PARTY,true,true);
    }
};

template<> PartyManager* Singleton<PartyManager>::ms_Singleton = 0;

PartyManager* PartyManager::getSingletonPtr()
{
	return ms_Singleton;
}

PartyManager& PartyManager::getSingleton()
{
	assert(ms_Singleton);
	return *ms_Singleton;
}

#endif
