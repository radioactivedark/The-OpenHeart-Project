#ifndef _GAMEPANELMANAGER_H_
#define _GAMEPANELMANAGER_H_

#include <MovieMaker.h>
#include <ActionsManager.h>
#include <FriendsManager.h>
#include <PartyManager.h>
#include <GroupManager.h>
#include <InfoManager.h>
#include <MiniMapManager.h>

class GamePanelManager : public Singleton<GamePanelManager>, public GamePanelListener
{
private:
    Gui *mGui;
    GuiPanel *mGamePanel;
    GuiPanel *mMoviePanel;
    GuiButton *mMovieButton;
    GuiPanel *mActionsPanel;
    GuiButton *mActionsButton;
    GuiPanel *mFriendsPanel;
    GuiButton *mFriendsButton;
    GuiTextComponent *mFriendsReminder;
    unsigned char mFriendsReminderCount;
    GuiPanel *mPartyPanel;
    GuiButton *mPartyButton;
    GuiTextComponent *mPartyReminder;
    unsigned char mPartyReminderCount;
    GuiPanel *mGroupPanel;
    GuiButton *mGroupButton;
    GuiTextComponent *mGroupReminder;
    unsigned char mGroupReminderCount;
    GuiPanel *mInfoPanel;
    GuiButton *mInfoButton;
    GuiPanel *mChatOutputPanel;
    GuiPanel *mChatInputPanel;
    GuiPanel *mChatMidPanel;
    GuiPanel *mChatFilterPanel;
    GuiButton *mChatButton;
    GuiPanel *mMiniMapPanel;
    GuiButton *mMiniMapButton;
public:
    GamePanelManager()
    {
        reset();
    }
    ~GamePanelManager()
    {
    }
    static GamePanelManager* getSingletonPtr();
    static GamePanelManager& getSingleton();
    void reset()
    {
        mFriendsReminderCount = 0;
        mPartyReminderCount = 0;
        mGroupReminderCount = 0;
    }
    void init()
    {
        mGui = Gui::getSingletonPtr();
        mGamePanel = mGui->addPanel("InGameButtonsScreen",true);
        mActionsButton = mGamePanel->getButton("InGameButtonsScreen/Action");
        mActionsButton->setOwnMaterials("GuiMat/ButtonActionUp","GuiMat/ButtonActionOver","GuiMat/ButtonActionDown");
        mMovieButton = mGamePanel->getButton("InGameButtonsScreen/Movie");
        mMovieButton->setOwnMaterials("GuiMat/ButtonMovieUp","GuiMat/ButtonMovieOver","GuiMat/ButtonMovieDown");
        mInfoButton = mGamePanel->getButton("InGameButtonsScreen/Info");
        mInfoButton->setOwnMaterials("GuiMat/ButtonInfoUp","GuiMat/ButtonInfoOver","GuiMat/ButtonInfoDown");
        mFriendsButton = mGamePanel->getButton("InGameButtonsScreen/Friends");
        mFriendsButton->setOwnMaterials("GuiMat/ButtonFriendsUp","GuiMat/ButtonFriendsOver","GuiMat/ButtonFriendsDown","GuiMat/ButtonFriendsHighlight");
        mFriendsReminder = mGamePanel->getTextComponent("InGameButtonsScreen/FriendsReminder");
        mFriendsReminder->setColours(ColourValue(1,1,1),ColourValue(1,1,1));
        mPartyButton = mGamePanel->getButton("InGameButtonsScreen/Party");
        mPartyButton->setOwnMaterials("GuiMat/ButtonPartyUp","GuiMat/ButtonPartyOver","GuiMat/ButtonPartyDown","GuiMat/ButtonPartyHighlight");
        mPartyReminder = mGamePanel->getTextComponent("InGameButtonsScreen/PartyReminder");
        mPartyReminder->setColours(ColourValue(1,1,1),ColourValue(1,1,1));
        mGroupButton = mGamePanel->getButton("InGameButtonsScreen/Group");
        mGroupButton->setOwnMaterials("GuiMat/ButtonGroupUp","GuiMat/ButtonGroupOver","GuiMat/ButtonGroupDown","GuiMat/ButtonGroupHighlight");
        mGroupReminder = mGamePanel->getTextComponent("InGameButtonsScreen/GroupReminder");
        mGroupReminder->setColours(ColourValue(1,1,1),ColourValue(1,1,1));
        mMiniMapButton = mGamePanel->getButton("InGameButtonsScreen/MiniMap");
        mMiniMapButton->setOwnMaterials("GuiMat/ButtonMiniMapUp","GuiMat/ButtonMiniMapOver","GuiMat/ButtonMiniMapDown","GuiMat/ButtonMiniMapHighlight");
        mChatButton = mGamePanel->getButton("InGameButtonsScreen/Chat");
        mChatButton->setOwnMaterials("GuiMat/ButtonChatUp","GuiMat/ButtonChatOver","GuiMat/ButtonChatDown","GuiMat/ButtonChatHighlight");

        mActionsPanel = ActionsManager::getSingletonPtr()->getPanel();
        mMoviePanel = MovieMaker::getSingletonPtr()->getPanel();
        mInfoPanel = InfoManager::getSingletonPtr()->getPanel();
        mFriendsPanel = FriendsManager::getSingletonPtr()->getPanel();
        mPartyPanel = PartyManager::getSingletonPtr()->getPanel();
        mGroupPanel = GroupManager::getSingletonPtr()->getPanel();
        mMiniMapPanel = MiniMapManager::getSingletonPtr()->getPanel();
        ChatManager::getSingletonPtr()->getPanels(&mChatOutputPanel,&mChatInputPanel,&mChatMidPanel,&mChatFilterPanel);

        updateReminder(mFriendsReminder,mFriendsButton,mFriendsReminderCount);
        updateReminder(mPartyReminder,mPartyButton,mPartyReminderCount);
        updateReminder(mGroupReminder,mGroupButton,mGroupReminderCount);
    }
    void buttonClicked(GuiComponent *button)
    {
        if(button->isButton())static_cast<GuiButton*>(button)->highlight(false);

        if(button==mActionsButton)togglePanel(mActionsPanel);
        else if(button==mMovieButton)togglePanel(mMoviePanel);
        else if(button==mInfoButton)togglePanel(mInfoPanel);
        else if(button==mFriendsButton)
        {
            togglePanel(mFriendsPanel);
            mFriendsReminderCount = 0;
            updateReminder(mFriendsReminder,mFriendsButton,mFriendsReminderCount);
        }
        else if(button==mPartyButton)
        {
            togglePanel(mPartyPanel);
            mPartyReminderCount = 0;
            updateReminder(mPartyReminder,mPartyButton,mPartyReminderCount);
        }
        else if(button==mGroupButton)
        {
            togglePanel(mGroupPanel);
            mGroupReminderCount = 0;
            updateReminder(mGroupReminder,mGroupButton,mGroupReminderCount);
        }
        else if(button==mMiniMapButton)MiniMapManager::getSingletonPtr()->toggleMode();
        else if(button==mChatButton)
        {
            mChatOutputPanel->toggleVisibility();
            mChatInputPanel->toggleVisibility();
            mChatMidPanel->toggleVisibility();
            mChatFilterPanel->show(false);
        }
    }
    void keyPressed(const char &index)
    {
        using namespace KeyboardIndex;
        switch(index)
        {
            case KB_CHAT:
                mChatButton->highlight(false);
                mChatOutputPanel->toggleVisibility();
                mChatInputPanel->toggleVisibility();
                mChatMidPanel->toggleVisibility();
                mChatFilterPanel->show(false);
                break;

            case KB_MINIMAP:
                MiniMapManager::getSingletonPtr()->toggleMode();
                break;

            case KB_INFO:
                togglePanel(mInfoPanel);
                break;

            case KB_ACTIONS:
                togglePanel(mActionsPanel);
                break;

            case KB_FRIENDS:
                mFriendsButton->highlight(false);
                togglePanel(mFriendsPanel);
                mFriendsReminderCount = 0;
                updateReminder(mFriendsReminder,mFriendsButton,mFriendsReminderCount);
                break;

            case KB_PARTY:
                mPartyButton->highlight(false);
                togglePanel(mPartyPanel);
                mPartyReminderCount = 0;
                updateReminder(mPartyReminder,mPartyButton,mPartyReminderCount);
                break;

            case KB_GROUPS:
                mGroupButton->highlight(false);
                togglePanel(mGroupPanel);
                mGroupReminderCount = 0;
                updateReminder(mGroupReminder,mGroupButton,mGroupReminderCount);
                break;

            case KB_MOVIEMAKER:
                togglePanel(mMoviePanel);
                break;

            default: break;
        }
    }
    void highlightButton(const unsigned char &panel, const bool &flag, const bool &incrementReminder=false)
    {
        switch(panel)
        {
            case GamePanelListener::GAMEPANEL_FRIENDS:
                mFriendsButton->highlight(flag);
                if(incrementReminder)mFriendsReminderCount++;
                updateReminder(mFriendsReminder,mFriendsButton,mFriendsReminderCount);
                break;
            case GamePanelListener::GAMEPANEL_PARTY:
                mPartyButton->highlight(flag);
                if(incrementReminder)mPartyReminderCount++;
                updateReminder(mPartyReminder,mPartyButton,mPartyReminderCount);
                break;
            case GamePanelListener::GAMEPANEL_GROUP:
                mGroupButton->highlight(flag);
                if(incrementReminder)mGroupReminderCount++;
                updateReminder(mGroupReminder,mGroupButton,mGroupReminderCount);
                break;
            case GamePanelListener::GAMEPANEL_CHAT: mChatButton->highlight(flag); break;
            default: break;
        }
    }
    void togglePanel(GuiPanel *panel)
    {
        panel->toggleVisibility();
        if(panel->isVisible())mGui->setActivePanel(panel);
    }
    void updateReminder(GuiTextComponent *reminder, GuiButton *button, const unsigned char &count)
    {
        const bool hasReminder = count>0;
        const String caption = StringConverter::toString(count);
        reminder->show(hasReminder);
        if(hasReminder)
        {
            reminder->setWidth(reminder->getCaptionWidth(caption));
            reminder->setLeft(button->getLeft() + button->getWidth() - reminder->getWidth());
            reminder->setCaption(caption);
        }
    }
};

template<> GamePanelManager* Singleton<GamePanelManager>::ms_Singleton = 0;

GamePanelManager* GamePanelManager::getSingletonPtr()
{
	return ms_Singleton;
}

GamePanelManager& GamePanelManager::getSingleton()
{
	assert(ms_Singleton);
	return *ms_Singleton;
}

#endif
