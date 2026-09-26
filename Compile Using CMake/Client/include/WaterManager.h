#ifndef _WATERMANAGER_H_
#define _WATERMANAGER_H_

#include <Collision.h>
#include <GridManager.h>

#define WATER_SIZE 500
#define WATER_DEFAULTMAT "WaterMat"
#define WATER_REFLECTMAT "WaterReflectMat"
#define WATER_UNDERWATERMAT "UnderWaterMat"

class ReflectionListener
{
public:
    ReflectionListener(){}
    virtual ~ReflectionListener(){}
    virtual void reflectionEvent(Viewport *v) = 0;
};

class WaterManager : public Singleton<WaterManager>, public RenderTargetListener
{
private:
    SceneManager *mSceneMgr;
    GridManager *mGridMgr;
    Camera *mCamera;
    std::vector<WaterPlane*> mWaterList;
    SceneNode *mOceanNode;
    bool mHasOcean;
    Real mOceanHeight;
    String mOceanMat;
    MovablePlane *mReflectPlane;
    SceneNode *mReflectNode;
    RenderTarget *mReflectTex;
    TextureUnitState *mReflectTexState;
    TextureUnitState *mUnderwaterTexState;
    bool mDoReflection;
    bool mIsUnderwater;
    unsigned int mWaterPlaneEntCounter;
    ReflectionListener *mReflectionListener;
public:
    WaterManager()
    {
        mSceneMgr = 0;
        mOceanNode = 0;
        mHasOcean = false;
        mOceanHeight = 0;
        mOceanMat = "";
        mDoReflection = true;
        mCamera = 0;
        mReflectNode = 0;
        mWaterPlaneEntCounter = 0;
        createWaterPlaneMesh();
        createReflectPlane();
        mReflectionListener = 0;
    }
    ~WaterManager()
    {
        clear();
        if(mReflectPlane)delete mReflectPlane;
    }
    static WaterManager* getSingletonPtr();
    static WaterManager& getSingleton();
    enum ReflectMask
    {
        REFLECTMASK_WEATHER = 1<<0
    };
    void init(SceneManager *sceneMgr, Camera *camera)
    {
        mSceneMgr = sceneMgr;
        mGridMgr = GridManager::getSingletonPtr();

        mCamera = camera;
        mReflectNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        mReflectNode->attachObject(mReflectPlane);
        if(mDoReflection)doReflection(true,true);
    }
    void clear()
    {
        while(!mWaterList.empty())
        {
            WaterPlane *water = mWaterList.back();
            mWaterList.pop_back();
            if(mSceneMgr)
            {
                water->mEnt->getParentSceneNode()->detachObject(water->mEnt);
                mSceneMgr->destroyEntity(water->mEnt);
            }
            delete water;
        }
        destroyOcean();
        mOceanNode = 0;
        mSceneMgr = 0;
        removeReflectionViewport();
        mCamera = 0;
        mReflectNode = 0;
        mWaterPlaneEntCounter = 0;
        mReflectionListener = 0;
        mIsUnderwater = 0;
    }
    void update(const Vector3 camPos)
    {
        if(!mDoReflection)return;
        //Find highest waterplane below camera
        WaterPlane *highestWater = 0;
        for(std::vector<WaterPlane*>::iterator it=mWaterList.begin(); it!=mWaterList.end(); it++)
        {
            WaterPlane *water = *it;
            const Real width = water->mSize.x*0.5f;
            const Real height = water->mSize.y*0.5f;
            if(water->mCenter.x-width<=camPos.x && water->mCenter.x+width>=camPos.x
                && water->mCenter.z-height<=camPos.z && water->mCenter.z+height>=camPos.z
                && water->mCenter.y<camPos.y && (!highestWater||highestWater->mCenter.y<water->mCenter.y))
                highestWater = water;
        }
        if(mHasOcean && (!highestWater||highestWater->mCenter.y<mOceanHeight))mReflectNode->setPosition(mOceanNode->getPosition());
        else
        {
            if(!highestWater && !mWaterList.empty())highestWater = mWaterList.back();
            if(highestWater)mReflectNode->setPosition(highestWater->mCenter);
        }
    }
    WaterPlane* createWaterPlane(const Vector3 &center, const Vector2 &size, const String &material)
    {
        Entity *ent = mSceneMgr->createEntity("WaterPlane"+StringConverter::toString(mWaterPlaneEntCounter++),"WaterPlaneMesh");
        WaterPlane *water = new WaterPlane(center,size,ent,material==WATER_REFLECTMAT);
        ent->setMaterialName(!water->mIsReflectMat||mDoReflection?material:WATER_DEFAULTMAT);
        mWaterList.push_back(water);
        return water;
    }
    void createWaterPlaneMesh()
    {
        Plane waterPlane;
		waterPlane.d = 0;
		waterPlane.normal = Vector3::UNIT_Y;

		MeshManager::getSingleton().createPlane("WaterPlaneMesh",
				ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
				waterPlane,WATER_SIZE,WATER_SIZE,4,4,true,1,5,5,Vector3::UNIT_Z);
    }
    void createReflectPlane()
    {
        mReflectPlane = new MovablePlane("ReflectPlane");
        mReflectPlane->d = 0;
        mReflectPlane->normal = Vector3::UNIT_Y;

        TexturePtr texture = TextureManager::getSingleton().createManual("RttTex",
                ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_2D,
                512, 512, 0, PF_R8G8B8, TU_RENDERTARGET );
		mReflectTex = texture->getBuffer()->getRenderTarget();
		mReflectTex->addListener(this);

		MaterialPtr mat = MaterialManager::getSingleton().getByName(WATER_REFLECTMAT);
        mReflectTexState = mat->getTechnique(0)->getPass(0)->createTextureUnitState("RttTex",1);
        //mReflectTexState->setColourOperationEx(LBX_BLEND_MANUAL, LBS_TEXTURE, LBS_CURRENT, ColourValue::White, ColourValue::White, 0.5f);
        mReflectTexState->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);

        MaterialPtr underwaterMat = MaterialManager::getSingleton().getByName(WATER_UNDERWATERMAT);
        mUnderwaterTexState = underwaterMat->getTechnique(0)->getPass(0)->createTextureUnitState("RttTex",1);
        mUnderwaterTexState->setTextureAddressingMode(TextureUnitState::TAM_CLAMP);
    }
    void createOceanPlaneMesh(const Vector2 &size)
    {
        if(MeshManager::getSingleton().resourceExists("OceanPlaneMesh"))MeshManager::getSingleton().remove("OceanPlaneMesh");

        Plane waterPlane;
		waterPlane.d = 0;
		waterPlane.normal = Vector3::UNIT_Y;

		MeshManager::getSingleton().createPlane("OceanPlaneMesh",
				ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
				waterPlane,size.x,size.y,16,16,true,1,size.x*0.01f,size.y*0.01f,Vector3::UNIT_Z);
    }
    void createOcean(const Vector3 &center,const Vector2 &size,const String &mat="")
    {
        if(mHasOcean)destroyOcean();
        createOceanPlaneMesh(size);
        Entity *ent = mSceneMgr->createEntity("Ocean","OceanPlaneMesh");
        if(mat=="")ent->setMaterialName(mDoReflection?WATER_REFLECTMAT:"OceanMat");
        else ent->setMaterialName(mat);
        if(!mOceanNode)mOceanNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        mOceanNode->attachObject(ent);
        mOceanNode->setPosition(center);

        mHasOcean = true;
        mOceanHeight = center.y;
        mOceanMat = mat;
        if(mReflectNode)mReflectNode->setPosition(center);
    }
    void destroyOcean()
    {
        if(!mHasOcean)return;
        mOceanNode->detachAllObjects();
        mSceneMgr->destroyEntity("Ocean");

        mHasOcean = false;
        mOceanHeight = 0;
    }
    void updateWaterCollision(Unit* unit)
    {
        bool isSwimming = false;
        bool headBelowWater = false;
        bool collides = false;
        const Vector3 unitPos = unit->getPosition();

        //Ocean collision
        if(mHasOcean)
        {
            collides = (unitPos.y <= mOceanHeight);
            isSwimming = (unitPos.y+unit->getHeadHeight() <= mOceanHeight);
            headBelowWater = (unitPos.y+unit->getHeadHeight()*1.2f <= mOceanHeight);
        }
        //Water body collision
        if(!collides)
        {
            Grid *activeGrid = mGridMgr->getActiveGrid();
            if(!activeGrid)return;
            collides = activeGrid->getWaterCollision(unitPos,unit->getHeadHeight(),&isSwimming,&headBelowWater);
        }
        //Apply collision changes
        if(collides || unit->mIsSwimming || unit->mIsWading)
        {
            unit->mAntiGravity = isSwimming;
            unit->mIsSwimming = isSwimming;
            unit->mIsWading = (collides&&!headBelowWater);
        }
    }
    const bool collidesWater(const Vector3 &pos)
    {
        //Ocean collision
        if(mHasOcean && pos.y <= mOceanHeight)return true;
        //Water body collision
        Grid *activeGrid = mGridMgr->getActiveGrid();
        if(!activeGrid)return false;
        return activeGrid->getWaterCollision(pos,0);
    }
    void doReflection(bool flag, const bool &force=false)
    {
        if(!CapabilitiesManager::getSingletonPtr()->hasShaders())flag = false;

        if(mDoReflection==flag && !force)return;
        mDoReflection = flag;
        if(!mCamera)return;
        if(mDoReflection)
        {
            initReflectionViewport();
            setAllWaterMaterials(mIsUnderwater?WATER_UNDERWATERMAT:WATER_REFLECTMAT);
        }
        else
        {
            removeReflectionViewport();
            setAllWaterMaterials("");
        }
    }
    const bool getDoReflection()
    {
        return mDoReflection;
    }
    void setUnderwater(const bool &flag)
    {
        mIsUnderwater = flag;
        if(flag)
        {
            if(mDoReflection)
            {
                initReflectionViewport();
                setAllWaterMaterials(WATER_UNDERWATERMAT);
            }
        }
        else
        {
            if(mDoReflection)
            {
                initReflectionViewport();
                setAllWaterMaterials(mDoReflection?WATER_REFLECTMAT:"");
            }
        }
    }
    void initReflectionViewport()
    {
        removeReflectionViewport();
        Viewport *v = mReflectTex->addViewport(mCamera);
        v->setClearEveryFrame(true);
        v->setOverlaysEnabled(false);
        v->setShadowsEnabled(false);
        v->setVisibilityMask(~REFLECTMASK_WEATHER);
        if(mReflectionListener)mReflectionListener->reflectionEvent(v);
        if(mIsUnderwater)mUnderwaterTexState->setProjectiveTexturing(true,mCamera);
        else mReflectTexState->setProjectiveTexturing(true,mCamera);
    }
    void removeReflectionViewport()
    {
        mReflectTexState->setProjectiveTexturing(false);
        mUnderwaterTexState->setProjectiveTexturing(false);
        mReflectTex->removeAllViewports();
        if(mReflectionListener)mReflectionListener->reflectionEvent(0);
    }
    void setAllWaterMaterials(const String &mat, const bool &reflectiveOnly=true)
    {
        if(mHasOcean && (!reflectiveOnly||mOceanMat==""))mSceneMgr->getEntity("Ocean")->setMaterialName(mat==""?"OceanMat":mat);
        for(std::vector<WaterPlane*>::iterator it=mWaterList.begin(); it!=mWaterList.end(); it++)
        {
            WaterPlane *water = *it;
            if(!reflectiveOnly||water->mIsReflectMat)water->mEnt->setMaterialName(mat==""?WATER_DEFAULTMAT:mat);
        }
    }
    void setAllWaterVisibility(const bool &flag, const bool &reflectiveOnly=true)
    {
        if(mHasOcean && (!reflectiveOnly||mOceanMat==""))mSceneMgr->getEntity("Ocean")->setVisible(flag);
        for(std::vector<WaterPlane*>::iterator it=mWaterList.begin(); it!=mWaterList.end(); it++)
        {
            WaterPlane *water = *it;
            if(!reflectiveOnly||water->mIsReflectMat)water->mEnt->setVisible(flag);
        }
    }
    void setReflectionListener(ReflectionListener *listener)
    {
        mReflectionListener = listener;
    }
    void preRenderTargetUpdate(const RenderTargetEvent& evt)
    {
        setAllWaterVisibility(false);
        if(!mIsUnderwater)
        {
            mCamera->enableReflection(mReflectPlane);
            mCamera->enableCustomNearClipPlane(mReflectPlane);
        }
    }
    void postRenderTargetUpdate(const RenderTargetEvent& evt)
    {
        setAllWaterVisibility(true);
        if(!mIsUnderwater)
        {
            mCamera->disableReflection();
            mCamera->disableCustomNearClipPlane();
        }
    }
    void cameraUnderwaterEvent(const bool &flag)
    {
        setUnderwater(flag);
    }
};

template<> WaterManager* Singleton<WaterManager>::ms_Singleton = 0;

WaterManager* WaterManager::getSingletonPtr()
{
	return ms_Singleton;
}

WaterManager& WaterManager::getSingleton()
{
	assert(ms_Singleton);
	return *ms_Singleton;
}

#endif
