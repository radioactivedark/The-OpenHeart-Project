#ifndef _SKYDEFINES_H_
#define _SKYDEFINES_H_

#define TIME_SPEED 0.334f   //5mins real time = 1h game time
#define SKY_FAR 15
#define SKY_MID 12
#define SKY_TWILIGHT_OFFSET 2
#define SKY_TWILIGHT_OFFSET_DOUBLED 4
#define FOG_RATIO .0001f
#define FOG_START 1000
#define FOG_END 15000
#define SKY_UPDATE_PERIOD 2.5f
#define NUM_STARS 100
#define NUM_CLOUDS 10
#define NUM_CLOUDMATS 15
#define WEATHER_TIME_MAX 7000

#define SKYMAKER_DIR "media/sky/"

struct Cloud
{
    SceneNode *mSceneNode;
    Entity *mEntity;
    Vector2 mVelocity;
    Real mFadeTimer;
    Real mFadeMax;
    bool mDoFadeIn;
    bool mDoFadeOut;
    Real mYaw;
    Cloud()
    {
        mSceneNode = 0;
        mEntity = 0;
        mVelocity = Vector2::ZERO;
        mFadeTimer = 0;
        mFadeMax = 0;
        mDoFadeIn = false;
        mDoFadeOut = false;
        mYaw = 0;
    }
    void updatePosition(const Real &timeElapsed)
    {
        const Vector3 pos = mSceneNode->getPosition();
        const Real dX = mVelocity.x*timeElapsed;
        const Real dZ = mVelocity.y*timeElapsed;
        const Real y = SKY_MID - Vector2(pos.x,pos.z).squaredLength()*0.05f;
        mSceneNode->translate(dX,y-pos.y,dZ);
    }
    void updateOrientation()
    {
        const Vector3 pos = mSceneNode->getPosition();
        mSceneNode->resetOrientation();
        mSceneNode->roll(Degree(-15)*(pos.x/SKY_MID));
        mSceneNode->pitch(Degree(15)*(pos.z/SKY_MID));
        mSceneNode->yaw(Degree(mYaw));
    }
    void setFadeTimer()
    {
        if(mVelocity.x==0 || mVelocity.y==0)return;
        mFadeMax = (Math::Abs(mVelocity.x)>Math::Abs(mVelocity.y) ? 0.9f/Math::Abs(mVelocity.x) : 0.9f/Math::Abs(mVelocity.y));
        mFadeTimer = mFadeMax;
    }
    const Real getFadeRatio()
    {
        if(mFadeMax==0)return 0;
        return (mDoFadeOut ? mFadeTimer/mFadeMax : (mFadeMax-mFadeTimer)/mFadeMax);
    }
    void randomizeScale()
    {
        const Real size = Math::RangeRandom(4,7);
        mSceneNode->setScale(size,1,size);
    }
    void randomizeMaterial()
    {
        const unsigned char mat = (int)Math::RangeRandom(1,NUM_CLOUDMATS+1);
        mEntity->setMaterialName("Sky/CloudMat"+StringConverter::toString(mat>NUM_CLOUDMATS?NUM_CLOUDMATS:mat));
    }
    void randomizeVelocity(const Vector2 &windVelocity)
    {
        mVelocity = windVelocity*Math::RangeRandom(2,6);
    }
};

#endif
