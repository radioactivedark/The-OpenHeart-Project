#ifndef _OBJECT_H_
#define _OBJECT_H_

#define FRICTION 500
#define TERMINAL_VELOCITY 500
#define GRAVITY 500
#define GROUND_THRESHOLD 200
#define MOVE_FORCE 1000
#define STEEP_ANGLE 60

#include <ObjectListener.h>

using namespace std;

class Object : public ObjectListenerObject
{
protected:
    //unsigned short mIndex;
    SceneNode *mSceneNode;
    std::vector<Entity*> mEntities;
    Vector3 mPosition;
    bool mHidden;               //for temporary hiding
    std::vector<ObjectListener*> mListenerList;
public:
    bool mIsInEarshot;          //for sounds, effects, nametag
    bool mIsInvisible;          //for permanent hiding, disables earshot
    Object()
    {
        mSceneNode = 0;
        mIsInEarshot = false;
        mHidden = false;
        mIsInvisible = false;
    }
    virtual ~Object()
    {
        destroy();
        mListenerList.clear();
    }
    virtual void create(SceneManager *sceneMgr, /*const unsigned short &index,*/ const bool pitchNode=false)
    {
        if(mSceneNode)destroy();
        //mIndex = index;
        static unsigned long mIndex;
        const String nodeName = "Object"+StringConverter::toString(mIndex++);
        if(sceneMgr->hasSceneNode(nodeName))mSceneNode = sceneMgr->getSceneNode(nodeName);
        else mSceneNode = sceneMgr->getRootSceneNode()->createChildSceneNode(nodeName);
    }
    void destroy()
    {
        fireObjectEvent(ObjectListener::OBJECT_DESTROYED);
        if(!mSceneNode)return;
        removeMeshes();
        //mIndex = 0;
        mSceneNode->resetToInitialState();
        mSceneNode = 0;     //Leave SceneManager to clear scenenode automatically
    }
    void addMesh(const String &meshName, const String &matName="")
    {
        if(meshName=="")
        {
            mEntities.push_back(0);
            return;
        }
        static unsigned long mIndex;
        Entity *ent = mSceneNode->getCreator()->createEntity("Ent"+StringConverter::toString(mIndex++)+"_"+StringConverter::toString((int)mEntities.size()),meshName);
        if(matName!="")ent->setMaterialName(matName);
        if(mSceneNode->numChildren())static_cast<SceneNode*>(mSceneNode->getChild(0))->attachObject(ent);
        else mSceneNode->attachObject(ent);
        mEntities.push_back(ent);
    }
    void removeMeshes()
    {
        if(!mSceneNode)return;
        if(mSceneNode->numChildren())static_cast<SceneNode*>(mSceneNode->getChild(0))->detachAllObjects();
        mSceneNode->detachAllObjects();
        SceneManager *sceneMgr = mSceneNode->getCreator();
        while(!mEntities.empty())
        {
            Entity *ent = mEntities.back();
            mEntities.pop_back();
            if(ent)sceneMgr->destroyEntity(ent);
        }
    }
    void addListener(ObjectListener *listener)
    {
        mListenerList.push_back(listener);
    }
    const bool removeListener(ObjectListener *listener)
    {
        for(std::vector<ObjectListener*>::iterator it=mListenerList.begin(); it!=mListenerList.end(); it++)
        {
            ObjectListener *compare = *it;
            if(compare==listener)
            {
                mListenerList.erase(it);
                return true;
            }
        }
        return false;
    }
    void setPosition(const Vector3 &pos)
    {
        mPosition = pos;
        mSceneNode->setPosition(mPosition);
    }
    const Vector3 getPosition()
    {
        return mPosition;
    }
    const Vector3 getScale()
    {
        return mSceneNode->getScale();
    }
    virtual const unsigned char getType()
    {
        return OBJECTTYPE_OBJECT;
    }
    void setVisible(const bool &flag)
    {
        if(!mHidden)mSceneNode->setVisible(flag);
    }
    void forceHide(const bool &flag)
    {
        mHidden = flag;
        if(flag)mSceneNode->setVisible(false);
    }
    SceneNode* getSceneNode()
    {
        return mSceneNode;
    }
    const std::vector<Entity*> getEntities()
    {
        return mEntities;
    }
    void setInvisible(const bool &flag)
    {
        mIsInvisible = flag;
        if(flag)
        {
            mIsInEarshot = false;
            mSceneNode->setVisible(false);
            fireObjectEvent(ObjectListener::OBJECT_INVISIBLE);
        }
        else fireObjectEvent(ObjectListener::OBJECT_VISIBLE);
    }
    void setQueryFlags(uint32 flags)
    {
        for(std::vector<Entity*>::iterator it=mEntities.begin(); it!=mEntities.end(); it++)
        {
            Entity *ent = *it;
            if(ent)ent->setQueryFlags(flags);
        }
    }
    void fireObjectEvent(const unsigned short &event)
    {
        for(std::vector<ObjectListener*>::iterator it=mListenerList.begin(); it!=mListenerList.end(); it++)
        {
            ObjectListener *listener = *it;
            listener->objectEvent(this,event);
        }
    }
};

struct AnimatedAnim
{
    Entity *mEnt;
    AnimationState *mAnim;
    Real mSpeed;
    Real m_fStartTime;
    AnimatedAnim(Entity *ent)
    {
        mEnt = ent;
        mAnim = 0;
    }
    void setAnimation(const String &animName, const Real &speed=1, const bool &loop=false, const bool &resetTime=true, const Real &fStartTime=0.0f)
    {
        if(mAnim)mAnim->setEnabled(false);
        AnimationState *anim = mEnt->getAnimationState(animName);
        anim->setEnabled(true);
        anim->setWeight(1);
        anim->setLoop(loop && fStartTime<=0.0f);
        if(resetTime)anim->setTimePosition(fStartTime);
        mAnim = anim;
        mSpeed = speed;
        m_fStartTime = fStartTime;
    }
    void stopAnimation()
    {
        if(mAnim)mAnim->setEnabled(false);
        mAnim = 0;
    }
    void setWeight(const Real &weight)
    {
        if(mAnim)mAnim->setWeight(weight);
    }
    const bool hasEnded()
    {
        if(mAnim)return (mSpeed<0.0f ? (mAnim->getTimePosition()<=m_fStartTime) : mAnim->hasEnded());
        return false;
    }
    void setTimePosition(const Real &time)
    {
        if(mAnim)mAnim->setTimePosition(time);
    }
    const Real getTimePosition()
    {
        if(mAnim)return mAnim->getTimePosition();
        return 0.0f;
    }
    const bool isOscillating()
    {
        if(mAnim)return (m_fStartTime > 0.0f);
        return false;
    }
};

struct FixedAnim
{
    Entity *mEnt;
    AnimationState *mAnim;
    FixedAnim(Entity *ent)
    {
        mEnt = ent;
        mAnim = 0;
    }
    void setAnimation(const String &animName, const Real &timePos=0)
    {
        if(mAnim)mAnim->setEnabled(false);
        AnimationState *anim = mEnt->getAnimationState(animName);
        anim->setEnabled(true);
        anim->setLoop(false);
        anim->setTimePosition(timePos * anim->getLength());
        mAnim = anim;
    }
    void stopAnimation()
    {
        if(mAnim)mAnim->setEnabled(false);
        mAnim = 0;
    }
    void setTime(const Real &timePos)
    {
        if(mAnim)mAnim->setTimePosition(timePos * mAnim->getLength());
    }
};

class Animated : public Object
{
protected:
    std::vector<AnimatedAnim*> mAnims;
    std::vector<FixedAnim*> mFixedAnims;
public:
    Animated()
    {
    }
    virtual ~Animated()
    {
        clearAnims();
    }
    void clearAnims()
    {
        while(!mAnims.empty())
        {
            AnimatedAnim *anim = mAnims.back();
            mAnims.pop_back();
            if(anim)delete anim;
        }
        while(!mFixedAnims.empty())
        {
            FixedAnim *anim = mFixedAnims.back();
            mFixedAnims.pop_back();
            if(anim)delete anim;
        }
    }
    virtual void update(const Real &timeElapsed)
    {
        updateAnimations(timeElapsed);
    }
    void updateAnimations(const Real &timeElapsed)
    {
        for(std::vector<AnimatedAnim*>::iterator i=mAnims.begin(); i!=mAnims.end(); i++)
        {
            AnimatedAnim *anim = *i;
            if(anim && anim->mAnim)
            {
                anim->mAnim->addTime(anim->mSpeed*timeElapsed);
                //Oscillating anims
                if(anim->isOscillating() && anim->hasEnded())
                {
                    anim->setTimePosition(anim->mSpeed>0.0f ? anim->mAnim->getLength() : anim->m_fStartTime);
                    anim->mSpeed *= -1;
                }
            }
        }
    }
    void addAnimState(const unsigned short &entIndex)
    {
        mAnims.push_back(mEntities[entIndex] ? new AnimatedAnim(mEntities[entIndex]) : 0);
    }
    AnimatedAnim* addAnimStateEntity(Entity *pEnt)
    {
        mAnims.push_back(new AnimatedAnim(pEnt));
        return mAnims.back();
    }
    void removeAnimStateEntity(Entity *pEnt)
    {
        for(std::vector<AnimatedAnim*>::iterator it=mAnims.begin(); it!=mAnims.end(); it++)
        {
            AnimatedAnim *pAnim = *it;
            if(pAnim && pAnim->mEnt==pEnt)
            {
                mAnims.erase(it);
                delete pAnim;
                return;
            }
        }
    }
    void addFixedAnimState(const unsigned short &entIndex)
    {
        mFixedAnims.push_back(mEntities[entIndex] ? new FixedAnim(mEntities[entIndex]) : 0);
    }
    void setAnimation(const unsigned short &animIndex, const String &animName, const Real &speed=1, const bool &loop=false)
    {
        if(animIndex>=(int)mAnims.size() || !mAnims[animIndex])return;
        mAnims[animIndex]->setAnimation(animName,speed,loop);
    }
    void stopAnimation(const unsigned short &animIndex)
    {
        if(animIndex>=(int)mAnims.size() || !mAnims[animIndex])return;
        mAnims[animIndex]->stopAnimation();
    }
    void setFixedAnimation(const unsigned short &animIndex, const String &animName, const Real &timePos=0)
    {
        if(animIndex>=(int)mFixedAnims.size() || !mFixedAnims[animIndex])return;
        mFixedAnims[animIndex]->setAnimation(animName,timePos);
    }
    void stopFixedAnimation(const unsigned short &animIndex)
    {
        if(animIndex>=(int)mFixedAnims.size() || !mFixedAnims[animIndex])return;
        mFixedAnims[animIndex]->stopAnimation();
    }
    void setFixedAnimTime(const unsigned short &animIndex, const Real &timePos)
    {
        if(animIndex>=(int)mFixedAnims.size() || !mFixedAnims[animIndex])return;
        mFixedAnims[animIndex]->setTime(timePos);
    }
    virtual const unsigned char getType()
    {
        return OBJECTTYPE_ANIMATED;
    }
};

class Movable : public Animated
{
protected:
    SceneNode *mPitchNode;
    Real mYaw;
    Real mPitch;
    Real mRoll;
    Vector3 m_cVelocity;
    Vector3 m_cVelocityChange;
    Vector3 m_cExternalVelocity;
    Vector3 m_cExternalVelocityChange;
    Vector3 mPenaltyDisplacement;
    Real mMaxVelocity;
    Real mGroundHeight;
    Real mGroundN, mGroundS, mGroundE, mGroundW;
    Real m_fRemoteGroundHeight;
    Real mFeetFront, mFeetBack;
public:
    bool mAntiGravity;
    bool mApplyMaxVelocity;
    bool mApplySlopeForces;
    Movable()
    {
        mPitchNode = 0;
        mFeetFront = mFeetBack = 4;
        mApplySlopeForces = true;
        clearForces();
    }
    virtual ~Movable()
    {
        clearForces();
        mPitchNode = 0;
    }
    virtual void create(SceneManager *sceneMgr, /*const unsigned short &index,*/const bool pitchNode=false)
    {
        if(mSceneNode)destroy();
        //mIndex = index;
        static unsigned long mIndex;
        const String nodeName = "ObjectM"+StringConverter::toString(mIndex++);
        if(sceneMgr->hasSceneNode(nodeName))mSceneNode = sceneMgr->getSceneNode(nodeName);
        else mSceneNode = sceneMgr->getRootSceneNode()->createChildSceneNode(nodeName);

        if(pitchNode)
        {
            const String pitchName = "ObjectP"+StringConverter::toString(mIndex);
            if(sceneMgr->hasSceneNode(pitchName))mPitchNode = sceneMgr->getSceneNode(pitchName);
            else mPitchNode = mSceneNode->createChildSceneNode(pitchName);
        }
    }
    virtual void preUpdate(const Real &timeElapsed)
    {
        updateMovement(timeElapsed);
    }
    virtual void update(const Real &timeElapsed)
    {
        updateMovementOffset(timeElapsed);
        updateAnimations(timeElapsed);
    }
    void clearForces()
    {
        mAntiGravity = false;
        m_cVelocity = m_cVelocityChange = Vector3::ZERO;
        m_cExternalVelocity = m_cExternalVelocityChange = Vector3::ZERO;
        mPenaltyDisplacement = Vector3::ZERO;
        mYaw = 0;
        mPitch = 0;
        mRoll = 0;
        mMaxVelocity = TERMINAL_VELOCITY;
        mApplyMaxVelocity = true;
        mGroundHeight = 0;
        mGroundN = mGroundS = mGroundE = mGroundW = 0;
        m_fRemoteGroundHeight = -1.0f;
    }
    void addVelocity(const Vector3 &v)
    {
        m_cVelocityChange += v;
    }
    void setVelocityY(const Real &y)
    {
        m_cVelocityChange.y = y;
    }
    void addExternalVelocity(const Vector3 &v)
    {
        m_cExternalVelocityChange += v;
    }
    void addPenaltyDisplacement(const Vector3 &d)
    {
        mPenaltyDisplacement += d;
    }
    const Vector3 getPositionAfterPenalty()
    {
        return mPosition + mPenaltyDisplacement;
    }
    const Vector3 getVelocity()
    {
        return m_cVelocity;
    }
    void setMaxVelocity(const Real &vel)
    {
        mMaxVelocity = vel;
    }
    void setYaw(const Real &value, const bool &instant=false)
    {
        if(instant)yaw(value-mYaw,true);
        else mYaw = value;
    }
    const Real getYaw()
    {
        return mYaw;
    }
    void yaw(const Real &yaw, const bool &instant=false)
    {
        mYaw += yaw;
        if(mYaw>=360)mYaw -= 360;
        if(mYaw<0)mYaw += 360;
        if(instant)mSceneNode->yaw(Degree(yaw));
    }
    void setPitch(const Real &pitch)
    {
        mPitch = pitch;
    }
    const Real getPitch()
    {
        return mPitch;
    }
    const Real getRoll()
    {
        return mRoll;
    }
    const bool isOnGentleSlope()
    {
        return (Math::Abs(mPitch)<STEEP_ANGLE && Math::Abs(mRoll)<STEEP_ANGLE);
    }
    SceneNode* getPitchNode()
    {
        return mPitchNode;
    }
    void updateMovement(const Real &timeElapsed)
    {
        //Update yaw
        Real currentYaw = mSceneNode->getOrientation().getYaw().valueDegrees();
		if(currentYaw < 0)currentYaw += 360;
		else if(currentYaw >= 360)currentYaw -= 360;
		Real yaw = mYaw - currentYaw;
		if(Math::Abs(yaw)>180)yaw -= 360 * (yaw<0?-1:1);
		if(Math::Abs(yaw)>0.1f)
		{
		    const Real ratio = 5*timeElapsed;
		    yaw *= (ratio<1?ratio:1);
		}
		if(yaw!=0)mSceneNode->yaw(Degree(yaw));

        const Real friction = FRICTION * timeElapsed;
        if(mAntiGravity)
        {
            //Air resistance
            applyAirResistanceOn(m_cVelocity,m_cVelocityChange,friction);
        }
        //Gravity
        else if(mPosition.y > mGroundHeight)m_cVelocityChange.y -= GRAVITY * timeElapsed;
        applyAirResistanceOn(m_cExternalVelocity,m_cExternalVelocityChange,friction);

        if(m_cVelocity!=Vector3::ZERO || m_cVelocityChange!=Vector3::ZERO)
        {
            //Friction
            applyFrictionOn(m_cVelocity,m_cVelocityChange,friction);
            //Cap velocity
            if(mApplyMaxVelocity)
            {
                capVelocity(m_cVelocity,m_cVelocityChange,mMaxVelocity,mAntiGravity);
            }
            //Terminal Velocity
            capValue(m_cVelocity.y,m_cVelocityChange.y,TERMINAL_VELOCITY);
        }
        if(m_cExternalVelocity!=Vector3::ZERO || m_cExternalVelocityChange!=Vector3::ZERO)
        {
            //Friction
            applyFrictionOn(m_cExternalVelocity,m_cExternalVelocityChange,friction);
            //Terminal Velocity
            capValue(m_cExternalVelocity.y,m_cExternalVelocityChange.y,TERMINAL_VELOCITY);
        }

        //Update displacement
        const Vector3 cResultantVelocityChange = m_cVelocityChange + m_cExternalVelocityChange;
        const Vector3 cResultantVelocity = m_cVelocity + m_cExternalVelocity;

        setPosition(mPosition + cResultantVelocity*timeElapsed + 0.5f*cResultantVelocityChange*timeElapsed);
        m_cVelocity += m_cVelocityChange;
        m_cVelocityChange = Vector3::ZERO;
        m_cExternalVelocity += m_cExternalVelocityChange;
        m_cExternalVelocityChange = Vector3::ZERO;
    }
    void updateMovementOffset(const Real &timeElapsed)
    {
        //Apply penalty displacement
        if(mPenaltyDisplacement!=Vector3::ZERO)
        {
            setPosition(mPosition + mPenaltyDisplacement);
            mPenaltyDisplacement = Vector3::ZERO;
        }
        //Clamp to ground
        const bool falling = (m_cVelocity.y<=0);
        const Real fallingSpeed = falling?m_cVelocity.y:0;
        if(mAntiGravity)
        {
            if(mPosition.y<mGroundHeight)setPosition(Vector3(mPosition.x,mGroundHeight,mPosition.z));
        }
        else if((mPosition.y - mGroundHeight) < (falling?GROUND_THRESHOLD*timeElapsed:0))
        {
            setPosition(Vector3(mPosition.x,mGroundHeight,mPosition.z));
            m_cVelocity.y = 0;
            m_cExternalVelocity.y = 0;
        }

        //Stop on steep slope
        const Radian angleNS = (mPitchNode ? Math::ATan((mGroundS-mGroundN)/(mFeetFront+mFeetBack)) : Radian(0));
        const Radian angleEW = (mPitchNode ? Math::ATan((mGroundW-mGroundE)/8) : Radian(0));
        if(mApplySlopeForces && (Math::Abs(angleNS)>=Degree(STEEP_ANGLE) || Math::Abs(angleEW)>=Degree(STEEP_ANGLE)) && isGrounded())m_cVelocity.x = m_cVelocity.z = 0;

        if(mPitchNode)
		{
		    //Decay pitch in air
		    if(!isGrounded())
		    {
		        const Real dPitch = mPitch*timeElapsed*2;
		        if(Math::Abs(mPitch)<Math::Abs(dPitch))mPitch -= dPitch;
		        else mPitch = 0;
		    }
		    //Pitch to north/south ground heights
		    else
		    {
		        setPitch(angleNS.valueDegrees());
                mRoll = angleEW.valueDegrees();
		    }

            //Apply slope forces
            if(mApplySlopeForces)
            {
                const Real forceNS = (Math::Abs(angleNS)>Degree(30) ? Math::Sin(angleNS)*MOVE_FORCE*timeElapsed : 0);
                const Real forceEW = (Math::Abs(angleEW)>Degree(30) ? Math::Sin(angleEW)*MOVE_FORCE*timeElapsed : 0);
                if(forceNS!=0 || forceEW!=0)
                {
                    if(isGrounded())
                    {
                        //Forces caused by slopes will not exceed max velocity
                        Vector3 slopeVelocity = mSceneNode->getOrientation()*Vector3(forceEW,0,forceNS);
                        Vector3 cExternalVelocity = m_cExternalVelocity;
                        capVelocity(cExternalVelocity,slopeVelocity,TERMINAL_VELOCITY);
                        //if(m_cExternalVelocity.squaredLength() < slopeVelocity.squaredLength())
                        addExternalVelocity(slopeVelocity);
                        //For falling from steep slopes
                        m_cVelocity.y = isOnGentleSlope()?0:fallingSpeed;
                    }
                    //Falling with any groundheight higher than current height deserves a push
                    else if(m_cVelocity.y<0)
                    {
                        if(mGroundN>mPosition.y || mGroundS>mPosition.y)addExternalVelocity(mSceneNode->getOrientation()*Vector3(0,0,forceNS));
                        if(mGroundE>mPosition.y || mGroundW>mPosition.y)addExternalVelocity(mSceneNode->getOrientation()*Vector3(forceEW,0,0));
                    }
                }
            }

            //Update pitch
		    Real currentPitch = mPitchNode->getOrientation().getPitch().valueDegrees();
		    if(currentPitch < 0)currentPitch += 360;
            else if(currentPitch >= 360)currentPitch -= 360;
            Real pitch = mPitch - currentPitch;
            if(Math::Abs(pitch)>180)pitch -= 360 * (pitch<0?-1:1);
            if(Math::Abs(pitch)>0.1f)
            {
                const Real ratio = 10*timeElapsed;
                pitch *= (ratio<1?ratio:1);
            }
            if(pitch!=0)mPitchNode->pitch(Degree(pitch));
		}
    }
    void applyFrictionOn(const Vector3 &cVelocity, Vector3 &cVelocityChange, const Real &friction)
    {
        const Vector3 cResultantVelocity = cVelocity + cVelocityChange;

        if(cResultantVelocity.x==0)
        {
            if(Math::Abs(cResultantVelocity.z) < friction)cVelocityChange.z = -cVelocity.z;
            else cVelocityChange.z -= friction * (cResultantVelocity.z<0?-1:1);
        }
        else if(cResultantVelocity.z==0)
        {
            if(Math::Abs(cResultantVelocity.x) < friction)cVelocityChange.x = -cVelocity.x;
            else cVelocityChange.x -= friction * (cResultantVelocity.x<0?-1:1);
        }
        else
        {
            const Radian angle = Math::ATan(Math::Abs(cResultantVelocity.z)/Math::Abs(cResultantVelocity.x));
            const Real x = Math::Cos(angle)*friction;
            const Real z = Math::Sin(angle)*friction;
            if(Math::Abs(cResultantVelocity.x) < x)cVelocityChange.x = -cVelocity.x;
            else cVelocityChange.x -= x * (cResultantVelocity.x<0?-1:1);
            if(Math::Abs(cResultantVelocity.z) < z)cVelocityChange.z = -cVelocity.z;
            else cVelocityChange.z -= z * (cResultantVelocity.z<0?-1:1);
        }
    }
    void applyAirResistanceOn(const Vector3 &cVelocity, Vector3 &cVelocityChange, const Real &friction)
    {
        const Real fResultantVelocity = cVelocity.y + cVelocityChange.y;

        if(fResultantVelocity!=0)
        {
            if(Math::Abs(fResultantVelocity) < friction)
            {
                cVelocityChange.y = -cVelocity.y;
            }
            else cVelocityChange.y -= friction * (fResultantVelocity<0?-1:1);
        }
    }
    void capValue(Real &fVelocity, Real &fVelocityChange, const Real &fCap)
    {
        //Is resultant velocity more than max velocity?
        if(Math::Abs(fVelocity + fVelocityChange) > fCap)
        {
            //Is velocity itself more than max velocity?
            if(Math::Abs(fVelocity) > fCap)
            {
                //Cap velocity, eliminate change
                fVelocity = fCap * (fVelocity<0?-1:1);
                fVelocityChange = 0;
            }
            //Change causes exceed in max velocity
            else
            {
                //Cap change
                const Real fChangeCap = fCap - Math::Abs(fVelocity);
                fVelocityChange = fChangeCap * (fVelocityChange<0?-1:1);
            }
        }
    }
    void capVelocity(Vector3 &cVelocity, Vector3 &cVelocityChange, const Real &maxVelocity, const bool &capY=false)
    {
        //Is resultant velocity more than max velocity?
        const Vector3 cResultantVelocity = cVelocity + cVelocityChange;
        const Real fMaxSquared = maxVelocity*maxVelocity;
        if(Vector2(cResultantVelocity.x,cResultantVelocity.z).squaredLength() >= fMaxSquared)
        {
            if(cResultantVelocity.x==0)capValue(cVelocity.z,cVelocityChange.z,maxVelocity);
            else if(cResultantVelocity.z==0)capValue(cVelocity.x,cVelocityChange.x,maxVelocity);
            else
            {
                const Radian angle = Math::ATan(Math::Abs(cResultantVelocity.z)/Math::Abs(cResultantVelocity.x));
                capValue(cVelocity.x, cVelocityChange.x, Math::Cos(angle)*maxVelocity);
                capValue(cVelocity.z, cVelocityChange.z, Math::Sin(angle)*maxVelocity);
            }
        }
        if(capY)
        {
           capValue(cVelocity.y,cVelocityChange.y,maxVelocity);
        }
    }
    const bool isGrounded()
    {
        return (mPosition.y <= mGroundHeight);
    }
    const Quaternion getOrientation(const bool &excludePitch=false)
    {
        if(!mPitchNode || excludePitch)return mSceneNode->getOrientation();
        return Quaternion(mSceneNode->getOrientation().getYaw(),Vector3::UNIT_Y)*Quaternion(mPitchNode->getOrientation().getPitch(),Vector3::UNIT_X)*Quaternion(Degree(0),Vector3::UNIT_Z);
    }
    void getGroundPoints(Vector3 &n, Vector3 &s, Vector3 &e, Vector3 &w)
    {
        if(!mSceneNode)return;
        const Quaternion dir = mSceneNode->getOrientation();
        n = mPosition + dir*Vector3(0,0,mFeetFront);
        s = mPosition + dir*Vector3(0,0,-mFeetBack);
        e = mPosition + dir*Vector3(mFeetFront,0,0);
        w = mPosition + dir*Vector3(-mFeetFront,0,0);
    }
    void setGroundHeights(const Real &n, const Real &s, const Real &e, const Real &w)
    {
        if(!mSceneNode)return;

        mGroundHeight = (n+s+e+w)*0.25f;
        mGroundN = n;
        mGroundS = s;
        mGroundE = e;
        mGroundW = w;
    }
    void setGroundHeights(const Real &n, const Real &s)
    {
        if(!mSceneNode)return;

        mGroundN = n;
        mGroundS = s;
    }
    void setGroundHeight(const Real &height)
    {
        mGroundHeight = height;
    }
    const Real getGroundHeight()
    {
        return mGroundHeight;
    }
    void setRemoteGroundHeight(const Real &fHeight)
    {
        m_fRemoteGroundHeight = fHeight;
    }
    const Real getRemoteGroundHeight()
    {
        return m_fRemoteGroundHeight;
    }
    const Real getGroundHeight(const unsigned char &dir)
    {
        switch(dir)
        {
            case 0: return mGroundN; break;
            case 1: return mGroundS; break;
            case 2: return mGroundE; break;
            case 3: return mGroundW; break;
            default: break;
        }
        return 0;
    }
    void setFeetDistances(const Real &front, const Real &back)
    {
        mFeetFront = front*getScale().z;
        mFeetBack = back*getScale().z;
    }
    const Real getFrontFeetDistance()
    {
        return mFeetFront;
    }
    const Real getHeadHeight()
    {
        return 8*getScale().y;
    }
    virtual const unsigned char getType()
    {
        return OBJECTTYPE_MOVABLE;
    }
};

#endif
