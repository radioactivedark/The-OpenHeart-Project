#ifndef _LIGHTMANAGER_H_
#define _LIGHTMANAGER_H_

#define MAX_LIGHTS 3

class LightManager : public Singleton<LightManager>
{
private:
    SceneManager *mSceneMgr;
    std::vector<Light*> mLightList;
    unsigned short mLightIndex;
public:
    LightManager()
    {
        mSceneMgr = 0;
        mLightList.clear();
        mLightIndex = 0;
    }
    ~LightManager()
    {
        clear();
    }
    void clear()
    {
        mLightList.clear();
        mLightIndex = 0;
    }
    void init(SceneManager *sceneMgr)
    {
        mSceneMgr = sceneMgr;
        setAmbientLight(ColourValue(0.75,0.75,0.75));
    }
    void setAmbientLight(const ColourValue &colour)
    {
        mSceneMgr->setAmbientLight(colour);
    }
    Light* createLight()
    {
        Light *light = 0;
        if(mLightList.size()<MAX_LIGHTS)
        {
            light = mSceneMgr->createLight("Light"+StringConverter::toString(mLightIndex++));
            mLightList.push_back(light);
        }
        return light;
    }
    void destroyLight(Light *light)
    {
        for(std::vector<Light*>::iterator it=mLightList.begin(); it!=mLightList.end(); it++)
        {
            Light *l = *it;
            if(l==light)
            {
                mSceneMgr->destroyLight(light);
                mLightList.erase(it);
                return;
            }
        }
    }
    static LightManager* getSingletonPtr();
    static LightManager& getSingleton();
};

template<> LightManager* Singleton<LightManager>::ms_Singleton = 0;

LightManager* LightManager::getSingletonPtr()
{
	return ms_Singleton;
}

LightManager& LightManager::getSingleton()
{
	assert(ms_Singleton);
	return *ms_Singleton;
}

#endif
