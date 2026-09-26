#ifndef _CONTROLLABLE_H_
#define _CONTROLLABLE_H_

#include <Object.h>

class Controllable : public Movable
{
protected:
    Vector3 mMoveTarget;
    bool mRunToTarget;
    bool mReverseYaw;
public:
    Controllable()
    {
        stop();
        mReverseYaw = false;
    }
    virtual ~Controllable()
    {
        stop();
    }
    virtual void update(const Real &timeElapsed)
    {
        updateControl(timeElapsed);
        updateMovementOffset(timeElapsed);
        updateAnimations(timeElapsed);
    }
    void updateControl(const Real &timeElapsed)
    {
        if(mRunToTarget)
        {
            const Vector3 moveDirection = mMoveTarget - mPosition;
            if(moveDirection.x==0)
            {
                addVelocity(Vector3(0,0,MOVE_FORCE * timeElapsed * (moveDirection.z<0?-1:1)));
                setYaw(moveDirection.z<0? 180 : 0);
            }
            else if(moveDirection.z==0)
            {
                addVelocity(Vector3(MOVE_FORCE * timeElapsed * (moveDirection.x<0?-1:1),0,0));
                setYaw(moveDirection.x<0? 270 : 90);
            }
            else
            {
                const Radian angle = Math::Abs(Math::ATan(moveDirection.x/moveDirection.z));
                addVelocity(Vector3(Math::Sin(angle)*(moveDirection.x<0?-1:1),0,Math::Cos(angle)*(moveDirection.z<0?-1:1))*MOVE_FORCE*timeElapsed);
                if(moveDirection.x>0)
                {
                    if(moveDirection.z<0)setYaw(180-angle.valueDegrees());
                    else setYaw(angle.valueDegrees());
                }
                else if(moveDirection.z<0)setYaw(180+angle.valueDegrees());
                else setYaw(360-angle.valueDegrees());
            }
            if(mReverseYaw)
            {
                Real yaw = getYaw() - 180;
                if(yaw<0)yaw += 360;
                setYaw(yaw);
            }

            const Vector3 cResultantOffset = m_cVelocity*timeElapsed + m_cVelocityChange*timeElapsed*0.5f;

            if(mAntiGravity && moveDirection.y!=0)
            {
                addVelocity(Vector3(0,MOVE_FORCE * timeElapsed * (moveDirection.y<0?-1:1),0));
                bool yCrossed = false;
                if(moveDirection.y>0)yCrossed = (cResultantOffset.y > moveDirection.y);
                else yCrossed = (cResultantOffset.y < moveDirection.y);
                if(yCrossed)
                {
                    setPosition(Vector3(mPosition.x,mMoveTarget.y,mPosition.z));
                    m_cVelocity.y = m_cVelocityChange.y = 0;
                }
            }

            bool xCrossed = false;
            bool zCrossed = false;
            if(moveDirection.x>0)xCrossed = (cResultantOffset.x > moveDirection.x);
            else xCrossed = (cResultantOffset.x < moveDirection.x);
            if(moveDirection.z>0)zCrossed = (cResultantOffset.z > moveDirection.z);
            else zCrossed = (cResultantOffset.z < moveDirection.z);

            if((xCrossed && zCrossed) || Vector2(moveDirection.x,moveDirection.z).squaredLength()<0.1f)
            {
                if(!mAntiGravity || mMoveTarget.y==mPosition.y)
                {
                    mRunToTarget = false;
                    fireControllableEvent(ObjectListener::CONTROLLABLE_STOPPED);
                }
                setPosition(Vector3(mMoveTarget.x,mPosition.y,mMoveTarget.z));
                m_cVelocity.x = m_cVelocityChange.x = 0;
                m_cVelocity.z = m_cVelocityChange.z = 0;
            }
        }
    }
    void setMoveTarget(const Vector3 &target)
    {
        mMoveTarget = target;
        mRunToTarget = true;
        fireControllableEvent(ObjectListener::CONTROLLABLE_STARTED);
    }
    void setReverseYaw(const bool &flag)
    {
        mReverseYaw = flag;
    }
    void stop()
    {
        mRunToTarget = false;
        fireControllableEvent(ObjectListener::CONTROLLABLE_STOPPED);
    }
    const bool isAutoRunning()
    {
        return mRunToTarget;
    }
    virtual const unsigned char getType()
    {
        return OBJECTTYPE_CONTROLLABLE;
    }
    void fireControllableEvent(const unsigned short &event)
    {
        for(std::vector<ObjectListener*>::iterator it=mListenerList.begin(); it!=mListenerList.end(); it++)
        {
            ObjectListener *listener = *it;
            listener->controllableEvent(this,event);
        }
    }
};

class Living : public Controllable
{
protected:
    int mHP;
    int mMaxHP;
    bool mIsDying;
    bool mIsDisabled;
    Real mDeathTimer;
public:
    Living()
    {
        resetDeath();
    }
    virtual ~Living()
    {
        resetDeath();
    }
    void resetDeath()
    {
        mHP = 0;
        mMaxHP = 0;
        mIsDying = false;
        mIsDisabled = false;
        mDeathTimer = 0;
    }
    virtual void update(const Real &timeElapsed)
    {
        if(mIsDying)updateDeath(timeElapsed);
        else if(!mIsDisabled)updateControl(timeElapsed);
        updateMovementOffset(timeElapsed);
        updateAnimations(timeElapsed);
    }
    void updateDeath(const Real &timeElapsed)
    {
        mDeathTimer -= timeElapsed;
        if(mDeathTimer<=0)
        {
            //DIE
            resetDeath();
            mIsDisabled = true;
        }
    }
    void kill()
    {
        mHP = 0;
        mIsDying = true;
        mDeathTimer = 3;
    }
    const bool addHP(const int &hp)
    {
        mHP += hp;
        if(mHP>mMaxHP)mHP = mMaxHP;
        else if(mHP<=0)
        {
            mHP = 0;
            return true;
        }
        return false;
    }
    void setMaxHP(const int &maxhp, const bool &sethp)
    {
        mMaxHP = maxhp;
        if(sethp)mHP = mMaxHP;
    }
    void setHPRatio(const Real &ratio)
    {
        mHP = (int)ratio*mMaxHP;
    }
    const Real getHPRatio()
    {
        if(mMaxHP==0)return 0;
        return Real(mHP/mMaxHP);
    }
    virtual const unsigned char getType()
    {
        return OBJECTTYPE_LIVING;
    }
};

#endif
