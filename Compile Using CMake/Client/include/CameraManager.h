#ifndef _CAMERAMANAGER_H_
#define _CAMERAMANAGER_H_

#include <CollisionManager.h>
#include <WaterManager.h>

class CameraListener
{
public:
    CameraListener(){}
    virtual ~CameraListener(){}
    virtual void cameraUnderwaterEvent(const bool &flag) = 0;
};

class CameraManager : public Singleton<CameraManager>
{
private:
    SceneManager *mSceneMgr;
    CollisionManager *mCollMgr;
    WaterManager *mWaterMgr;
    ScreenEffectsManager *mScreenFXMgr;
    Camera *mCamera;
    SceneNode *mCamOffset;
    SceneNode *mCamNode;
    SceneNode *mCamPivot;
    SceneNode *mCamTrackNode;
    NodeAnimationTrack *mCamTrack;
    AnimationState *mCamAnim;
    Real mZoom;
    bool mFreeCamera;
    bool mChaseCamera;
    bool mAnimateCamera;
    Real mYaw, mPitch, mFreePitch;
    Real mScaleY, mScaleZ;
    Vector3 mCamChaseDestination;
    bool mZoomDisabled;
    bool mIsUnderwater;
    std::vector<CameraListener*> mListeners;
public:
    CameraManager()
    {
        mZoom = 0.5f;
        mAnimateCamera = false;
        reset();
    }
    ~CameraManager()
    {
        reset();
    }
    static CameraManager* getSingletonPtr();
    static CameraManager& getSingleton();
    void reset()
    {
        if(mAnimateCamera)destroyCameraTrack();
        mCamera = 0;
        mCollMgr = 0;
        mListeners.clear();
    }
    void init(Camera *cam, SceneManager *sMgr)
    {
        mSceneMgr = sMgr;
        mCollMgr = CollisionManager::getSingletonPtr();
        mWaterMgr = WaterManager::getSingletonPtr();
        mScreenFXMgr = ScreenEffectsManager::getSingletonPtr();
        mCamera = cam;
        mCamera->setDirection(Vector3::UNIT_Z);
        mCamOffset = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        mCamPivot = mCamOffset->createChildSceneNode();
        mCamNode = mCamPivot->createChildSceneNode();
        mCamNode->attachObject(mCamera);
        mYaw = 0;
        mPitch = 0;
        mFreePitch = 0;
        mScaleY = mScaleZ = 1;

        mCamTrackNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        mCamAnim = 0;
        mCamTrack = 0;
        mAnimateCamera = false;
        mFreeCamera = false;
        mChaseCamera = false;
        mCamChaseDestination = Vector3::ZERO;
        mZoomDisabled = false;
        mIsUnderwater = false;
    }
    void update(const Real &timeElapsed)
    {
        //Camera track
        if(mAnimateCamera && mCamAnim)
        {
            mCamAnim->addTime(timeElapsed);
            updateCamWaterCollision();
            return;
        }
        //Camera chase
        if(mChaseCamera)
        {
            const Real weight = Math::Pow(0.0000005f,timeElapsed);
            mCamPivot->setPosition(weight*mCamPivot->getPosition() + (1-weight)*mCamChaseDestination);
        }
        //Collisions
        updateCamCollision();
        updateCamWaterCollision();
    }
    void updateCamCollision()
    {
        mCamOffset->setPosition(Vector3::ZERO);
        /*const Vector3 currentPosition = mCamPivot->getPosition()+(mFreeCamera?Vector3::ZERO:mCamPivot->getOrientation()*mCamNode->getPosition());
        const Real groundHeight = mCollMgr->getGroundHeight(currentPosition,false) + 5;
        if(currentPosition.y<groundHeight)
        {
            if(mFreeCamera)mCamPivot->translate(0,groundHeight-currentPosition.y,0,Node::TS_WORLD);
            else mCamOffset->setPosition(0,groundHeight-currentPosition.y,0);
        }
        */
        //Get highest ground
        if(mFreeCamera || mZoomDisabled)
        {
            const Vector3 currentPosition = mCamPivot->getPosition();
            const Real groundHeight = mCollMgr->getGroundHeight(currentPosition,false) + 5;
            if(currentPosition.y<groundHeight)mCamPivot->translate(0,groundHeight-currentPosition.y,0,Node::TS_WORLD);
        }
        //Get closest collision from pivot to camera
        else
        {
            const Vector3 currentPosition = mCamPivot->getPosition();
            const Vector3 dirVect = mCamPivot->getOrientation()*mCamNode->getPosition();
            Vector3 result = currentPosition;
            const bool hasObstruction = (mCollMgr->getMeshCollision(Ray(currentPosition+Vector3(0,1,0),dirVect),result) ? ((result-currentPosition).squaredLength()<=dirVect.squaredLength()) : false);
            //No obstruction between camera and pivot, normal terrain coll detection
            if(!hasObstruction)
            {
                const Vector3 actualPosition = currentPosition+dirVect;
                const Real groundHeight = mCollMgr->getGroundHeight(actualPosition,false) + 5;
                if(actualPosition.y<groundHeight)mCamOffset->setPosition(0,groundHeight-actualPosition.y,0);
            }
            else
            {
                const Vector3 obstructionPos = result;
                const bool hasMeshCeiling = mCollMgr->getMeshCollision(Ray(currentPosition+Vector3(0,mScaleY*10.0f,0),Vector3::UNIT_Y),result);
                //Obstruction between camera and pivot but no ceiling, mesh coll detection from top
                if(!hasMeshCeiling)
                {
                    const Vector3 actualPosition = currentPosition+dirVect;
                    const Real groundHeight = mCollMgr->getGroundHeight(actualPosition,true) + 5;
                    if(actualPosition.y<groundHeight)mCamOffset->setPosition(0,groundHeight-actualPosition.y,0);
                }
                //Both obstruction and ceiling (e.g. inside cave), shrink camera distance
                else
                {
                    mCamOffset->setPosition(obstructionPos-(currentPosition+dirVect));
                }
            }
        }
    }
    void updateCamWaterCollision()
    {
        const bool underwater = mWaterMgr->collidesWater(getPosition());
        if(underwater==mIsUnderwater)return;
        mIsUnderwater = underwater;
        mWaterMgr->setUnderwater(underwater);
        mScreenFXMgr->setUnderwater(underwater);
        for(std::vector<CameraListener*>::iterator it=mListeners.begin(); it!=mListeners.end(); it++)
        {
            CameraListener *listener = *it;
            listener->cameraUnderwaterEvent(underwater);
        }
    }
    void translate(const Real &x, const Real &y, const Real &z)
    {
        mCamPivot->translate(x,y,z,Node::TS_LOCAL);
    }
    void setPosition(const Vector3 &pos)
    {
        if(!mChaseCamera)mCamPivot->setPosition(pos);
        mCamChaseDestination = pos;
    }
    const Vector3 getPosition(const bool &addOffset=true)
    {
        if(mAnimateCamera)return mCamTrackNode->getPosition();
        if(!mFreeCamera)
        {
            if(addOffset)return mCamOffset->getPosition()+mCamPivot->getPosition()+mCamPivot->getOrientation()*mCamNode->getPosition();
            else return mCamPivot->getPosition()+mCamPivot->getOrientation()*mCamNode->getPosition();
        }
        if(addOffset)return (mCamPivot->getPosition()+mCamOffset->getPosition());
        return mCamPivot->getPosition();
    }
    const Vector3 getDirection()
    {
        if(mAnimateCamera)return mCamTrackNode->getOrientation()*mCamera->getOrientation()*Vector3::NEGATIVE_UNIT_Z;
        if(mFreeCamera)return mCamPivot->getOrientation()*Vector3::NEGATIVE_UNIT_Z;
        return mCamNode->_getDerivedOrientation()*Vector3::NEGATIVE_UNIT_Z;
    }
    void yaw(const Real &degree)
    {
        if(!mCamera)return;
        mCamPivot->yaw(Degree(degree),Node::TS_WORLD);

        mYaw += degree;
        if(mYaw>=360)mYaw -= 360;
        else if(mYaw<0)mYaw += 360;
    }
    void pitch(Real degree)
    {
        if(!mCamera)return;
        if(mFreeCamera)
        {
            if(Math::Abs(mFreePitch+degree)>90)degree = 90*(degree<0?-1:1) - mFreePitch;
            mCamPivot->pitch(Degree(degree));
            mFreePitch += degree;
        }
        else
        {
            if(Math::Abs(mPitch+degree)>90)degree = 90*(degree<0?-1:1) - mPitch;
            mCamNode->pitch(Degree(degree));
            mPitch += degree;
        }
    }
    const Real getYaw()
    {
        return mYaw;
    }
    const Real getPitch()
    {
        return mPitch;
    }
    void setPitchYaw(const Real &pitchValue, const Real &yawValue)
    {
        pitch(-mPitch);
        pitch(pitchValue);
        yaw(-mYaw);
        yaw(yawValue);
    }
    void createCameraTrack(const Real &length)
    {
        destroyCameraTrack();
        Animation *tAnim = mSceneMgr->createAnimation("CameraTrack",length);
		tAnim->setInterpolationMode(Animation::IM_SPLINE);
		mCamTrack = tAnim->createNodeTrack(0, mCamTrackNode);
    }
    void addCameraTrackKeyFrame(const Real &timePosition, const Vector3 &translation, const Quaternion &rotation=Quaternion(Degree(180),Vector3::UNIT_Y))
	{
		if(!mCamTrack)return;
		TransformKeyFrame *tKey = mCamTrack->createNodeKeyFrame(timePosition);
		tKey->setTranslate(translation);
		tKey->setRotation(rotation);
	}
	void doCameraTrack(const bool &loop=false, const bool &autoTrack=false, SceneNode *autoTrackTarget=0, const Vector3 &autoTrackOffset=Vector3::ZERO)
	{
		if(!mCamTrack)return;
		mCamAnim = mSceneMgr->createAnimationState("CameraTrack");
		mCamAnim->setEnabled(true);
		mCamAnim->setLoop(loop);

		if(mCamera->isAttached() && mCamera->getParentSceneNode()==mCamNode)mCamNode->detachObject(mCamera);
		mCamTrackNode->attachObject(mCamera);
		mCamera->setPosition(Vector3::ZERO);
		if(autoTrack)mCamera->setAutoTracking(true,autoTrackTarget,autoTrackOffset);

		mAnimateCamera = true;
	}
    void destroyCameraTrack()
    {
        if(mCamAnim)
		{
		    mCamTrackNode->setOrientation(Quaternion::IDENTITY);
			mCamAnim->setEnabled(false);
			mSceneMgr->destroyAnimationState("CameraTrack");
			mCamAnim = 0;
			mCamera->setAutoTracking(false);
			mCamera->setDirection(Vector3::UNIT_Z);
			if(mCamTrack)mCamTrack->removeAllKeyFrames();
			mCamTrack = 0;
			mSceneMgr->getAnimation("CameraTrack")->destroyAllNodeTracks();
			mSceneMgr->destroyAnimation("CameraTrack");
		}
		if(mCamera->isAttached() && mCamera->getParentSceneNode()==mCamTrackNode)
		{
			mCamTrackNode->detachObject(mCamera);
			mCamNode->attachObject(mCamera);
		}
		mAnimateCamera = false;
    }
    const Quaternion pyr(const Real &pitch, const Real &yaw, const Real &roll)
    {
        return Quaternion(Degree(yaw),Vector3::UNIT_Y)*Quaternion(Degree(pitch),Vector3::UNIT_X)*Quaternion(Degree(roll),Vector3::UNIT_Z);
    }
    void setCameraZoom(const Real &zoom, const Real &yOffset=10)
    {
        if(mZoomDisabled)return;
        mZoom = zoom;
        mCamNode->setPosition(0,(yOffset+mZoom*25)*mScaleY,-mZoom*80*mScaleZ);
    }
    void zoom(const Real &zoom)
    {
        if(mZoomDisabled)return;
        mZoom += zoom;
        if(mZoom<0.15f)mZoom = 0.15f;
        else if(mZoom>1.2f)mZoom = 1.2f;
        setCameraZoom(mZoom);
    }
    void disableCameraZoom(const bool &flag, const Real &yOffset=10)
    {
        mZoomDisabled = flag;
        if(flag)mCamNode->setPosition(0,yOffset*mScaleY,0);
        else setCameraZoom(mZoom);
    }
    void setFreeCamera(const bool &flag)
    {
        if(mFreeCamera && !flag)
        {
            mCamPivot->pitch(Degree(-mFreePitch));
            mCamNode->pitch(Degree(mPitch));
            setCameraZoom(mZoom);
        }
        else if(!mFreeCamera && flag)
        {
            mCamNode->pitch(Degree(-mPitch));
            mCamPivot->pitch(Degree(mFreePitch));
            mCamNode->setPosition(0,0,0);
        }
        mFreeCamera = flag;
    }
    void setChaseCamera(const bool &flag)
    {
        mChaseCamera = flag;
    }
    const bool isFreeCamera()
    {
        return mFreeCamera;
    }
    void setScale(const Vector3 &scale)
    {
        mScaleY = scale.y;
        mScaleZ = scale.z;
        setCameraZoom(mZoom);
    }
    Camera* getCamera()
    {
        return mCamera;
    }
    void addListener(CameraListener *listener)
    {
        mListeners.push_back(listener);
    }
};

template<> CameraManager* Singleton<CameraManager>::ms_Singleton = 0;

CameraManager* CameraManager::getSingletonPtr()
{
	return ms_Singleton;
}

CameraManager& CameraManager::getSingleton()
{
	assert(ms_Singleton);
	return *ms_Singleton;
}

#endif
