#ifndef _UNIT_H_
#define _UNIT_H_

#include <Controllable.h>
#include <EmoteManager.h>
#include <MarkingManager.h>
#include <SoundManager.h>
#include <LipSyncManager.h>
#include <ItemsManager.h>

#define NUM_SCALABLES 10
#define NUM_COLOURABLES 10
#define MAX_MESHES 5
#define MAX_MATS 4
#define JUMP_LIMIT 0.2f

struct AnimData
{
    String mName;
    Real mSpeed;
    bool mLoop;
    bool mReset;
    Real mTransitTime;
    Real m_fStartTime;
    AnimData(const String &name="", const Real &speed=1, const bool &loop=false, const bool &resetTime=true, const Real &transitTime=0.1f, const Real &fStartTime=0.0f)
    {
        mName = name;
        mSpeed = speed;
        mLoop = loop;
        mReset = resetTime;
        mTransitTime = transitTime;
        m_fStartTime = fStartTime;
    }
};

class AnimGroupListener
{
public:
    virtual const bool animEnded(const bool &interrupted) = 0;
};

class AnimGroup
{
private:
    std::vector<AnimatedAnim*> mAnims;
    std::vector<AnimData> mAnimQueue;
    String mCurrentAnim;
    AnimData mNextAnim;
    Real mTransitionTimer;
    Real mTransitionTimerMax;
    bool mTransition;
    AnimGroupListener *mAnimEndListener;
public:
    AnimGroup()
    {
        clear();
    }
    ~AnimGroup()
    {
        clear();
    }
    void clear()
    {
        mAnims.clear();
        mAnimQueue.clear();
        mCurrentAnim = "";
        mNextAnim = AnimData();
        mTransitionTimer = 0;
        mTransitionTimerMax = 0;
        mTransition = false;
        mAnimEndListener = 0;
    }
    void add(AnimatedAnim *anim)
    {
        if(anim)mAnims.push_back(anim);
    }
    void update(const Real &timeElapsed)
    {
        if(!mTransition)
        {
            const bool animEnded = (!mAnims.empty() && mAnims.front()->hasEnded());
            if(animEnded && animEndedCallback(false))return;

            //Dequeue if current anim has ended
            if(!mAnimQueue.empty() && animEnded)
            {
                AnimData anim = mAnimQueue.front();
                mAnimQueue.erase(mAnimQueue.begin());
                transitAnim(anim.mName,anim.mSpeed,anim.mLoop,anim.mReset,anim.mTransitTime);
            }
            return;
        }
        //Update anim weight transition
        mTransitionTimer -= timeElapsed;
        if(mTransitionTimer<=0)
        {
            for(std::vector<AnimatedAnim*>::iterator i=mAnims.begin(); i!=mAnims.end(); i++)
            {
                AnimatedAnim *anim = *i;
                if(mNextAnim.mName!="")anim->setAnimation(mNextAnim.mName,mNextAnim.mSpeed,mNextAnim.mLoop,false,mNextAnim.m_fStartTime);
                else anim->stopAnimation();
            }
            mCurrentAnim = mNextAnim.mName;
            mNextAnim = AnimData();
            mTransition = false;
        }
        else
            for(std::vector<AnimatedAnim*>::iterator i=mAnims.begin(); i!=mAnims.end(); i++)
            {
                AnimatedAnim *anim = *i;
                anim->setWeight(mTransitionTimer/mTransitionTimerMax);
                if(mNextAnim.mName!="")
                {
                    AnimationState *state = anim->mEnt->getAnimationState(mNextAnim.mName);
                    state->setWeight((mTransitionTimerMax-mTransitionTimer)/mTransitionTimerMax);
                    state->addTime(mNextAnim.mSpeed * timeElapsed);
                }
            }
    }
    void transitAnim(const String &name, const Real &speed, const bool &loop, const bool &resetTime=true, const Real &transitTime=0.1f, const bool &cancelNextAnim=false, const Real &fStartTime=0.0f)
    {
        animEndedCallback(true);

        if(mNextAnim.mName==name)
        {
            mNextAnim = AnimData(name,speed,loop,resetTime,transitTime,fStartTime);
            return;
        }
        if(mCurrentAnim==name)
        {
            //Undo transition of current anim
            if(mTransition)
            {
                for(std::vector<AnimatedAnim*>::iterator i=mAnims.begin(); i!=mAnims.end(); i++)
                {
                    AnimatedAnim *anim = *i;
                    if(mNextAnim.mName!="")anim->setAnimation(mNextAnim.mName,mNextAnim.mSpeed,mNextAnim.mLoop,false,fStartTime);
                    else anim->stopAnimation();

                    AnimationState *state = anim->mEnt->getAnimationState(name);
                    state->setEnabled(true);
                    state->setLoop(loop && fStartTime<=0.0f);
                }
                mNextAnim = AnimData(name,speed,loop,resetTime,transitTime,fStartTime);
                mTransitionTimer = mTransitionTimerMax - mTransitionTimer;
            }
            else
                for(std::vector<AnimatedAnim*>::iterator i=mAnims.begin(); i!=mAnims.end(); i++)
                {
                    AnimatedAnim *anim = *i;
                    anim->mSpeed = speed;
                    anim->mEnt->getAnimationState(name)->setLoop(loop && fStartTime<=0.0f);
                    anim->m_fStartTime = fStartTime;
                }
            return;
        }

        if(mNextAnim.mName!="")
        {
            //Skip existing transition
            if(cancelNextAnim)
            {
                for(int i=0;i<(int)mAnims.size();i++)mAnims[i]->mEnt->getAnimationState(mNextAnim.mName)->setEnabled(false);
                mTransitionTimer = transitTime*mTransitionTimer/mTransitionTimerMax;
            }
            //Complete existing transition
            else
            {
                setAnim(mNextAnim.mName,mNextAnim.mSpeed,mNextAnim.mLoop,false);
                mTransitionTimer = transitTime;
            }
        }
        else mTransitionTimer = transitTime;

        mNextAnim = AnimData(name,speed,loop,resetTime,transitTime,fStartTime);
        if(mNextAnim.mName!="")
            for(std::vector<AnimatedAnim*>::iterator i=mAnims.begin(); i!=mAnims.end(); i++)
            {
                AnimatedAnim *anim = *i;
                AnimationState *state = anim->mEnt->getAnimationState(name);
                state->setEnabled(true);
                state->setLoop(loop && fStartTime<=0.0f);
                state->setWeight(0);
                if(resetTime)state->setTimePosition(speed<0?state->getLength():fStartTime);
            }
        mTransitionTimerMax = transitTime;
        mTransition = true;
    }
    void setAnim(const String &name, const Real &speed, const bool &loop, const bool &resetTime=true, const Real &fStartTime=0.0f)
    {
        animEndedCallback(true);

        for(std::vector<AnimatedAnim*>::iterator i=mAnims.begin(); i!=mAnims.end(); i++)
        {
            AnimatedAnim *anim = *i;
            if(mNextAnim.mName!="" && mNextAnim.mName!=name)anim->mEnt->getAnimationState(mNextAnim.mName)->setEnabled(false);
            anim->setAnimation(name,speed,loop,resetTime,fStartTime);
        }
        mCurrentAnim = name;
    }
    void queueAnim(const String &name, const Real &speed, const bool &loop, const bool &resetTime=true, const Real &transitTime=0.1f)
    {
        mAnimQueue.push_back(AnimData(name,speed,loop,resetTime,transitTime));
    }
    void clearAnimQueue()
    {
        mAnimQueue.clear();
    }
    void endTransition()
    {
        mTransitionTimer = 0;
    }
    void setAnimTime(const Real &time)
    {
        for(std::vector<AnimatedAnim*>::iterator i=mAnims.begin(); i!=mAnims.end(); i++)
        {
            AnimatedAnim *anim = *i;
            anim->setTimePosition(time);
        }
    }
    const Real getAnimTime()
    {
        //Retrieve first anim's time position, if any
        if(mAnims.size()>0)return mAnims[0]->getTimePosition();
        return 0;
    }
    void setAnimEndListener(AnimGroupListener *listener)
    {
        mAnimEndListener = listener;
    }
    const bool animEndedCallback(const bool &interrupted)
    {
        if(!mAnimEndListener)return false;

        AnimGroupListener *listener = mAnimEndListener;
        mAnimEndListener = 0;
        return listener->animEnded(interrupted);
    }
};

class Unit : public Living, public AnimGroupListener
{
protected:
    Real mBlinkStateL;
    Real mBlinkStateMaxL;
    Real mBlinkStateR;
    Real mBlinkStateMaxR;
    Real mBlinkTimeout;
    Real mHeadEWState;
    Real mHeadNSState;
    Real mHeadEWTarget;
    Real mHeadNSTarget;
    std::vector<Bone*> mHeadBone;
    std::vector<Bone*> mNeckBone;
    AnimGroup mPrimaryAnims;
    AnimGroup mSecondaryAnims;
    AnimGroup m_cEquipAnims;
    unsigned char mAction;
    unsigned char mMovementModifier;
    unsigned char mEmoteIndex;
    unsigned char mDefaultEmote;
    Emote *mPrevEmote;
    Emote *mNextEmote;
    Real mEmoteTimer;
    bool mIsIdle;
    bool mIsSpecialAction;
    bool m_bIsSkillAction;
    ISound *mWalkSound;
    unsigned char mSoundType;
    bool mIsJumping;
    Real mJumpDuration;
    unsigned short mLipSyncToken;
    unsigned char mPreset;
    std::vector<MaterialPtr> mClonedMatList;
    unsigned char mCallType;
    ISound *mCallSound;
    Entity *m_pEquipEnt;
    unsigned char m_nEquip;
    std::vector<Entity*> m_vpItems;
    String m_szItems;
public:
    unsigned int mCharID;
    unsigned int mUserID;
    String mName;
    String mUsername;
    bool mIsMoving;
    bool mIsSwimming;
    bool mIsWading;
    bool mIsLocal;
    enum Action
    {
        ACT_NONE,
        ACT_IDLE,
        ACT_RUN,
        ACT_WALK,
        ACT_CRAWL,
        ACT_WALKREVERSE,
        ACT_CRAWLREVERSE,
        ACT_JUMP,
        ACT_FALL,
        ACT_CROUCH,
        ACT_SIT,
        ACT_LAY,
        ACT_PLOP,
        ACT_SIDELAY,
        ACT_LEAN,
        ACT_OVERTURN,
        ACT_CURL,
        ACT_SWIM,
        ACT_DANCE1,
        ACT_DANCE2,
        ACT_DANCE3,
        ACT_STRETCH,
        ACT_CALL1,
        ACT_DASH,
        ACT_FLY
    };
    enum MovementModifier
    {
        MOVE_NONE,
        MOVE_RUN,
        MOVE_WALK,
        MOVE_CRAWL,
        MOVE_WALKREVERSE,
        MOVE_CRAWLREVERSE
    };
    enum CallType
    {
        CALL_NONE,
        CALL_ROAR,
        CALL_HOWL
    };
    Unit()
    {
        mWalkSound = 0;
        mCallSound = 0;
        mCharID = 0;
        mUserID = 0;
        mName = "";
        mUsername = "";
        mIsLocal = true;
        m_szItems = "";
        resetUnit();
        m_pEquipEnt = 0;
    }
    virtual ~Unit()
    {
        removeAllItems();
        uninitUnit();
        resetUnit();
        removeEquip();
        while(!mClonedMatList.empty())
        {
            MaterialPtr clonedMat = mClonedMatList.back();
            mClonedMatList.pop_back();
            if(!clonedMat.isNull())MaterialManager::getSingleton().remove(clonedMat->getHandle());
        }
    }
    void resetUnit()
    {
        mBlinkStateL = mBlinkStateMaxL = 1;
        mBlinkStateR = mBlinkStateMaxR = 1;
        mBlinkTimeout = 0;
        mHeadEWState = 0;
        mHeadNSState = 0;
        mHeadEWTarget = 0;
        mHeadNSTarget = 0;
        mHeadBone.clear();
        mNeckBone.clear();
        mAction = ACT_NONE;
        mMovementModifier = MOVE_NONE;
        mEmoteIndex = 0;
        mDefaultEmote = EmoteManager::getSingleton().getEmoteIndex("Normal");
        mPrevEmote = 0;
        mNextEmote = 0;
        mEmoteTimer = 0;
        mIsIdle = false;
        mIsSpecialAction = false;
        m_bIsSkillAction = false;

        mIsMoving = false;
        mIsSwimming = false;
        mIsWading = false;
        mIsInEarshot = false;
        stopJump();

        if(mWalkSound)
        {
            SoundManager::getSingletonPtr()->stopLoopedSound(mWalkSound);
            mWalkSound = 0;
        }
        mSoundType = 0;
        if(mLipSyncToken)LipSyncManager::getSingletonPtr()->stopLipSync(mLipSyncToken);
        mLipSyncToken = 0;
        mPreset = 0;
        mCallType = 0;
        if(mCallSound)
        {
            SoundManager::getSingletonPtr()->stopLoopedSound(mCallSound);
            mCallSound = 0;
        }
        m_nEquip = 0;
    }
    void initUnit()
    {
        //Body anim
        if(mEntities.size()>0 && mEntities[0])
        {
            mEntities[0]->getSkeleton()->setBlendMode(ANIMBLEND_CUMULATIVE);

            addAnimState(0);                    //Primary anim (0)
            addAnimState(0);                    //Secondary anim (1)

            addFixedAnimState(0);   //BlinkL (0)
            addFixedAnimState(0);   //BlinkR (1)
            addFixedAnimState(0);   //Eye look east/west (2)
            addFixedAnimState(0);   //Eye look north/south (3)
            addFixedAnimState(0);   //Head turn north/south (4)

            SkeletonInstance* skel = mEntities[0]->getSkeleton();
            Bone *bone = skel->getBone("head");
            bone->setManuallyControlled(true);
            mHeadBone.push_back(bone);
            bone = skel->getBone("neck2");
            bone->setManuallyControlled(true);
            mNeckBone.push_back(bone);

            mPrimaryAnims.add(mAnims[0]);
            mSecondaryAnims.add(mAnims[1]);
        }
        //Tail/Mane/Ears/Tuft anim
        for(int i=1;i<(int)mEntities.size();i++)
        {
            if(mEntities[i])mEntities[i]->getSkeleton()->setBlendMode(ANIMBLEND_CUMULATIVE);

            addAnimState(i);  //Primary anim (2,4,6,...)
            addAnimState(i);  //Secondary anim (3,5,7,...)

            if(i>1 && mEntities[i])
            {
                addFixedAnimState(i);   //Head turn north/south (5,6,7..)

                SkeletonInstance* skel = mEntities[i]->getSkeleton();
                Bone *bone = skel->getBone("head");
                bone->setManuallyControlled(true);
                mHeadBone.push_back(bone);
                bone = skel->getBone("neck2");
                bone->setManuallyControlled(true);
                mNeckBone.push_back(bone);
            }

            mPrimaryAnims.add(mAnims[i*2]);
            mSecondaryAnims.add(mAnims[i*2+1]);
        }

        setAction(ACT_IDLE);
        mPrimaryAnims.endTransition();
        mSecondaryAnims.endTransition();
        mPrimaryAnims.update(0);
        mSecondaryAnims.update(0);
        setMovementModifier(MOVE_RUN);
        setFixedAnimation(0,"BlinkL",0);
        setFixedAnimation(1,"BlinkR",0);
        setEmote(mDefaultEmote,false);
    }
    void uninitUnit()
    {
        mPrimaryAnims.clear();
        mSecondaryAnims.clear();
        m_cEquipAnims.clear();
    }
    virtual void preUpdate(const Real &timeElapsed)
    {
        updateJump(timeElapsed);
        updateMovement(timeElapsed);
    }
    virtual void update(const Real &timeElapsed)
    {
        if(mIsDying)updateDeath(timeElapsed);
        else if(!mIsDisabled)updateControl(timeElapsed);
        updateMovementOffset(timeElapsed);
        updateHeadAnimations(timeElapsed);
        updateActions(timeElapsed);
        mPrimaryAnims.update(timeElapsed);
        mSecondaryAnims.update(timeElapsed);
        m_cEquipAnims.update(timeElapsed);
        updateAnimations(timeElapsed);
        updateEmote(timeElapsed);
        updateLipSync(timeElapsed);
        updateSounds();
    }
    void updateHeadAnimations(const Real &timeElapsed)
    {
        //Blink
        bool leftBlinkEnd = false;
        if(mBlinkStateL<mBlinkStateMaxL)
        {
            mBlinkStateL += 10*timeElapsed;
            if(mBlinkStateL>mBlinkStateMaxL)mBlinkStateL = mBlinkStateMaxL;
            setFixedAnimTime(0,1-Math::Abs(mBlinkStateL));
        }
        else leftBlinkEnd = true;
        if(mBlinkStateR<mBlinkStateMaxR)
        {
            mBlinkStateR += 10*timeElapsed;
            if(mBlinkStateR>mBlinkStateMaxR)mBlinkStateR = mBlinkStateMaxR;
            setFixedAnimTime(1,1-Math::Abs(mBlinkStateR));
        }
        else if(leftBlinkEnd)
        {
            mBlinkTimeout -= timeElapsed;
            if(mBlinkTimeout<=0)
            {
                mBlinkTimeout = Math::UnitRandom()*10;
                mBlinkStateL = -mBlinkStateMaxL;
                mBlinkStateR = -mBlinkStateMaxR;
            }
        }

        //Head turn East/West
        const Real dEW = mHeadEWTarget - mHeadEWState;
        if(Math::Abs(dEW)>0.01f)
        {
            const Real ratio = 5*timeElapsed;
            mHeadEWState += dEW*(ratio<1?ratio:1);
        }
        updateHeadEWState();
        //Head turn North/South
        const Real dNS = mHeadNSTarget - mHeadNSState;
        if(Math::Abs(dNS)>0.01f)
        {
            const Real ratio = 5*timeElapsed;
            mHeadNSState += dNS*(ratio<1?ratio:1);
            updateHeadNSState();
        }
    }
    void updateHeadEWState()
    {
        for(std::vector<Bone*>::iterator i=mHeadBone.begin(); i!=mHeadBone.end(); i++)
        {
            Bone *bone = *i;
            bone->reset();
            bone->yaw(Degree(mHeadEWState*-45),Node::TS_WORLD);
        }
        for(std::vector<Bone*>::iterator i=mNeckBone.begin(); i!=mNeckBone.end(); i++)
        {
            Bone *bone = *i;
            bone->reset();
            bone->yaw(Degree(mHeadEWState*-45),Node::TS_WORLD);
        }
    }
    void updateHeadNSState()
    {
        for(int i=0;i<(int)mHeadBone.size();i++)setFixedAnimation(i+4,(mHeadNSState<0?"TurnUp":"TurnDown"),Math::Abs(mHeadNSState));
    }
    void setEyelidState(const Real &ratioL, const Real &ratioR)
    {
        if(ratioL!=mBlinkStateMaxL)
        {
            mBlinkStateMaxL = ratioL;
            if(mBlinkStateL>=mBlinkStateMaxL)mBlinkStateL = -mBlinkStateL;
        }
        if(ratioR!=mBlinkStateMaxR)
        {
            mBlinkStateMaxR = ratioR;
            if(mBlinkStateR>=mBlinkStateMaxR)mBlinkStateR = -mBlinkStateR;
        }
    }
    void updateActions(const Real &timeElapsed)
    {
        //Hit

        //Skill
        if(m_bIsSkillAction)
        {
            return;
        }
        //Swim
        if(mIsSwimming)
        {
            setAction(ACT_SWIM);
            return;
        }
        //Fly
        if(mAntiGravity)
        {
            setAction(ACT_FLY);
            return;
        }
        //Jump/fall
        if(m_cVelocity.y!=0 || mAction==ACT_JUMP)
        {
            if(m_cVelocity.y>0)setAction(ACT_JUMP);
            else setAction(ACT_FALL);
            return;
        }
        //Run/walk/crawl
        if(mIsMoving)
        {
            if(mMovementModifier==MOVE_RUN)setAction(ACT_RUN);
            else if(mMovementModifier==MOVE_WALK)setAction(ACT_WALK);
            else if(mMovementModifier==MOVE_WALKREVERSE)setAction(ACT_WALKREVERSE);
            else if(mMovementModifier==MOVE_CRAWL)setAction(ACT_CRAWL);
            else if(mMovementModifier==MOVE_CRAWLREVERSE)setAction(ACT_CRAWLREVERSE);
            return;
        }
        //Idle
        if(!mIsIdle)setAction(mMovementModifier==MOVE_CRAWL?ACT_CROUCH:ACT_IDLE);
    }
    void setAction(const unsigned char &action, const bool &instant=false)
    {
        if(mAction==action)return;
        mPrimaryAnims.clearAnimQueue();
        mIsIdle = false;
        const bool wasSpecialAction = mIsSpecialAction;
        mIsSpecialAction = false;
        m_bIsSkillAction = false;
        switch(action)
        {
            default:
            case ACT_IDLE:
            {
                bool doQueue = false;
                if(instant)mPrimaryAnims.setAnim("Idle",0.05f,true,true);
                else
                {
                    if(mAction==ACT_SIT){mPrimaryAnims.transitAnim("SitTransit",-0.5f,false); doQueue=true;}

                    if(doQueue)mPrimaryAnims.queueAnim("Idle",0.05f,true,true,0.5f);
                    else mPrimaryAnims.transitAnim("Idle",0.05f,true,true,mAction==ACT_FALL?0.3f:0.5f);
                }

                mIsIdle = true;
                mIsSpecialAction = true;
            }
                break;
            case ACT_RUN:
                mPrimaryAnims.transitAnim("Run",1.7,true);
                break;
            case ACT_WALK:
                mPrimaryAnims.transitAnim("Walk",1,true);
                break;
            case ACT_CRAWL:
                mPrimaryAnims.transitAnim("Crawl",0.25f,true,true,0.1f,mAction==ACT_CROUCH);
                break;
            case ACT_WALKREVERSE:
                mPrimaryAnims.transitAnim("Walk",-1,true);
                break;
            case ACT_CRAWLREVERSE:
                mPrimaryAnims.transitAnim("Crawl",-0.25f,true,true,0.1f,mAction==ACT_CROUCH);
                break;
            case ACT_JUMP:
                mPrimaryAnims.transitAnim("Jump",0.8f,false);
                break;
            case ACT_FALL:
                mPrimaryAnims.transitAnim("Fall",0.2f,true,true,(mAction==ACT_JUMP?0.6f:0.2f));
                break;
            case ACT_CROUCH:
                if(instant)mPrimaryAnims.setAnim("Crouch",0.05f,true,true);
                else mPrimaryAnims.transitAnim("Crouch",0.05f,true,true,(mAction==ACT_IDLE?0.4f:0.2f),mAction==ACT_CRAWL);
                mIsIdle = true;
                mIsSpecialAction = true;
                break;
            case ACT_SIT:
                if(instant)mPrimaryAnims.setAnim("SitIdle",0.05f,true);
                else
                {
                    if(mAction==ACT_LAY)mPrimaryAnims.transitAnim("LayTransit",-0.5f,false);
                    else if(mAction!=ACT_IDLE)mPrimaryAnims.transitAnim("SitIdle",0.05f,true,true,0.5f);
                    else
                    {
                        mPrimaryAnims.transitAnim("SitTransit",0.5f,false);
                        mPrimaryAnims.queueAnim("SitIdle",0.05f,true);
                    }
                }
                mIsIdle = true;
                mIsSpecialAction = true;
                break;
            case ACT_LAY:
                if(instant)mPrimaryAnims.setAnim("LayIdle",0.05f,true);
                else
                {
                    if(mAction==ACT_PLOP)mPrimaryAnims.transitAnim("LayIdle",0.05f,true,true,0.5f);
                    else
                    {
                        if(mAction==ACT_SIDELAY)mPrimaryAnims.transitAnim("SideLayTransit",-0.5f,false);
                        else if(mAction!=ACT_SIT)mPrimaryAnims.transitAnim("LayIdle",0.05f,true,true,0.5f);
                        else
                        {
                            mPrimaryAnims.transitAnim("LayTransit",0.5f,false);
                            mPrimaryAnims.queueAnim("LayIdle",0.05f,true);
                        }
                    }
                }
                mIsIdle = true;
                mIsSpecialAction = true;
                break;
            case ACT_PLOP:
                if(instant)mPrimaryAnims.setAnim("PlopIdle",0.05f,true);
                else
                {
                    if(mAction==ACT_CURL)mPrimaryAnims.transitAnim("CurlTransit",-0.5f,false);
                    else mPrimaryAnims.transitAnim("PlopIdle",0.05f,true,true,0.5f);
                }
                mIsIdle = true;
                mIsSpecialAction = true;
                break;
            case ACT_CURL:
                if(instant)mPrimaryAnims.setAnim("CurlIdle",0.02f,true);
                else
                {
                    if(mAction!=ACT_PLOP)mPrimaryAnims.transitAnim("CurlIdle",0.02f,true,true,0.2f);
                    else
                    {
                        mPrimaryAnims.transitAnim("CurlTransit",0.5f,false);
                        mPrimaryAnims.queueAnim("CurlIdle",0.02f,true);
                    }
                }
                mIsIdle = true;
                mIsSpecialAction = true;
                break;
            case ACT_SIDELAY:
                if(instant)mPrimaryAnims.setAnim("SideLayIdle",0.05f,true);
                else
                {
                    if(mAction==ACT_LEAN)mPrimaryAnims.transitAnim("LeanTransit",-0.5f,false);
                    else if(mAction!=ACT_LAY)mPrimaryAnims.transitAnim("SideLayIdle",0.05f,true,true,0.4f);
                    else
                    {
                        mPrimaryAnims.transitAnim("SideLayTransit",0.5f,false);
                        mPrimaryAnims.queueAnim("SideLayIdle",0.05f,true);
                    }
                }
                mIsIdle = true;
                mIsSpecialAction = true;
                break;
            case ACT_LEAN:
                if(instant)mPrimaryAnims.setAnim("LeanIdle",0.02f,true);
                else
                {
                    if(mAction==ACT_OVERTURN)mPrimaryAnims.transitAnim("OverturnTransit",-0.5f,false,true,0);
                    else if(mAction!=ACT_SIDELAY)mPrimaryAnims.transitAnim("LeanIdle",0.02f,true,true,0.3f);
                    else
                    {
                        mPrimaryAnims.transitAnim("LeanTransit",0.5f,false);
                        mPrimaryAnims.queueAnim("LeanIdle",0.02f,true);
                    }
                }
                mIsIdle = true;
                mIsSpecialAction = true;
                break;
            case ACT_OVERTURN:
                if(instant)mPrimaryAnims.setAnim("OverturnIdle",0.02f,true);
                else
                {
                    if(mAction!=ACT_LEAN)mPrimaryAnims.transitAnim("OverturnIdle",0.02f,true,true,0.2f);
                    else
                    {
                        mPrimaryAnims.transitAnim("OverturnTransit",0.5f,false);
                        mPrimaryAnims.queueAnim("OverturnIdle",0.02f,true,true,0);
                    }
                }
                mIsIdle = true;
                mIsSpecialAction = true;
                break;
            case ACT_SWIM:
                mPrimaryAnims.transitAnim("Run",1,true);
                break;
            case ACT_DANCE1:
                mPrimaryAnims.transitAnim("Dance1",0.5f,true);
                mIsIdle = true;
                mIsSpecialAction = true;
                break;
            case ACT_DANCE2:
                mPrimaryAnims.transitAnim("Dance2",0.5f,true);
                mIsIdle = true;
                mIsSpecialAction = true;
                break;
            case ACT_DANCE3:
                mPrimaryAnims.transitAnim("Dance3",0.5f,true);
                mIsIdle = true;
                mIsSpecialAction = true;
                break;
            case ACT_STRETCH:
                mPrimaryAnims.transitAnim("Stretch",0.1f,false);
                mPrimaryAnims.setAnimEndListener(this);
                mIsIdle = true;
                mIsSpecialAction = true;
                break;
            case ACT_CALL1:
                mPrimaryAnims.transitAnim("Call1",(mCallType==CALL_HOWL ? 0.1f : 0.2f),false);
                mPrimaryAnims.setAnimEndListener(this);
                mIsIdle = true;
                mIsSpecialAction = true;
                doCall();
                break;
            case ACT_DASH:
                mPrimaryAnims.transitAnim("Run",2.0f,true,true,0.05f);
                m_bIsSkillAction = true;
                break;
            case ACT_FLY:
                mPrimaryAnims.transitAnim("Jump",0.02f,true,true,0.1f,false,0.1f);
                break;
        }

        //Equip actions
        setEquipAction(action, instant);

        mAction = action;

        //Special action stopped
        if(wasSpecialAction && !mIsSpecialAction)fireUnitEvent(ObjectListener::UNIT_SPECIALACTION_STOPPED);
    }
    void setEquipAction(const unsigned char &action, const bool &instant=false)
    {
        switch(action)
        {
            default:
                if(instant)m_cEquipAnims.setAnim("Idle",0.05f,true,true);
                else m_cEquipAnims.transitAnim("Idle",0.05f,true,mAction==ACT_FLY,0.5f);
                break;
            case ACT_FLY:
                m_cEquipAnims.transitAnim("Fly",0.5f,true,true,0.1f);
                break;
        }
    }
    const unsigned char getAction()
    {
        return mAction;
    }
    const bool isSpecialAction()
    {
        return mIsSpecialAction;
    }
    void setMovementModifier(const unsigned char &mod)
    {
        if(mMovementModifier==mod)return;
        switch(mod)
        {
            default:
            case MOVE_RUN: mMaxVelocity = 200; break;
            case MOVE_WALK:
            case MOVE_WALKREVERSE:
                mMaxVelocity = 30;
                break;
            case MOVE_CRAWL:
            case MOVE_CRAWLREVERSE:
                mMaxVelocity = 10;
                break;
        }
        mMovementModifier = mod;
        setReverseYaw(mMovementModifier==MOVE_WALKREVERSE || mMovementModifier==MOVE_CRAWLREVERSE);
    }
    const bool isRunningModifier()
    {
        return (mMovementModifier==MOVE_RUN);
    }
    void setHeadYawPitch(const Real &y, const Real &p)
    {
        Real currentYaw = mSceneNode->getOrientation().getYaw().valueDegrees();
		if(currentYaw < 0)currentYaw += 360;
		else if(currentYaw >= 360)currentYaw -= 360;
        Real yaw = y - currentYaw;
        if(yaw<0)yaw += 360;
        yaw -= 180;

        mHeadEWTarget = (180-Math::Abs(yaw))*0.0111f * (yaw<0.01f?-1:1);
        if(mHeadEWTarget>1.2f)mHeadEWTarget = 1.2f;
        else if(mHeadEWTarget<-1.2f)mHeadEWTarget = -1.2f;

        Real currentPitch = mSceneNode->getOrientation().getPitch().valueDegrees();
		if(currentPitch < 0)currentPitch += 360;
		else if(currentPitch >= 360)currentPitch -= 360;
        Real pitch = (p<0?p+360:p) - currentPitch;
        if(pitch<0)pitch += 360;
        pitch -= 180;

        mHeadNSTarget = (180-Math::Abs(pitch))*0.0111f * (pitch<0?1:-1);
        if(mHeadNSTarget>1.0f)mHeadNSTarget = 1.0f;
        else if(mHeadNSTarget<-1.0f)mHeadNSTarget = -1.0f;
    }
    const Real convertHeadEWState(const unsigned char &value)
    {
        return ((Real(value)-127)/127 * 1.2f);
    }
    const Real convertHeadNSState(const unsigned char &value)
    {
        return ((Real(value)-127)/127 * 1.0f);
    }
    const unsigned char convertHeadEWState(const float &value)
    {
        return ((unsigned char)(value/1.2f * 127) + 127);
    }
    const unsigned char convertHeadNSState(const float &value)
    {
        return ((unsigned char)(value/1.0f * 127) + 127);
    }
    void setHeadEWState(const Real &value, const bool &instant=false)
    {
        mHeadEWTarget = value;
        if(instant)
        {
            mHeadEWState = value;
            updateHeadEWState();
        }
    }
    void setHeadNSState(const Real &value, const bool &instant=false)
    {
        mHeadNSTarget = value;
        if(instant)
        {
            mHeadNSState = value;
            updateHeadNSState();
        }
    }
    const Real getHeadEWState()
    {
        return mHeadEWTarget;
    }
    const Real getHeadNSState()
    {
        return mHeadNSTarget;
    }
    void setEmote(const unsigned char &index, const bool &transit=true)
    {
        if(mEmoteIndex==index && mNextEmote)return;
        //End previous transition
        if(mEmoteTimer>0)
        {
            if(mPrevEmote)updateEmoteState(mPrevEmote,0);
            if(mNextEmote)updateEmoteState(mNextEmote,1);
        }

        Emote *emote = EmoteManager::getSingleton().getEmote(index);
        if(!emote)
        {
            emote = EmoteManager::getSingleton().getEmote(0);
            mEmoteIndex = 0;
        }
        else mEmoteIndex = index;
        mPrevEmote = mNextEmote;
        mNextEmote = emote;
        if(transit && mPrevEmote)mEmoteTimer = 1;
        else
        {
            if(mPrevEmote)updateEmoteState(mPrevEmote,0);
            updateEmoteState(mNextEmote,1);
        }
        setEyelidState(emote->mEyelidL,emote->mEyelidR);
    }
    void updateEmoteState(Emote *emote, const Real &ratio)
    {
        for(int i=0;i<2;i++)
        {
            Entity *ent = 0;
            if(i==0 && mEntities.size()>0)ent = mEntities[0];       //body+head
            else if(i==1 && mEntities.size()>3)ent = mEntities[3];  //ears
            if(!ent)continue;
            for(std::vector<pair<String,Real> >::iterator i=emote->mList.begin(); i!=emote->mList.end(); i++)
            {
                const pair<String,Real> line = *i;
                AnimationState* anim = ent->getAnimationState(line.first);
                if(ratio>0)
                {
                    anim->setEnabled(true);
                    anim->setLoop(false);
                    anim->setTimePosition(line.second * ratio * anim->getLength());
                }
                else anim->setEnabled(false);
            }
        }
    }
    void updateEmote(const Real &timeElapsed)
    {
        if(mEmoteTimer<=0)return;
        mEmoteTimer -= timeElapsed;
        if(mEmoteTimer<0)mEmoteTimer = 0;
        if(mEmoteTimer>=0.5f)updateEmoteState(mPrevEmote,(mEmoteTimer-0.5f)*2);
        else updateEmoteState(mNextEmote,(0.5f-mEmoteTimer)*2);
    }
    const unsigned char getEmote()
    {
        return mEmoteIndex;
    }
    void setDefaultEmote(const unsigned char &emote)
    {
        mDefaultEmote = emote;
    }
    const unsigned char getDefaultEmote()
    {
        return mDefaultEmote;
    }
    const bool isIdle()
    {
        return mIsIdle;
    }
    const Vector3 getHeadPosition(const bool &local=false)
    {
        if(mEntities.size()<=0)return (local?Vector3::ZERO:mSceneNode->getPosition());
        return (local?Vector3::ZERO:mSceneNode->getPosition()) + mSceneNode->getScale()* (getOrientation()*mEntities[0]->getSkeleton()->getBone("head")->_getDerivedPosition());
    }
    void setModScale(const unsigned char &part, const char &scale)
    {
        const String name[] = {"Muzzle","Chin","Nose","Forehead","Ear","Eye","Weight"};
        AnimationState *anim = 0;
        if(part==4 && mEntities.size()>3 && mEntities[3])anim = mEntities[3]->getAnimationState(name[part]+"Scale");
        else if(mEntities.size()>0 && mEntities[0])anim = mEntities[0]->getAnimationState(name[part]+"Scale");
        if(!anim)return;
        if(scale==0)anim->setEnabled(false);
        else
        {
            anim->setEnabled(true);
            Real ratio = float(scale)/255+0.5f;
            if(ratio<0)ratio = 0;
            else if(ratio>1)ratio = 1;
            anim->setTimePosition(ratio*anim->getLength());
        }
    }
    void setScale(const char &x,const char &y,const char &z)
    {
        Real ratio[3];
        ratio[0] = float(x)/255+1;
        ratio[1] = float(y)/255+1;
        ratio[2] = float(z)/255+1;
        for(int i=0;i<3;i++)
        {
            if(ratio[i]<0.5f)ratio[i] = 0.5f;
            else if(ratio[i]>1.5f)ratio[i] = 1.5f;
        }
        mSceneNode->setScale(ratio[0],ratio[1],ratio[2]);
    }
    void setScales(char *scale)
    {
        for(int i=0;i<NUM_SCALABLES-3;i++)setModScale(i,scale[i]);
        setScale(scale[NUM_SCALABLES-3],scale[NUM_SCALABLES-2],scale[NUM_SCALABLES-1]);
    }
    const std::vector<char> getScales()
    {
        std::vector<char> list;

        const String name[] = {"Muzzle","Chin","Nose","Forehead","Ear","Eye","Weight"};
        for(int i=0;i<NUM_SCALABLES-3;i++)
        {
            AnimationState *anim = 0;
            if(i==4 && mEntities.size()>3 && mEntities[3])anim = mEntities[3]->getAnimationState(name[i]+"Scale");
            else if(mEntities.size()>0 && mEntities[0])anim = mEntities[0]->getAnimationState(name[i]+"Scale");
            if(anim)
            {
                if(anim->getEnabled() && anim->getLength()>0)
                {
                    const Real ratio = anim->getTimePosition()/anim->getLength();
                    list.push_back(char((ratio-0.5f)*255));
                }
                else list.push_back(0);
            }
            else list.push_back(0);
        }
        const Vector3 scale = mSceneNode->getScale();
        for(int i=0;i<3;i++)list.push_back(char((scale[i]-1)*255));

        return list;
    }
    void setColour(const unsigned char &part, const Vector4 &colour, const bool &hasBodyMark=false, const bool &hasHeadMark=false, const bool &hasTailMark=false)
    {
        if(mPreset>0)return;
        switch(part)
        {
            //Body.mesh: 0,3 - eyes; 1,5 - head; 2,4 - body
            //Pelt
            case 0:
                if(mEntities.size()>0 && mEntities[0])
                {
                    setSubEntityColour(mEntities[0]->getSubEntity("bodyL"),1,colour);
                    setSubEntityColour(mEntities[0]->getSubEntity("bodyR"),1,colour);
                    setSubEntityColour(mEntities[0]->getSubEntity("headL"),1,colour);
                    setSubEntityColour(mEntities[0]->getSubEntity("headR"),1,colour);
                }
                if(mEntities.size()>1 && mEntities[1])
                {
                    setSubEntityColour(mEntities[1]->getSubEntity(0),1,colour);
                }
                if(mEntities.size()>3 && mEntities[3])
                {
                    setSubEntityColour(mEntities[3]->getSubEntity("headL"),1,colour);
                    setSubEntityColour(mEntities[3]->getSubEntity("headR"),1,colour);
                }
                if(mEntities.size()>4 && mEntities[4])
                {
                    const unsigned short numSubs = mEntities[4]->getNumSubEntities();
                    const String mats[4] = {"bodyMatL","bodyMatR","headMatL","headMatR"};
                    for(int i=0;i<numSubs;i++)
                        for(int j=0;j<4;j++)
                        {
                            SubEntity *subEnt = mEntities[4]->getSubEntity(i);
                            if(StringUtil::endsWith(subEnt->getMaterialName(),mats[j],false))
                            {
                                setSubEntityColour(subEnt,1,colour);
                                break;
                            }
                        }
                }
                break;
            //Underfur
            case 1:
                if(mEntities.size()>0 && mEntities[0])
                {
                    setSubEntityColour(mEntities[0]->getSubEntity("bodyL"),2,colour);
                    setSubEntityColour(mEntities[0]->getSubEntity("bodyR"),2,colour);
                    setSubEntityColour(mEntities[0]->getSubEntity("headL"),2,colour);
                    setSubEntityColour(mEntities[0]->getSubEntity("headR"),2,colour);
                }
                if(mEntities.size()>3 && mEntities[3])
                {
                    setSubEntityColour(mEntities[3]->getSubEntity("headL"),2,colour);
                    setSubEntityColour(mEntities[3]->getSubEntity("headR"),2,colour);
                }
                if(mEntities.size()>4 && mEntities[4])
                {
                    const unsigned short numSubs = mEntities[4]->getNumSubEntities();
                    const String mats[4] = {"bodyMatL","bodyMatR","headMatL","headMatR"};
                    for(int i=0;i<numSubs;i++)
                        for(int j=0;j<4;j++)
                        {
                            SubEntity *subEnt = mEntities[4]->getSubEntity(i);
                            if(StringUtil::endsWith(subEnt->getMaterialName(),mats[j],false))
                            {
                                setSubEntityColour(subEnt,2,colour);
                                break;
                            }
                        }
                }
                break;
            //Eyes
            case 2:
                if(mEntities.size()>0 && mEntities[0])
                {
                    setSubEntityColour(mEntities[0]->getSubEntity("eyesL"),1,colour);
                    setSubEntityColour(mEntities[0]->getSubEntity("eyesR"),1,colour);
                }
                break;
            //Above Eyes
            case 3:
                if(mEntities.size()>0 && mEntities[0])
                {
                    setSubEntityColour(mEntities[0]->getSubEntity("headL"),3,colour);
                    setSubEntityColour(mEntities[0]->getSubEntity("headR"),3,colour);
                    setSubEntityColour(mEntities[0]->getSubEntity("eyesL"),2,colour);
                    setSubEntityColour(mEntities[0]->getSubEntity("eyesR"),2,colour);
                }
                break;
            //Below Eyes
            case 4:
                if(mEntities.size()>0 && mEntities[0])
                {
                    setSubEntityColour(mEntities[0]->getSubEntity("headL"),4,colour);
                    setSubEntityColour(mEntities[0]->getSubEntity("headR"),4,colour);
                }
                break;
            //Nose
            case 5:
                if(mEntities.size()>0 && mEntities[0])
                {
                    setSubEntityColour(mEntities[0]->getSubEntity("headL"),5,colour);
                    setSubEntityColour(mEntities[0]->getSubEntity("headR"),5,colour);
                }
                break;
            //Tail tip
            case 6:
                if(mEntities.size()>1 && mEntities[1])
                {
                    setSubEntityColour(mEntities[1]->getSubEntity(0),2,colour);
                }
                break;
            //Mane
            case 7:
                if(mEntities.size()>2 && mEntities[2])
                {
                    setSubEntityColour(mEntities[2]->getSubEntity(0),1,colour);
                    if(mEntities[2]->getNumSubEntities()>1)setSubEntityColour(mEntities[2]->getSubEntity(1),1,colour);
                }
                if(mEntities.size()>4 && mEntities[4])
                {
                    const unsigned short numSubs = mEntities[4]->getNumSubEntities();
                    for(int i=0;i<numSubs;i++)
                    {
                        SubEntity *subEnt = mEntities[4]->getSubEntity(i);
                        if(StringUtil::endsWith(subEnt->getMaterialName(),"maneMat",false))setSubEntityColour(subEnt,1,colour);
                    }
                }
                break;
            //Markings
            case 8:
                if(mEntities.size()>0 && mEntities[0])
                {
                    if(hasBodyMark)
                    {
                        setSubEntityColour(mEntities[0]->getSubEntity("bodyL"),3,colour);
                        setSubEntityColour(mEntities[0]->getSubEntity("bodyR"),3,colour);
                    }
                    if(hasHeadMark)
                    {
                        setSubEntityColour(mEntities[0]->getSubEntity("headL"),6,colour);
                        setSubEntityColour(mEntities[0]->getSubEntity("headR"),6,colour);
                    }
                }
                if(mEntities.size()>1 && mEntities[1])
                {
                    if(hasTailMark)setSubEntityColour(mEntities[1]->getSubEntity(0),3,colour);
                }
                if(mEntities.size()>3 && mEntities[3])
                {
                    if(hasHeadMark)
                    {
                        setSubEntityColour(mEntities[3]->getSubEntity("headL"),6,colour);
                        setSubEntityColour(mEntities[3]->getSubEntity("headR"),6,colour);
                    }
                }
                break;
            //Equip
            case 9:
                if(m_pEquipEnt)
                {
                    const unsigned short nNumSubs = m_pEquipEnt->getNumSubEntities();
                    for(int i=0;i<nNumSubs;i++)
                    {
                        setSubEntityColour(m_pEquipEnt->getSubEntity(i),1,colour);
                    }
                }
                break;

            default: break;
        }
    }
    void setSubEntityColour(SubEntity *subEnt, const unsigned short &part, const Vector4 &colour)
    {
        subEnt->setCustomParameter(part,colour);
        const String clonedMatName = CapabilitiesManager::getSingletonPtr()->assertCustomColour(subEnt,part,true,mSceneNode->getName(),&mClonedMatList);
        if(clonedMatName!="")subEnt->setMaterialName(clonedMatName);
    }
    const std::vector<Vector4> getColours(const bool &hasBodyMark, const bool &hasHeadMark, const bool &hasTailMark)
    {
        std::vector<Vector4> list;
        if(mPreset>0)return list;

        //Pelt, underfur, eyes, above eyes, below eyes, nose
        if(mEntities.size()>0 && mEntities[0])
        {
            list.push_back(mEntities[0]->getSubEntity("bodyL")->getCustomParameter(1));
            list.push_back(mEntities[0]->getSubEntity("bodyL")->getCustomParameter(2));
            list.push_back(mEntities[0]->getSubEntity("eyesL")->getCustomParameter(1));
            list.push_back(mEntities[0]->getSubEntity("headL")->getCustomParameter(3));
            list.push_back(mEntities[0]->getSubEntity("headL")->getCustomParameter(4));
            list.push_back(mEntities[0]->getSubEntity("headL")->getCustomParameter(5));
        }
        else
        {
            for(int i=0;i<6;i++)list.push_back(Vector4::ZERO);
        }
        //Tailtip
        if(mEntities.size()>1 && mEntities[1])list.push_back(mEntities[1]->getSubEntity(0)->getCustomParameter(2));
        else list.push_back(Vector4::ZERO);
        //Mane
        if(mEntities.size()>2 && mEntities[2])list.push_back(mEntities[2]->getSubEntity(0)->getCustomParameter(1));
        else if(mEntities.size()>4 && mEntities[4])
        {
            bool hasManeMat = false;
            const unsigned short numSubs = mEntities[4]->getNumSubEntities();
            for(int i=0;i<numSubs;i++)
            {
                SubEntity *subEnt = mEntities[4]->getSubEntity(i);
                if(StringUtil::endsWith(subEnt->getMaterialName(),"maneMat",false))
                {
                    list.push_back(subEnt->getCustomParameter(1));
                    hasManeMat = true;
                    break;
                }
            }
            if(!hasManeMat)list.push_back(Vector4::ZERO);
        }
        else list.push_back(Vector4::ZERO);
        //Markings
        if((hasBodyMark||hasHeadMark) && mEntities.size()>0 && mEntities[0])
        {
            if(hasBodyMark)list.push_back(mEntities[0]->getSubEntity("bodyL")->getCustomParameter(3));
            else list.push_back(mEntities[0]->getSubEntity("headL")->getCustomParameter(6));
        }
        else if(hasTailMark && mEntities.size()>1 && mEntities[1])list.push_back(mEntities[1]->getSubEntity(0)->getCustomParameter(3));
        else list.push_back(Vector4::ZERO);
        //Equip
        if(m_pEquipEnt)
        {
            list.push_back(m_pEquipEnt->getSubEntity(0)->getCustomParameter(1));
        }
        else list.push_back(Vector4::ZERO);

        return list;
    }
    void setMarkings(const String &speciesStr, const unsigned char &index, const unsigned char &part)
    {
        if(mPreset>0)return;
        switch(part)
        {
            case 0:
                if(mEntities.size()>0 && mEntities[0])
                {
                    if(index)
                    {
                        const String bodyMat = speciesStr+"bodyMark"+StringConverter::toString(index);
                        mEntities[0]->getSubEntity("bodyL")->setMaterialName(bodyMat);
                        mEntities[0]->getSubEntity("bodyR")->setMaterialName(bodyMat);
                    }
                    else
                    {
                        mEntities[0]->getSubEntity("bodyL")->setMaterialName(speciesStr+"bodyMatL");
                        mEntities[0]->getSubEntity("bodyR")->setMaterialName(speciesStr+"bodyMatR");
                    }
                }
                break;
            case 1:
                if(mEntities.size()>0 && mEntities[0])
                {
                    if(index)
                    {
                        const String headMat = speciesStr+"headMark"+StringConverter::toString(index);
                        mEntities[0]->getSubEntity("headL")->setMaterialName(headMat);
                        mEntities[0]->getSubEntity("headR")->setMaterialName(headMat);
                    }
                    else
                    {
                        mEntities[0]->getSubEntity("headL")->setMaterialName(speciesStr+"headMatL");
                        mEntities[0]->getSubEntity("headR")->setMaterialName(speciesStr+"headMatR");
                    }
                }
                if(mEntities.size()>3 && mEntities[3])
                {
                    if(index)
                    {
                        const String headMat = speciesStr+"headMark"+StringConverter::toString(index);
                        mEntities[3]->getSubEntity("headL")->setMaterialName(headMat);
                        mEntities[3]->getSubEntity("headR")->setMaterialName(headMat);
                    }
                    else
                    {
                        mEntities[3]->getSubEntity("headL")->setMaterialName(speciesStr+"headMatL");
                        mEntities[3]->getSubEntity("headR")->setMaterialName(speciesStr+"headMatR");
                    }
                }
                break;
            case 2:
                if(mEntities.size()>1 && mEntities[1])
                {
                    if(index)
                    {
                        const String tailMat = speciesStr+"tailMark"+StringConverter::toString(index);
                        mEntities[1]->setMaterialName(tailMat);
                    }
                    else mEntities[1]->setMaterialName(speciesStr+"tailMat");
                }
                break;
        }
    }
    const std::vector<unsigned char> getMarkings()
    {
        std::vector<unsigned char> list;
        if(mPreset>0)return list;

        if(mEntities.size()>0 && mEntities[0])
        {
            String eyeMat = mEntities[0]->getSubEntity("eyesL")->getMaterialName();
            if(eyeMat.find_last_of('L')!=string::npos)
            {
                eyeMat.erase(0,eyeMat.find_last_of('L')+1);
                list.push_back(StringConverter::parseInt(eyeMat));
            }
            else list.push_back(0);
        }
        else list.push_back(0);

        for(int i=0;i<3;i++)
        {
            SubEntity *subEnt = 0;
            switch(i)
            {
                case 0: if(mEntities.size()>0 && mEntities[0])subEnt = mEntities[0]->getSubEntity("bodyL"); break;
                case 1: if(mEntities.size()>0 && mEntities[0])subEnt = mEntities[0]->getSubEntity("headL"); break;
                case 2: if(mEntities.size()>1 && mEntities[1])subEnt = mEntities[1]->getSubEntity(0); break;
            }
            if(subEnt)
            {
                String mat = subEnt->getMaterialName();
                if(mat.length()>9)
                {
                    mat.erase(0,9);
                    list.push_back(StringConverter::parseInt(mat));
                }
                else list.push_back(0);
            }
            else list.push_back(0);
        }

        return list;
    }
    const bool setPreset(const unsigned char &index, const String &username="")
    {
        if(setMaterial((username=="" ? "preset_" : username+"_" )+StringConverter::toString(index)+"_",0,true))
        {
            mPreset = index;
            return true;
        }
        return false;
    }
    const bool testPreset(const unsigned char &index, const String &username="")
    {
        return testMaterial((username=="" ? "preset_" : username+"_" )+StringConverter::toString(index)+"_");
    }
    const unsigned char getPreset()
    {
        return mPreset;
    }
    const bool testMaterial(const String &mat)
    {
        return MaterialManager::getSingletonPtr()->resourceExists(mat+"bodyMatL");
    }
    const bool setMaterial(const String &mat, const unsigned char &eyeMat=0, const bool &bIsPreset=false)
    {
        if(!testMaterial(mat))return false;

        mPreset = 0;
        if(mEntities.size()>0 && mEntities[0])
        {
            //Body
            mEntities[0]->getSubEntity("bodyL")->setMaterialName(mat+"bodyMatL");
            mEntities[0]->getSubEntity("bodyR")->setMaterialName(mat+"bodyMatR");
            //Head
            mEntities[0]->getSubEntity("headL")->setMaterialName(mat+"headMatL");
            mEntities[0]->getSubEntity("headR")->setMaterialName(mat+"headMatR");
            //Eye
            mEntities[0]->getSubEntity("eyesL")->setMaterialName(mat+"eyeMatL"+(eyeMat>0?StringConverter::toString(eyeMat):""));
            mEntities[0]->getSubEntity("eyesR")->setMaterialName(mat+"eyeMatR"+(eyeMat>0?StringConverter::toString(eyeMat):""));
        }
        if(mEntities.size()>1 && mEntities[1])
        {
            mEntities[1]->setMaterialName(mat+"tailMat");
        }
        if(mEntities.size()>2 && mEntities[2])
        {
            const bool isAlt = (mEntities[2]->getSubEntity(0)->getMaterialName().find_first_of('2')!=String::npos);
            mEntities[2]->setMaterialName(mat+"maneMat"+(isAlt?"2":""));
        }
        if(mEntities.size()>3 && mEntities[3])
        {
            mEntities[3]->getSubEntity("headL")->setMaterialName(mat+"headMatL");
            mEntities[3]->getSubEntity("headR")->setMaterialName(mat+"headMatR");
        }
        if(mEntities.size()>4 && mEntities[4])
        {
            const unsigned short numSubs = mEntities[4]->getNumSubEntities();
            const String mats[5] = {"bodyMatL","bodyMatR","headMatL","headMatR","maneMat"};
            for(int i=0;i<numSubs;i++)
                for(int j=0;j<5;j++)
                {
                    SubEntity *subEnt = mEntities[4]->getSubEntity(i);
                    if(StringUtil::endsWith(subEnt->getMaterialName(),mats[j],false))
                    {
                        subEnt->setMaterialName(mat+mats[j]);
                        break;
                    }
                }
        }
        //Equips only change material for presets
        if(m_pEquipEnt && bIsPreset)
        {
            //Refresh mesh if material is lost
            if(m_pEquipEnt->getNumSubEntities() > 0 && m_pEquipEnt->getSubEntity(0)->getMaterialName()=="BaseWhite")
            {
                setEquip(m_nEquip);
            }

            const unsigned short nNumSubs = m_pEquipEnt->getNumSubEntities();
            const String aszMats[2] = {"MatL","MatR"};
            const String aszPresetMats[2] = {"equipMatL","equipMatR"};
            for(int i=0;i<nNumSubs;i++)
                for(int j=0;j<2;j++)
                {
                    SubEntity *pSubEnt = m_pEquipEnt->getSubEntity(i);
                    if(StringUtil::endsWith(pSubEnt->getMaterialName(),aszMats[j],false))
                    {
                        pSubEnt->setMaterialName(mat + aszPresetMats[j]);
                        break;
                    }
                }
        }
        return true;
    }
    void setFullMaterial(const String &mat)
    {
        for(std::vector<Entity*>::iterator it=mEntities.begin(); it!=mEntities.end(); it++)
        {
            Entity *ent = *it;
            if(ent)ent->setMaterialName(mat);
        }
    }
    const Real getPrimaryAnimTime()
    {
        return mPrimaryAnims.getAnimTime();
    }
    void setPrimaryAnimTime(const Real &time)
    {
        mPrimaryAnims.setAnimTime(time);
    }
    const unsigned char getType()
    {
        return OBJECTTYPE_UNIT;
    }
    void updateSounds()
    {
        //Out of scene
        if(!mIsInEarshot)
        {
            if(mWalkSound)
            {
                SoundManager::getSingletonPtr()->stopLoopedSound(mWalkSound);
                mWalkSound = 0;
                mSoundType = 0;
                return;
            }
        }

        unsigned char newSound = 0;

        if(mIsWading)newSound = ACT_SWIM;
        else if(isGrounded() && mIsMoving && !mIsSwimming && !mAntiGravity)
        {
            if(mMovementModifier==MOVE_RUN)newSound = ACT_RUN;
            else if(mMovementModifier==MOVE_WALK)newSound = ACT_WALK;
        }

        if(mSoundType!=newSound)
        {
            if(mWalkSound)SoundManager::getSingletonPtr()->stopLoopedSound(mWalkSound);
            mSoundType = newSound;
            switch(mSoundType)
            {
                case ACT_RUN:
                    mWalkSound = SoundManager::getSingletonPtr()->play3DSound(SOUND_RUNGRASS,mPosition,true);
                    break;
                case ACT_WALK:
                    mWalkSound = SoundManager::getSingletonPtr()->play3DSound(SOUND_WALKGRASS,mPosition,true);
                    break;
                case ACT_SWIM:
                    mWalkSound = SoundManager::getSingletonPtr()->play3DSound(SOUND_WATERWADE,mPosition,true);
                    break;
                default: mWalkSound = 0; break;
            }
        }
        else if(mWalkSound)mWalkSound->setPosition(SoundManager::parseVec3df(mPosition));
    }
    void startJump()
    {
        mIsJumping = true;
    }
    void stopJump()
    {
        mIsJumping = false;
        mJumpDuration = JUMP_LIMIT;
    }
    void updateJump(const Real &timeElapsed)
    {
        if(mIsJumping)
        {
            if(mJumpDuration<JUMP_LIMIT)
            {
                addVelocity(Vector3(0,GRAVITY*(isGrounded()?0.3f:timeElapsed),0));
                mJumpDuration += timeElapsed;
            }
            else if(isGrounded() && isOnGentleSlope())mJumpDuration = 0;
        }
    }
    void setLipSyncSpeech(const String &speech)
    {
        LipSyncManager *lsMgr = LipSyncManager::getSingletonPtr();
        if(mLipSyncToken)lsMgr->stopLipSync(mLipSyncToken,(mEntities.size()>0?mEntities[0]:0));
        mLipSyncToken = lsMgr->pushLipSync(speech);
    }
    void updateLipSync(const Real &timeElapsed)
    {
        if(!mLipSyncToken)return;
        Entity *ent = (mEntities.size()>0?mEntities[0]:0);
        if(!ent)return;
        LipSyncManager::getSingletonPtr()->updateLipSync(&mLipSyncToken,ent,timeElapsed);
    }
    void fireUnitEvent(const unsigned short &event)
    {
        for(std::vector<ObjectListener*>::iterator it=mListenerList.begin(); it!=mListenerList.end(); it++)
        {
            ObjectListener *listener = *it;
            listener->unitEvent(this,event);
        }
    }
    const bool animEnded(const bool &interrupted)
    {
        bool returnEnd = false;

        switch(mAction)
        {
            case ACT_CALL1:
                setEmote(mDefaultEmote);
                if(mCallSound)
                {
                    SoundManager::getSingletonPtr()->stopLoopedSound(mCallSound);
                    mCallSound = 0;
                }
            case ACT_STRETCH:
                if(!interrupted)
                {
                    returnEnd = true;
                    setAction(ACT_IDLE);
                }
                break;
        }

        return returnEnd;
    }
    void setCallType(const unsigned char &type)
    {
        mCallType = type;
    }
    const unsigned char getCallType()
    {
        return mCallType;
    }
    void doCall()
    {
        String emote;
        String sound;
        switch(mCallType)
        {
            case CALL_ROAR:
                emote = "Roar";
                sound = SOUND_ROAR;
                break;
            case CALL_HOWL:
                emote = "Howl";
                sound = SOUND_HOWL;
                break;
            default:
                return;
        }

        setEmote(EmoteManager::getSingletonPtr()->getEmoteIndex(emote));
        if(mCallSound)SoundManager::getSingletonPtr()->stopLoopedSound(mCallSound);
        mCallSound = SoundManager::getSingletonPtr()->play3DSound(sound.c_str(),mPosition,false,true);
    }
    Entity* addItem(const unsigned short &nIndex, const char &nSpecies)
    {
        if(mEntities.size() <= 0 || !mEntities[0])return 0;

        Item *pItem = ItemsManager::getSingletonPtr()->getItem(nIndex);
        if(!pItem)
        {
            LoggerManager::getSingleton().logMessage("Cannot find item with index " + StringConverter::toString(nIndex));
            return 0;
        }
        if(!SaveFile::resourceExists(pItem->m_szMesh))
        {
            LoggerManager::getSingleton().logMessage("Cannot find item mesh " + pItem->m_szMesh);
            return 0;
        }
        if(!mEntities[0]->getSkeleton()->hasBone(pItem->m_szBone))
        {
            LoggerManager::getSingleton().logMessage("Cannot find item bone " + pItem->m_szBone);
            return 0;
        }

        static unsigned long s_nIndex;
        Entity *pEnt = mSceneNode->getCreator()->createEntity("Item"+StringConverter::toString(s_nIndex++),pItem->m_szMesh);
        if(pItem->m_szMaterial!="")
        {
            if(!MaterialManager::getSingletonPtr()->resourceExists(pItem->m_szMaterial))
            {
                LoggerManager::getSingleton().logMessage("Cannot find item material " + pItem->m_szMaterial);
            }
            pEnt->setMaterialName(pItem->m_szMaterial);
        }
        const Vector3 cOffset = pItem->getOffset(nSpecies);
        const Vector3 cRotation = pItem->getRotation(nSpecies);

        const Quaternion cQuat = Quaternion(Degree(cRotation.y),Vector3::UNIT_Y)*Quaternion(Degree(cRotation.x),Vector3::UNIT_X)*Quaternion(Degree(cRotation.z),Vector3::UNIT_Z);

        mEntities[0]->attachObjectToBone(pItem->m_szBone,pEnt,cQuat,cOffset);
        m_vpItems.push_back(pEnt);

        return pEnt;
    }
    void addItems(const String &szItems, const char &nSpecies)
    {
        if(szItems=="")return;

        const StringVector vszIndex = StringUtil::split(szItems,";");
        for(int i=0; i<(int)vszIndex.size(); i++)
        {
            const unsigned short nIndex = StringConverter::parseInt(vszIndex[i]);
            addItem(nIndex,nSpecies);
        }
        m_szItems = szItems;
    }
    void removeAllItems()
    {
        m_szItems = "";
        if(mEntities.size() <= 0 || !mEntities[0])return;

        while(!m_vpItems.empty())
        {
            Entity *pEnt = m_vpItems.back();
            m_vpItems.pop_back();

            mEntities[0]->detachObjectFromBone(pEnt);
            mSceneNode->getCreator()->destroyEntity(pEnt);
        }
    }
    const String getItems()
    {
        return m_szItems;
    }
    void setEquip(const unsigned char nIndex)
    {
        if(mEntities.size() <= 0 || !mEntities[0])return;
        if(m_pEquipEnt)removeEquip();

        m_nEquip = nIndex;

        String szMesh = "";
        Vector3 cOffset = Vector3::ZERO;
        Vector3 cRotation = Vector3::ZERO;

        switch(m_nEquip)
        {
            case 1:
            case 2:
                szMesh = "wings" + StringConverter::toString(m_nEquip) + ".mesh";
                cOffset = Vector3(0,2.2f,0.2f);
                cRotation = Vector3(0,0,0);
                break;
            default: return;
        }

        static unsigned long s_nIndex;
        m_pEquipEnt = mSceneNode->getCreator()->createEntity("Equip"+StringConverter::toString(s_nIndex++),szMesh);

        const Quaternion cQuat = Quaternion(Degree(cRotation.y),Vector3::UNIT_Y)*Quaternion(Degree(cRotation.x),Vector3::UNIT_X)*Quaternion(Degree(cRotation.z),Vector3::UNIT_Z);

        mEntities[0]->attachObjectToBone("Main",m_pEquipEnt,cQuat,cOffset);
        m_cEquipAnims.add(addAnimStateEntity(m_pEquipEnt));
        setEquipAction(ACT_IDLE,true);
        m_cEquipAnims.endTransition();
    }
    void removeEquip()
    {
        m_nEquip = 0;

        if(!m_pEquipEnt)return;
        if(mEntities.size() <= 0 || !mEntities[0])return;

        m_cEquipAnims.clear();
        removeAnimStateEntity(m_pEquipEnt);
        mEntities[0]->detachObjectFromBone(m_pEquipEnt);
        mSceneNode->getCreator()->destroyEntity(m_pEquipEnt);

        m_pEquipEnt = 0;
    }
    const unsigned char getEquip()
    {
        return m_nEquip;
    }
};

#endif
