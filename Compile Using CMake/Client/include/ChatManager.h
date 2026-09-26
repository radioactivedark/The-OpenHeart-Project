#ifndef _CHATMANAGER_H_
#define _CHATMANAGER_H_

#define NUM_CHANNELS 7

#include <GuiTextHandler.h>
#include <NetworkManager.h>
#include <Chat.h>
#include <GamePanelListener.h>

struct Chat
{
    String mCaption;
    unsigned char mType;
    String mSayer;
    bool mJoinLast;
    unsigned int mCharID;
    Chat(const String &caption, const unsigned char &type, const String &sayer="", const unsigned int &charID=0)
    {
        mCaption = caption;
        mType = type;
        mSayer = sayer;
        mJoinLast = false;
        mCharID = charID;
    }
    Chat(Chat *chat)
    {
        mCaption = chat->mCaption;
        mType = chat->mType;
        mSayer = chat->mSayer;
        mJoinLast = chat->mJoinLast;
        mCharID = chat->mCharID;
    }
};

struct ClickableLine
{
    unsigned short mIndex;
    unsigned int mCharID;
    unsigned char mType;
    ClickableLine()
    {
        mIndex = 0;
        mCharID = 0;
        mType = 0;
    }
    ClickableLine(const unsigned short &index, const unsigned int &charID, const unsigned char &type)
    {
        mIndex = index;
        mCharID = charID;
        mType = type;
    }
};

class ChatListener
{
public:
    ChatListener(){}
    virtual ~ChatListener(){}
    virtual const bool chatLineClicked(const ClickableLine &line) = 0;
    virtual const bool chatCommand(const String &command) = 0;
    virtual const bool chatLocal(const String &caption) = 0;
};

class ChatManager : public Singleton<ChatManager>
{
private:
    Gui *mGui;
    GuiTextHandler *mGuiTextHandler;
    NetworkManager *mNetMgr;
    GuiTextArea *mInputArea;
    GuiSlider *mInputSlider;
    std::vector<Chat*> mChat;
    unsigned short mNumChat;
    std::vector<GuiTextComponent*> mLine;
    std::vector<ClickableLine> mClickableLine;
    GuiPanel *mOutputPanel;
    GuiPanel *mInputPanel;
    GuiPanel *mMidPanel;
    GuiPanel *mFilterPanel;
    GuiSlider *mOutputSlider;
    GuiSizer *mOutputSizerN;
    GuiSizer *mOutputSizerE;
    GuiButton *mChannelButton;
    GuiTextField *mWhisperField;
    String mWhisperTarget;
    GuiButton *mFilterButton;
    std::vector<String> mChannelList;
    bool mFilter[NUM_CHANNELS];
    unsigned char mChannel;
    unsigned short mChatFontSize;
    std::vector<ChatListener*> mListeners;
    GamePanelListener *mGamePanelListener;
    bool mMuteSelfOnLocal;
    String mUserName;
    String mCharName;
public:
    unsigned char mSystemChatNameType;
    unsigned char mLocalChatNameType;
    unsigned char mGeneralChatNameType;
    ChatManager()
    {
        mInputArea = 0;
        mChat.clear();
        mNumChat = 0;
        mLine.clear();
        mOutputPanel = 0;
        mMidPanel = 0;
        mFilterPanel = 0;
        mOutputSlider = 0;
        mOutputSizerN = 0;
        mOutputSizerE = 0;
        mWhisperField = 0;
        mChannelList.clear();
        for(int i=0;i<NUM_CHANNELS;i++)mFilter[i] = false;
        mChannel = 0;
        mChatFontSize = 14;
        mGamePanelListener = 0;
        mMuteSelfOnLocal = false;
        mUserName = "";
        mCharName = "";
        mSystemChatNameType = 1;
        mLocalChatNameType = 0;
        mGeneralChatNameType = 1;
    }
    ~ChatManager()
    {
        reset();
        clearChat();
    }
    static ChatManager* getSingletonPtr();
    static ChatManager& getSingleton();
    void init()
    {
        mGui = Gui::getSingletonPtr();
        mGuiTextHandler = GuiTextHandler::getSingletonPtr();
        mNetMgr = NetworkManager::getSingletonPtr();

        mOutputPanel = mGui->addPanel("ChatOutputScreen",true);
        mInputPanel = mGui->addPanel("ChatInputScreen",true);
        mMidPanel = mGui->addPanel("ChatMidScreen",true);
        mFilterPanel = mGui->addPanel("ChatFilterScreen",true);

        const Real screenWidthExtra = *(mGui->mWindowWidth) - 640;
        mOutputSlider = mOutputPanel->getSlider("ChatOutputScreen/S");
        mOutputSizerN = mOutputPanel->getSizer("ChatOutputScreen/Sizer1");
        mOutputSizerE = mOutputPanel->getSizer("ChatOutputScreen/Sizer2");
        mOutputSizerN->setMinSize(0.09f);
        mOutputSizerE->setMinSize(0.3f - screenWidthExtra/6400);
        mInputArea = mInputPanel->getTextArea("ChatInputScreen/Input");
        mInputArea->setCharHeight(mChatFontSize);
        mInputArea->mMaxLength = 256;
        mInputSlider = static_cast<GuiSlider*>(mInputArea->getSliderRef());
        createOutputLines();

        mChannelButton = mMidPanel->getButton("ChatMidScreen/Channel");
        mWhisperField = mMidPanel->getTextField("ChatMidScreen/Whisper");
        mWhisperField->show(false);
        mWhisperField->mMaxLength = 20;
        mWhisperField->bindString(&mWhisperTarget);
        mWhisperField->setCharHeight(14+int(screenWidthExtra/160));
        mWhisperField->setCaptionTopOffset(screenWidthExtra/320);
        mFilterButton = mMidPanel->getButton("ChatMidScreen/Filter");

        mFilterPanel->show(false);
        for(int i=0;i<NUM_CHANNELS;i++)mFilterPanel->getCheckBox("ChatFilterScreen/C"+StringConverter::toString(i))->setChecked(!mFilter[i]);

        mChannelList.push_back("Local");
        mChannelList.push_back("General");
        mChannelList.push_back("Group");
        mChannelList.push_back("Party");
        mChannelList.push_back("Whisper");

        refresh(true);
        updateChannel();
    }
    void reset()
    {
        clearOutputLines();
        mChannelList.clear();
        mInputArea = 0;
        mListeners.clear();
        mGamePanelListener = 0;
        mMuteSelfOnLocal = false;
    }
    void update(const Real &timeElapsed)
    {
    }
    void refresh(const bool &clampSlider)
    {
        if(mChat.empty())return;

        if(clampSlider)mOutputSlider->setValue(1);

        std::vector<pair<unsigned short,Chat> > filteredChat;
        int lineIndex = -1;
        for(std::vector<Chat*>::iterator i=mChat.begin(); i!=mChat.end(); i++)
        {
            Chat *chat = *i;
            if(!chat->mJoinLast)lineIndex++;
            if(chat->mType>=NUM_CHANNELS || !mFilter[chat->mType])filteredChat.push_back(pair<unsigned short,Chat>(lineIndex,Chat(chat)));
        }
        const unsigned short firstLine = mOutputSlider->getFirstLineOf(filteredChat.size(),mLine.size());

        //Fill output lines with relevant chat
        unsigned short i = 0;
        std::vector<GuiTextComponent*>::iterator it = mLine.begin();
        std::vector<ClickableLine>::iterator cit = mClickableLine.begin();
        while(it!=mLine.end())
        {
            const unsigned short line = firstLine+i;
            if(line>=(int)filteredChat.size())break;

            const pair<unsigned short,Chat> *fc = &filteredChat[line];
            GuiTextComponent *textLine = *it;
            textLine->setCaption(fc->second.mCaption);
            if(fc->second.mCharID!=0)
            {
                //textLine->setCaption(filteredChat[line].mSayer+" has made a movie clip.");
                textLine->mClickable = true;
                ClickableLine *clickableLine = &*cit;
                clickableLine->mIndex = fc->first;
                clickableLine->mCharID = fc->second.mCharID;
                clickableLine->mType = fc->second.mType;
            }
            else textLine->mClickable = false;

            const pair<ColourValue,ColourValue> colour = getColoursForType(fc->second.mType);
            textLine->setColours(colour.first,colour.second);

            it++;
            cit++;
            i++;
        }
        //Clear remaining output lines
        while(it!=mLine.end())
        {
            GuiTextComponent *textLine = *it;
            textLine->setCaption("");
            textLine->mClickable = false;
            it++;
        }

        filteredChat.clear();
    }
    void clearChat()
    {
        while(!mChat.empty())
        {
            Chat *chat = mChat.back();
            mChat.pop_back();
            delete chat;
        }
        mNumChat = 0;
    }
    const pair<ColourValue,ColourValue> getColoursForType(const unsigned char &type)
    {
        switch(type)
        {
            default:
            case CHAT_LOCAL:
                return pair<ColourValue,ColourValue>(ColourValue(1,1,1),ColourValue(1,1,1));
            case CHAT_GENERAL:
                return pair<ColourValue,ColourValue>(ColourValue(0.5,1,0.5f),ColourValue(0.5,1,0.5f));
            case CHAT_GROUP:
                return pair<ColourValue,ColourValue>(ColourValue(0,1,0),ColourValue(0,1,0));
            case CHAT_PARTY:
                return pair<ColourValue,ColourValue>(ColourValue(1,1,0),ColourValue(1,1,0));
            case CHAT_WHISPER:
                return pair<ColourValue,ColourValue>(ColourValue(0,1,1),ColourValue(0,1,1));
            case CHAT_MOVIE:
                return pair<ColourValue,ColourValue>(ColourValue(1,1,0.5f),ColourValue(1,1,0.5f));
            case CHAT_ACTION:
                return pair<ColourValue,ColourValue>(ColourValue(0.5f,0.5f,1),ColourValue(0.5f,0.5f,1));
            case CHAT_SYSTEM:
                return pair<ColourValue,ColourValue>(ColourValue(1,0.5f,0),ColourValue(1,0.5f,0));
            case CHAT_MOD:
                return pair<ColourValue,ColourValue>(ColourValue(1,0,0),ColourValue(1,0,0));
            case CHAT_MODWHISPER:
                return pair<ColourValue,ColourValue>(ColourValue(1,0.2f,0.2f),ColourValue(1,0.2f,0.2f));
        }
    }
    void createOutputLines()
    {
        const Real charHeight = mInputArea->getCharHeight();
        const unsigned char numLines = (unsigned char)((mOutputPanel->getHeight()-0.02f)/charHeight);
        const Real width = mOutputPanel->getOverlayContainer()->getWidth() - mOutputSlider->getOverlayContainer()->getWidth();
        for(int i=0;i<numLines;i++)
        {
            OverlayContainer *container = static_cast<OverlayContainer*>(OverlayManager::getSingleton().createOverlayElementFromTemplate("GuiTemplate/TextComponent","Panel","ChatLine"+StringConverter::toString(i)));
            container->setDimensions(width,charHeight);
            container->setPosition(0.01f,0.01f+i*charHeight);
            GuiTextComponent *line = new GuiTextComponent(container,"ChatOutputScreen/ChatLine"+StringConverter::toString(i),mGui->mWindowWidth,mGui->mWindowHeight);
            mOutputPanel->addComponent(line);
            line->mClickable = false;
            line->setCharHeight((int)mInputArea->getCharHeight(true));
            mLine.push_back(line);
            mClickableLine.push_back(ClickableLine());
        }
    }
    void clearOutputLines()
    {
        while(!mLine.empty())
        {
            GuiTextComponent *line = mLine.back();
            mLine.pop_back();
            mOutputPanel->removeComponent(line);
            const String containerName = line->getOverlayContainer()->getName();
            delete line;
            OverlayManager::getSingleton().destroyOverlayElement(containerName);
        }
        mClickableLine.clear();
    }
    void refreshOutputLinesWidth()
    {
        const Real width = mOutputPanel->getOverlayContainer()->getWidth() - mOutputSlider->getOverlayContainer()->getWidth();
        for(std::vector<GuiTextComponent*>::iterator it=mLine.begin(); it!=mLine.end(); it++)
        {
            GuiTextComponent *line = *it;
            line->getOverlayContainer()->setWidth(width);
        }
    }
    const unsigned short pushChatLine(String caption, const String &sayer="", char type=-1, const unsigned int &charID=0, const bool &whisperFrom=false)
    {
        if(mLine.size()<=0)return mNumChat;
        if(type==-1)type = mChannel;
        switch(type)
        {
            case CHAT_LOCAL:
            case CHAT_GENERAL:
            case CHAT_GROUP:
            case CHAT_PARTY:
                if(sayer!="")caption.insert(0,"<"+sayer+"> ");
                break;
            case CHAT_ACTION:
                if(sayer!="")caption.insert(0,sayer+" ");
                break;
            case CHAT_WHISPER:
            case CHAT_MODWHISPER:
                if(whisperFrom)caption.insert(0,sayer+">> ");
                else caption.insert(0,mWhisperTarget+"<< ");
                break;
            case CHAT_MOVIE:
                caption = sayer+" has made a movie clip.";
                break;
            default: break;
        }

        mLine[0]->wrapCaption(caption);
        const StringVector line = StringUtil::split(caption,"\n");
        for(int i=0;i<(int)line.size();i++)
        {
            Chat *chat = new Chat(line[i],type,(i==0?sayer:""),charID);
            if(i!=0)chat->mJoinLast = true;
            mChat.push_back(chat);
        }
        refresh( !mOutputSlider->isHeld()&&(type<NUM_CHANNELS?!mFilter[(int)type]:true) );

        if(mGamePanelListener && !mOutputPanel->isVisible())mGamePanelListener->highlightButton(GamePanelListener::GAMEPANEL_CHAT,true);

        return mNumChat++;
    }
    void remakeChatLines()
    {
        if(mLine.size()<=0)return;

        std::vector<Chat*> newList;
        std::vector<Chat*>::iterator it=mChat.begin();
        Chat tempLine("",0);
        while(it!=mChat.end())
        {
            Chat *chat = *it;
            //Connect this line to previous
            if(chat->mJoinLast)tempLine.mCaption += chat->mCaption;
            //This is a new line
            else
            {
                //Templine contains something
                if(tempLine.mCaption!="")
                {
                    //Wrap templine and insert new lines in templine into newlist
                    mLine[0]->wrapCaption(tempLine.mCaption);
                    const StringVector line = StringUtil::split(tempLine.mCaption,"\n");
                    for(int i=0;i<(int)line.size();i++)
                    {
                        Chat *chat = new Chat(line[i],tempLine.mType,(i==0?tempLine.mSayer:""));
                        if(i!=0)chat->mJoinLast = true;
                        newList.push_back(chat);
                    }
                    //Clear templine
                    tempLine.mCaption = "";
                }
                //Store new line
                tempLine.mCaption = chat->mCaption;
                tempLine.mType = chat->mType;
                tempLine.mSayer = chat->mSayer;
            }
            it++;
        }
        //Templine contains something
        if(tempLine.mCaption!="")
        {
            //Wrap templine and insert new lines in templine into newlist
            mLine[0]->wrapCaption(tempLine.mCaption);
            const StringVector line = StringUtil::split(tempLine.mCaption,"\n");
            for(int i=0;i<(int)line.size();i++)
            {
                Chat *chat = new Chat(line[i],tempLine.mType,(i==0?tempLine.mSayer:""));
                if(i!=0)chat->mJoinLast = true;
                newList.push_back(chat);
            }
        }
        //Assign new chat list
        clearChat();
        mChat = newList;
        newList.clear();
    }
    void systemMessage(const String &caption, const char &type=CHAT_SYSTEM, const unsigned int &charID=0)
    {
        pushChatLine(caption,"",type,charID);
    }
    void buttonClicked(GuiComponent *button)
    {
        unsigned short index = 0;
        if(button==mChannelButton)
        {
            if(mGui->isComboBoxOpen()&&mGui->getComboBoxName()=="ChatChannel")mGui->hideComboBox();
            else mGui->showComboBox("ChatChannel",mChannelList);
        }
        else if(button==mFilterButton)mFilterPanel->toggleVisibility();
        else if(button->nameIsIndex("ChatFilterScreen/C",&index))
        {
            mFilter[index] = !static_cast<GuiCheckBox*>(button)->isChecked();
            refresh(false);
        }
    }
    void changeEvent(GuiComponent *component, const unsigned char &event)
    {
        unsigned short index = 0;
        if(component==mInputArea && event==ChangeListener::EVENT_KEYPRESSED)
        {
            const String caption = mInputArea->getCaption();
            if(caption.length()>0)
            {
                //Command
                if(caption[0]=='/')
                {
                    for(std::vector<ChatListener*>::iterator it=mListeners.begin(); it!=mListeners.end(); it++)
                    {
                        ChatListener *listener = *it;
                        listener->chatCommand(caption);
                    }
                }
                else if(mChannel==CHAT_WHISPER)
                {
                    if(mWhisperTarget=="")mGui->showAlertBox("Enter a whisper target.");
                    else
                    {
                        mNetMgr->sendChat(caption,mChannel,mWhisperTarget);
                        pushChatLine(caption,mUserName,mMuteSelfOnLocal?CHAT_MODWHISPER:-1);
                    }
                }
                else
                {
                    bool doLipSync = false;
                    if(mChannel==CHAT_LOCAL)
                    {
                        if(!mMuteSelfOnLocal)
                        {
                            pushChatLine(caption,getLocalChatName(mUserName,mCharName));
                            for(std::vector<ChatListener*>::iterator it=mListeners.begin(); it!=mListeners.end(); it++)
                            {
                                ChatListener *listener = *it;
                                if(listener->chatLocal(caption))doLipSync = true;
                            }
                        }
                    }
                    else if(mChannel==CHAT_GENERAL)
                    {
                        if(!mMuteSelfOnLocal)pushChatLine(caption,getGeneralChatName(mUserName,mCharName));
                    }
                    else if(mChannel==CHAT_GROUP || mChannel==CHAT_PARTY)
                    {
                        pushChatLine(caption,mCharName);
                    }
                    else pushChatLine(caption,mUserName);

                    mNetMgr->sendChat(caption,mChannel,"",doLipSync);
                }
                mInputArea->setCaption("");
            }
        }
        else if(component==mOutputSlider)
        {
            refresh(false);
        }
        else if(component->isSizer())
        {
            if(component==mOutputSizerN)
            {
                mOutputSlider->setWidth(mOutputPanel->getHeight()-0.04f);
                mOutputSizerE->updateSize();
                clearOutputLines();
                createOutputLines();
                refresh(false);
            }
            else if(component==mOutputSizerE)
            {
                mInputPanel->setWidth(mOutputPanel->getWidth());
                mInputArea->setWidth(mOutputPanel->getWidth()-0.04f);
                mInputSlider->setLeft(mOutputPanel->getWidth()-0.03f);
                mOutputSlider->setLeft(mOutputPanel->getWidth()-0.03f);
                mFilterButton->setLeft(mOutputPanel->getWidth()-mFilterButton->getWidth());
                mFilterPanel->setLeft(mOutputPanel->getWidth()+0.02f);
                mMidPanel->setWidth(mOutputPanel->getWidth());
                mWhisperField->setWidth(mOutputPanel->getWidth()-mChannelButton->getWidth()-mFilterButton->getWidth());
                mOutputSizerN->updateSize();
                refreshOutputLinesWidth();
                remakeChatLines();
                mInputArea->setCaption(mInputArea->getCaption());
                refresh(true);
            }
        }
        else if(component->nameIsIndex("ChatOutputScreen/ChatLine",&index) && event==ChangeListener::EVENT_MOUSEPRESSED)
        {
            if(index<(int)mClickableLine.size())
            {
                const ClickableLine line = mClickableLine[index];
                for(std::vector<ChatListener*>::iterator it=mListeners.begin(); it!=mListeners.end(); it++)
                {
                    ChatListener *listener = *it;
                    listener->chatLineClicked(line);
                }
            }
        }
    }
    void comboBoxEvent(const String &name)
    {
        if(name=="ChatChannel")
        {
            mChannel = mGui->getComboBoxList()->getSelectedLine();
            updateChannel();
        }
    }
    void updateChannel()
    {
        const pair<ColourValue,ColourValue> colour = getColoursForType(mChannel);
        mInputArea->setColours(colour.first,colour.second);
        switch(mChannel)
        {
            case CHAT_LOCAL: mChannelButton->setCaption("Local"); break;
            case CHAT_GENERAL: mChannelButton->setCaption("General"); break;
            case CHAT_GROUP: mChannelButton->setCaption("Group"); break;
            case CHAT_PARTY: mChannelButton->setCaption("Party"); break;
            case CHAT_WHISPER: mChannelButton->setCaption("Whisper"); break;
            default: break;
        }
        mWhisperField->show(mChannel==CHAT_WHISPER);
    }
    void setChannel(const unsigned char &channel)
    {
        mChannel = channel;
        updateChannel();
    }
    void setWhisperTarget(const String &name)
    {
        mWhisperField->setCaption(name);
    }
    void enterPressed()
    {
        GuiTextHandler::getSingleton().setActiveField(mInputArea,0,0,false);
        if(!mInputPanel->isVisible())
        {
            mOutputPanel->show(true);
            mInputPanel->show(true);
            mMidPanel->show(true);
        }
    }
    void setInputName(const String &username, const String &charname)
    {
        mUserName = username;
        mCharName = charname;
    }
    void setChatFontSize(const unsigned short &value)
    {
        mChatFontSize = value;
        if(mInputArea)
        {
            mInputArea->setCharHeight(mChatFontSize);
            clearOutputLines();
            createOutputLines();
            remakeChatLines();
            refresh(true);
            if(GuiTextHandler::getSingleton().getActiveField()==mInputArea)GuiTextHandler::getSingleton().updateField();
            else
            {
                mInputArea->setActiveLine(0);
                mInputArea->setCaption(mInputArea->getCaption());
            }
        }
    }
    const unsigned short getChatFontSize()
    {
        return mChatFontSize;
    }
    void addListener(ChatListener *listener)
    {
        mListeners.push_back(listener);
    }
    void setGamePanelListener(GamePanelListener *listener)
    {
        mGamePanelListener = listener;
    }
    void getPanels(GuiPanel **output, GuiPanel **input, GuiPanel **mid, GuiPanel **filter)
    {
        *output = mOutputPanel;
        *input = mInputPanel;
        *mid = mMidPanel;
        *filter = mFilterPanel;
    }
    void setMuteSelfOnLocal(const bool &flag)
    {
        mMuteSelfOnLocal = flag;
    }
    const bool isHideSystemChat()
    {
        return (mSystemChatNameType==3);
    }
    const String getSystemChatName(const String &username, const String &charname)
    {
        if(mSystemChatNameType==1)return username;
        else if(mSystemChatNameType==2)return charname+"["+username+"]";
        return charname;
    }
    const String getLocalChatName(const String &username, const String &charname)
    {
        if(mLocalChatNameType==1)return username;
        else if(mLocalChatNameType==2)return charname+"["+username+"]";
        return charname;
    }
    const String getGeneralChatName(const String &username, const String &charname)
    {
        if(mGeneralChatNameType==1)return username;
        else if(mGeneralChatNameType==2)return charname+"["+username+"]";
        return charname;
    }
    void appendInputArea(const String &caption)
    {
        mInputArea->setCaption(mInputArea->getCaption() + caption);
    }
};

template<> ChatManager* Singleton<ChatManager>::ms_Singleton = 0;

ChatManager* ChatManager::getSingletonPtr()
{
	return ms_Singleton;
}

ChatManager& ChatManager::getSingleton()
{
	assert(ms_Singleton);
	return *ms_Singleton;
}

#endif

