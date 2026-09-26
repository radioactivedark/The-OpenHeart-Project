#ifndef _WEATHERMANAGER_H_
#define _WEATHERMANAGER_H_

#include <SkyDefines.h>
#include <CameraManager.h>

struct Weather
{
    String mName;
    String mParticle;
    Vector3 mOffset;
    Real mEmissionRate;
    bool mHasOwnRate;
    ColourValue mSkyTop;
    ColourValue mSkyBot;
    ColourValue mClouds;
    bool mHasSkyShader;
    Weather(const String &name)
    {
        mName = name;
        mParticle = "";
        mOffset = Vector3::ZERO;
        mEmissionRate = 0;
        mHasOwnRate = false;
        mSkyTop = mSkyBot = mClouds = ColourValue();
        mHasSkyShader = false;
    }
    const pair<ColourValue,ColourValue> getSkyColours(const Real &ratio)
    {
        const Real diffRatio = 1-ratio;
        const ColourValue topDiff = ColourValue()-mSkyTop;
        const ColourValue botDiff = ColourValue()-mSkyBot;
        return pair<ColourValue,ColourValue>(mSkyTop+topDiff*diffRatio,mSkyBot+botDiff*diffRatio);
    }
    const ColourValue getCloudColour(const Real &ratio)
    {
        const Real diffRatio = 1-ratio;
        const ColourValue cloudDiff = ColourValue()-mClouds;
        return mClouds+cloudDiff*diffRatio;
    }
};

struct WeatherPeriod
{
    pair<unsigned short,unsigned short> mPeriod;  //Start and end time of weather period
    String mType;
    bool mFadeIn;
    bool mFadeOut;
    WeatherPeriod()
    {
        mPeriod = pair<unsigned short,unsigned short>(0,0);
        mType = "";
        mFadeIn = mFadeOut = false;
    }
    const bool isInPeriod(const Real &time)
    {
        //Wrapped period
        if(mPeriod.first>mPeriod.second)return (mPeriod.first<=time || time<mPeriod.second);
        else return (mPeriod.first<=time && time<mPeriod.second);
    }
    const Real getFadeRatio(const Real &time)
    {
        Real ratio = 0;
        //Wrapped period
        if(mPeriod.first>mPeriod.second)
        {
            const Real max = (WEATHER_TIME_MAX - mPeriod.first) + mPeriod.second;
            if(max>0)
            {
                if(time>=mPeriod.first)ratio = time - mPeriod.first;
                else ratio = time + (WEATHER_TIME_MAX - mPeriod.first);
                ratio /= max;
            }
        }
        else
        {
            const Real max = mPeriod.second - mPeriod.first;
            if(max>0)ratio = (time - mPeriod.first)/max;
        }
        return (mFadeIn ? ratio : 1-ratio);
    }
};

class WeatherManager : public CameraListener
{
private:
    SceneManager *mSceneMgr;
    Viewport *mViewport;
    Viewport *mReflectViewport;
    std::vector<Weather*> mWeatherList;
    std::vector<WeatherPeriod*> mWeatherCycle;
    SceneNode *mWeatherNode;
    ParticleSystem *mWeatherPS;
    Entity *mSky;
    std::vector<Cloud*> *mCloudList;
    bool mHasWeather;
    Real mWeatherTime;
    WeatherPeriod *mCurrentPeriod;
    Weather *mCurrentWeather;
    ColourValue *mSkyColourTop;
    ColourValue *mSkyColourBot;
    ColourValue *mCloudColour;
    bool mFreezeTime;
public:
    WeatherManager()
    {
        mWeatherList.clear();
        mWeatherCycle.clear();
        mSceneMgr = 0;
        mWeatherNode = 0;
        mWeatherPS = 0;
        reset();
    }
    ~WeatherManager()
    {
        reset();
        clearWeatherList();
    }
    void loadWeathers()
    {
        clearWeatherList();

        const std::vector<String> list = SaveFile::findResourceNames("*.weather");
        for(std::vector<String>::const_iterator i = list.begin(); i!=list.end(); i++)
        {
            const String filename = *i;

            SaveFile sf;
            if(!sf.loadAnywhere(filename))continue;

            loadWeather(&sf);
        }
    }
    void loadEncryptedWeather(const String &weatherName, const String &cycleFilename)
    {
        SaveFile sf;
        if(!sf.loadZip(weatherName+".weather",ZIP_TEMP_DIR+cycleFilename+".fhw"+String(ZIP_TEMP_EXTENSION)))return;

        loadWeather(&sf);
    }
    void loadWeather(SaveFile *sf)
    {
        for(sf->beginSectionIterator(); sf->hasMoreSections(); sf->nextSection())
        {
            Weather *weather = new Weather(sf->peekNextSectionName());

            for(sf->beginSettingIterator(); sf->hasMoreSettings(); sf->nextSetting())
            {
                const String tag = sf->getSettingTag();
                if(tag=="ps")weather->mParticle = sf->getSettingValue();
                else if(tag=="offset")weather->mOffset = StringConverter::parseVector3(sf->getSettingValue());
                else if(tag=="rate")
                {
                    weather->mEmissionRate = StringConverter::parseReal(sf->getSettingValue());
                    weather->mHasOwnRate = true;
                }
                else if(tag=="skytop")
                {
                    weather->mSkyTop = StringConverter::parseColourValue(sf->getSettingValue());
                    weather->mHasSkyShader = true;
                }
                else if(tag=="skybot")
                {
                    weather->mSkyBot = StringConverter::parseColourValue(sf->getSettingValue());
                    weather->mHasSkyShader = true;
                }
                else if(tag=="clouds")
                {
                    weather->mClouds = StringConverter::parseColourValue(sf->getSettingValue());
                    weather->mHasSkyShader = true;
                }
            }

            mWeatherList.push_back(weather);
        }
    }
    void loadWeatherCycle(const String &filename, const bool &useFhz=true)
    {
        clearWeatherCycle();
        if(filename=="")return;

        SaveFile sf;
        //if(!sf.loadAnywhere(filename+".cycle"))return;
        if(useFhz)
        {
            if(!ZipManager::getSingletonPtr()->loadByAnyMeans(&sf,filename,".fhw",".cycle"))
            {
                LoggerManager::getSingleton().logMessage("ERROR::Weather: failed to load WeatherCycle "+filename);
                return;
            }
        }
        else
        {
            if(!sf.loadAnywhere(filename+".cycle"))
            {
                LoggerManager::getSingleton().logMessage("ERROR::Weather: failed to load WeatherCycle "+filename);
                return;
            }
        }

        for(sf.beginSectionIterator(); sf.hasMoreSections(); sf.nextSection())
        {
            const StringVector periodStr = StringUtil::split(sf.peekNextSectionName(),"-",1);
            if(periodStr.size()<2)continue;

            WeatherPeriod *period = new WeatherPeriod();
            period->mPeriod.first = StringConverter::parseInt(periodStr[0]);
            period->mPeriod.second = StringConverter::parseInt(periodStr[1]);

            for(sf.beginSettingIterator(); sf.hasMoreSettings(); sf.nextSetting())
            {
                const String tag = sf.getSettingTag();
                if(tag=="type")
                {
                    //Check if weather already loaded, if not, find in zip
                    const String weatherName = period->mType = sf.getSettingValue();
                    bool found = false;
                    for(std::vector<Weather*>::iterator it=mWeatherList.begin(); it!=mWeatherList.end(); it++)
                    {
                        Weather *weather = *it;
                        if(weather->mName==weatherName)
                        {
                            found = true;
                            break;
                        }
                    }
                    if(!found)loadEncryptedWeather(weatherName,filename+".fhw");
                }
                else if(tag=="fade")
                {
                    period->mFadeIn = (sf.getSettingValue()=="in");
                    period->mFadeOut = (sf.getSettingValue()=="out");
                }
            }

            mWeatherCycle.push_back(period);
        }
    }
    void clearWeatherList()
    {
        while(!mWeatherList.empty())
        {
            Weather *weather = mWeatherList.back();
            mWeatherList.pop_back();
            delete weather;
        }
    }
    void clearWeatherCycle()
    {
        while(!mWeatherCycle.empty())
        {
            WeatherPeriod *period = mWeatherCycle.back();
            mWeatherCycle.pop_back();
            delete period;
        }
    }
    void reset()
    {
        clearWeatherCycle();
        mHasWeather = false;
        if(mWeatherNode)mWeatherNode->detachAllObjects();
        mWeatherNode = 0;
        if(mWeatherPS)mSceneMgr->destroyParticleSystem(mWeatherPS);
        mWeatherPS = 0;
        mSky = 0;
        mCloudList = 0;
        mWeatherTime = 0;
        mCurrentPeriod = 0;
        mCurrentWeather = 0;
        mFreezeTime = false;
        mReflectViewport = 0;
    }
    void init(SceneManager *sceneMgr, Viewport *viewport, ColourValue *skyTop, ColourValue *skyBot, ColourValue *cloud)
    {
        mSceneMgr = sceneMgr;
        CameraManager::getSingletonPtr()->addListener(this);
        mViewport = viewport;
        mSkyColourTop = skyTop;
        mSkyColourBot = skyBot;
        mCloudColour = cloud;
    }
    void createWeather(SceneNode *skyNode, Entity *sky, std::vector<Cloud*> *cloudList)
    {
        if(mHasWeather)return;
        mHasWeather = true;
        mWeatherNode = skyNode->createChildSceneNode();
        mSky = sky;
        mCloudList = cloudList;
    }
    void update(const Real &timeElapsed, const bool &skyUpdated)
    {
        updateTime(timeElapsed);
        if(!mHasWeather)return;

        if(mCurrentPeriod && mCurrentPeriod->isInPeriod(mWeatherTime))updateCurrentPeriod(skyUpdated);
        else
        {
            //Change current weather period
            for(std::vector<WeatherPeriod*>::iterator i=mWeatherCycle.begin(); i!=mWeatherCycle.end(); i++)
            {
                WeatherPeriod *period = *i;
                if(period->isInPeriod(mWeatherTime))
                {
                    enterPeriod(period);
                    return;
                }
            }
            //No new weather period
            if(mCurrentPeriod)exitCurrentPeriod();
        }
    }
    void updateTime(const Real &timeElapsed)
    {
        if(mFreezeTime)return;

        mWeatherTime += timeElapsed*TIME_SPEED;
        if(mWeatherTime>WEATHER_TIME_MAX)mWeatherTime -= WEATHER_TIME_MAX;
    }
    void enterPeriod(WeatherPeriod *period)
    {
        Weather *prevWeather = mCurrentWeather;
        mCurrentPeriod = period;
        mCurrentWeather = getWeatherByName(mCurrentPeriod->mType);
        if(!mCurrentWeather)
        {
            throw(Exception(9,"No Weather by name '"+mCurrentPeriod->mType+"'",""));
            return;
        }

        changeWeather(mCurrentWeather,prevWeather);

        if(mCurrentPeriod->mFadeIn)setWeatherRatio(0);
        else setWeatherRatio(1);
    }
    void changeWeather(Weather *current, Weather *prevWeather)
    {
        mCurrentWeather = current;
        //No particle effect
        if(mCurrentWeather->mParticle=="")
        {
            mWeatherNode->detachAllObjects();
            if(mWeatherPS)mSceneMgr->destroyParticleSystem(mWeatherPS);
            mWeatherPS = 0;
        }
        //New particle effect
        else if(!prevWeather || prevWeather->mParticle!=mCurrentWeather->mParticle)
        {
            mWeatherNode->detachAllObjects();
            if(mWeatherPS)mSceneMgr->destroyParticleSystem(mWeatherPS);
            if(!ParticleSystemManager::getSingletonPtr()->getTemplate(mCurrentWeather->mParticle))
            {
                LoggerManager::getSingleton().logMessage("WeatherManager::changeWeather: ParticleSystem not found: " + mCurrentWeather->mParticle);
                mWeatherPS = 0;
                return;
            }
            mWeatherPS = mSceneMgr->createParticleSystem("WeatherPS",mCurrentWeather->mParticle);
            mWeatherPS->setVisibilityFlags(WaterManager::REFLECTMASK_WEATHER);
            mWeatherNode->attachObject(mWeatherPS);
            mWeatherNode->setPosition(mCurrentWeather->mOffset);
        }

        if(!mCurrentWeather->mHasOwnRate && mWeatherPS)mCurrentWeather->mEmissionRate = mWeatherPS->getEmitter(0)->getEmissionRate();
        if(!mCurrentWeather->mHasSkyShader)
        {
            shadeSky(pair<ColourValue,ColourValue>(ColourValue(),ColourValue()));
            shadeClouds(ColourValue());
        }
    }
    void updateCurrentPeriod(const bool &skyUpdated)
    {
        if(mCurrentPeriod->mFadeIn || mCurrentPeriod->mFadeOut)
        {
            const Real ratio = mCurrentPeriod->getFadeRatio(mWeatherTime);
            setWeatherRatio(ratio);
        }
        else if(skyUpdated)setWeatherRatio(1);
    }
    void setWeatherRatio(const Real &ratio)
    {
        if(mWeatherPS)mWeatherPS->getEmitter(0)->setEmissionRate(ratio*mCurrentWeather->mEmissionRate);
        if(mCurrentWeather->mHasSkyShader)
        {
            shadeSky(mCurrentWeather->getSkyColours(ratio));
            shadeClouds(mCurrentWeather->getCloudColour(ratio));
        }
    }
    void exitCurrentPeriod()
    {
        mWeatherNode->detachAllObjects();
        if(mWeatherPS)mSceneMgr->destroyParticleSystem(mWeatherPS);
        mWeatherPS = 0;
        shadeSky(pair<ColourValue,ColourValue>(ColourValue(),ColourValue()));
        shadeClouds(ColourValue());
        mCurrentPeriod = 0;
        mCurrentWeather = 0;
    }
    void shadeSky(const pair<ColourValue,ColourValue> &colour)
    {
        if(!mSky)return;
        const ColourValue skyTop = *mSkyColourTop;
        const ColourValue skyBot = *mSkyColourBot;
        const ColourValue topResult = colour.first*skyTop;
        const ColourValue botResult = colour.second*skyBot;

        SubEntity *subEnt = mSky->getSubEntity(0);
        subEnt->setCustomParameter(1,Vector4(topResult.r,topResult.g,topResult.b,topResult.a));
        subEnt->setCustomParameter(2,Vector4(botResult.r,botResult.g,botResult.b,botResult.a));
        CapabilitiesManager::getSingletonPtr()->assertCustomColour(subEnt,1);
        CapabilitiesManager::getSingletonPtr()->assertCustomColour(subEnt,2);
        mSceneMgr->setFog(FOG_LINEAR, botResult, FOG_RATIO, FOG_START, FOG_END);
        mViewport->setBackgroundColour(botResult);
        if(mReflectViewport)mReflectViewport->setBackgroundColour(botResult);
    }
    void shadeClouds(const ColourValue &colour)
    {
        if(!mCloudList)return;
        const ColourValue base = *mCloudColour;
        const ColourValue result = colour*base;

        for(std::vector<Cloud*>::iterator i=mCloudList->begin(); i!=mCloudList->end(); i++)
        {
            Cloud *cloud = *i;
            SubEntity *subEnt = cloud->mEntity->getSubEntity(0);
            const Real fadeRatio = subEnt->getCustomParameter(1).w;
            subEnt->setCustomParameter(1,Vector4(result.r,result.g,result.b,fadeRatio));
            CapabilitiesManager::getSingletonPtr()->assertCustomColour(subEnt,1);
        }
    }
    void setWeatherTime(const Real &time)
    {
        mWeatherTime = time;
    }
    Weather* getWeatherByName(const String &name)
    {
        for(std::vector<Weather*>::iterator i=mWeatherList.begin(); i!=mWeatherList.end(); i++)
        {
            Weather *weather = *i;
            if(weather->mName==name)return weather;
        }
        return 0;
    }
    void freezeTime(const bool &flag)
    {
        mFreezeTime = flag;
    }
    void setReflectViewport(Viewport *v)
    {
        mReflectViewport = v;
    }
    void cameraUnderwaterEvent(const bool &flag)
    {
        if(mWeatherPS)mWeatherPS->setVisible(!flag);
    }
};

#endif
