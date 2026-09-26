#ifndef _CONTROLMANAGER_H_
#define _CONTROLMANAGER_H_

#include <CollisionManager.h>
#include <CameraManager.h>
#include <UnitManager.h>
#include <NetworkManager.h>
#include <ActionsManager.h>
#include <InfoManager.h>
#include <SkillsManager.h>

class ControlManager : public Singleton<ControlManager>, public ObjectListener, public SkillsListener
{
private:
    Unit *mPlayer;
    CollisionManager *mCollMgr;
    CameraManager *mCameraMgr;
    UnitManager *mUnitMgr;
    NetworkManager *mNetMgr;
    InfoManager *mInfoMgr;
    Gui *mGui;
    ActionsManager *mActionsMgr;
    SkillsManager *mSkillsMgr;
    unsigned char mControlMode;
    Real mCamSpeed;

    SceneNode *mPointerNode;
    Entity *mPointer;
    SceneNode *mSelectPointerNode;
    BillboardSet *mSelectPointerSet;
    Billboard *mSelectPointer;
    bool mDisabled;
    bool mHeadTracking;
    bool mRelativeMode;
    bool m_bPlayerDisabled;

    Object *mSelectObject;
    Real mDoubleClickTimer;
public:
    enum ControlMode
    {
        CONTROL_FIXED,
        CONTROL_FREECAM,
        CONTROL_CHASE,
        CONTROL_FIRSTPERSON
    };
    bool up,down,left,right,front,back;
    bool turnUp,turnDown,turnLeft,turnRight;
    ControlManager()
    {
        mControlMode = CONTROL_FIXED;
        mPointerNode = 0;
        mPointer = 0;
        mSelectPointerNode = 0;
        mSelectPointerSet = 0;
        mSelectPointer = 0;
        mHeadTracking = true;
        mRelativeMode = false;
        mActionsMgr = 0;
        mSkillsMgr = 0;
        reset();
    }
    ~ControlManager()
    {
        reset();
    }
    static ControlManager* getSingletonPtr();
    static ControlManager& getSingleton();
    void reset()
    {
        mPlayer = 0;
        mCameraMgr = 0;
        up = down = left = right = front = back = false;
        turnUp = turnDown = turnLeft = turnRight = false;
        mCamSpeed = 200;
        mDisabled = false;
        m_bPlayerDisabled = false;

        if(mPointer)
        {
            SceneManager *sceneMgr = mPointerNode->getCreator();
            mPointerNode->detachAllObjects();
            sceneMgr->destroyEntity(mPointer);
            mPointerNode = 0;
            mPointer = 0;
        }
        if(mSelectPointerSet)
        {
            SceneManager *sceneMgr = mSelectPointerNode->getCreator();
            mSelectPointerNode->detachAllObjects();
            sceneMgr->destroyBillboardSet(mSelectPointerSet);
            mSelectPointerNode = 0;
            mSelectPointerSet = 0;
            mSelectPointer = 0;
        }
        if(mActionsMgr)
        {
            mActionsMgr->reset();
            mActionsMgr = 0;
        }
        if(mSkillsMgr)
        {
            mSkillsMgr->clear();
            mSkillsMgr = 0;
        }

        mSelectObject = 0;
        mDoubleClickTimer = 0;
        mInfoMgr = 0;
    }
    void init(Unit *player, SceneManager *sceneMgr, const bool &inGame=false)
    {
        mPlayer = player;
        if(player)
        {
            player->addListener(this);
            if(mControlMode==CONTROL_FREECAM)mControlMode = CONTROL_FIXED;
        }
        mCollMgr = CollisionManager::getSingletonPtr();
        mCollMgr->setMouseQueryMask(Collision::UNIT_MASK|Collision::MESH_COLL_MASK);
        mCameraMgr = CameraManager::getSingletonPtr();
        mUnitMgr = UnitManager::getSingletonPtr();
        mNetMgr = NetworkManager::getSingletonPtr();
        setControlMode(player?mControlMode:CONTROL_FREECAM);
        mGui = Gui::getSingletonPtr();

        mSkillsMgr = SkillsManager::getSingletonPtr();
        mSkillsMgr->init(mPlayer);
        mSkillsMgr->addListener(this);
        if(mPlayer)
        {
            if(inGame)mInfoMgr = InfoManager::getSingletonPtr();
            mActionsMgr = ActionsManager::getSingletonPtr();
            mActionsMgr->init(mPlayer);
            mSkillsMgr->addListener(mActionsMgr);
        }


        if(sceneMgr)
        {
            mPointerNode = sceneMgr->getRootSceneNode()->createChildSceneNode();
            mPointer = sceneMgr->createEntity("ControlPointer","cursor.mesh");
            mPointer->getAnimationState("Idle")->setEnabled(true);
            mPointer->getAnimationState("Idle")->setLoop(true);
            mPointerNode->attachObject(mPointer);
            mPointerNode->setVisible(false);

            mSelectPointerNode = sceneMgr->getRootSceneNode()->createChildSceneNode();
            mSelectPointerSet = sceneMgr->createBillboardSet("SelectPointerSet",1);
            mSelectPointerSet->setMaterialName("GuiMat/SelectPointer");
            mSelectPointerSet->setDefaultDimensions(10,10);//20,20);
            mSelectPointerSet->setBillboardType(BBT_POINT);//BBT_PERPENDICULAR_COMMON);
            //mSelectPointerSet->setCommonDirection(Vector3::UNIT_Y);
            //mSelectPointerSet->setCommonUpVector(Vector3::UNIT_Z);
            mSelectPointer = mSelectPointerSet->createBillboard(Vector3::ZERO);
            mSelectPointerNode->attachObject(mSelectPointerSet);
            mSelectPointerNode->setVisible(false);
        }
    }
    Unit* getPlayer()
    {
        return mPlayer;
    }
    void setPlayer(Unit *player)
    {
        if(mPlayer)mPlayer->removeListener(this);
        mPlayer = player;
        if(mPlayer)mPlayer->addListener(this);
        if(mActionsMgr)mActionsMgr->setPlayer(mPlayer);
        else if(mPlayer)
        {
            mActionsMgr = ActionsManager::getSingletonPtr();
            mActionsMgr->init(mPlayer);
            if(mSkillsMgr)mSkillsMgr->addListener(mActionsMgr);
        }
        if(mSkillsMgr)mSkillsMgr->setPlayer(mPlayer);
    }
    void update(const Real &timeElapsed)
    {
        updateControls(timeElapsed);
        if(mPointer && mPointer->isVisible())mPointer->getAnimationState("Idle")->addTime(timeElapsed);
        if(mDoubleClickTimer>0)mDoubleClickTimer -= timeElapsed;

        if(mSkillsMgr)mSkillsMgr->update(timeElapsed);
    }
    void updateCameraScale()
    {
        if(mPlayer)mCameraMgr->setScale(mPlayer->getScale());
    }
    void updateCamera()
    {
        if(mPlayer && (mControlMode==CONTROL_FIXED || mControlMode==CONTROL_CHASE || mControlMode==CONTROL_FIRSTPERSON))mCameraMgr->setPosition(mPlayer->getPosition());

        if(mPlayer && mPointer && mPointer->isVisible())
        {
            const Real dist = mPlayer->getPosition().distance(mPointerNode->getPosition());
            mPointerNode->setScale((dist>=100?1:(dist*0.01f))*Vector3::UNIT_SCALE);
        }

        //Select pointer updates after units are updated
        if(mSelectPointerSet && mSelectObject && mSelectPointerSet->isVisible())
        {
            mSelectPointerNode->setPosition(mSelectObject->getPosition()+mSelectObject->getScale().z*Vector3(0,6,0));
            const Real dist = mCameraMgr->getPosition().distance(mSelectPointerNode->getPosition());
            mSelectPointerNode->setScale((dist<=100?1:(dist*0.01f))*Vector3::UNIT_SCALE);
        }
    }
    void updateControls(const Real &timeElapsed)
    {
        if(mDisabled)return;
        //Camera turn controls
        if(turnUp||turnDown)mCameraMgr->pitch(90*timeElapsed*(turnDown?1:-1));
        if(turnLeft||turnRight)mCameraMgr->yaw(90*timeElapsed*(turnLeft?1:-1));

        //Movement controls
        const bool fblr = (front||back||left||right);

        if(mPlayer && (mControlMode==CONTROL_FIXED || mControlMode==CONTROL_CHASE || mControlMode==CONTROL_FIRSTPERSON))
        {
            if(mPlayer->isAutoRunning())
            {
                if(fblr)
                {
                    mPlayer->stop();
                    mPlayer->mIsMoving = ((!mRelativeMode&&fblr)||front||back);
                }
            }
            else mPlayer->mIsMoving = (!m_bPlayerDisabled && ((!mRelativeMode&&fblr)||front||back));

            //Head orientaion
            if(mHeadTracking)mPlayer->setHeadYawPitch(mCameraMgr->getYaw(),mCameraMgr->getPitch());

            if(!m_bPlayerDisabled)
            {
                if(fblr)
                {
                    //Relative running mode
                    if(mRelativeMode)
                    {
                        if(left)mPlayer->yaw(180*timeElapsed);
                        else if(right)mPlayer->yaw(-180*timeElapsed);

                        if(back)mPlayer->addVelocity(mPlayer->getOrientation(true)*Vector3::NEGATIVE_UNIT_Z*MOVE_FORCE*timeElapsed);
                        else if(front)mPlayer->addVelocity(mPlayer->getOrientation(true)*Vector3::UNIT_Z*MOVE_FORCE*timeElapsed);
                    }
                    //Absolute running mode
                    else
                    {
                        Real yaw = mCameraMgr->getYaw();
                        Vector2 moveVect = Vector2::ZERO;
                        //Check right angles
                        if(yaw==0||yaw==90||yaw==180||yaw==270)
                        {
                            //North/South
                            if(yaw==0 || yaw==180)moveVect.y = (yaw==180? -1 : 1);
                            //East/West
                            else moveVect.x = (yaw==270? -1 : 1);
                        }
                        else
                        {
                            moveVect.y = Math::Cos(Degree(yaw));
                            moveVect.x = Math::Sin(Degree(yaw));
                        }
                        //Apply forces
                        if(front||back)
                        {
                            mPlayer->addVelocity(Vector3(moveVect.x,0,moveVect.y)*MOVE_FORCE*timeElapsed*(back?-1:1));
                        }
                        if(left||right)
                        {
                            const Vector2 perpVect = moveVect.perpendicular();
                            mPlayer->addVelocity(Vector3(perpVect.x,0,perpVect.y)*MOVE_FORCE*timeElapsed*(left?-1:1));
                        }

                        if(left)yaw += back?(mControlMode==CONTROL_FIRSTPERSON?-45:135):(front?45:90);
                        else if(right)yaw -= back?(mControlMode==CONTROL_FIRSTPERSON?-45:135):(front?45:90);
                        else if(back && mControlMode!=CONTROL_FIRSTPERSON)yaw += 180;
                        if(yaw>=360)yaw -= 360;
                        else if(yaw<0)yaw += 360;
                        mPlayer->setYaw(yaw);
                    }
                }
                if(up||down)
                {
                    if(mPlayer->mAntiGravity)mPlayer->addVelocity(Vector3(0,1,0)*MOVE_FORCE*timeElapsed*(down?-1:1));
                }
            }
        }
        else if(mControlMode==CONTROL_FREECAM)
        {
            if(!(fblr||up||down))return;
            mCameraMgr->translate(left||right ? (mCamSpeed*timeElapsed*(left?1:-1)) : 0,
                                  up||down ? (mCamSpeed*timeElapsed*(up?1:-1)) : 0,
                                  front||back ? (mCamSpeed*timeElapsed*(front?1:-1)) : 0);
        }
    }
    void setControlMode(const unsigned char &mode)
    {
        if(!mPlayer && (mode==CONTROL_FIXED || mode==CONTROL_CHASE || mode==CONTROL_FIRSTPERSON))return;

        mControlMode = mode;
        if(mControlMode==CONTROL_FREECAM)
        {
            mCameraMgr->setFreeCamera(true);
            mCameraMgr->setChaseCamera(false);
            if(mPlayer)
            {
                mCameraMgr->setPosition(mPlayer->getPosition()+Quaternion(Degree(mCameraMgr->getYaw()),Vector3::UNIT_Y)*(mPlayer->getScale()*Vector3(0,16,-41)));
                if(front||back||left||right)mPlayer->mIsMoving = false;
                if(mRelativeMode&&back)mActionsMgr->doWalkReverse(false);
                mPlayer->forceHide(false);
            }
        }
        else
        {
            mCameraMgr->setPosition(mPlayer->getPosition());
            mCameraMgr->setChaseCamera(mode==CONTROL_CHASE);
            mCameraMgr->setFreeCamera(false);
            if(mode==CONTROL_FIRSTPERSON)
            {
                mCameraMgr->disableCameraZoom(true);
                mPlayer->forceHide(true);
            }
            else
            {
                mCameraMgr->disableCameraZoom(false);
                mPlayer->forceHide(false);
            }
        }
    }
    void changeCamSpeed(const bool &up)
    {
        mCamSpeed *= (up?2:0.5f);
        if(mCamSpeed>3200)mCamSpeed = 3200;
        else if(mCamSpeed<50)mCamSpeed = 50;
    }
    void keyPressed(const char &index)
    {
        using namespace KeyboardIndex;
        switch(index)
        {
            case KB_FRONT: front = true; break;
            case KB_BACK:
                back  = true;
                if(mPlayer && mRelativeMode && !m_bPlayerDisabled && (mControlMode==CONTROL_FIXED || mControlMode==CONTROL_CHASE || mControlMode==CONTROL_FIRSTPERSON))mActionsMgr->doWalkReverse(true);
                break;
            case KB_LEFT: left = true; break;
            case KB_RIGHT: right  = true; break;
            case KB_UP:
                up = true;
                break;
            case KB_DOWN:
                down = true;
                break;
            case KB_AUTORUN: front = !front; break; //autorun
            case KB_CAM_UP: turnUp = true; break;
            case KB_CAM_DOWN: turnDown = true; break;
            case KB_CAM_LEFT: turnLeft = true; break;
            case KB_CAM_RIGHT: turnRight = true; break;

            case KB_JUMP:
                if(mPlayer && !mDisabled && !m_bPlayerDisabled)
                {
                    mPlayer->startJump();
                    mNetMgr->sendCharJump(true);
                }
                break;
            case KB_WALK:
            case KB_CROUCH:
                if(mPlayer && !mDisabled && !m_bPlayerDisabled)mActionsMgr->keyPressed(index);
                break;
            case KB_HEADTRACK: if(mPlayer  && !mDisabled)mHeadTracking = !mHeadTracking; break;
            case KB_CAM_FIRSTPERSON: if(!mDisabled)setControlMode(CONTROL_FIRSTPERSON); break;
            case KB_CAM_FIXED: if(!mDisabled)setControlMode(CONTROL_FIXED); break;
            case KB_CAM_FREE: if(!mDisabled)setControlMode(CONTROL_FREECAM); break;
            case KB_CAM_CHASE: if(!mDisabled)setControlMode(CONTROL_CHASE); break;
            case KB_CAM_SLOWER: changeCamSpeed(false); break;
            case KB_CAM_FASTER: changeCamSpeed(true); break;

            case KB_EMOTE0: case KB_EMOTE1: case KB_EMOTE2: case KB_EMOTE3: case KB_EMOTE4:
            case KB_EMOTE5: case KB_EMOTE6: case KB_EMOTE7: case KB_EMOTE8: case KB_EMOTE9:
                if(mPlayer && !mDisabled)mActionsMgr->keyPressed(index);
                break;
            case KB_SIT:
            case KB_STAND:
                if(mPlayer && !mDisabled && !m_bPlayerDisabled)mActionsMgr->keyPressed(index);
                break;
            case KB_RUNMODE: setRelativeMode(!mRelativeMode);
            case KB_EQUIP:
                if(mPlayer && !mDisabled && mSkillsMgr && !m_bPlayerDisabled)mSkillsMgr->keyPressed(index);
                break;

            default: break;
        }
    }
    void keyReleased(const char &index)
    {
        using namespace KeyboardIndex;
        switch(index)
        {
            case KB_FRONT: front = false; break;
            case KB_BACK:
                back  = false;
                if(mPlayer && mRelativeMode)mActionsMgr->doWalkReverse(false);
                break;
            case KB_LEFT: left = false; break;
            case KB_RIGHT: right  = false; break;
            case KB_UP:
                up = false;
                break;
            case KB_DOWN:
                down = false;
                break;
            case KB_CAM_UP: turnUp = false; break;
            case KB_CAM_DOWN: turnDown = false; break;
            case KB_CAM_LEFT: turnLeft = false; break;
            case KB_CAM_RIGHT: turnRight = false; break;

            case KB_JUMP:
                if(mPlayer && !mDisabled/* && !m_bPlayerDisabled*/)
                {
                    mPlayer->stopJump();
                    mNetMgr->sendCharJump(false);
                }
                break;
            case KB_WALK:
            case KB_CROUCH:
                if(mPlayer && !mDisabled && !m_bPlayerDisabled)mActionsMgr->keyReleased(index);
                break;
            case KB_EMOTE0: case KB_EMOTE1: case KB_EMOTE2: case KB_EMOTE3: case KB_EMOTE4:
            case KB_EMOTE5: case KB_EMOTE6: case KB_EMOTE7: case KB_EMOTE8: case KB_EMOTE9:
                if(mPlayer && !mDisabled)mActionsMgr->keyReleased(index);
                break;

            default: break;
        }
    }
    void mouseLeftDown()
    {
        if(mPlayer && !mDisabled)doMousePicking();
    }
    void doMousePicking()
    {
        SceneNode *hitNode = 0;
        const pair<bool,Vector3> result = mCollMgr->doMousePicking(mCameraMgr->getCamera(),mGui->getCursorX(),mGui->getCursorY(),&hitNode,true);

        //Terrain hit
        if(result.first && !hitNode && !m_bPlayerDisabled)
        {
            mPlayer->setMoveTarget(result.second);
            mPointerNode->setPosition(result.second);
            mPointerNode->setVisible(true);
        }
        //Object hit
        Object *selectObject = 0;
        if(hitNode)
        {
            Unit *unit = mUnitMgr->getUnitBySceneNode(hitNode);
            if(unit)selectObject = unit;
        }
        //Double click event
        if(mSelectObject && mSelectObject==selectObject && mDoubleClickTimer>0)
        {
            mDoubleClickTimer = 0;
            Unit *unit = (mSelectObject->getType()==Object::OBJECTTYPE_UNIT ? static_cast<Unit*>(mSelectObject) : 0);
            if(unit && unit!=mPlayer && mInfoMgr)mInfoMgr->viewInfo(unit->mUserID,unit->mCharID,unit->mUsername,unit->mName);
            return;
        }
        mSelectObject = selectObject;
        mDoubleClickTimer = 0.5f;
        if(mSelectObject)
        {
            //mSelectPointerNode->setScale(mMousePickNode->getScale().z*Vector3::UNIT_SCALE);
            mSelectPointerNode->setVisible(true);
        }
        else mSelectPointerNode->setVisible(false);
    }
    const bool objectEvent(ObjectListenerObject *object, const unsigned char &event)
    {
        if(!object)return false;

        if((event==ObjectListener::OBJECT_DESTROYED || event==ObjectListener::OBJECT_INVISIBLE) && object==mSelectObject)
        {
            mSelectObject = 0;
            mSelectPointerNode->setVisible(false);
        }

        return true;
    }
    const bool controllableEvent(ObjectListenerObject *object, const unsigned char &event)
    {
        if(object!=mPlayer)return false;

        if(mPointerNode && event==ObjectListener::CONTROLLABLE_STOPPED)mPointerNode->setVisible(false);

        return true;
    }
    const bool unitEvent(ObjectListenerObject *object, const unsigned char &event)
    {
        return false;
    }
    void changeEvent(GuiComponent *component, const unsigned char &event)
    {
        if(StringUtil::startsWith(component->getName(),"action"))mActionsMgr->changeEvent(component,event);
    }
    void buttonClicked(GuiComponent *button)
    {
        if(StringUtil::startsWith(button->getName(),"action"))mActionsMgr->buttonClicked(button);
    }
    void disable(const bool &flag)
    {
        mDisabled = flag;
    }
    void setRelativeMode(const bool &flag)
    {
        mRelativeMode = flag;
        if(mPlayer && back)mActionsMgr->doWalkReverse(flag);
    }
    const bool getRelativeMode()
    {
        return mRelativeMode;
    }
    ActionsManager* getActionsManager()
    {
        return mActionsMgr;
    }
    void disableControl(const bool &bFlag)
    {
        m_bPlayerDisabled = bFlag;
        if(bFlag && mPlayer)
        {
            mPlayer->stop();
        }
    }
};

template<> ControlManager* Singleton<ControlManager>::ms_Singleton = 0;

ControlManager* ControlManager::getSingletonPtr()
{
	return ms_Singleton;
}

ControlManager& ControlManager::getSingleton()
{
	assert(ms_Singleton);
	return *ms_Singleton;
}

#endif
