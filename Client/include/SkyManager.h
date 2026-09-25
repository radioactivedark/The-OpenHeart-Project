#ifndef _SKYMANAGER_H_
#define _SKYMANAGER_H_

#include <LightManager.h>
#include <WeatherManager.h>

struct SkyColours
{
    //Sky Top, sky bottom
    pair<ColourValue,ColourValue> mDawn;
    pair<ColourValue,ColourValue> mDay;
    pair<ColourValue,ColourValue> mDusk;
    pair<ColourValue,ColourValue> mNight;
    ColourValue mSunRise;
    ColourValue mSun;
    ColourValue mSunSet;
    ColourValue mMoon;
    ColourValue mDawnLight;
    ColourValue mDayLight;
    ColourValue mDuskLight;
    ColourValue mNightLight;
    ColourValue mDawnCloud;
    ColourValue mDayCloud;
    ColourValue mDuskCloud;
    ColourValue mNightCloud;
    SkyColours()
    {
        mDawn = pair<ColourValue,ColourValue>(Colour(1,7,97),Colour(255,230,157));
        mDay = pair<ColourValue,ColourValue>(Colour(19,72,175),Colour(192,229,253));
        mDusk = pair<ColourValue,ColourValue>(Colour(20,20,100),Colour(255,220,160));
        mNight = pair<ColourValue,ColourValue>(Colour(0,0,15),Colour(35,27,95));
        mSunRise = Colour(180,150,90);
        mSun = Colour(255,255,255);
        mSunSet = Colour(180,130,90);
        mMoon = Colour(255,255,255);
        mDawnLight = Colour(220,200,80);
        mDayLight = Colour(255,200,100);
        mDuskLight = Colour(255,150,80);
        mNightLight = Colour(10,10,100);
        mDawnCloud = Colour(223,199,145);
        mDayCloud = Colour(220,230,255);
        mDuskCloud = Colour(227,185,140);
        mNightCloud = Colour(40,27,65);
    }
    const ColourValue Colour(const unsigned char &r,const unsigned char &g,const unsigned char &b)
    {
        return ColourValue(float(r)/255,float(g)/255,float(b)/255);
    }
    const ColourValue between(const Real &ratio, const ColourValue &start, const ColourValue &end)
    {
        const ColourValue diff = end - start;
        return (start + ratio*diff);
    }
    const pair<ColourValue,ColourValue> between(const Real &ratio, const pair<ColourValue,ColourValue> &start, const pair<ColourValue,ColourValue> &end)
    {
        const ColourValue topDiff = end.first - start.first;
        const ColourValue botDiff = end.second - start.second;
        return pair<ColourValue,ColourValue>(start.first + ratio*topDiff,start.second + ratio*botDiff);
    }
    const ColourValue getColour(const unsigned char &index)
    {
        switch(index)
        {
            case 0: return mDawn.first;
            case 1: return mDawn.second;
            case 2: return mDay.first;
            case 3: return mDay.second;
            case 4: return mDusk.first;
            case 5: return mDusk.second;
            case 6: return mNight.first;
            case 7: return mNight.second;
            case 8: return mSunRise;
            case 9: return mSun;
            case 10: return mSunSet;
            case 11: return mMoon;
            case 12: return mDawnLight;
            case 13: return mDayLight;
            case 14: return mDuskLight;
            case 15: return mNightLight;
            case 16: return mDawnCloud;
            case 17: return mDayCloud;
            case 18: return mDuskCloud;
            case 19: return mNightCloud;
            default: break;
        }
        return ColourValue();
    }
    void setColour(const unsigned char &index, const ColourValue &colour)
    {
        switch(index)
        {
            case 0: mDawn.first = colour; break;
            case 1: mDawn.second = colour; break;
            case 2: mDay.first = colour; break;
            case 3: mDay.second = colour; break;
            case 4: mDusk.first = colour; break;
            case 5: mDusk.second = colour; break;
            case 6: mNight.first = colour; break;
            case 7: mNight.second = colour; break;
            case 8: mSunRise = colour; break;
            case 9: mSun = colour; break;
            case 10: mSunSet = colour; break;
            case 11: mMoon = colour; break;
            case 12: mDawnLight = colour; break;
            case 13: mDayLight = colour; break;
            case 14: mDuskLight = colour; break;
            case 15: mNightLight = colour; break;
            case 16: mDawnCloud = colour; break;
            case 17: mDayCloud = colour; break;
            case 18: mDuskCloud = colour; break;
            case 19: mNightCloud = colour; break;
            default: break;
        }
    }
};

class SkyManager : public Singleton<SkyManager>, public ReflectionListener
{
private:
    SceneManager *mSceneMgr;
    LightManager *mLightMgr;
    CameraManager *mCameraMgr;
    Viewport *mViewport;
    Viewport *mReflectViewport;
    WeatherManager *mWeatherMgr;

    SceneNode *mSkyNode;
    Entity *mSky;
    Light *mSkyLight;

    SceneNode *mSunNode;
    BillboardSet *mSunSet;
    Billboard *mSun;
    BillboardSet *mSunRadianceSet;
    Billboard *mSunRadiance;

    SceneNode *mMoonNode;
    BillboardSet *mMoonSet;
    Billboard *mMoon;
    BillboardSet *mMoonRadianceSet;
    Billboard *mMoonRadiance;

    SceneNode *mStarNode;
    BillboardSet *mStarSet;
    Real mStarBrightness;

    std::vector<Cloud*> mCloudList;

    SceneNode *mCeilingNode;
    Entity *mCeiling;

    Real mDayTime;
    Real mUpdateTimer;
    unsigned char mDayState;
    bool mHasSky;
    SkyColours mSkyColours;
    Vector2 mWindVelocity;
    ColourValue mSkyColourTop;  //Current sky colour values
    ColourValue mSkyColourBot;
    ColourValue mCloudColour;   //Current cloud colour value
    bool mFreezeTime;
    bool mShowSky;
    bool mFastForward;
public:
    SkyManager()
    {
        mWeatherMgr = new WeatherManager();
        mSceneMgr = 0;
        reset();
    }
    ~SkyManager()
    {
        reset();
        delete mWeatherMgr;
    }
    enum DayState
    {
        DAY_NONE,
        DAY_DAWN,
        DAY_MORNING,
        DAY_NOON,
        DAY_DUSK,
        DAY_NIGHT
    };
    void init(SceneManager *sceneMgr, Viewport *viewport)
    {
        mSceneMgr = sceneMgr;
        mLightMgr = LightManager::getSingletonPtr();
        mCameraMgr = CameraManager::getSingletonPtr();
        mViewport = viewport;
        WaterManager::getSingletonPtr()->setReflectionListener(this);

        mWeatherMgr->init(sceneMgr,viewport,&mSkyColourTop,&mSkyColourBot,&mCloudColour);

        mWindVelocity = Vector2(-0.005f,0.01f);
    }
    void reset()
    {
        while(!mCloudList.empty())
        {
            Cloud *cloud = mCloudList.back();
            mCloudList.pop_back();
            delete cloud;
        }
        mHasSky = false;
        mSkyNode = 0;
        mSky = 0;
        mDayTime = 0;
        mUpdateTimer = 0;
        mDayState = DAY_NONE;
        mFreezeTime = false;
        mShowSky = true;
        mFastForward = false;
        mCeilingNode = 0;
        mCeiling = 0;
        mSkyColours = SkyColours();

        mWeatherMgr->reset();
        mReflectViewport = 0;
        WaterManager::getSingletonPtr()->setReflectionListener(0);
    }
    void createSky(Light *skyLight)
    {
        mHasSky = true;

        //Sky
        if(!mSkyNode)mSkyNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        mSky = mSceneMgr->createEntity("Sky","skyDome.mesh");
        mSky->setMaterialName("Sky/SkyMat");
        mSky->setCastShadows(false);
        SceneNode *skyDomeNode = mSkyNode->createChildSceneNode();
        skyDomeNode->attachObject(mSky);
        mSkyNode->setScale(1000,1000,1000);
        skyDomeNode->setScale(0.8f,0.8f,0.8f);

        //Sun
        mSunSet = mSceneMgr->createBillboardSet("SunSet",1);
        mSunSet->setMaterialName("Sky/SunMat");
        mSunSet->setDefaultDimensions(7.5,7.5);
        mSunSet->setCastShadows(false);
        mSun = mSunSet->createBillboard(Vector3::ZERO);
        mSunRadianceSet = mSceneMgr->createBillboardSet("SunRadianceSet",1);
        mSunRadianceSet->setMaterialName("Sky/SunRadianceMat");
        mSunRadianceSet->setDefaultDimensions(18,18);
        mSunRadianceSet->setCastShadows(false);
        mSunRadiance = mSunRadianceSet->createBillboard(Vector3::ZERO);
        mSunNode = mSkyNode->createChildSceneNode();
        mSunNode->attachObject(mSunSet);
        mSunNode->attachObject(mSunRadianceSet);

        //Moon
        mMoonSet = mSceneMgr->createBillboardSet("MoonSet",1);
        mMoonSet->setMaterialName("Sky/MoonMat");
        mMoonSet->setDefaultDimensions(6,6);
        mMoonSet->setCastShadows(false);
        mMoon = mMoonSet->createBillboard(Vector3::ZERO);
        mMoonRadianceSet = mSceneMgr->createBillboardSet("MoonRadianceSet",1);
        mMoonRadianceSet->setMaterialName("Sky/SunRadianceMat");
        mMoonRadianceSet->setDefaultDimensions(12,12);
        mMoonRadianceSet->setCastShadows(false);
        mMoonRadiance = mMoonRadianceSet->createBillboard(Vector3::ZERO);
        mMoonNode = mSkyNode->createChildSceneNode();
        mMoonNode->attachObject(mMoonSet);
        mMoonNode->attachObject(mMoonRadianceSet);

        //Stars
        mStarSet = mSceneMgr->createBillboardSet("StarSet",NUM_STARS);
        mStarSet->setMaterialName("Sky/StarMat");
        mStarSet->setDefaultDimensions(0.025f,0.025f);
        mMoonSet->setCastShadows(false);
        mStarNode = mSkyNode->createChildSceneNode();
        mStarNode->attachObject(mStarSet);
        const Real starFar = (SKY_FAR-1)*(SKY_FAR-1);
        for(int i=0; i<NUM_STARS; i++)
        {
            const Real x = Math::RangeRandom(-SKY_FAR,SKY_FAR);
            const Real z = Math::RangeRandom(-SKY_FAR,SKY_FAR);
            const Real y = Vector2(x,z).squaredLength()<starFar? Math::RangeRandom(SKY_FAR-1,SKY_FAR) :Math::RangeRandom(1,SKY_FAR-1);
            Billboard *star = mStarSet->createBillboard(x,y,z);
            star->setRotation(Degree(Math::RangeRandom(0,360)));
            const Real size = Math::RangeRandom(0.12f,0.3f);
            star->setDimensions(size,size);
        }
        mStarBrightness = 1;

        //Light
        mSkyLight = skyLight;
        mSkyLight->setType(Light::LT_DIRECTIONAL);
        mSkyLight->setSpecularColour(ColourValue(1,1,1));
        mSkyLight->setVisible(true);

        //Clouds
        for(int i=0; i<NUM_CLOUDS; i++)
        {
            Cloud *cloud = new Cloud();
            SceneNode *node = cloud->mSceneNode = mSkyNode->createChildSceneNode();
            Entity *ent = cloud->mEntity = mSceneMgr->createEntity("CloudEnt"+StringConverter::toString(i),"cloud.mesh");
            const unsigned char mat = (int)Math::RangeRandom(1,NUM_CLOUDMATS+1);
            ent->setMaterialName("Sky/CloudMat"+StringConverter::toString(mat>NUM_CLOUDMATS?NUM_CLOUDMATS:mat));
            ent->setCastShadows(false);
            SubEntity *subEnt = ent->getSubEntity(0);
            subEnt->setCustomParameter(1,Vector4(1,1,1,1));
            CapabilitiesManager::getSingletonPtr()->assertCustomColour(subEnt,1);
            node->attachObject(cloud->mEntity);

            const Real x = Math::RangeRandom(-SKY_MID,SKY_MID);
            const Real z = Math::RangeRandom(-SKY_MID,SKY_MID);
            node->setPosition(x,0,z);
            cloud->updatePosition(0);

            cloud->mYaw = Math::RangeRandom(0,360);
            cloud->updateOrientation();

            cloud->randomizeScale();
            cloud->randomizeMaterial();
            cloud->randomizeVelocity(mWindVelocity);

            mCloudList.push_back(cloud);
        }
    }
    void createWeather(const String &filename, const bool &useFhz=true)
    {
        if(!mSkyNode)mSkyNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        mWeatherMgr->loadWeatherCycle(filename,useFhz);
        mWeatherMgr->createWeather(mSkyNode,mSky,&mCloudList);
    }
    void createCeiling(const Vector3 &size, const String &mat)
    {
        if(!mCeilingNode)mCeilingNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        if(mCeiling)destroyCeiling();
        mCeiling = mSceneMgr->createEntity("Sky/Ceiling","ceiling.mesh");
        mCeiling->setMaterialName(mat);
        mCeilingNode->attachObject(mCeiling);
        const Vector3 midpoint = size*0.5f;
        mCeilingNode->setPosition(midpoint.x,size.y+20,midpoint.z);
        mCeilingNode->setScale(midpoint.x,200,midpoint.z);

        setSkyColours(pair<ColourValue,ColourValue>(ColourValue(0,0,0),ColourValue(0,0,0)));
    }
    void destroyCeiling()
    {
        if(!mCeilingNode || !mCeiling)return;
        mCeilingNode->detachAllObjects();
        mSceneMgr->destroyEntity(mCeiling);
        mCeiling = 0;
    }
    void update(const Real &timeElapsed)
    {
        const bool doUpdate = (updateTime(timeElapsed) || mDayState==DAY_NONE);
        if(mSkyNode)mSkyNode->setPosition(mCameraMgr->getPosition());
        if(mHasSky)
        {
            if(doUpdate||mFastForward)updateSky();
            updateStars();
            updateClouds(timeElapsed);
        }
        mWeatherMgr->update(timeElapsed,doUpdate);
    }
    const bool updateTime(const Real &timeElapsed)
    {
        if(mFreezeTime)return false;

        if(mFastForward)mDayTime += timeElapsed*TIME_SPEED*200;
        else mDayTime += timeElapsed*TIME_SPEED;
        if(mDayTime>2400)mDayTime -= 2400;

        mUpdateTimer += timeElapsed;
        if(mUpdateTimer>=SKY_UPDATE_PERIOD)
        {
            mUpdateTimer = 0;
            return true;
        }
        return false;
    }
    void updateSky(const bool &forced=false)
    {
        if(forced)mDayState = DAY_NONE;

        if(mDayTime>=600 && mDayTime<800)updateDawn();  //2h
        else if(mDayTime>=800 && mDayTime<1300)updateMorning(); //5h
        else if(mDayTime>=1300 && mDayTime<1800)updateNoon();   //5h
        else if(mDayTime>=1800 && mDayTime<2000)updateDusk();  //2h
        else updateNight(); //10h
    }
    void updateDawn()
    {
        if(mDayState!=DAY_DAWN)
        {
            mDayState = DAY_DAWN;
            setDayElementsVisible(true);
            setNightElementsVisible(true);

            mMoonNode->setPosition(SKY_FAR,SKY_FAR,SKY_FAR);
            setStarsBrightness(1);
        }
        const Real ratio = (mDayTime-600)/200;
        const bool firstHalf = (ratio < 0.5f);
        const Real halfRatio = (firstHalf ? ratio*2 : (ratio-0.5f)*2);

        mSunNode->setPosition(SKY_FAR,SKY_TWILIGHT_OFFSET_DOUBLED*ratio-SKY_TWILIGHT_OFFSET,0);
        mSkyLight->setDirection(-1,-0.25f - ratio*0.5f,0);

        if(firstHalf)
        {
            mSun->setColour(mSkyColours.between(halfRatio,ColourValue(0,0,0),mSkyColours.mSunRise));
            mSunRadiance->setColour(mSkyColours.between(halfRatio,ColourValue(0,0,0),mSkyColours.mSunRise*0.7f));
            mSkyLight->setDiffuseColour(mSkyColours.between(halfRatio,mSkyColours.mNightLight,mSkyColours.mDawnLight));
            setSkyColours(mSkyColours.between(halfRatio,mSkyColours.mNight,mSkyColours.mDawn));
            setCloudsColour(mSkyColours.between(halfRatio,mSkyColours.mNightCloud,mSkyColours.mDawnCloud));

            mMoonNode->setVisible(true);
            const ColourValue moonColour = mSkyColours.between(halfRatio,mSkyColours.mMoon,ColourValue(0,0,0));
            mMoon->setColour(moonColour);
            mMoonRadiance->setColour(moonColour*0.5f);
            mStarNode->setVisible(true);
            mStarBrightness = 1-halfRatio;
        }
        else
        {
            mSun->setColour(mSkyColours.between(halfRatio,mSkyColours.mSunRise,mSkyColours.mSun));
            mSunRadiance->setColour(mSkyColours.between(halfRatio,mSkyColours.mSunRise*0.7f,ColourValue(0,0,0)));
            mSkyLight->setDiffuseColour(mSkyColours.between(halfRatio,mSkyColours.mDawnLight,mSkyColours.mDayLight));
            setSkyColours(mSkyColours.between(halfRatio,mSkyColours.mDawn,mSkyColours.mDay));
            setCloudsColour(mSkyColours.between(halfRatio,mSkyColours.mDawnCloud,mSkyColours.mDayCloud));

            mMoonNode->setVisible(false);
            mStarNode->setVisible(false);
        }
    }
    void updateDusk()
    {
        if(mDayState!=DAY_DUSK)
        {
            mDayState = DAY_DUSK;
            setDayElementsVisible(true);
            setNightElementsVisible(true);

            mMoonNode->setPosition(SKY_FAR,SKY_FAR,SKY_FAR);
            setStarsBrightness(0);
        }
        const Real ratio = (mDayTime-1800)/200;
        const bool firstHalf = (ratio < 0.5f);
        const Real halfRatio = (firstHalf ? ratio*2 : (ratio-0.5f)*2);

        mSunNode->setPosition(-SKY_FAR,SKY_TWILIGHT_OFFSET_DOUBLED*(1-ratio)-SKY_TWILIGHT_OFFSET,0);
        mSkyLight->setDirection(1,-0.75f + ratio*0.5f,0);

        if(firstHalf)
        {
            mSun->setColour(mSkyColours.between(halfRatio,mSkyColours.mSun,mSkyColours.mSunSet));
            mSunRadiance->setColour(mSkyColours.between(halfRatio,ColourValue(0,0,0),mSkyColours.mSunSet*0.7f));
            mSkyLight->setDiffuseColour(mSkyColours.between(halfRatio,mSkyColours.mDayLight,mSkyColours.mDuskLight));
            setSkyColours(mSkyColours.between(halfRatio,mSkyColours.mDay,mSkyColours.mDusk));
            setCloudsColour(mSkyColours.between(halfRatio,mSkyColours.mDayCloud,mSkyColours.mDuskCloud));

            mMoonNode->setVisible(false);
            mStarNode->setVisible(false);
        }
        else
        {
            mSun->setColour(mSkyColours.between(halfRatio,mSkyColours.mSunSet,ColourValue(0,0,0)));
            mSunRadiance->setColour(mSkyColours.between(halfRatio,mSkyColours.mSunSet*0.7f,ColourValue(0,0,0)));
            mSkyLight->setDiffuseColour(mSkyColours.between(halfRatio,mSkyColours.mDuskLight,mSkyColours.mNightLight));
            setSkyColours(mSkyColours.between(halfRatio,mSkyColours.mDusk,mSkyColours.mNight));
            setCloudsColour(mSkyColours.between(halfRatio,mSkyColours.mDuskCloud,mSkyColours.mNightCloud));

            mMoonNode->setVisible(true);
            const ColourValue moonColour = mSkyColours.between(halfRatio,ColourValue(0,0,0),mSkyColours.mMoon);
            mMoon->setColour(moonColour);
            mMoonRadiance->setColour(moonColour*0.5f);
            mStarNode->setVisible(true);
            mStarBrightness = halfRatio;
        }
    }
    void updateMorning()
    {
        if(mDayState!=DAY_MORNING)
        {
            mDayState = DAY_MORNING;
            setDayElementsVisible(true);
            setNightElementsVisible(false);

            mSun->setColour(mSkyColours.mSun);
            mSunRadiance->setColour(ColourValue(0,0,0));

            mSkyLight->setDiffuseColour(mSkyColours.mDayLight);

            setSkyColours(mSkyColours.mDay);
            setCloudsColour(mSkyColours.mDayCloud);
        }
        const Real ratio = (mDayTime-800)/500;

        mSunNode->setPosition(SKY_FAR*(1-ratio),SKY_FAR*ratio+SKY_TWILIGHT_OFFSET,0);
        mSkyLight->setDirection(ratio<0.2f ? Vector3(-1,-0.75f - ratio*1.25f,0) : Vector3((1-ratio)*-1.25f,-1,0));
    }
    void updateNoon()
    {
        if(mDayState!=DAY_NOON)
        {
            mDayState = DAY_NOON;
            setDayElementsVisible(true);
            setNightElementsVisible(false);

            mSun->setColour(mSkyColours.mSun);
            mSunRadiance->setColour(ColourValue(0,0,0));

            mSkyLight->setDiffuseColour(mSkyColours.mDayLight);

            setSkyColours(mSkyColours.mDay);
            setCloudsColour(mSkyColours.mDayCloud);
        }
        const Real ratio = (mDayTime-1300)/500;

        mSunNode->setPosition(-SKY_FAR*ratio,SKY_FAR*(1-ratio)+SKY_TWILIGHT_OFFSET,0);
        mSkyLight->setDirection(ratio>0.8f ? Vector3(1,-0.75f - (1-ratio)*1.25f,0) : Vector3(ratio*1.25f,-1,0));
    }
    void updateNight()
    {
        if(mDayState!=DAY_NIGHT)
        {
            mDayState = DAY_NIGHT;
            setDayElementsVisible(false);
            setNightElementsVisible(true);

            mMoonNode->setPosition(SKY_FAR,SKY_FAR,SKY_FAR);
            mMoon->setColour(mSkyColours.mMoon);
            mMoonRadiance->setColour(mSkyColours.mMoon*0.5f);

            mSkyLight->setDirection(-1,-1,-1);
            mSkyLight->setDiffuseColour(mSkyColours.mNightLight);

            setSkyColours(mSkyColours.mNight);
            setCloudsColour(mSkyColours.mNightCloud);
            setStarsBrightness(1);
        }
        //const Real ratio = (mDayTime>=2000 ? (mDayTime-2000)/400 : mDayTime/600);
    }
    void setDayElementsVisible(const bool &flag)
    {
        mSunNode->setVisible(flag);
    }
    void setNightElementsVisible(const bool &flag)
    {
        mMoonNode->setVisible(flag);
        mStarNode->setVisible(flag);
    }
    void setSkyColours(const pair<ColourValue,ColourValue> &colour)
    {
        mSkyColourTop = colour.first;
        mSkyColourBot = colour.second;
        if(mSky)
        {
            SubEntity *subEnt = mSky->getSubEntity(0);
            subEnt->setCustomParameter(1,Vector4(mSkyColourTop.r,mSkyColourTop.g,mSkyColourTop.b,mSkyColourTop.a));
            subEnt->setCustomParameter(2,Vector4(mSkyColourBot.r,mSkyColourBot.g,mSkyColourBot.b,mSkyColourBot.a));
            CapabilitiesManager::getSingletonPtr()->assertCustomColour(subEnt,1);
            CapabilitiesManager::getSingletonPtr()->assertCustomColour(subEnt,2);
        }
        mSceneMgr->setFog(FOG_LINEAR, mSkyColourBot, FOG_RATIO, FOG_START, FOG_END);
        mViewport->setBackgroundColour(mSkyColourBot);
        if(mReflectViewport)mReflectViewport->setBackgroundColour(mSkyColourBot);
    }
    void updateStars()
    {
        if(!mStarSet->isVisible())return;
        for(int i=0;i<5;i++)twinkleRandomStar(mStarBrightness);
    }
    void setStarsBrightness(const Real &value)
    {
        for(int i=0;i<NUM_STARS;i++)
        {
            Billboard *star = mStarSet->getBillboard(i);
            const Real ratio = Math::RangeRandom(0,value);
            star->setColour(ColourValue(ratio,ratio,ratio));
        }
        mStarBrightness = value;
    }
    void twinkleRandomStar(const Real &max)
    {
        Billboard *star = mStarSet->getBillboard((int)Math::RangeRandom(0,NUM_STARS-1));
        const Real ratio = Math::RangeRandom(0,max);
        star->setColour(ColourValue(ratio,ratio,ratio));
    }
    void setCloudsColour(const ColourValue &colour)
    {
        mCloudColour = colour;
        for(std::vector<Cloud*>::iterator i=mCloudList.begin(); i!=mCloudList.end(); i++)
        {
            Cloud *cloud = *i;
            SubEntity *subEnt = cloud->mEntity->getSubEntity(0);
            const Real fadeRatio = subEnt->getCustomParameter(1).w;
            subEnt->setCustomParameter(1,Vector4(colour.r,colour.g,colour.b,fadeRatio));
            CapabilitiesManager::getSingletonPtr()->assertCustomColour(subEnt,1);
        }
    }
    void setCloudsVisible(const bool &flag)
    {
        for(std::vector<Cloud*>::iterator i=mCloudList.begin(); i!=mCloudList.end(); i++)
        {
            Cloud *cloud = *i;
            cloud->mSceneNode->setVisible(flag);
        }
    }
    void updateClouds(const Real &timeElapsed)
    {
        for(std::vector<Cloud*>::iterator i=mCloudList.begin(); i!=mCloudList.end(); i++)
        {
            Cloud *cloud = *i;
            SceneNode *node = cloud->mSceneNode;
            cloud->updatePosition(timeElapsed);
            cloud->updateOrientation();
            //Fading in/out
            if(cloud->mDoFadeIn || cloud->mDoFadeOut)
            {
                cloud->mFadeTimer -= timeElapsed;
                if(cloud->mFadeTimer<=0)
                {
                    //Fade out ended
                    if(cloud->mDoFadeOut)
                    {
                        cloud->mDoFadeOut = false;
                        if(Math::Abs(node->getPosition().x) > SKY_MID)
                        {
                            const Real z = Math::RangeRandom(-SKY_MID,SKY_MID);
                            node->setPosition(SKY_MID*(node->getPosition().x<0?1:-1),0,z);
                        }
                        else
                        {
                            const Real x = Math::RangeRandom(-SKY_MID,SKY_MID);
                            node->setPosition(x,0,SKY_MID*(node->getPosition().z<0?1:-1));
                        }
                        cloud->updatePosition(0);
                        cloud->mYaw = Math::RangeRandom(0,360);
                        cloud->updateOrientation();
                        cloud->randomizeScale();
                        cloud->randomizeMaterial();
                        cloud->randomizeVelocity(mWindVelocity);
                        cloud->mDoFadeIn = true;
                        cloud->setFadeTimer();
                    }
                    //Fade in ended
                    else
                    {
                        cloud->mDoFadeIn = false;
                        cloud->mFadeTimer = 0;
                    }
                }
                SubEntity *subEnt = cloud->mEntity->getSubEntity(0);
                const Vector4 colour = subEnt->getCustomParameter(1);
                subEnt->setCustomParameter(1,Vector4(colour.x,colour.y,colour.z,cloud->getFadeRatio()));
                CapabilitiesManager::getSingletonPtr()->assertCustomColour(subEnt,1);
            }
            //Exited boundaries
            else if(Math::Abs(node->getPosition().x) > SKY_MID || Math::Abs(node->getPosition().z) > SKY_MID)
            {
                cloud->mDoFadeOut = true;
                cloud->setFadeTimer();
            }
        }
    }
    static SkyManager* getSingletonPtr();
    static SkyManager& getSingleton();
    void setDayTime(const Real &time)
    {
        mDayTime = time;
    }
    const Real getDayTime()
    {
        return mDayTime;
    }
    void setWeatherTime(const Real &time)
    {
        mWeatherMgr->setWeatherTime(time);
    }
    void loadWeathers()
    {
        mWeatherMgr->loadWeathers();
    }
    void freezeTime(const bool &flag, const bool &weather=true)
    {
        mFreezeTime = flag;
        if(weather)mWeatherMgr->freezeTime(flag);
    }
    void showSky(const bool &flag)
    {
        mSky->setVisible(flag);
        mSkyLight->setVisible(flag);
        setDayElementsVisible(flag);
        setNightElementsVisible(flag);
        setCloudsVisible(flag);
        if(flag)
        {
            mDayState = DAY_NONE;
            updateSky();
        }
    }
    void setSkyColours(const SkyColours &colours)
    {
        mSkyColours = colours;
    }
    void setSunMat(const String &mat)
    {
        if(mSunSet)mSunSet->setMaterialName(mat==""?"Sky/SunMat":mat);
    }
    void setMoonMat(const String &mat)
    {
        if(mMoonSet)mMoonSet->setMaterialName(mat==""?"Sky/MoonMat":mat);
    }
    void setFastForward(const bool &flag)
    {
        mFastForward = flag;
    }
    WeatherManager* getWeatherManager()
    {
        return mWeatherMgr;
    }
    void loadSky(const String &filename, const bool &useFhz=true)
    {
        if(filename=="")return;

        SaveFile sf;
        //if(!sf.loadAnywhere(filename+".sky"))return;
        if(useFhz)
        {
            if(!ZipManager::getSingletonPtr()->loadByAnyMeans(&sf,filename,".fhs",".sky"))
            {
                LoggerManager::getSingleton().logMessage("ERROR::Sky: failed to load Sky "+filename);
                return;
            }
        }
        else
        {
            if(!sf.loadAnywhere(filename+".sky"))
            {
                LoggerManager::getSingleton().logMessage("ERROR::Sky: failed to load Sky "+filename);
                return;
            }
        }

        String buffer;
        for(int i=0;i<20;i++)
        {
            buffer = "";
            sf.getSetting(StringConverter::toString(i),buffer,"Colour");
            mSkyColours.setColour(i,StringConverter::parseColourValue(buffer));
        }
        buffer = "";
        sf.getSetting("0",buffer,"Mat");
        setSunMat(buffer);
        buffer = "";
        sf.getSetting("1",buffer,"Mat");
        setMoonMat(buffer);
    }
    void setReflectViewport(Viewport *v, const bool &update=false)
    {
        mReflectViewport = v;
        mWeatherMgr->setReflectViewport(v);
        if(update && mReflectViewport && mViewport)mReflectViewport->setBackgroundColour(mViewport->getBackgroundColour());
    }
    void reflectionEvent(Viewport *v)
    {
        setReflectViewport(v,true);
    }
};

template<> SkyManager* Singleton<SkyManager>::ms_Singleton = 0;

SkyManager* SkyManager::getSingletonPtr()
{
	return ms_Singleton;
}

SkyManager& SkyManager::getSingleton()
{
	assert(ms_Singleton);
	return *ms_Singleton;
}

#endif
