#ifndef _LIPSYNCMANAGER_H_
#define _LIPSYNCMANAGER_H_

#define LIPSYNC_NUMEMOTES 6

struct LipSync
{
    unsigned short mToken;
    String mSpeech;
    Real mTimer;
    char mStartLetter;
    char mEndLetter;
    int mUpdatePointer;
    Real mEmoteFadeOutWeight[LIPSYNC_NUMEMOTES];
    queue<pair<unsigned short,Real> > mSpeedModifiers;
    Real mSpeed;
    LipSync(const unsigned short &token, const String &speech)
    {
        mToken = token;
        mSpeech = speech;
        mTimer = 0;
        mStartLetter = 0;
        mEndLetter = 0;
        mUpdatePointer = -1;
        mSpeed = 1;

        //Process speech
        StringUtil::toLowerCase(mSpeech);
        for(int i=0;i<(int)mSpeech.length();i++)
        {
            switch(mSpeech[i])
            {
                case 'c':
                    //ch
                    if(i+1<(int)mSpeech.length() && mSpeech[i+1]=='h')
                    {
                        mSpeech[i] = 'U';
                        mSpeech.erase(i+1,1);
                    }
                    break;
                case 's':
                    //sh
                    if(i+1<(int)mSpeech.length() && mSpeech[i+1]=='h')
                    {
                        mSpeech[i] = 'U';
                        mSpeech.erase(i+1,1);
                    }
                    break;
                case 'h':
                case 'k':
                case 'q':
                    //h/k/q before e
                    if(i+1<(int)mSpeech.length() && mSpeech[i+1]=='e')
                    {
                        mSpeech[i] = 'E';
                        mSpeech.erase(i+1,1);
                    }
                    //silent h/k/q
                    else mSpeech.erase(i,1);
                    break;
                case 'u':
                    //u after o/t/d
                    if(i>0 && (mSpeech[i-1]=='o'||mSpeech[i-1]=='t'||mSpeech[i-1]=='d'))mSpeech[i] = 'U';
                    //u before e
                    else if(i+1<(int)mSpeech.length() && mSpeech[i+1]=='e')
                    {
                        mSpeech[i] = 'U';
                        mSpeech.erase(i+1,1);
                    }
                    break;
                case 'y':
                    //first y or y after e
                    if(i==0 || mSpeech[i-1]==' ' || mSpeech[i-1]=='e')mSpeech[i] = 'e';
                    break;
                case 'e':
                    //ea
                    if(i+1<(int)mSpeech.length() && mSpeech[i+1]=='a')
                    {
                        mSpeech[i] = 'E';
                        mSpeech.erase(i+1,1);
                    }
                    //last e (but not 2 lettered word)
                    else if((i+1>=(int)mSpeech.length()||isEndOfWord(mSpeech[i+1])) && i>1 && !isEndOfWord(mSpeech[i-2]))mSpeech.erase(i,1);
                    break;
                case 'w':
                    //first w
                    if(i==0 || mSpeech[i-1]==' ')mSpeech[i] = 'U';
                    else if(i>0)
                    {
                        //ow
                        if(mSpeech[i-1]=='o')
                        {
                            mSpeech[i-1] = 'a';
                            mSpeech[i] = 'o';
                        }
                        //aw
                        else if(mSpeech[i-1]=='a')
                        {
                            mSpeech[i-1] = 'O';
                            mSpeech.erase(i,1);
                        }
                    }
                    break;
                case 'r':
                    //first r
                    if(i==0 || isEndOfWord(mSpeech[i-1]))mSpeech[i] = 'R';
                    break;
                case 'd':
                    //lood
                    if(i>2 && mSpeech[i-3]=='l'&&mSpeech[i-2]=='o'&&mSpeech[i-1]=='o')
                    {
                        mSpeech[i-2] = 'a';
                        mSpeech.erase(i-1,1);
                    }
                    break;

                //punctuations
                case '\'':
                    mSpeech.erase(i,1);
                    break;

                //modifiers
                case '(':
                    {
                        const size_t index = mSpeech.find_first_of(')');
                        if(index!=string::npos)
                        {
                            const Real speed = StringConverter::parseReal(mSpeech.substr(i+1,index-i));
                            if(speed>0)
                            {
                                mSpeedModifiers.push(pair<unsigned short,Real>(i,speed));
                                mSpeech.erase(i,index-i+1);
                            }
                        }
                    }
                    break;

                default: break;
            }
        }
    }
    const bool isEndOfWord(const char &c)
    {
        return (c==' '||c=='.'||c==','||c=='!'||c=='?'||c=='('||c==')'||c==';'||c==':');
    }
    const bool update(Entity *ent, const Real &timeElapsed)
    {
        const unsigned short pointer = (int)mTimer;
        if(pointer>(int)mSpeech.length())
        {
            if(pointer>(int)mSpeech.length()+1)
            {
                fadeOutLipSyncEmotes(ent,1,true);
                return true;
            }
            else
            {
                if(mUpdatePointer<pointer)
                {
                    stop(ent);
                    mUpdatePointer++;
                }
                fadeOutLipSyncEmotes(ent,mTimer-pointer,true);
            }
        }
        else
        {
            if(!mSpeedModifiers.empty())
            {
                const pair<unsigned short,Real> speedModifier = mSpeedModifiers.front();
                if(pointer >= speedModifier.first)
                {
                    mSpeed = speedModifier.second;
                    mSpeedModifiers.pop();
                }
            }
            animateFor(ent,pointer,mTimer-pointer);
        }

        mTimer += timeElapsed*10*mSpeed;
        return false;
    }
    void animateFor(Entity *ent, const unsigned short &pointer, const Real &time)
    {
        if(mUpdatePointer<pointer)
        {
            if(mUpdatePointer==-1)setEmoteFadeOutWeights(ent);
            else if(mUpdatePointer==0)fadeOutLipSyncEmotes(ent,1);
            mUpdatePointer++;
            updateLetterState(ent,1);
            mStartLetter = pointer>0?mSpeech[pointer-1]:0;
            mEndLetter = pointer<(int)mSpeech.length()?mSpeech[pointer]:0;
            return;
        }
        if(mUpdatePointer==0)fadeOutLipSyncEmotes(ent,time);
        updateLetterState(ent,time);
    }
    void stop(Entity *ent)
    {
        updateLetterState(ent,1);
    }
    void updateLetterState(Entity *ent, const Real &ratio)
    {
        const Real invRatio = 1-ratio;
        const String anim[LIPSYNC_NUMEMOTES] = {"JawOpen","ShowTeeth","LipShrink","TongueUp","BiteLip","ShowTongue"};
        Real weight[2][LIPSYNC_NUMEMOTES];
        char letter[2];
        letter[0] = mStartLetter;
        letter[1] = mEndLetter;
        for(int i=0;i<2;i++)
        {
            for(int j=0;j<LIPSYNC_NUMEMOTES;j++)weight[i][j] = 0;
            switch(letter[i])
            {
                //ah
                case 'a':
                case 'u':
                    weight[i][0] = 0.5f;
                    break;
                //ey
                case 'e':
                    weight[i][0] = 0.25f;
                    weight[i][1] = 0.7f;
                    break;
                //ee
                case 's':
                case 'x':
                case 'z':
                    weight[i][1] = 0.7f;
                    break;
                //eh
                case 'E':
                    weight[i][0] = 0.4f;
                    weight[i][1] = 0.7f;
                    break;
                //ih
                case 'i':
                case 'y':
                    weight[i][0] = 0.25f;
                    weight[i][1] = 0.25f;
                    break;
                //oh
                case 'o':
                    weight[i][0] = 0.25f;
                    weight[i][2] = 1;
                    break;
                //aw
                case 'O':
                    weight[i][0] = 0.5f;
                    weight[i][2] = 1;
                    break;
                //oo
                case 'U':
                    weight[i][0] = 0.1f;
                    weight[i][2] = 1;
                    break;
                //t
                case 'd':
                case 'n':
                case 'r':
                case 't':
                    weight[i][0] = 0.15f;
                    weight[i][1] = 0.25f;
                    weight[i][3] = 0.2f;
                    break;
                //t
                case 'R':
                    weight[i][0] = 0.15f;
                    weight[i][1] = 0.25f;
                    weight[i][2] = 0.5f;
                    weight[i][3] = 0.2f;
                    break;
                //f
                case 'f':
                case 'v':
                    weight[i][4] = 1;
                    break;
                //l
                case 'l':
                    weight[i][0] = 0.1f;
                    weight[i][1] = 0.3f;
                    weight[i][5] = 0.25f;
                    break;
                //m
                case 'b':
                case 'm':
                case 'p':
                default:
                    break;
            }
        }

        for(int j=0;j<LIPSYNC_NUMEMOTES;j++)
        {
            AnimationState *state = ent->getAnimationState(anim[j]);
            if(weight[0][j]>0)
            {
                //Fade only if next letter doesn't have this anim
                if(weight[1][j]<=0)
                {
                    if(invRatio>0)
                    {
                        state->setEnabled(true);
                        state->setLoop(false);
                        state->setTimePosition(weight[0][j] * invRatio * state->getLength());
                    }
                    else state->setEnabled(false);
                }
            }
            if(weight[1][j]>0)
            {
                //Previous letter had this anim, do transit
                if(weight[0][j]>0)
                {
                    state->setEnabled(true);
                    state->setLoop(false);
                    state->setTimePosition((weight[0][j]+(weight[1][j]-weight[0][j])*ratio) * state->getLength());
                }
                else
                {
                    if(ratio>0)
                    {
                        state->setEnabled(true);
                        state->setLoop(false);
                        state->setTimePosition(weight[1][j] * ratio * state->getLength());
                    }
                    else state->setEnabled(false);
                }
            }
        }
    }
    void setEmoteFadeOutWeights(Entity *ent)
    {
        const String anim[LIPSYNC_NUMEMOTES] = {"JawOpen","ShowTeeth","LipShrink","TongueUp","BiteLip","ShowTongue"};
        for(int i=0;i<LIPSYNC_NUMEMOTES;i++)
        {
            AnimationState *state = ent->getAnimationState(anim[i]);
            if(state->getEnabled() && state->getLength()>0)mEmoteFadeOutWeight[i] = state->getTimePosition()/state->getLength();
            else mEmoteFadeOutWeight[i] = 0;
        }
    }
    void fadeOutLipSyncEmotes(Entity *ent, const Real &fadeRatio, const bool &resume=false)
    {
        const Real ratio = resume?fadeRatio:(1-fadeRatio);
        const String anim[LIPSYNC_NUMEMOTES] = {"JawOpen","ShowTeeth","LipShrink","TongueUp","BiteLip","ShowTongue"};
        for(int i=0;i<LIPSYNC_NUMEMOTES;i++)
        {
            AnimationState *state = ent->getAnimationState(anim[i]);
            if(ratio>0)
            {
                state->setEnabled(true);
                state->setLoop(false);
                state->setTimePosition(mEmoteFadeOutWeight[i] * ratio * state->getLength());
            }
            else state->setEnabled(false);
        }
    }
};

class LipSyncManager : public Singleton<LipSyncManager>
{
private:
    std::vector<LipSync*> mLipSyncs;
public:
    LipSyncManager()
    {
        mLipSyncs.clear();
    }
    ~LipSyncManager()
    {
        while(!mLipSyncs.empty())
        {
            LipSync *ls = mLipSyncs.back();
            mLipSyncs.pop_back();
            delete ls;
        }
    }
    static LipSyncManager* getSingletonPtr();
    static LipSyncManager& getSingleton();
    const unsigned short pushLipSync(const String &speech)
    {
        unsigned short token = 1;
        for(std::vector<LipSync*>::iterator it=mLipSyncs.begin(); it!=mLipSyncs.end(); it++)
        {
            LipSync *ls = *it;
            if(ls->mToken>token)
            {
                mLipSyncs.insert(it,new LipSync(token,speech));
                return token;
            }
            token++;
        }
        mLipSyncs.push_back(new LipSync(token,speech));
        return token;
    }
    void updateLipSync(unsigned short *token, Entity *ent, const Real &timeElapsed)
    {
        const unsigned short tokenVal = *token;
        for(std::vector<LipSync*>::iterator it=mLipSyncs.begin(); it!=mLipSyncs.end(); it++)
        {
            LipSync *ls = *it;
            if(ls->mToken==tokenVal)
            {
                if(ls->update(ent,timeElapsed))
                {
                    *token = 0;
                    mLipSyncs.erase(it);
                    delete ls;
                }
                return;
            }
        }
    }
    void stopLipSync(const unsigned short &token, Entity *ent=0)
    {
        for(std::vector<LipSync*>::iterator it=mLipSyncs.begin(); it!=mLipSyncs.end(); it++)
        {
            LipSync *ls = *it;
            if(ls->mToken==token)
            {
                if(ent)
                {
                    ls->stop(ent);
                    ls->fadeOutLipSyncEmotes(ent,1,true);
                }
                mLipSyncs.erase(it);
                delete ls;
                return;
            }
        }
    }
};

template<> LipSyncManager* Singleton<LipSyncManager>::ms_Singleton = 0;

LipSyncManager* LipSyncManager::getSingletonPtr()
{
	return ms_Singleton;
}

LipSyncManager& LipSyncManager::getSingleton()
{
	assert(ms_Singleton);
	return *ms_Singleton;
}

#endif

