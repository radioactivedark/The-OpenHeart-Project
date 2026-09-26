#ifndef _ACTIONSMANAGER_H_
#define _ACTIONSMANAGER_H_

#define MAX_EMOTE_HOTKEYS 10

#include <Gui.h>
#include <Unit.h>
#include <NetworkManager.h>
#include <ChatManager.h>
#include <SkillsManager.h>

class ActionsManager : public Singleton<ActionsManager>, public ObjectListener, public ChatListener, public SkillsListener
{
private:
    Gui *mGui;
    EmoteManager *mEmoteMgr;
    NetworkManager *mNetMgr;
    Unit *mPlayer;

    GuiPanel *mPanel;
    GuiHighlightList *mActionList;
    GuiHighlightList *mEmoteList;
    GuiCheckBox *mWalkCheckBox;
    GuiCheckBox *mCrouchCheckBox;
    GuiCheckBox *mLipSyncCheckBox;

    unsigned char mEmoteHotkey[MAX_EMOTE_HOTKEYS];
    unsigned char mHotkeyHeld;
    bool mShiftDown;
    bool mIsMovingReverse;
    bool mDoLipSync;
    bool m_bPlayerDisabled;

    unsigned char *m_pPlayerDataEmote;
public:
    ActionsManager()
    {
        loadEmoteHotkeys();
        reset();
    }
    ~ActionsManager()
    {
        saveEmoteHotkeys();
    }
    static ActionsManager* getSingletonPtr();
    static ActionsManager& getSingleton();
    void init(Unit *player)
    {
        mPlayer = player;
        if(mPlayer)mPlayer->addListener(this);

        mGui = Gui::getSingletonPtr();
        mEmoteMgr = EmoteManager::getSingletonPtr();
        mNetMgr = NetworkManager::getSingletonPtr();

        mPanel = mGui->addPanel("ActionScreen");
        mPanel->show(false);

        mActionList = mPanel->getHighlightList("ActionScreen/Action");
        mActionList->pushLine("Idle");
        mActionList->pushLine("Sit");
        mActionList->pushLine("Sit");
        mActionList->pushLine("Headbang");
        mActionList->pushLine("Headswing");
        mActionList->pushLine("Buttswing");
        mActionList->pushLine("Stretch");
        mActionList->pushLine((mPlayer&&mPlayer->getCallType()==Unit::CALL_HOWL) ? "Howl" : "Roar");
        mActionList->setSelection(0);
        mActionList->update(0);
        mActionList->setAllowReclick(true);
        mWalkCheckBox = mPanel->getCheckBox("ActionScreen/Walk");
        mCrouchCheckBox = mPanel->getCheckBox("ActionScreen/Crouch");
        mLipSyncCheckBox = mPanel->getCheckBox("ActionScreen/LipSync");
        mLipSyncCheckBox->setChecked(mDoLipSync);

        mEmoteList = mPanel->getHighlightList("ActionScreen/Emote");
        mEmoteMgr->fillList(mEmoteList);
        if(mPlayer)mEmoteList->changeLine(mPlayer->getDefaultEmote()," (Default)",true);
        for(int i=0;i<MAX_EMOTE_HOTKEYS;i++)setHotkeyAsEmote(i);
        mEmoteList->setSelection(0);
        mEmoteList->update(0);
    }
    void reset()
    {
        mPlayer = 0;
        mHotkeyHeld = 0;
        mShiftDown = false;
        mIsMovingReverse = false;
        m_bPlayerDisabled = false;
        m_pPlayerDataEmote = 0;
    }
    void setPlayer(Unit *player)
    {
        if(mPlayer)mPlayer->removeListener(this);
        mPlayer = player;
        if(mPlayer)
        {
            mPlayer->addListener(this);
            mEmoteList->changeLine(mPlayer->getDefaultEmote()," (Default)",true);
        }
    }
    void changeEvent(GuiComponent *component, const unsigned char &event)
    {
        if(component==mActionList)actionClicked();
        else if(component==mEmoteList)emoteClicked();
    }
    void buttonClicked(GuiComponent *button)
    {
        if(button->nameIs("ActionScreen/EmoteDefault"))setEmoteAsDefault(mEmoteList->getSelectedLine());
        else if(button==mWalkCheckBox)
        {
            doWalk(mWalkCheckBox->isChecked());
            if(mWalkCheckBox->isChecked())mCrouchCheckBox->setChecked(false);
        }
        else if(button==mCrouchCheckBox)
        {
            doCrouch(mCrouchCheckBox->isChecked());
            if(mCrouchCheckBox->isChecked())mWalkCheckBox->setChecked(false);
        }
        else if(button==mLipSyncCheckBox)
        {
            mDoLipSync = mLipSyncCheckBox->isChecked();
        }
    }
    void loadEmoteHotkeys()
    {
        SaveFile sf;
        if(!sf.load("emoteHotkeys.cfg"))
        {
            mEmoteHotkey[0] = 0;
            mEmoteHotkey[1] = 1;
            mEmoteHotkey[2] = 2;
            mEmoteHotkey[3] = 3;
            mEmoteHotkey[4] = 4;
            mEmoteHotkey[5] = 5;
            mEmoteHotkey[6] = 6;
            mEmoteHotkey[7] = 7;
            mEmoteHotkey[8] = 8;
            mEmoteHotkey[9] = 9;
            mDoLipSync = true;
        }
        else
        {
            String buffer = "";
            for(int i=0;i<MAX_EMOTE_HOTKEYS;i++)
            {
                if(sf.getSetting(StringConverter::toString(i),buffer))mEmoteHotkey[i] = StringConverter::parseInt(buffer);
            }
            if(sf.getSetting("lipsync",buffer))mDoLipSync = StringConverter::parseBool(buffer);
        }
    }
    void saveEmoteHotkeys()
    {
        SaveFile sf("emoteHotkeys.cfg");
        for(int i=0;i<MAX_EMOTE_HOTKEYS;i++)sf.setSetting(StringConverter::toString(i),StringConverter::toString(mEmoteHotkey[i]));
        sf.setSetting("lipsync",StringConverter::toString(mDoLipSync));
        sf.save();
    }
    void actionClicked()
    {
        if(!mPlayer || m_bPlayerDisabled)return;

        switch(mActionList->getSelectedLine())
        {
            case 0: mPlayer->setAction(Unit::ACT_IDLE); break;
            case 1: doSit(false); return;
            case 2: doSit(true); return;
            case 3: mPlayer->setAction(Unit::ACT_DANCE1); break;
            case 4: mPlayer->setAction(Unit::ACT_DANCE2); break;
            case 5: mPlayer->setAction(Unit::ACT_DANCE3); break;
            case 6: mPlayer->setAction(Unit::ACT_STRETCH); break;
            case 7: mPlayer->setAction(Unit::ACT_CALL1); break;
            default: break;
        }
        resetActionList(false);
    }
    void emoteClicked()
    {
        const unsigned char emote = mEmoteList->getSelectedLine();
        setEmote(emote);
        if(mHotkeyHeld)
        {
            removeHotkeyAsEmote(mHotkeyHeld-1);
            mEmoteHotkey[mHotkeyHeld-1] = emote;
            setHotkeyAsEmote(mHotkeyHeld-1);
            mEmoteList->update();
        }
    }
    void setHotkeyEmote(const unsigned char &hotkey)
    {
        setEmote(mEmoteHotkey[hotkey]);
        mEmoteList->setSelection(mEmoteHotkey[hotkey]);
        mEmoteList->update();
    }
    void setEmote(const unsigned char &emote)
    {
        if(!mPlayer)return;
        mPlayer->setEmote(emote);
        mNetMgr->sendCharEmote(emote);
    }
    void setHotkeyAsEmote(const unsigned char &i)
    {
        Emote *emote = mEmoteMgr->getEmote(mEmoteHotkey[i]);
        if(emote)mEmoteList->changeLine(mEmoteHotkey[i],emote->mName + " [Key "+StringConverter::toString(i)+"]" + (mPlayer&&mPlayer->getDefaultEmote()==mEmoteHotkey[i]?"(Default)":""));
    }
    void removeHotkeyAsEmote(const unsigned char &i)
    {
        Emote *emote = mEmoteMgr->getEmote(mEmoteHotkey[i]);
        if(emote)mEmoteList->changeLine(mEmoteHotkey[i],emote->mName + (mPlayer&&mPlayer->getDefaultEmote()==mEmoteHotkey[i]?"(Default)":""));
    }
    void setEmoteAsDefault(const unsigned char &emote)
    {
        if(!mPlayer || emote==mPlayer->getDefaultEmote())return;

        Emote *prevEmote = mEmoteMgr->getEmote(mPlayer->getDefaultEmote());
        if(prevEmote)
        {
            int hotkeyedEmote = -1;
            for(int i=0;i<MAX_EMOTE_HOTKEYS;i++)
            {
                if(mEmoteHotkey[i]==mPlayer->getDefaultEmote())
                {
                    hotkeyedEmote = i;
                    break;
                }
            }
            mEmoteList->changeLine(mPlayer->getDefaultEmote(),prevEmote->mName + (hotkeyedEmote==-1?"": (" [Key "+StringConverter::toString(hotkeyedEmote)+"]") ));
        }

        mPlayer->setDefaultEmote(emote);
        if(m_pPlayerDataEmote)*m_pPlayerDataEmote = emote;
        mNetMgr->sendCharEmote(emote,true);
        mEmoteList->changeLine(emote,"(Default)",true);
        mEmoteList->update();
    }
    void keyPressed(const char &index)
    {
        using namespace KeyboardIndex;
        switch(index)
        {
            case KB_WALK:
                doWalk(true);
                mWalkCheckBox->setChecked(true);
                mCrouchCheckBox->setChecked(false);
                mShiftDown = true;
                break;
            case KB_CROUCH:
                doCrouch(true);
                mCrouchCheckBox->setChecked(true);
                mWalkCheckBox->setChecked(false);
                break;
            case KB_EMOTE0: case KB_EMOTE1: case KB_EMOTE2: case KB_EMOTE3: case KB_EMOTE4:
            case KB_EMOTE5: case KB_EMOTE6: case KB_EMOTE7: case KB_EMOTE8: case KB_EMOTE9:
                mHotkeyHeld = index - KB_EMOTE0 + 1;
                setHotkeyEmote(mHotkeyHeld-1);
                break;
            case KB_SIT:
                mActionList->setSelection(mShiftDown?2:1);
                doSit(mShiftDown);
                break;
            case KB_STAND:
                doRise();
                break;
            default: break;
        }
    }
    void keyReleased(const char &index)
    {
        using namespace KeyboardIndex;
        switch(index)
        {
            case KB_WALK:
                doWalk(false);
                mWalkCheckBox->setChecked(false);
                mShiftDown = false;
                break;
            case KB_CROUCH:
                doCrouch(false);
                mCrouchCheckBox->setChecked(false);
                break;
            case KB_EMOTE0: case KB_EMOTE1: case KB_EMOTE2: case KB_EMOTE3: case KB_EMOTE4:
            case KB_EMOTE5: case KB_EMOTE6: case KB_EMOTE7: case KB_EMOTE8: case KB_EMOTE9:
                mHotkeyHeld = 0;
                break;
            default: break;
        }
    }
    const bool objectEvent(ObjectListenerObject *object, const unsigned char &event)
    {
        return false;
    }
    const bool controllableEvent(ObjectListenerObject *object, const unsigned char &event)
    {
        return false;
    }
    const bool unitEvent(ObjectListenerObject *object, const unsigned char &event)
    {
        if(object!=mPlayer)return false;
        if(event==UNIT_SPECIALACTION_STOPPED)resetActionList();
        return true;
    }
    void doWalk(const bool &flag)
    {
        if(!mPlayer || mIsMovingReverse)return;
        mPlayer->setMovementModifier(flag?Unit::MOVE_WALK:Unit::MOVE_RUN);
        mNetMgr->sendCharMoveModifier(flag?Unit::MOVE_WALK:Unit::MOVE_RUN);
    }
    void doCrouch(const bool &flag)
    {
        if(!mPlayer)return;
        if(mIsMovingReverse)
        {
            mPlayer->setMovementModifier(flag?Unit::MOVE_CRAWLREVERSE:Unit::MOVE_WALKREVERSE);
            mNetMgr->sendCharMoveModifier(flag?Unit::MOVE_CRAWLREVERSE:Unit::MOVE_WALKREVERSE);
            return;
        }
        if(!flag || !mPlayer->mAntiGravity)
        {
            mPlayer->setMovementModifier(flag?Unit::MOVE_CRAWL:Unit::MOVE_RUN);
            mNetMgr->sendCharMoveModifier(flag?Unit::MOVE_CRAWL:Unit::MOVE_RUN);
            if(flag)
            {
                if(mPlayer->isIdle())mPlayer->setAction(Unit::ACT_CROUCH);
                resetActionList();
            }
            else if(mPlayer->getAction()==Unit::ACT_CROUCH)mPlayer->setAction(Unit::ACT_IDLE);
        }
    }
    void doWalkReverse(const bool &flag)
    {
        if(!mPlayer || mIsMovingReverse==flag)return;
        mIsMovingReverse = flag;
        if(flag)
        {
            if(mCrouchCheckBox->isChecked())
            {
                mPlayer->setMovementModifier(Unit::MOVE_CRAWLREVERSE);
                mNetMgr->sendCharMoveModifier(Unit::MOVE_CRAWLREVERSE);
            }
            else
            {
                mPlayer->setMovementModifier(Unit::MOVE_WALKREVERSE);
                mNetMgr->sendCharMoveModifier(Unit::MOVE_WALKREVERSE);
            }
        }
        else if(mWalkCheckBox->isChecked())doWalk(true);
        else if(mCrouchCheckBox->isChecked())doCrouch(true);
        else
        {
            mPlayer->setMovementModifier(Unit::MOVE_RUN);
            mNetMgr->sendCharMoveModifier(Unit::MOVE_RUN);
        }
    }
    void doSit(const bool &plop)
    {
        if(!mPlayer)return;
        switch(mPlayer->getAction())
        {
            case Unit::ACT_SIT:
                mPlayer->setAction(Unit::ACT_LAY);
                mActionList->changeLine(1,"Side Lay");
                mActionList->changeLine(2,"Plop Down");
                mActionList->update();
                break;
            case Unit::ACT_LAY:
                if(plop)
                {
                    mPlayer->setAction(Unit::ACT_PLOP);
                    mActionList->changeLine(2,"Curl");
                    mActionList->update();
                }
                else
                {
                    mPlayer->setAction(Unit::ACT_SIDELAY);
                    mActionList->changeLine(1,"Lean");
                    mActionList->update();
                }
                break;
            case Unit::ACT_PLOP: mPlayer->setAction(Unit::ACT_CURL); break;
            case Unit::ACT_SIDELAY:
                mPlayer->setAction(Unit::ACT_LEAN);
                mActionList->changeLine(1,"Roll Over");
                mActionList->update();
                break;
            case Unit::ACT_CURL: break;
            case Unit::ACT_LEAN: mPlayer->setAction(Unit::ACT_OVERTURN); break;
            case Unit::ACT_OVERTURN: break;
            default:
                mPlayer->setAction(Unit::ACT_SIT);
                mActionList->changeLine(1,"Lay");
                mActionList->changeLine(2,"Lay");
                mActionList->update();
                break;
        }
    }
    void doRise()
    {
        if(!mPlayer)return;
        switch(mPlayer->getAction())
        {
            case Unit::ACT_SIT:
                mPlayer->setAction(Unit::ACT_IDLE);
                mActionList->changeLine(1,"Sit");
                mActionList->changeLine(2,"Sit");
                mActionList->update();
                break;
            case Unit::ACT_LAY:
                mPlayer->setAction(Unit::ACT_SIT);
                mActionList->changeLine(1,"Lay");
                mActionList->changeLine(2,"Lay");
                mActionList->update();
                break;
            case Unit::ACT_PLOP:
                mPlayer->setAction(Unit::ACT_LAY);
                mActionList->changeLine(2,"Plop Down");
                mActionList->update();
                break;
            case Unit::ACT_CURL:
                mPlayer->setAction(Unit::ACT_PLOP);
                break;
            case Unit::ACT_SIDELAY:
                mPlayer->setAction(Unit::ACT_LAY);
                mActionList->changeLine(1,"Side Lay");
                mActionList->update();
                break;
            case Unit::ACT_LEAN:
                mPlayer->setAction(Unit::ACT_SIDELAY);
                mActionList->changeLine(1,"Lean");
                mActionList->update();
                break;
            case Unit::ACT_OVERTURN:
                mPlayer->setAction(Unit::ACT_LEAN);
                break;
            default: break;
        }
    }
    void resetActionList(const bool resetSelection=true)
    {
        if(resetSelection)mActionList->setSelection(0);
        mActionList->changeLine(1,"Sit");
        mActionList->changeLine(2,"Sit");
        mActionList->update();
    }
    GuiPanel* getPanel()
    {
        return mPanel;
    }
    const bool chatLineClicked(const ClickableLine &line){return false;}
    const bool chatCommand(const String &command){return false;}
    const bool chatLocal(const String &caption)
    {
        if(mDoLipSync)
        {
            mPlayer->setLipSyncSpeech(caption);
            return true;
        }
        return false;
    }
    void disableControl(const bool &bFlag)
    {
        m_bPlayerDisabled = bFlag;
    }
    void setPlayerDataEmotePointer(unsigned char *pEmote)
    {
        m_pPlayerDataEmote = pEmote;
    }
};

template<> ActionsManager* Singleton<ActionsManager>::ms_Singleton = 0;

ActionsManager* ActionsManager::getSingletonPtr()
{
	return ms_Singleton;
}

ActionsManager& ActionsManager::getSingleton()
{
	assert(ms_Singleton);
	return *ms_Singleton;
}

#endif

