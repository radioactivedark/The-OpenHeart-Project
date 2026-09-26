#ifndef _MOVIEMAKER_H_
#define _MOVIEMAKER_H_

#define MOVIE_MAXACTIONS 10
#define MOVIE_MAXEMOTES 10

#include <Gui.h>
#include <ScreenEffectsManager.h>
#include <CameraManager.h>
#include <UnitManager.h>
#include <ChatManager.h>
#include <Networkmanager.h>

struct MovieCamMovement
{
    const String mName;
    std::vector< std::pair<Real,Vector3> > mTimePos;
    std::vector<Vector3> mRotation;
    bool mAutoTrack;
    bool mHeadTrack;
    MovieCamMovement(const String &name) : mName(name)
    {
        mAutoTrack = true;
        mHeadTrack = false;
    }
};

struct MovieInfo
{
    String mMovie;
    String mActorName;
    unsigned int mActorCharID;
    Real mCamYaw;
    Vector3 mPosition;
    Real mYaw;
    Real mHeadNS;
    Real mHeadEW;
    MovieInfo()
    {
        mMovie = "";
        mActorName = "";
        mActorCharID = 0;
        mCamYaw = 0;
        mPosition = Vector3::ZERO;
        mYaw = 0;
        mHeadNS = mHeadEW = 0;
    }
};

class MovieClip
{
public:
    Real mDuration;
    unsigned char mCamMovement;
    Real mSpeechTime;
    String mSpeech;
    std::vector< std::pair<Real,unsigned char> > mAction;
    std::vector< std::pair<Real,unsigned char> > mEmote;
    MovieClip()
    {
        mDuration = 0;
        mCamMovement = 0;
        mSpeechTime = 0;
        mSpeech = "";
        mAction.clear();
        mEmote.clear();
    }
    ~MovieClip()
    {
        mAction.clear();
        mEmote.clear();
    }
    const bool fromString(const String &caption)
    {
        mDuration = 0;
        mCamMovement = 0;
        mSpeechTime = 0;
        mSpeech = "";
        mAction.clear();
        mEmote.clear();

        if(caption.length()<6)return false;
        String firstSix = caption;
        if(caption.length()>6)firstSix.erase(6);
        String buffer = firstSix;
        buffer.erase(2);
        mDuration = FFStringToReal(buffer.c_str());             //0-1: duration
        buffer = firstSix;
        buffer.erase(4);
        buffer.erase(0,2);
        mCamMovement = FFStringToUChar(buffer.c_str());         //2-3: camera
        buffer = firstSix;
        buffer.erase(5);
        buffer.erase(0,4);
        const unsigned char numActions = FFStringToUChar(buffer.c_str());   //4: num actions
        buffer = firstSix;
        buffer.erase(0,5);
        const unsigned char numEmotes = FFStringToUChar(buffer.c_str());   //5: num emotes
        if(caption.length()==6)return true;
        buffer = caption;
        buffer.erase(0,6);
        for(int i=0;i<numActions;i++)                           //actions: n - n+1: actionTime, n+2 - n+3: action
        {
            String buffer2 = buffer;
            if(buffer.length()>=4)buffer.erase(0,4);
            if(buffer2.length()>4)buffer2.erase(4);
            String actionStr = buffer2;
            actionStr.erase(2);
            const Real time = FFStringToReal(actionStr.c_str());
            actionStr = buffer2;
            actionStr.erase(0,2);
            const unsigned char action = FFStringToUChar(actionStr.c_str());
            mAction.push_back(pair<Real,unsigned char>(time,action));
        }
        for(int i=0;i<numEmotes;i++)                           //emotes: n - n+1: emoteTime, n+2 - n+3: emote
        {
            String buffer2 = buffer;
            if(buffer.length()>=4)buffer.erase(0,4);
            if(buffer2.length()>4)buffer2.erase(4);
            String emoteStr = buffer2;
            emoteStr.erase(2);
            const Real time = FFStringToReal(emoteStr.c_str());
            emoteStr = buffer2;
            emoteStr.erase(0,2);
            const unsigned char emote = FFStringToUChar(emoteStr.c_str());
            mEmote.push_back(pair<Real,unsigned char>(time,emote));
        }
        if(buffer.length()>2)                                  //n - n+1: speech time (optional), >=n+2: speech (optional)
        {
            String buffer2 = buffer;
            buffer2.erase(2);
            mSpeechTime = FFStringToReal(buffer2.c_str());
            buffer.erase(0,2);
            mSpeech = buffer;
        }
        return true;
    }
    const String toString()
    {
        String buffer = realToFFString(mDuration);
        buffer += ucharToFFString(mCamMovement);
        buffer += ucharToFFString((unsigned char)mAction.size(),false);
        buffer += ucharToFFString((unsigned char)mEmote.size(),false);
        for(std::vector< std::pair<Real,unsigned char> >::iterator it=mAction.begin(); it!=mAction.end(); it++)
        {
            const pair<Real,unsigned char> action = *it;
            buffer += realToFFString(action.first);
            buffer += ucharToFFString(action.second);
        }
        for(std::vector< std::pair<Real,unsigned char> >::iterator it=mEmote.begin(); it!=mEmote.end(); it++)
        {
            const pair<Real,unsigned char> emote = *it;
            buffer += realToFFString(emote.first);
            buffer += ucharToFFString(emote.second);
        }
        if(mSpeech!="")
        {
            buffer += realToFFString(mSpeechTime);
            buffer += mSpeech;
        }
        return buffer;
    }
    const String realToFFString(const Real &value, const bool &appendZero=true)
    {
        if(value>25.5f)return "ff";
        if(value<=0)return "00";
        const int i = int(value * 10);
        char buffer[4] = "";
        itoa(i,buffer,16);
        return (appendZero&&i<16?"0":"") + String(buffer);
    }
    const String ucharToFFString(const unsigned char &value, const bool &appendZero=true)
    {
        char buffer[4] = "";
        itoa(value,buffer,16);
        return (appendZero&&value<16?"0":"") + String(buffer);
    }
    const Real FFStringToReal(const char *value)
    {
        const int i = strtol(value,0,16);
        return Real(i) * 0.1f;
    }
    const unsigned char FFStringToUChar(const char *value)
    {
        return (unsigned char)strtol(value,0,16);
    }
};

class MovieMaker : public Singleton<MovieMaker>, public ChatListener
{
private:
    Gui *mGui;
    ScreenEffectsManager *mScreenFXMgr;
    CameraManager *mCameraMgr;
    ControlManager *mControlMgr;
    ChatManager *mChatMgr;
    NetworkManager *mNetMgr;
    UnitManager *mUnitMgr;
    NameTagManager *mNameTagMgr;

    std::map<unsigned short,MovieInfo> mMovieList;
    std::map<unsigned int,PlayerData> mPlayerDataList;
    Unit *mPlayer;
    Unit *mActor;
    Unit *mOrigActor;
    MovieClip mMovie;
    MovieClip mPlayingMovie;
    Vector3 mActorOrigPos;
    Real mActorOrigYaw;
    unsigned char mActorOrigAction;
    bool mPlaying;
    Real mPlayTime;
    std::vector<MovieCamMovement*> mCamMovements;
    Overlay *mSpeechOverlay;
    OverlayElement *mSpeechCaption;

    GuiPanel *mPanel;
    GuiSlider *mDurationSlider;
    GuiTextField *mDurationField;
    GuiHighlightList *mCamMovementList;

    GuiSlider *mSpeechTimeSlider;
    GuiTextField *mSpeechTimeField;
    GuiTextArea *mSpeechArea;

    GuiHighlightList *mActionList;
    GuiHighlightList *mActionSelectList;
    OverlayElement *mActionSelectLabel;
    GuiSlider *mActionTimeSlider;
    GuiTextField *mActionTimeField;
    OverlayElement *mActionTimeLabel;
    GuiButton *mActionNewButton;
    GuiButton *mActionDeleteButton;
    pair<Real,unsigned char> *mCurrentAction;

    GuiHighlightList *mEmoteList;
    GuiHighlightList *mEmoteSelectList;
    OverlayElement *mEmoteSelectLabel;
    GuiSlider *mEmoteTimeSlider;
    GuiTextField *mEmoteTimeField;
    OverlayElement *mEmoteTimeLabel;
    GuiButton *mEmoteNewButton;
    GuiButton *mEmoteDeleteButton;
    pair<Real,unsigned char> *mCurrentEmote;
public:
    MovieMaker()
    {
        mGui = Gui::getSingletonPtr();
        mScreenFXMgr = ScreenEffectsManager::getSingletonPtr();
        mCameraMgr = CameraManager::getSingletonPtr();
        mControlMgr = ControlManager::getSingletonPtr();
        mChatMgr = ChatManager::getSingletonPtr();
        mNetMgr = NetworkManager::getSingletonPtr();
        mUnitMgr = UnitManager::getSingletonPtr();
        mNameTagMgr = NameTagManager::getSingletonPtr();

        clear();
    }
    ~MovieMaker()
    {
        while(!mCamMovements.empty())
        {
            MovieCamMovement *camMovement = mCamMovements.back();
            mCamMovements.pop_back();
            delete camMovement;
        }
        clearMovieList();
        mPlayerDataList.clear();
    }
    static MovieMaker* getSingletonPtr();
    static MovieMaker& getSingleton();
    void init()
    {
        mChatMgr->addListener(this);

        mSpeechOverlay = OverlayManager::getSingleton().getByName("MovieScreen");
        mSpeechCaption = OverlayManager::getSingleton().getOverlayElement("MovieScreen/Speech");
        mSpeechOverlay->hide();

        mPanel = mGui->addPanel("MovieMakerScreen");
        mPanel->show(false);
        mDurationSlider = mPanel->getSlider("MovieMakerScreen/Duration");
        mDurationField = mPanel->getTextField("MovieMakerScreen/DurationTF");
        mDurationField->mMaxLength = 4;
        mCamMovementList = mPanel->getHighlightList("MovieMakerScreen/CamMovement");
        for(std::vector<MovieCamMovement*>::iterator it=mCamMovements.begin(); it!=mCamMovements.end(); it++)
        {
            MovieCamMovement *camMovement = *it;
            mCamMovementList->pushLine(camMovement->mName);
        }
        mCamMovementList->setSelection(0);
        mCamMovementList->update(0);

        mSpeechTimeSlider = mPanel->getSlider("MovieMakerScreen/SpeechTime");
        mSpeechTimeField = mPanel->getTextField("MovieMakerScreen/SpeechTimeTF");
        mSpeechTimeField->mMaxLength = 4;
        mSpeechArea = mPanel->getTextArea("MovieMakerScreen/Speech");
        mSpeechArea->mMaxLength = 128;
        mSpeechArea->mAllowNewLine = true;
        mSpeechArea->bindString(&mMovie.mSpeech);

        mActionList = mPanel->getHighlightList("MovieMakerScreen/Action");
        mActionSelectList = mPanel->getHighlightList("MovieMakerScreen/ActionSelect");
        mActionSelectList->pushLine("Sit");
        mActionSelectList->pushLine("Lay");
        mActionSelectList->pushLine("Crouch");
        mActionSelectList->pushLine("Plop Down");
        mActionSelectList->pushLine("Side Lay");
        mActionSelectList->pushLine("Lean");
        mActionSelectList->pushLine("Roll Over");
        mActionSelectList->pushLine("Curl");
        mActionSelectList->pushLine("Headbang");
        mActionSelectList->pushLine("Headswing");
        mActionSelectList->pushLine("Buttswing");
        mActionSelectList->setSelection(0);
        mActionSelectList->update(0);
        mActionSelectLabel = OverlayManager::getSingleton().getOverlayElement("MovieMakerScreen/ActionSelectLabel");
        mActionTimeSlider = mPanel->getSlider("MovieMakerScreen/ActionTime");
        mActionTimeField = mPanel->getTextField("MovieMakerScreen/ActionTimeTF");
        mActionTimeField->mMaxLength = 4;
        mActionTimeLabel = OverlayManager::getSingleton().getOverlayElement("MovieMakerScreen/ActionTimeLabel");
        mActionNewButton = mPanel->getButton("MovieMakerScreen/ActionNew");
        mActionDeleteButton = mPanel->getButton("MovieMakerScreen/ActionDelete");
        mCurrentAction = 0;

        mEmoteList = mPanel->getHighlightList("MovieMakerScreen/Emote");
        mEmoteSelectList = mPanel->getHighlightList("MovieMakerScreen/EmoteSelect");
        EmoteManager::getSingletonPtr()->fillList(mEmoteSelectList);
        mEmoteSelectList->setSelection(0);
        mEmoteSelectList->update(0);
        mEmoteSelectLabel = OverlayManager::getSingleton().getOverlayElement("MovieMakerScreen/EmoteSelectLabel");
        mEmoteTimeSlider = mPanel->getSlider("MovieMakerScreen/EmoteTime");
        mEmoteTimeField = mPanel->getTextField("MovieMakerScreen/EmoteTimeTF");
        mEmoteTimeField->mMaxLength = 4;
        mEmoteTimeLabel = OverlayManager::getSingleton().getOverlayElement("MovieMakerScreen/EmoteTimeLabel");
        mEmoteNewButton = mPanel->getButton("MovieMakerScreen/EmoteNew");
        mEmoteDeleteButton = mPanel->getButton("MovieMakerScreen/EmoteDelete");
        mCurrentEmote = 0;

        updatePanel();
    }
    void clear()
    {
        mPlayer = 0;
        mActor = 0;
        mOrigActor = 0;
        mPlaying = false;
        mPlayTime = 0;
        mMovie = MovieClip();
        mMovie.mDuration = 5;
    }
    void clearMovieList()
    {
        mMovieList.clear();
    }
    void loadCamMovements(const String &filename)
    {
        SaveFile sf(filename);
        for(sf.beginSectionIterator(); sf.hasMoreSections(); sf.nextSection())
        {
            MovieCamMovement *camMovement = new MovieCamMovement(sf.peekNextSectionName());

            for(sf.beginSettingIterator(); sf.hasMoreSettings(); sf.nextSetting())
            {
                const String tag = sf.getSettingTag();
                if(tag=="NoAutoTrack")camMovement->mAutoTrack = !StringConverter::parseBool(sf.getSettingValue());
                else if(tag=="HeadTrack")camMovement->mHeadTrack = StringConverter::parseBool(sf.getSettingValue());
                else
                {
                    const StringVector part = StringUtil::split(sf.getSettingValue(),";",2);
                    if(part.size()>1)camMovement->mTimePos.push_back(pair<Real,Vector3>(StringConverter::parseReal(part[0]),StringConverter::parseVector3(part[1])));
                    if(part.size()>2)camMovement->mRotation.push_back(StringConverter::parseVector3(part[2]));
                    else if(camMovement->mAutoTrack)camMovement->mRotation.push_back(Vector3::ZERO);
                }
            }
            mCamMovements.push_back(camMovement);
        }
    }
    void updatePanel()
    {
        mDurationField->setCaption(StringConverter::toString(mMovie.mDuration));
        mDurationSlider->setValue(mMovie.mDuration/25.5f);

        updateSpeechTime();
        updateActionTimes(mMovie.mDuration);
        updateActionPanel();
        updateActionList();
        updateEmotePanel();
        updateEmoteList();
    }
    void updateSpeechTime()
    {
        mMovie.mSpeechTime = int(mSpeechTimeSlider->getValue()*mMovie.mDuration*10);
        mMovie.mSpeechTime *= 0.1f;
        mSpeechTimeField->setCaption(StringConverter::toString(mMovie.mSpeechTime));
    }
    void updateActionTimes(const Real &prevDuration)
    {
        for(std::vector< std::pair<Real,unsigned char> >::iterator it=mMovie.mAction.begin(); it!=mMovie.mAction.end(); it++)
        {
            pair<Real,unsigned char> *action = &*it;
            if(prevDuration>0)
            {
                const Real ratio = action->first/prevDuration;
                action->first = int(ratio*mMovie.mDuration*10);
                action->first *= 0.1f;
            }
            else action->first = 0;
        }
        updateActionTimeSlider();
        updateActionList();
    }
    void updateActionTimeSlider()
    {
        const short selection = mActionList->getSelectedLine();
        if(selection<0 || selection>=(int)mMovie.mAction.size())return;
        const Real value = mMovie.mAction[selection].first;
        mActionTimeSlider->setValue( (mMovie.mDuration>0?(value/mMovie.mDuration):0) );
        mActionTimeField->setCaption(StringConverter::toString(value));
    }
    void updateActionPanel()
    {
        const short selection = mActionList->getSelectedLine();
        const bool doShow = (selection>=0 && selection<(int)mMovie.mAction.size());
        mActionDeleteButton->show(doShow);
        mActionSelectList->show(doShow);
        if(doShow && mCurrentAction)
        {
            mActionSelectList->setSelection(mCurrentAction->second);
            mActionSelectList->update();
        }
        mActionTimeSlider->show(doShow);
        mActionTimeField->show(doShow);
        if(doShow && mCurrentAction)
        {
            mActionTimeSlider->setValue(mMovie.mDuration>0?mCurrentAction->first/mMovie.mDuration:0);
            mActionTimeField->setCaption(StringConverter::toString(mCurrentAction->first));
        }
        if(doShow)
        {
            mActionSelectLabel->show();
            mActionTimeLabel->show();
        }
        else
        {
            mActionSelectLabel->hide();
            mActionTimeLabel->hide();
        }
        mActionNewButton->show((int)mMovie.mAction.size()<MOVIE_MAXACTIONS);
    }
    void newAction()
    {
        if((int)mMovie.mAction.size()>=MOVIE_MAXACTIONS)return;
        if(mCurrentAction)mMovie.mAction.push_back(*mCurrentAction);
        else mMovie.mAction.push_back(pair<Real,unsigned char>(0,0));
        mCurrentAction = &mMovie.mAction.back();
        updateActionList();
        mActionList->setSelection((int)mActionList->getNumLines()-1,true);
        updateActionPanel();
    }
    void deleteAction()
    {
        const short selection = mActionList->getSelectedLine();
        if(selection<0 || selection>=(int)mMovie.mAction.size())return;
        short line = 0;
        for(std::vector< std::pair<Real,unsigned char> >::iterator it=mMovie.mAction.begin(); it!=mMovie.mAction.end(); it++)
        {
            const pair<Real,unsigned char> action = *it;
            if(line==selection)
            {
                it = mMovie.mAction.erase(it);
                if(it!=mMovie.mAction.end())mCurrentAction = &*it;
                else if(!mMovie.mAction.empty())mCurrentAction = &mMovie.mAction.back();
                else mCurrentAction = 0;
                break;
            }
            else line++;
        }
        updateActionList();
        updateActionPanel();
    }
    void updateActionList()
    {
        const short selection = mActionList->getSelectedLine();
        mActionList->clear();

        for(std::vector< std::pair<Real,unsigned char> >::iterator it=mMovie.mAction.begin(); it!=mMovie.mAction.end(); it++)
        {
            const pair<Real,unsigned char> action = *it;
            mActionList->pushLine(mActionSelectList->getLine(action.second)+" ("+StringConverter::toString(action.first)+")");
        }

        if(selection>=0)
        {
            if(selection>=mActionList->getNumLines())mActionList->setSelection(mActionList->getNumLines()-1,true);
            else mActionList->setSelection(selection,true);
        }
        mActionList->update();
    }
    void updateEmoteTimes(const Real &prevDuration)
    {
        for(std::vector< std::pair<Real,unsigned char> >::iterator it=mMovie.mEmote.begin(); it!=mMovie.mEmote.end(); it++)
        {
            pair<Real,unsigned char> *emote = &*it;
            if(prevDuration>0)
            {
                const Real ratio = emote->first/prevDuration;
                emote->first = int(ratio*mMovie.mDuration*10);
                emote->first *= 0.1f;
            }
            else emote->first = 0;
        }
        updateEmoteTimeSlider();
        updateEmoteList();
    }
    void updateEmoteTimeSlider()
    {
        const short selection = mEmoteList->getSelectedLine();
        if(selection<0 || selection>=(int)mMovie.mEmote.size())return;
        const Real value = mMovie.mEmote[selection].first;
        mEmoteTimeSlider->setValue( (mMovie.mDuration>0?(value/mMovie.mDuration):0) );
        mEmoteTimeField->setCaption(StringConverter::toString(value));
    }
    void updateEmotePanel()
    {
        const short selection = mEmoteList->getSelectedLine();
        const bool doShow = (selection>=0 && selection<(int)mMovie.mEmote.size());
        mEmoteDeleteButton->show(doShow);
        mEmoteSelectList->show(doShow);
        if(doShow && mCurrentEmote)
        {
            mEmoteSelectList->setSelection(mCurrentEmote->second);
            mEmoteSelectList->update();
        }
        mEmoteTimeSlider->show(doShow);
        mEmoteTimeField->show(doShow);
        if(doShow && mCurrentEmote)
        {
            mEmoteTimeSlider->setValue(mMovie.mDuration>0?mCurrentEmote->first/mMovie.mDuration:0);
            mEmoteTimeField->setCaption(StringConverter::toString(mCurrentEmote->first));
        }
        if(doShow)
        {
            mEmoteSelectLabel->show();
            mEmoteTimeLabel->show();
        }
        else
        {
            mEmoteSelectLabel->hide();
            mEmoteTimeLabel->hide();
        }
        mEmoteNewButton->show((int)mMovie.mEmote.size()<MOVIE_MAXEMOTES);
    }
    void newEmote()
    {
        if((int)mMovie.mEmote.size()>=MOVIE_MAXEMOTES)return;
        if(mCurrentEmote)mMovie.mEmote.push_back(*mCurrentEmote);
        else mMovie.mEmote.push_back(pair<Real,unsigned char>(0,0));
        mCurrentEmote = &mMovie.mEmote.back();
        updateEmoteList();
        mEmoteList->setSelection((int)mEmoteList->getNumLines()-1,true);
        updateEmotePanel();
    }
    void deleteEmote()
    {
        const short selection = mEmoteList->getSelectedLine();
        if(selection<0 || selection>=(int)mMovie.mEmote.size())return;
        short line = 0;
        for(std::vector< std::pair<Real,unsigned char> >::iterator it=mMovie.mEmote.begin(); it!=mMovie.mEmote.end(); it++)
        {
            const pair<Real,unsigned char> Emote = *it;
            if(line==selection)
            {
                it = mMovie.mEmote.erase(it);
                if(it!=mMovie.mEmote.end())mCurrentEmote = &*it;
                else if(!mMovie.mEmote.empty())mCurrentEmote = &mMovie.mEmote.back();
                else mCurrentEmote = 0;
                break;
            }
            else line++;
        }
        updateEmoteList();
        updateEmotePanel();
    }
    void updateEmoteList()
    {
        const short selection = mEmoteList->getSelectedLine();
        mEmoteList->clear();

        for(std::vector< std::pair<Real,unsigned char> >::iterator it=mMovie.mEmote.begin(); it!=mMovie.mEmote.end(); it++)
        {
            const pair<Real,unsigned char> Emote = *it;
            mEmoteList->pushLine(mEmoteSelectList->getLine(Emote.second)+" ("+StringConverter::toString(Emote.first)+")");
        }

        if(selection>=0)
        {
            if(selection>=mEmoteList->getNumLines())mEmoteList->setSelection(mEmoteList->getNumLines()-1,true);
            else mEmoteList->setSelection(selection,true);
        }
        mEmoteList->update();
    }
    void buttonClicked(GuiComponent *button)
    {
        if(button==mActionNewButton)newAction();
        else if(button==mActionDeleteButton)deleteAction();
        else if(button==mEmoteNewButton)newEmote();
        else if(button==mEmoteDeleteButton)deleteEmote();
        else if(button->nameIs("MovieMakerScreen/Test"))
        {
            if(mPlayer)
            {
                MovieInfo movie;
                movie.mMovie = mMovie.toString();
                movie.mPosition = mPlayer->getPosition();
                movie.mCamYaw = mCameraMgr->getYaw();
                movie.mYaw = mPlayer->getYaw();
                movie.mHeadNS = mPlayer->getHeadNSState();
                movie.mHeadEW = mPlayer->getHeadEWState();
                movie.mActorCharID = mPlayer->mCharID;
                movie.mActorName = mPlayer->mUsername;
                PlayerData playerData(mPlayer);
                playMovie(movie,playerData);
            }
        }
        else if(button->nameIs("MovieMakerScreen/Publish"))mGui->showAlertBox("Publish movie clip?","MovieMaker/Publish",true);
    }
    void changeEvent(GuiComponent *component, const unsigned char &event)
    {
        if(component==mDurationSlider)
        {
            const Real prevDuration = mMovie.mDuration;
            mMovie.mDuration = int(mDurationSlider->getValue()*255);
            mMovie.mDuration *= 0.1f;
            mDurationField->setCaption(StringConverter::toString(mMovie.mDuration));
            updateSpeechTime();
            updateActionTimes(prevDuration);
            updateEmoteTimes(prevDuration);
        }
        else if(component==mDurationField)
        {
            const Real prevDuration = mMovie.mDuration;
            const Real value = StringConverter::parseReal(mDurationField->getCaption());
            if(value>25.5f)mMovie.mDuration = 25.5f;
            else if(value<0)mMovie.mDuration = 0;
            else
            {
                mMovie.mDuration = int(value*10);
                mMovie.mDuration *= 0.1f;
            }
            mDurationField->setCaption(StringConverter::toString(mMovie.mDuration));
            mDurationSlider->setValue(mMovie.mDuration/25.5f);
            updateSpeechTime();
            updateActionTimes(prevDuration);
            updateEmoteTimes(prevDuration);
        }
        else if(component==mCamMovementList)mMovie.mCamMovement = mCamMovementList->getSelectedLine();
        else if(component==mSpeechTimeSlider)updateSpeechTime();
        else if(component==mSpeechTimeField)
        {
            const Real value = StringConverter::parseReal(mSpeechTimeField->getCaption());
            if(value>mMovie.mDuration)mMovie.mSpeechTime = mMovie.mDuration;
            else if(value<0)mMovie.mSpeechTime = 0;
            else
            {
                mMovie.mSpeechTime = int(value*10);
                mMovie.mSpeechTime *= 0.1f;
            }
            mSpeechTimeField->setCaption(StringConverter::toString(mMovie.mSpeechTime));
            mSpeechTimeSlider->setValue( (mMovie.mDuration>0?(mMovie.mSpeechTime/mMovie.mDuration):0) );
        }
        else if(component==mActionTimeSlider)
        {
            if(mCurrentAction)
            {
                mCurrentAction->first = int(mActionTimeSlider->getValue()*mMovie.mDuration*10);
                mCurrentAction->first *= 0.1f;
                mActionTimeField->setCaption(StringConverter::toString(mCurrentAction->first));
                updateActionList();
            }
        }
        else if(component==mActionTimeField)
        {
            if(mCurrentAction)
            {
                const Real value = StringConverter::parseReal(mActionTimeField->getCaption());
                if(value>mMovie.mDuration)mCurrentAction->first = mMovie.mDuration;
                else if(value<0)mCurrentAction->first = 0;
                else
                {
                    mCurrentAction->first = int(value*10);
                    mCurrentAction->first *= 0.1f;
                }
                mActionTimeField->setCaption(StringConverter::toString(mCurrentAction->first));
                mActionTimeSlider->setValue( (mMovie.mDuration>0?(mCurrentAction->first/mMovie.mDuration):0) );
                updateActionList();
            }
        }
        else if(component==mActionSelectList)
        {
            if(mCurrentAction)
            {
                mCurrentAction->second = mActionSelectList->getSelectedLine();
                updateActionList();
            }
        }
        else if(component==mActionList)
        {
            const short selection = mActionList->getSelectedLine();
            if(selection<0 || selection>=(int)mMovie.mAction.size())return;
            mCurrentAction = &mMovie.mAction[selection];
            updateActionPanel();
        }
        else if(component==mEmoteTimeSlider)
        {
            if(mCurrentEmote)
            {
                mCurrentEmote->first = int(mEmoteTimeSlider->getValue()*mMovie.mDuration*10);
                mCurrentEmote->first *= 0.1f;
                mEmoteTimeField->setCaption(StringConverter::toString(mCurrentEmote->first));
                updateEmoteList();
            }
        }
        else if(component==mEmoteTimeField)
        {
            if(mCurrentEmote)
            {
                const Real value = StringConverter::parseReal(mEmoteTimeField->getCaption());
                if(value>mMovie.mDuration)mCurrentEmote->first = mMovie.mDuration;
                else if(value<0)mCurrentEmote->first = 0;
                else
                {
                    mCurrentEmote->first = int(value*10);
                    mCurrentEmote->first *= 0.1f;
                }
                mEmoteTimeField->setCaption(StringConverter::toString(mCurrentEmote->first));
                mEmoteTimeSlider->setValue( (mMovie.mDuration>0?(mCurrentEmote->first/mMovie.mDuration):0) );
                updateEmoteList();
            }
        }
        else if(component==mEmoteSelectList)
        {
            if(mCurrentEmote)
            {
                mCurrentEmote->second = mEmoteSelectList->getSelectedLine();
                updateEmoteList();
            }
        }
        else if(component==mEmoteList)
        {
            const short selection = mEmoteList->getSelectedLine();
            if(selection<0 || selection>=(int)mMovie.mEmote.size())return;
            mCurrentEmote = &mMovie.mEmote[selection];
            updateEmotePanel();
        }
    }
    void setPlayer(Unit *unit)
    {
        mPlayer = unit;
    }
    const bool isPlaying()
    {
        return mPlaying;
    }
    void playMovie(const MovieInfo &movie, PlayerData &playerData)
    {
        if(!mPlayingMovie.fromString(movie.mMovie))return;
        if(mPlaying)stopMovie();
        mActor = mUnitMgr->createUnit(playerData,movie.mActorName);
        //Place actor in movie position
        mActor->setPosition(movie.mPosition);
        mActor->setYaw(movie.mYaw,true);
        mActor->setHeadNSState(movie.mHeadNS,true);
        mActor->setHeadEWState(movie.mHeadEW,true);
        WaterManager::getSingletonPtr()->updateWaterCollision(mActor);
        mUnitMgr->initUnitPosition(mActor,!mActor->mAntiGravity);
        //Hide original player
        mOrigActor = mUnitMgr->getUnitByCharID(movie.mActorCharID);
        if(mOrigActor)mOrigActor->forceHide(true);
        //Hide GUI
        mControlMgr->disable(true);
        mNameTagMgr->showNameTags(false,true);
        mGui->showAll(false);

        startCameraTrack(mActor->getPosition(),movie.mCamYaw);
        mPlaying = true;
        mPlayTime = 0;
        mScreenFXMgr->doFadeIn(0.25f,ColourValue::Black);

        //Start Action
        for(std::vector< std::pair<Real,unsigned char> >::iterator it=mPlayingMovie.mAction.begin(); it!=mPlayingMovie.mAction.end(); it++)
        {
            const pair<Real,unsigned char> action = *it;
            if(action.first==0)setActorAction(action.second,true);
        }
        //Start Emote
        for(std::vector< std::pair<Real,unsigned char> >::iterator it=mPlayingMovie.mEmote.begin(); it!=mPlayingMovie.mEmote.end(); it++)
        {
            const pair<Real,unsigned char> emote = *it;
            if(emote.first==0)setActorEmote(emote.second,true);
        }
    }
    void stopMovie()
    {
        mPlaying = false;
        mSpeechOverlay->hide();
        mCameraMgr->destroyCameraTrack();
        mControlMgr->disable(false);
        if(mNameTagMgr->getShowNameTags())mNameTagMgr->showNameTags(true);
        mGui->showAll(true);
        if(mActor)
        {
            mUnitMgr->destroyUnit(mActor);
            mActor = 0;
            //Show original player
            if(mOrigActor)
            {
                mOrigActor->forceHide(false);
                mOrigActor = 0;
            }
        }
        mScreenFXMgr->doFadeIn(0.25f,ColourValue::Black);
    }
    void update(const Real &timeElapsed)
    {
        if(!mPlaying)return;
        const Real prevTime = mPlayTime;
        mPlayTime += timeElapsed;
        if(mPlayTime>=mPlayingMovie.mDuration)
        {
            stopMovie();
            return;
        }
        //Speech
        if(prevTime<=mPlayingMovie.mSpeechTime && mPlayTime>=mPlayingMovie.mSpeechTime)
        {
            mSpeechOverlay->show();
            Font *font = dynamic_cast<Font*>(FontManager::getSingleton().getByName(mSpeechCaption->getParameter("font_name")).getPointer());
            const Real charHeight = StringConverter::parseReal(mSpeechCaption->getParameter("char_height"));//(*mGui->mWindowHeight);
            String speech = mPlayingMovie.mSpeech;
            GuiTextField::wrapCaption(speech,font,charHeight,mSpeechCaption->getWidth());
            mSpeechCaption->setCaption(speech);
            mActor->setLipSyncSpeech(mPlayingMovie.mSpeech);
        }
        //Action
        for(std::vector< std::pair<Real,unsigned char> >::iterator it=mPlayingMovie.mAction.begin(); it!=mPlayingMovie.mAction.end(); it++)
        {
            const pair<Real,unsigned char> action = *it;
            if(prevTime<action.first && mPlayTime>=action.first)setActorAction(action.second);
        }
        //Emote
        for(std::vector< std::pair<Real,unsigned char> >::iterator it=mPlayingMovie.mEmote.begin(); it!=mPlayingMovie.mEmote.end(); it++)
        {
            const pair<Real,unsigned char> emote = *it;
            if(prevTime<emote.first && mPlayTime>=emote.first)setActorEmote(emote.second);
        }
    }
    void startCameraTrack(const Vector3 &pos, const Real &camYaw)
    {
        if(mPlayingMovie.mCamMovement>=(int)mCamMovements.size())return;
        mCameraMgr->createCameraTrack(mPlayingMovie.mDuration);

        MovieCamMovement *camMovement = mCamMovements[mPlayingMovie.mCamMovement];
        std::vector<Vector3>::iterator rit=camMovement->mRotation.begin();
        for(std::vector<std::pair<Real,Vector3> >::iterator it=camMovement->mTimePos.begin(); it!=camMovement->mTimePos.end(); it++)
        {
            const pair<Real,Vector3> timePos = *it;
            if(rit!=camMovement->mRotation.end())
            {
                const Vector3 rot = *rit;
                mCameraMgr->addCameraTrackKeyFrame(timePos.first*mPlayingMovie.mDuration,pos+mCameraMgr->pyr(0,camYaw,0)*timePos.second,mCameraMgr->pyr(rot.x,rot.y+camYaw,rot.z));
            }
            else mCameraMgr->addCameraTrackKeyFrame(timePos.first*mPlayingMovie.mDuration,pos+timePos.second);
            if(rit!=camMovement->mRotation.end())rit++;
        }

        mCameraMgr->doCameraTrack(true,camMovement->mAutoTrack,camMovement->mAutoTrack?mActor->getSceneNode():0,camMovement->mHeadTrack?mActor->getHeadPosition(true):Vector3::ZERO);
    }
    void setActorAction(const unsigned char &action, const bool &instant=false)
    {
        switch(action)
        {
            case 0: mActor->setAction(Unit::ACT_SIT,instant); break;
            case 1: mActor->setAction(Unit::ACT_LAY,instant); break;
            case 2: mActor->setAction(Unit::ACT_CROUCH,instant); break;
            case 3: mActor->setAction(Unit::ACT_PLOP,instant); break;
            case 4: mActor->setAction(Unit::ACT_SIDELAY,instant); break;
            case 5: mActor->setAction(Unit::ACT_LEAN,instant); break;
            case 6: mActor->setAction(Unit::ACT_OVERTURN,instant); break;
            case 7: mActor->setAction(Unit::ACT_CURL,instant); break;
            case 8: mActor->setAction(Unit::ACT_DANCE1,instant); break;
            case 9: mActor->setAction(Unit::ACT_DANCE2,instant); break;
            case 10: mActor->setAction(Unit::ACT_DANCE3,instant); break;
            default: break;
        }
    }
    void setActorEmote(const unsigned char &emote, const bool &instant=false)
    {
        mActor->setEmote(emote,!instant);
    }
    const bool chatLineClicked(const ClickableLine &line)
    {
        if(line.mType!=CHAT_MOVIE)return false;

        std::map<unsigned short,MovieInfo>::iterator it = mMovieList.find(line.mIndex);
        if(it!=mMovieList.end())
        {
            const MovieInfo movie = it->second;
            mGui->showAlertBox("Play "+movie.mActorName+"'s movie?","MovieMaker/Play"+StringConverter::toString(line.mIndex),true);
        }
        else mGui->showAlertBox("Could not find movie. "+StringConverter::toString(line.mIndex));
        return true;
    }
    const bool chatCommand(const String &command){return false;}
    const bool chatLocal(const String &caption){return false;}
    void alertBoxEvent(const String &name, const bool &flag)
    {
        if(!flag)return;
        if(name=="MovieMaker/Publish")
        {
            const String caption = mMovie.toString();
            const Real camYaw = mCameraMgr->getYaw();
            mNetMgr->sendMovie(caption,camYaw);
            if(mPlayer)pushMovie(caption,mPlayer,camYaw);
        }
        else if(StringUtil::startsWith(name,"MovieMaker/Play",false))
        {
            String buffer = name;
            buffer.erase(0,15);
            //Find movie
            std::map<unsigned short,MovieInfo>::iterator it = mMovieList.find(StringConverter::parseInt(buffer));
            if(it!=mMovieList.end())
            {
                const MovieInfo movie = it->second;
                //Find actor's PlayerData
                std::map<unsigned int,PlayerData>::iterator jt = mPlayerDataList.find(movie.mActorCharID);
                if(jt!=mPlayerDataList.end())playMovie(movie,jt->second);
            }
        }
    }
    void pushMovie(const String &movie, Unit *actor, const Real &camYaw)
    {
        const unsigned short line = mChatMgr->pushChatLine("",actor->mUsername,CHAT_MOVIE,actor->mCharID);
        MovieInfo movieInfo;
        movieInfo.mMovie = movie;
        movieInfo.mActorName = actor->mUsername;
        movieInfo.mActorCharID = actor->mCharID;
        movieInfo.mCamYaw = camYaw;
        movieInfo.mPosition = actor->getPosition();
        movieInfo.mYaw = actor->getYaw();
        movieInfo.mHeadNS = actor->getHeadNSState();
        movieInfo.mHeadEW = actor->getHeadEWState();
        mMovieList[line] = movieInfo;
        mPlayerDataList[actor->mCharID] = PlayerData(actor);

        //mDebug = StringConverter::toString(line);
    }
    //String mDebug;
    GuiPanel* getPanel()
    {
        return mPanel;
    }
};

template<> MovieMaker* Singleton<MovieMaker>::ms_Singleton = 0;

MovieMaker* MovieMaker::getSingletonPtr()
{
	return ms_Singleton;
}

MovieMaker& MovieMaker::getSingleton()
{
	assert(ms_Singleton);
	return *ms_Singleton;
}

#endif
