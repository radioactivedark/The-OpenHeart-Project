#ifndef _MAPMAKER_H_
#define _MAPMAKER_H_

#define MAPMAKER_TEMP "temp"
#define MAPMAKER_DIR "media/terrains/"
#define MAPMAKER_OFFICIALIZER false

#include <ObjectMaker.h>
#include <SkyMaker.h>
#include <MapManager.h>
#include "md5.h"

class MapMakerObject
{
public:
    SceneNode *mNode;
    String mObjectName; //Used as Material name for Water and map name for gate
    bool mIsWater;
    std::vector< std::pair<SceneNode*,Real> > mSphereNodes;
    bool mIsGate;
    Vector3 mGatePosition;
    Real mGateYaw;
    bool mIsLarge;
    bool mIsGateBlackMat;
    MapMakerObject()
    {
        mNode = 0;
        mObjectName = "";
        mIsWater = false;
        mIsGate = false;
        mGatePosition = Vector3::ZERO;
        mGateYaw = 0;
        mIsLarge = false;
        mIsGateBlackMat = false;
    }
    ~MapMakerObject()
    {
        if(!mNode)return;
        SceneManager *sceneMgr = mNode->getCreator();

        if(mIsWater || mIsGate)
        {
            if(mNode->numAttachedObjects()>0)
            {
                MovableObject *movable = mNode->detachObject((unsigned short)(0));
                sceneMgr->destroyMovableObject(movable);
            }
        }
        else
        {
            SceneNode::ChildNodeIterator it = mNode->getChildIterator();
            while(it.hasMoreElements())
            {
                SceneNode *node = static_cast<SceneNode*>(it.getNext());
                for(int i=(int)node->numAttachedObjects()-1; i>=0; i--)
                {
                    MovableObject *movable = node->detachObject(i);
                    sceneMgr->destroyMovableObject(movable);
                }
            }
        }
        mSphereNodes.clear();
    }
    void highlight(const bool &flag)
    {
        if(mIsWater)
        {
            if(mNode->numAttachedObjects()>0)
                static_cast<Entity*>(mNode->getAttachedObject(0))->setMaterialName(flag?"Collision/Highlight": (mObjectName==""?WATER_DEFAULTMAT:mObjectName) );
        }
        else if(mIsGate)mNode->showBoundingBox(flag);
        else
        {
            SceneNode::ChildNodeIterator it = mNode->getChildIterator();
            while(it.hasMoreElements())
            {
                SceneNode *node = static_cast<SceneNode*>(it.getNext());
                const String type = node->getAttachedObject(0)->getMovableType();
                if(type!="BillboardSet" && type!="ParticleSystem")node->showBoundingBox(flag);
            }
        }
    }
    const bool isContainingNode(SceneNode *node)
    {
        if(mIsWater)return (node==mNode);
        else if(mIsGate)return (node==mNode);
        return (node->getParentSceneNode()==mNode);
    }
    void setScale(const Vector3 &scale)
    {
        if(mIsWater)mNode->setScale(Vector3(scale.x/WATER_SIZE,1,scale.z/WATER_SIZE));
        else if(mIsGate)mNode->setScale(Vector3(scale.y,scale.y,scale.y));
        else
        {
            mNode->setScale(scale);
            for(std::vector< std::pair<SceneNode*,Real> >::iterator it=mSphereNodes.begin(); it!=mSphereNodes.end(); it++)
            {
                std::pair<SceneNode*,Real> sphere = *it;
                sphere.first->setScale(Vector3::UNIT_SCALE*sphere.second*scale.y);
            }
        }
    }
    const Vector3 getScale()
    {
        if(mIsWater)return Vector3(mNode->getScale().x*WATER_SIZE,1,mNode->getScale().z*WATER_SIZE);
        if(mIsGate)return Vector3(1,mNode->getScale().y,1);
        return mNode->getScale();
    }
    void getEntityList(std::vector<Entity*> *list)
    {
        SceneNode::ChildNodeIterator it = mNode->getChildIterator();
        while(it.hasMoreElements())
        {
            SceneNode *node = static_cast<SceneNode*>(it.getNext());
            MovableObject *movable = node->getAttachedObject(0);
            if(movable->getMovableType()=="Entity")list->push_back(static_cast<Entity*>(movable));
        }
    }
};

class MapMaker
{
private:
    SceneManager *mSceneMgr;
    Gui *mGui;
    CameraManager *mCameraMgr;
    CollisionManager *mCollMgr;
    LightManager *mLightMgr;
    SkyManager *mSkyMgr;
    WaterManager *mWaterMgr;
    MapManager *mMapMgr;
    WorldObjectManager *mWorldObjectMgr;
    GuiMultiPanel *mPanel;

    String mMapName;
    GuiTextField *mMapField;
    String mDisplayName;
    GuiTextField *mDisplayField;
    GuiHighlightList *mMapList;
    GuiSlider *mMapListSlider;
    GuiCheckBox *mHasPortalCheckBox;
    GuiTextField *mPortalPosField[3];
    OverlayElement *mPortalLabel;
    OverlayElement *mPortalPosLabel[3];
    GuiTextField *mPortalYawField;
    OverlayElement *mPortalYawLabel;
    String mMusic;
    GuiTextField *mMusicField;
    GuiCheckBox *m_pNoFlyingCheckBox;

    String mTerrainHeightMap;
    String mTerrainAlphaTex[2];
    unsigned short mTerrainTex[6];
    unsigned char mTerrainTexPart;
    Vector3 mTerrainSize;
    Real mTerrainBoundary;
    GuiTextField *mTerrainMapField;
    GuiTextField *mTerrainAlphaField;
    GuiTextField *mTerrainWidthField;
    GuiTextField *mTerrainHeightField;
    GuiTextField *mTerrainBoundaryField;
    GuiHighlightList *mTerrainTexList;
    GuiSlider *mTerrainTexListSlider;
    OverlayElement *mTerrainTexLabel;
    GuiButton *mTerrainTexUp;
    GuiButton *mTerrainTexDn;

    bool mHasSky;
    String mSkyFile;
    String mWeatherFile;
    String mCeilingMat;
    Real mCeilingHeight;
    GuiCheckBox *mSkyCheckBox;
    OverlayElement *mSkyFileLabel;
    GuiTextField *mSkyField;
    GuiTextField *mWeatherField;
    OverlayElement *mCeilingHeightLabel;
    GuiTextField *mCeilingHeightField;

    bool mHasOcean;
    Vector3 mOceanCenter;
    Vector2 mOceanSize;
    String mOceanMat;
    GuiCheckBox *mOceanCheckBox;
    GuiTextField *mOceanHeightField;
    GuiTextField *mOceanMatField;

    GuiHighlightList *mObjectSelectList;
    GuiSlider *mObjectSelectListSlider;
    GuiHighlightList *mObjectGroupList;
    GuiSlider *mObjectGroupListSlider;

    GuiTextField *mGateMapField;
    GuiTextField *mGatePosField[3];
    GuiTextField *mGateYawField;
    GuiCheckBox *mGateBlackMatCheckBox;

    GuiTextField *mWaterField;
    String mWaterMat;

    std::vector<MapMakerObject*> mObjectList;
    MapMakerObject *mCurrentObject;
    std::vector<MapMakerObject*> mSelectionHistory;
    GuiHighlightList *mSelectTypeList;
    unsigned char mSelectObjectType;
    unsigned short mObjectIndex;
    GuiTextField *mPosField[3];
    GuiTextField *mScaleField[3];
    Vector3 mPosition;
    Vector3 mScale;
    OverlayElement *mObjectIsLargeLabel;
    GuiCheckBox *mObjectIsLargeCheckBox;

    GuiPanel *mMiniMapPanel;
    GuiMap *mMiniMap;

    unsigned short mHeldType;
    bool mHeldDirection;
    Vector3 mHeldValue;
    Real mSpeedModifier;
    bool mWasTesting;
    bool mMoveMode;
    bool mPlaceOnMesh;
    bool mRandomizeClone;
    Vector3 mRandomDefaultScale;
    Vector3 mCursorPosition;
public:
    bool mShowCollisions;
    MapMaker()
    {
        mSceneMgr = 0;
        mCurrentObject = 0;
        clear();
    }
    ~MapMaker()
    {
        deleteData();
    }
    enum
    {
        HELD_NONE,
        OBJECT_POSITION,
        OBJECT_SCALE,
        OBJECT_ROTATION,
    };
    void init(SceneManager *sceneMgr)
    {
        mSceneMgr = sceneMgr;
        mGui = Gui::getSingletonPtr();
        mCameraMgr = CameraManager::getSingletonPtr();
        mCollMgr = CollisionManager::getSingletonPtr();
        mLightMgr = LightManager::getSingletonPtr();
        mSkyMgr = SkyManager::getSingletonPtr();
        mSkyMgr->createSky(mLightMgr->createLight());
        mWaterMgr = WaterManager::getSingletonPtr();
        mMapMgr = MapManager::getSingletonPtr();
        mWorldObjectMgr = WorldObjectManager::getSingletonPtr();

        mPanel = mGui->addMultiPanel("MapMakerScreen",true);

        //Map page
        mMapField = mPanel->getTextField("MapMakerScreen/Name");
        mMapField->bindString(&mMapName);
        mMapField->mMaxLength = 20;
        mDisplayField = mPanel->getTextField("MapMakerScreen/DisplayName");
        mDisplayField->bindString(&mDisplayName);
        mDisplayField->mMaxLength = 32;
        mMapList = mPanel->getHighlightList("MapMakerScreen/Maps");
        mMapListSlider = mPanel->getSlider("MapMakerScreen/MapsLS");
        mPanel->getCheckBox("MapMakerScreen/ShowColl")->setChecked(mShowCollisions);

        loadAllMaps();

        //Portal page
        mHasPortalCheckBox = mPanel->getCheckBox("MapMakerScreen/HasPortal");
        mHasPortalCheckBox->setChecked(true);
        mPortalLabel = OverlayManager::getSingleton().getOverlayElement("MapMakerScreen/PortalPosLabel");
        for(int i=0;i<3;i++)
        {
            mPortalPosField[i] = mPanel->getTextField("MapMakerScreen/PortalPos"+StringConverter::toString(i));
            mPortalPosField[i]->mMaxLength = 6;
            mPortalPosLabel[i] = OverlayManager::getSingleton().getOverlayElement("MapMakerScreen/PortalPos"+StringConverter::toString(i)+"Label");
        }
        mPortalYawField = mPanel->getTextField("MapMakerScreen/PortalYaw");
        mPortalYawField->mMaxLength = 6;
        mPortalYawLabel = OverlayManager::getSingleton().getOverlayElement("MapMakerScreen/PortalYawLabel");

        updateHasPortal();

        //Skills page
        m_pNoFlyingCheckBox = mPanel->getCheckBox("MapMakerScreen/NoFlying");
        m_pNoFlyingCheckBox->setChecked(false);

        //Music page
        mMusicField = mPanel->getTextField("MapMakerScreen/Music");
        mMusicField->bindString(&mMusic);
        mMusicField->mMaxLength = 32;

        //Terrain page
        mTerrainMapField = mPanel->getTextField("MapMakerScreen/TerrainMap");
        mTerrainMapField->bindString(&mTerrainHeightMap);
        mTerrainMapField->mMaxLength = 32;
        mTerrainAlphaField = mPanel->getTextField("MapMakerScreen/TerrainAlpha");
        mTerrainAlphaField->mMaxLength = 32;
        mTerrainWidthField = mPanel->getTextField("MapMakerScreen/TerrainWidth");
        mTerrainWidthField->mMaxLength = 6;
        mTerrainHeightField = mPanel->getTextField("MapMakerScreen/TerrainHeight");
        mTerrainHeightField->mMaxLength = 6;
        mTerrainBoundaryField = mPanel->getTextField("MapMakerScreen/TerrainBoundary");
        mTerrainBoundaryField->mMaxLength = 6;
        mTerrainTexList = mPanel->getHighlightList("MapMakerScreen/TerrainTexture");
        mTerrainTexListSlider = mPanel->getSlider("MapMakerScreen/TerrainTextureLS");
        mTerrainTexLabel = OverlayManager::getSingleton().getOverlayElement("MapMakerScreen/TerrainTexPartLabel");
        mTerrainTexUp = mPanel->getButton("MapMakerScreen/TerrainTexPartUp");
        mTerrainTexDn = mPanel->getButton("MapMakerScreen/TerrainTexPartDn");

        mTerrainWidthField->setCaption(StringConverter::toString(mTerrainSize.x));
        mTerrainHeightField->setCaption(StringConverter::toString(mTerrainSize.y));
        mTerrainBoundaryField->setCaption(StringConverter::toString(mTerrainBoundary));
        loadAllTextures();

        //Sky page
        mSkyFileLabel = OverlayManager::getSingleton().getOverlayElement("MapMakerScreen/SkyFileLabel");
        mSkyCheckBox = mPanel->getCheckBox("MapMakerScreen/SkyEnable");
        mSkyCheckBox->setChecked(true);
        mSkyField = mPanel->getTextField("MapMakerScreen/SkyFile");
        mSkyField->bindString(&mSkyFile);
        mSkyField->mMaxLength = 32;
        mWeatherField = mPanel->getTextField("MapMakerScreen/WeatherFile");
        mWeatherField->bindString(&mWeatherFile);
        mWeatherField->mMaxLength = 32;
        mCeilingMat = "CeilingMat";
        mCeilingHeightLabel = OverlayManager::getSingleton().getOverlayElement("MapMakerScreen/CeilingHeightLabel");
        mCeilingHeightField = mPanel->getTextField("MapMakerScreen/CeilingHeight");
        mCeilingHeightField->mMaxLength = 5;
        mCeilingHeight = 500;
        mCeilingHeightField->setCaption(StringConverter::toString(mCeilingHeight));
        mCeilingHeightLabel->hide();
        mCeilingHeightField->show(false);

        //Ocean page
        mOceanCheckBox = mPanel->getCheckBox("MapMakerScreen/OceanEnable");
        mOceanCheckBox->setChecked(false);
        mOceanHeightField = mPanel->getTextField("MapMakerScreen/OceanHeight");
        mOceanHeightField->mMaxLength = 6;
        mOceanHeightField->setCaption(StringConverter::toString(mOceanCenter.y));
        mOceanMatField = mPanel->getTextField("MapMakerScreen/OceanMat");
        mOceanMatField->mMaxLength = 32;
        mOceanMatField->bindString(&mOceanMat);

        //Object page
        mObjectSelectList = mPanel->getHighlightList("MapMakerScreen/Object");
        mObjectSelectListSlider = mPanel->getSlider("MapMakerScreen/ObjectLS");
        mObjectGroupList = mPanel->getHighlightList("MapMakerScreen/ObjectGroup");
        mObjectGroupListSlider = mPanel->getSlider("MapMakerScreen/ObjectGroupLS");

        loadAllObjectGroups();

        //Gate page
        mGateMapField = mPanel->getTextField("MapMakerScreen/GateMap");
        mGateMapField->mMaxLength = 32;
        for(int i=0;i<3;i++)
        {
            mGatePosField[i] = mPanel->getTextField("MapMakerScreen/GatePos"+StringConverter::toString(i));
            mGatePosField[i]->mMaxLength = 6;
        }
        mGateYawField = mPanel->getTextField("MapMakerScreen/GateYaw");
        mGateYawField->mMaxLength = 6;
        mGateBlackMatCheckBox = mPanel->getCheckBox("MapMakerScreen/GateBlackMat");

        //Water page
        mWaterField = mPanel->getTextField("MapMakerScreen/WaterMat");
        mWaterField->bindString(&mWaterMat);
        mWaterField->mMaxLength = 32;

        //Controls
        mSelectTypeList = mPanel->getHighlightList("MapMakerScreen/SelectType");
        mSelectTypeList->pushLine("Object");
        mSelectTypeList->pushLine("Gate");
        mSelectTypeList->pushLine("Water");
        mObjectIsLargeLabel = OverlayManager::getSingleton().getOverlayElement("MapMakerScreen/IsLargeLabel");
        mObjectIsLargeCheckBox = mPanel->getCheckBox("MapMakerScreen/IsLarge");
        for(int i=0;i<3;i++)
        {
            mPanel->getButton("MapMakerScreen/PosUp"+StringConverter::toString(i))->setHoldable(true);
            mPanel->getButton("MapMakerScreen/PosDn"+StringConverter::toString(i))->setHoldable(true);
            mPanel->getButton("MapMakerScreen/ScaleUp"+StringConverter::toString(i))->setHoldable(true);
            mPanel->getButton("MapMakerScreen/ScaleDn"+StringConverter::toString(i))->setHoldable(true);
            mPanel->getButton("MapMakerScreen/RotUp"+StringConverter::toString(i))->setHoldable(true);
            mPanel->getButton("MapMakerScreen/RotDn"+StringConverter::toString(i))->setHoldable(true);

            mPosField[i] = mPanel->getTextField("MapMakerScreen/Pos"+StringConverter::toString(i));
            mPosField[i]->mMaxLength = 5;
            mScaleField[i] = mPanel->getTextField("MapMakerScreen/Scale"+StringConverter::toString(i));
            mScaleField[i]->mMaxLength = 5;
        }

        //MiniMap
        mMiniMapPanel = mGui->addPanel("MiniMapScreen");
        mMiniMap = mMiniMapPanel->getMap("MiniMapScreen/Mini");
        mMiniMap->setMaterial("GuiMat/MiniMap");
        mMiniMap->addMarker(0,"GuiMat/MiniMapMarker");
        mMiniMap->setActiveMarker(0);

        //ButtonsScreen
        GuiPanel *panel = mGui->addPanel("MapMakerButtonsScreen",true);
        GuiButton *button = panel->getButton("MapMakerButtonsScreen/MiniMap");
        button->setOwnMaterials("GuiMat/ButtonMiniMapUp","GuiMat/ButtonMiniMapOver","GuiMat/ButtonMiniMapDown");

        if(mWasTesting)
        {
            mWasTesting = false;
            const String mapName = mMapName;
            const String displayName = mDisplayName;
            loadMap("temp");
            mMapField->setCaption(mapName);
            mDisplayField->setCaption(displayName);
        }
        else
        {
            remakeTerrain();
            reset();
        }
    }
    void clear()
    {
        mMapName = "";
        mWasTesting = false;

        mTerrainHeightMap = "defaultTerrain.png";
        mTerrainSize = Vector3(5000,300,5000);
        mTerrainBoundary = 0;
        mTerrainAlphaTex[0] = "defaultMask.png";
        mTerrainAlphaTex[1] = "";
        mTerrainTex[0] = 1;
        for(int i=1;i<6;i++)mTerrainTex[i] = 0;

        mHasSky = true;
        mSkyFile = "";
        mWeatherFile = "";

        mHasOcean= false;
        mOceanCenter = Vector3(5000,50,5000);

        mShowCollisions = false;
    }
    void deleteData()
    {
        while(!mObjectList.empty())
        {
            MapMakerObject *object = mObjectList.back();
            mObjectList.pop_back();
            delete object;
        }
        mObjectIndex = 0;
        mCurrentObject = 0;
        mSelectionHistory.clear();
        mWorldObjectMgr->clear();
    }
    void reset()
    {
        mTerrainTexPart = 0;
        updateTerrainTexPart();

        if(mCurrentObject)mCurrentObject->highlight(false);
        mCurrentObject = 0;
        mSelectionHistory.clear();
        mPosition = Vector3::ZERO;
        mScale = Vector3::UNIT_SCALE;
        updatePosition();
        updateScale();

        mHeldType = HELD_NONE;
        mHeldDirection = false;
        mHeldValue = Vector3::ZERO;
        mSpeedModifier = 1.0f;
        mMoveMode = false;
        mPlaceOnMesh = false;
        mRandomizeClone = false;
        mRandomDefaultScale = Vector3::UNIT_SCALE;
        mSelectObjectType = 0;
        mSelectTypeList->setSelection(mSelectObjectType);
        mCollMgr->setMouseQueryMask(Collision::WORLDOBJECT_MASK);
    }
    void update(const Real &timeElapsed)
    {
        updateObjectAtMousePosition();
        updateMiniMap();

        switch(mHeldType)
        {
            case OBJECT_POSITION:
                mPosition += timeElapsed*50*mHeldValue*(mHeldDirection?1:-1)*mSpeedModifier;
                updatePosition();
                break;
            case OBJECT_SCALE:
                mScale += timeElapsed*1*mHeldValue*(mHeldDirection?1:-1)*mSpeedModifier;
                updateScale();
                break;
            case OBJECT_ROTATION: updateRotation(timeElapsed*90*mHeldValue*(mHeldDirection?1:-1)*mSpeedModifier); break;
            default: break;
        }

        mGui->getPanelByName("DebugScreen")->getComponentByName("DebugScreen/Debug")->setCaption(StringConverter::toString(mCursorPosition)+", Yaw:"+StringConverter::toString(mCameraMgr->getYaw()));
    }
    void addNewObject(const String &objectName)
    {
        MapMakerObject *object = new MapMakerObject();
        object->mObjectName = objectName;
        object->mNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();

        WorldObject *worldObject = mWorldObjectMgr->getWorldObject(object->mObjectName);
        if(!worldObject)
        {
            LoggerManager::getSingleton().logMessage("ERROR::MapMaker: unable to load "+object->mObjectName);
            return;
        }

        unsigned short counter = 0;
        //Mesh
        for(std::vector<WorldSubMesh*>::iterator it=worldObject->mSubMeshList.begin(); it!=worldObject->mSubMeshList.end(); it++)
        {
            WorldSubMesh *subMesh = *it;
            Entity *ent = mSceneMgr->createEntity("MapMakerEnt_"+StringConverter::toString(mObjectIndex)+"_"+StringConverter::toString(counter++),subMesh->mMesh);
            if(!subMesh->mMaterial.empty())
            {
                unsigned short subEntIndex = 0;
                for(std::vector<String>::iterator k=subMesh->mMaterial.begin(); k!=subMesh->mMaterial.end()&&subEntIndex<ent->getNumSubEntities(); k++)
                {
                    const String matName = *k;
                    ent->getSubEntity(subEntIndex)->setMaterialName(matName);
                    subEntIndex++;
                }
            }
            SceneNode *node = object->mNode->createChildSceneNode();
            node->attachObject(ent);
            node->setPosition(subMesh->mOffset);
            node->setScale(subMesh->mScale);
            node->setOrientation(subMesh->mOrientation);
            ent->setQueryFlags(Collision::WORLDOBJECT_MASK);
            if(subMesh->mIsCollidable)ent->addQueryFlags(Collision::MESH_COLL_MASK);
        }
        //Collision
        counter = 0;
        for(std::vector<Collision*>::iterator it=worldObject->mCollList.begin(); it!=worldObject->mCollList.end(); it++)
        {
            Collision *coll = *it;
            const String meshType = (coll->isBox() ? "CollBox" : "CollSphere");
            Entity *ent = mSceneMgr->createEntity("MapMakerColl_"+StringConverter::toString(mObjectIndex)+"_"+StringConverter::toString(counter++),meshType);
            SceneNode *node = object->mNode->createChildSceneNode();
            node->setInheritOrientation(false);
            if(coll->isSphere())
            {
                node->setInheritScale(false);
                object->mSphereNodes.push_back(pair<SceneNode*,Real>(node,coll->getSize().y));
            }
            node->attachObject(ent);
            node->setPosition(coll->getCenter());
            node->setScale(coll->getSize());
            ent->setMaterialName(coll->isWater()?"Collision/HighlightBlue":"Collision/Highlight");
            ent->setQueryFlags(Collision::WORLDOBJECT_MASK);
        }
        //Billboard
        counter = 0;
        for(std::vector<WorldBillboard*>::iterator it=worldObject->mBillboardList.begin(); it!=worldObject->mBillboardList.end(); it++)
        {
            WorldBillboard *billboardInfo = *it;
            BillboardSet *set = mSceneMgr->createBillboardSet("MapMakerBB_"+StringConverter::toString(mObjectIndex)+"_"+StringConverter::toString(counter++),1);
            set->setMaterialName(billboardInfo->mMaterial);
            set->setDefaultDimensions(billboardInfo->mSize.x,billboardInfo->mSize.y);
            set->createBillboard(0,0,0);
            SceneNode *node = object->mNode->createChildSceneNode();
            node->attachObject(set);
            node->setPosition(billboardInfo->mOffset);

            Entity *selectEnt = mSceneMgr->createEntity("MapMakerBBBox_"+StringConverter::toString(mObjectIndex)+"_"+StringConverter::toString(counter-1),"CollSphere");
            node = object->mNode->createChildSceneNode();
            node->attachObject(selectEnt);
            node->setPosition(billboardInfo->mOffset);
            node->setScale(0.5f*Vector3(billboardInfo->mSize.x,billboardInfo->mSize.y,billboardInfo->mSize.x));
            selectEnt->setMaterialName("GuiMat/Blank");
            selectEnt->setQueryFlags(Collision::WORLDOBJECT_MASK);
        }
        //Particle
        counter = 0;
        for(std::vector<WorldParticle*>::iterator it=worldObject->mParticleList.begin(); it!=worldObject->mParticleList.end(); it++)
        {
            WorldParticle *particleInfo = *it;
            if(!ParticleSystemManager::getSingletonPtr()->getTemplate(particleInfo->mParticle))
            {
                LoggerManager::getSingleton().logMessage("MapMaker::addNewObject: ParticleSystem not found: " + particleInfo->mParticle);
                continue;
            }
            ParticleSystem *ps = mSceneMgr->createParticleSystem("MapMakerPS_"+StringConverter::toString(mObjectIndex)+"_"+StringConverter::toString(counter++),particleInfo->mParticle);
            SceneNode *node = object->mNode->createChildSceneNode();
            node->attachObject(ps);
            node->setPosition(particleInfo->mOffset);
            node->setScale(particleInfo->mScale);
            node->setOrientation(particleInfo->mOrientation);

            Entity *selectEnt = mSceneMgr->createEntity("MapMakerPSBox_"+StringConverter::toString(mObjectIndex)+"_"+StringConverter::toString(counter-1),"CollSphere");
            node = object->mNode->createChildSceneNode();
            node->attachObject(selectEnt);
            node->setPosition(particleInfo->mOffset);
            node->setScale(0.5f*OBJECTMAKER_COLL_DEFAULT_SIZE*particleInfo->mScale);
            node->setOrientation(particleInfo->mOrientation);
            selectEnt->setMaterialName("GuiMat/Blank");
            selectEnt->setQueryFlags(Collision::WORLDOBJECT_MASK);
        }
        //Light
        counter = 0;
        for(std::vector<WorldLight*>::iterator it=worldObject->mLightList.begin(); it!=worldObject->mLightList.end(); it++)
        {
            WorldLight *lightInfo = *it;
            Entity *selectEnt = mSceneMgr->createEntity("MapMakerLTBox_"+StringConverter::toString(mObjectIndex)+"_"+StringConverter::toString(counter++),"CollSphere");
            SceneNode *node = object->mNode->createChildSceneNode();
            node->attachObject(selectEnt);
            node->setPosition(lightInfo->mOffset);
            node->setScale(0.5f*OBJECTMAKER_COLL_DEFAULT_SIZE*Vector3::UNIT_SCALE);
            selectEnt->setMaterialName("Collision/Highlight");
            selectEnt->setQueryFlags(Collision::WORLDOBJECT_MASK);
        }
        mObjectIndex++;

        if(mCurrentObject)mCurrentObject->highlight(false);
        mCurrentObject = object;
        mCurrentObject->highlight(true);
        mObjectList.push_back(object);
        mSelectionHistory.clear();
        mSelectionHistory.push_back(object);
        mSelectObjectType = 0;
        mSelectTypeList->setSelection(mSelectObjectType);
        mCollMgr->setMouseQueryMask(Collision::WORLDOBJECT_MASK);
        if(mRandomizeClone)mRandomDefaultScale = mScale;

        mScale = Vector3::UNIT_SCALE;
        updateScale();
        mObjectIsLargeCheckBox->setChecked(false);
        mMoveMode = true;
    }
    void addNewGate()
    {
        const String mapName = mGateMapField->getCaption();
        if(mapName=="")
        {
            mGui->showAlertBox("Destination Map cannot be blank!");
            return;
        }
        MapMakerObject *gate = new MapMakerObject();
        gate->mObjectName = mapName;
        gate->mNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        gate->mIsGate = true;
        gate->mIsGateBlackMat = mGateBlackMatCheckBox->isChecked();

        Entity *ent = mSceneMgr->createEntity("MapMakerGate_"+StringConverter::toString(mObjectIndex),"FlatCircle");
        gate->mNode->attachObject(ent);
        ent->setMaterialName(gate->mIsGateBlackMat?"GateMat/Black":"GateMat/Gate");
        ent->setQueryFlags(Collision::WORLDGATE_MASK);
        mObjectIndex++;

        gate->mGatePosition = Vector3(StringConverter::parseReal(mGatePosField[0]->getCaption()),StringConverter::parseReal(mGatePosField[1]->getCaption()),StringConverter::parseReal(mGatePosField[2]->getCaption()));
        gate->mGateYaw = StringConverter::parseReal(mGateYawField->getCaption());


        if(mCurrentObject)mCurrentObject->highlight(false);
        mCurrentObject = gate;
        mCurrentObject->highlight(true);
        mObjectList.push_back(gate);
        mSelectionHistory.clear();
        mSelectionHistory.push_back(gate);
        mSelectObjectType = 1;
        mSelectTypeList->setSelection(mSelectObjectType);
        mCollMgr->setMouseQueryMask(Collision::WORLDGATE_MASK);

        mScale = Vector3(1,30,1);
        updateScale();
        updateGateFields();
        mMoveMode = true;
    }
    void addNewWater()
    {
        MapMakerObject *water = new MapMakerObject();
        water->mObjectName = mWaterMat;
        water->mNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        water->mIsWater = true;

        Entity *ent = mSceneMgr->createEntity("MapMakerWater_"+StringConverter::toString(mObjectIndex),"WaterPlaneMesh");
        water->mNode->attachObject(ent);
        ent->setMaterialName(mWaterMat==""?WATER_DEFAULTMAT:mWaterMat);
        ent->setQueryFlags(Collision::WORLDWATER_MASK);
        mObjectIndex++;

        if(mCurrentObject)mCurrentObject->highlight(false);
        mCurrentObject = water;
        mCurrentObject->highlight(true);
        mObjectList.push_back(water);
        mSelectionHistory.clear();
        mSelectionHistory.push_back(water);
        mSelectObjectType = 2;
        mSelectTypeList->setSelection(mSelectObjectType);
        mCollMgr->setMouseQueryMask(Collision::WORLDWATER_MASK);

        mScale = Vector3(WATER_SIZE,0,WATER_SIZE);
        updateScale();
        mMoveMode = true;
    }
    void updatePosition()
    {
        for(int i=0;i<3;i++)
        {
            const Real pos = i==0?mPosition.x:(i==1?mPosition.y:mPosition.z);
            mPosField[i]->setCaption(StringConverter::toString(pos,Math::Abs(pos)<1000?4:6));
        }
        if(!mCurrentObject)return;
        mCurrentObject->mNode->setPosition(mPosition);
    }
    void updateScale()
    {
        for(int i=0;i<3;i++)
        {
            const Real scale = i==0?mScale.x:(i==1?mScale.y:mScale.z);
            mScaleField[i]->setCaption(StringConverter::toString(scale,Math::Abs(scale)<1000?4:6));
        }
        if(!mCurrentObject)return;
        mCurrentObject->setScale(mScale);
    }
    void updateRotation(const Vector3 &pyr)
    {
        if(!mCurrentObject || mCurrentObject->mIsWater)return;
        if(pyr.x!=0)mCurrentObject->mNode->pitch(Degree(pyr.x),Node::TS_WORLD);
        if(pyr.y!=0)mCurrentObject->mNode->yaw(Degree(pyr.y),Node::TS_WORLD);
        if(pyr.z!=0)mCurrentObject->mNode->roll(Degree(pyr.z),Node::TS_WORLD);
    }
    void updateTerrainTexPart()
    {
        mTerrainTexLabel->setCaption(StringConverter::toString(mTerrainTexPart+1));
        mTerrainTexList->setSelection(mTerrainTex[mTerrainTexPart]);
        mTerrainTexListSlider->updateSliderToListSelection();
        mTerrainTexDn->show(mTerrainTexPart>0);
        mTerrainTexUp->show(mTerrainTexPart<5);
        mTerrainAlphaField->bindString(&mTerrainAlphaTex[(mTerrainTexPart<3?0:1)]);
    }
    void updateTerrainTexture()
    {
        saveTerrainMaterialFile("temp");
        MaterialManager::getSingletonPtr()->remove("Terrain/temp");
        DataStreamPtr matPtr = ResourceGroupManager::getSingletonPtr()->openResource("tempTerrain.material","Terrains/temp");
        MaterialManager::getSingletonPtr()->parseScript(matPtr,"Terrains/temp");
    }
    void remakeTerrain()
    {
        updateTerrainTexture();
        saveTerrainFile("temp");
        if(validateTerrainFile("temp/tempTerrain.cfg"))mSceneMgr->setWorldGeometry("tempTerrain.cfg");
        else
        {
            mGui->showAlertBox("Failed to load terrain!");
            mSceneMgr->setWorldGeometry("defaultTerrain.cfg");
        }
        updateSky();
        updateOcean();
        updateMiniMapTexture();
    }
    void updateSky()
    {
        mHasSky = mSkyCheckBox->isChecked();
        mSkyMgr->showSky(mHasSky);
        mSkyFileLabel->setCaption(mHasSky?"Sky File:":"Ceiling Material:");
        if(!mHasSky)
        {
            mSkyField->bindString(&mCeilingMat);
            mCeilingHeightLabel->show();
            mCeilingHeightField->show(true);
            mCeilingHeight = StringConverter::parseReal(mCeilingHeightField->getCaption());
            mCeilingHeightField->setCaption(StringConverter::toString(mCeilingHeight));
            mSkyMgr->createCeiling(Vector3(mTerrainSize.x,mCeilingHeight,mTerrainSize.z),mCeilingMat);
        }
        else
        {
            mSkyField->bindString(&mSkyFile);
            mSkyMgr->destroyCeiling();
            mCeilingHeightLabel->hide();
            mCeilingHeightField->show(false);
        }
    }
    void updateOcean()
    {
        if(mOceanCheckBox->isChecked())
        {
            mHasOcean = true;
            mOceanCenter = Vector3(mTerrainSize.x*0.5f,mOceanCenter.y,mTerrainSize.z*0.5f);
            mOceanSize = Vector2(mTerrainSize.x*1.2f,mTerrainSize.z*1.2f);
            mWaterMgr->createOcean(mOceanCenter,mOceanSize,mOceanMat);
        }
        else
        {
            mHasOcean = false;
            mWaterMgr->destroyOcean();
        }
    }
    void updateGateFields()
    {
        if(!mCurrentObject || !mCurrentObject->mIsGate)return;
        mGateMapField->setCaption(mCurrentObject->mObjectName);
        mGatePosField[0]->setCaption(StringConverter::toString(mCurrentObject->mGatePosition.x));
        mGatePosField[1]->setCaption(StringConverter::toString(mCurrentObject->mGatePosition.y));
        mGatePosField[2]->setCaption(StringConverter::toString(mCurrentObject->mGatePosition.z));
        mGateYawField->setCaption(StringConverter::toString(mCurrentObject->mGateYaw));
        mGateBlackMatCheckBox->setChecked(mCurrentObject->mIsGateBlackMat);
    }
    void updateHasPortal()
    {
        const bool hasPortal = mHasPortalCheckBox->isChecked();
        if(hasPortal)
        {
            mPortalLabel->show();
            mPortalYawLabel->show();
        }
        else
        {
            mPortalLabel->hide();
            mPortalYawLabel->hide();
        }
        for(int i=0;i<3;i++)
        {
            mPortalPosField[i]->show(hasPortal);
            if(hasPortal)mPortalPosLabel[i]->show();
            else mPortalPosLabel[i]->hide();
        }
        mPortalYawField->show(hasPortal);
    }
    void loadAllTextures()
    {
        mTerrainTexList->clear();
        mTerrainTexList->pushLine("None");
        const StringVector list = SaveFile::findResourceNames("TerrainTextures","*.jpg");
        for(StringVector::const_iterator i = list.begin(); i!=list.end(); i++)
        {
            String line = *i;
            mTerrainTexList->pushLine(line);
        }
        mTerrainTexList->update(0);
        mTerrainTexListSlider->setValue(0);
    }
    void loadAllMaps(const bool &addResourceGroups=false)
    {
        mMapList->clear();
        const StringVector list = SaveFile::findResourceNames("Terrains","*.map");
        for(StringVector::const_iterator i = list.begin(); i!=list.end(); i++)
        {
            String line = *i;
            line.erase((int)line.length()-4,4);
            if(line!=MAPMAKER_TEMP)mMapList->pushLine(line);
        }
        mMapList->update(0);
        mMapListSlider->setValue(0);
    }
    void loadAllObjectGroups()
    {
        mObjectGroupList->clear();
        const std::vector<String> list = SaveFile::findResourceNames("*.object");
        for(std::vector<String>::const_iterator i = list.begin(); i!=list.end(); i++)
        {
            String line = *i;
            line.erase((int)line.length()-7,7);
            if(line!=OBJECTMAKER_TEMP_GROUP)mObjectGroupList->pushLine(line);
        }
        mObjectGroupList->update(0);
        mObjectGroupListSlider->setValue(0);
    }
    void loadObjectGroup()
    {
        const String groupName = mObjectGroupList->getSelection();
        SaveFile sf;
        if(!sf.loadAnywhere(groupName+".object"))
        {
            mGui->showAlertBox("Cannot find Object Group "+groupName+"!");
            return;
        }
        mObjectSelectList->clear();
        mWorldObjectMgr->clearObjects();
        for(sf.beginSectionIterator(); sf.hasMoreSections(); sf.nextSection())
        {
            mObjectSelectList->pushLine(sf.peekNextSectionName());
        }
        mObjectSelectList->update(0);
        mObjectSelectListSlider->setValue(0);
        mWorldObjectMgr->loadObjects(groupName,false);
    }
    void buttonClicked(GuiComponent *button)
    {
        if(button==mHasPortalCheckBox)updateHasPortal();
        else if(button==mObjectIsLargeCheckBox)
        {
            if(mCurrentObject)mCurrentObject->mIsLarge = mObjectIsLargeCheckBox->isChecked();
        }
        else if(button==mGateBlackMatCheckBox)
        {
            if(mCurrentObject && mCurrentObject->mIsGate)
            {
                mCurrentObject->mIsGateBlackMat = mGateBlackMatCheckBox->isChecked();
                if(mCurrentObject->mNode->numAttachedObjects()>0)
                    static_cast<Entity*>(mCurrentObject->mNode->getAttachedObject(0))->setMaterialName(mCurrentObject->mIsGateBlackMat?"GateMat/Black":"GateMat/Gate");
            }
        }
        else if(button->nameIs("MapMakerScreen/Save"))doSaveMap();
        else if(button->nameIs("MapMakerScreen/Export"))doSaveMap(false,true);
        else if(button->nameIs("MapMakerScreen/ShowColl"))mShowCollisions = static_cast<GuiCheckBox*>(button)->isChecked();
        else if(button->nameIs("MapMakerScreen/Load"))doLoadMap();
        else if(button==mTerrainTexUp)
        {
            mTerrainTexPart++;
            updateTerrainTexPart();
        }
        else if(button==mTerrainTexDn)
        {
            mTerrainTexPart--;
            updateTerrainTexPart();
        }
        else if(button->nameIs("MapMakerScreen/SkyEnable"))updateSky();
        else if(button->nameIs("MapMakerScreen/OceanEnable"))updateOcean();
        else if(button->nameIs("MapMakerScreen/ObjectNew"))
        {
            const String groupName = mObjectGroupList->getSelection();
            const String objectName = mObjectSelectList->getSelection();
            if(groupName!=""&&objectName!="")addNewObject(groupName + "/" + objectName);
        }
        else if(button->nameIs("MapMakerScreen/GateNew"))addNewGate();
        else if(button->nameIs("MapMakerScreen/WaterNew"))addNewWater();
        else if(button->nameIs("MapMakerScreen/Move"))mMoveMode = (!mMoveMode && mCurrentObject);
        else if(button->nameIs("MapMakerScreen/Clone"))cloneObject();
        else if(button->nameIs("MapMakerScreen/RotReset"))
        {
            if(mCurrentObject)mCurrentObject->mNode->resetOrientation();
        }
        else if(button->nameIs("MapMakerScreen/Update"))
        {
            updatePosition();
            updateScale();
        }
        else if(button->nameIs("MapMakerScreen/Hide"))
        {
            if(mCurrentObject)
            {
                mCurrentObject->mNode->setVisible(false);
                mCurrentObject->highlight(false);
                mCurrentObject = 0;
                mSelectionHistory.clear();
            }
        }
        else if(button->nameIs("MapMakerScreen/ShowAll"))
        {
            for(std::vector<MapMakerObject*>::iterator it=mObjectList.begin(); it!=mObjectList.end(); it++)
            {
                MapMakerObject *object = *it;
                object->mNode->setVisible(true);
            }
        }
        else if(button->nameIs("MapMakerScreen/Delete"))deleteObject();
        else if(button->nameIs("MapMakerScreen/PlaceOnMesh"))mPlaceOnMesh = static_cast<GuiCheckBox*>(button)->isChecked();
        else if(button->nameIs("MapMakerScreen/RandomizeClone"))
        {
            mRandomizeClone = static_cast<GuiCheckBox*>(button)->isChecked();
            if(mRandomizeClone)mRandomDefaultScale = mScale;
        }
        else if(button->nameIs("MapMakerButtonsScreen/MiniMap"))mMiniMapPanel->toggleVisibility();
    }
    void buttonHeld(GuiComponent *button)
    {
        unsigned short index = 0;
        bool held = false;
        if(button->nameIsIndex("MapMakerScreen/PosUp",&index))
        {
            mHeldDirection = true;
            mHeldType = OBJECT_POSITION;
            held = true;
        }
        else if(button->nameIsIndex("MapMakerScreen/PosDn",&index))
        {
            mHeldDirection = false;
            mHeldType = OBJECT_POSITION;
            held = true;
        }
        else if(button->nameIsIndex("MapMakerScreen/ScaleUp",&index))
        {
            mHeldDirection = true;
            mHeldType = OBJECT_SCALE;
            held = true;
        }
        else if(button->nameIsIndex("MapMakerScreen/ScaleDn",&index))
        {
            mHeldDirection = false;
            mHeldType = OBJECT_SCALE;
            held = true;
        }
        else if(button->nameIsIndex("MapMakerScreen/RotUp",&index))
        {
            mHeldDirection = true;
            mHeldType = OBJECT_ROTATION;
            held = true;
        }
        else if(button->nameIsIndex("MapMakerScreen/RotDn",&index))
        {
            mHeldDirection = false;
            mHeldType = OBJECT_ROTATION;
            held = true;
        }

        if(held)mHeldValue = Vector3(index==0?1:0,index==1?1:0,index==2?1:0);
    }
    void buttonReleased(GuiComponent *button)
    {
        mHeldType = HELD_NONE;
        mHeldValue = Vector3::ZERO;
    }
    void doMousePicking()
    {
        std::vector<SceneNode*> resultList;
        mCollMgr->doMousePicking(mCameraMgr->getCamera(),mGui->getCursorX(),mGui->getCursorY(),&resultList);
        //Nothing selected
        if(resultList.empty() && mCurrentObject)
        {
            mCurrentObject->highlight(false);
            mCurrentObject = 0;
            mSelectionHistory.clear();
        }
        //Remove redundant history
        for(std::vector<MapMakerObject*>::iterator it=mSelectionHistory.begin(); it!=mSelectionHistory.end(); it++)
        {
            MapMakerObject *object = *it;
            bool isFound = false;
            for(std::vector<SceneNode*>::iterator jt=resultList.begin(); jt!=resultList.end(); jt++)
            {
                SceneNode *node = *jt;
                if(object->isContainingNode(node))
                {
                    isFound = true;
                    break;
                }
            }
            if(!isFound)
            {
                it = mSelectionHistory.erase(it);
                if(it==mSelectionHistory.end())break;
            }
        }

        for(std::vector<SceneNode*>::iterator it=resultList.begin(); it!=resultList.end(); it++)
        {
            SceneNode *node = *it;
            bool isInHistory = false;
            for(std::vector<MapMakerObject*>::iterator jt=mSelectionHistory.begin(); jt!=mSelectionHistory.end(); jt++)
            {
                MapMakerObject *object = *jt;
                if(object->isContainingNode(node))
                {
                    isInHistory = true;
                    break;
                }
            }
            //New object selected
            if(!isInHistory)
            {
                for(std::vector<MapMakerObject*>::iterator jt=mObjectList.begin(); jt!=mObjectList.end(); jt++)
                {
                    MapMakerObject *object = *jt;
                    if(object->isContainingNode(node))
                    {
                        selectObject(object);
                        mSelectionHistory.push_back(object);
                        return;
                    }
                }
                return;
            }
        }
        //Loop history selection
        if(!mSelectionHistory.empty())
        {
            selectObject(mSelectionHistory.front());
            mSelectionHistory.erase(mSelectionHistory.begin());
            mSelectionHistory.push_back(mCurrentObject);
        }
    }
    void updateObjectAtMousePosition()
    {
        std::vector<Entity*> meshCollList;
        //Exclude mesh collision of active object
        if(mPlaceOnMesh && mMoveMode && mCurrentObject)
        {
            std::vector<Entity*> entList;
            mCurrentObject->getEntityList(&entList);
            for(std::vector<Entity*>::iterator it=entList.begin(); it!=entList.end(); it++)
            {
                Entity *ent = *it;
                if((ent->getQueryFlags()&Collision::MESH_COLL_MASK) != 0)
                {
                    ent->removeQueryFlags(Collision::MESH_COLL_MASK);
                    meshCollList.push_back(ent);
                }
            }
        }

        const pair<bool,Vector3> result = mCollMgr->doMousePicking(mCameraMgr->getCamera(),mGui->getCursorX(),mGui->getCursorY(),0,mPlaceOnMesh,false);

        if(mPlaceOnMesh && mMoveMode && mCurrentObject)
        {
            for(std::vector<Entity*>::iterator it=meshCollList.begin(); it!=meshCollList.end(); it++)
            {
                Entity *ent = *it;
                ent->addQueryFlags(Collision::MESH_COLL_MASK);
            }
        }

        if(result.first)
        {
            if(mMoveMode && mCurrentObject)
            {
                mPosition = result.second;
                if(mCurrentObject->mIsWater)mPosition += Vector3(0,50,0);
                if(mCurrentObject->mIsGate)mPosition += Vector3(0,30,0);
                updatePosition();
            }
            mCursorPosition = result.second;
        }
    }
    void mouseLeftDown()
    {
        if(mMoveMode)
        {
            mMoveMode = false;
            return;
        }
        else doMousePicking();
    }
    void changeEvent(GuiComponent *component, const unsigned char &event)
    {
        if(component->isList())
        {
            if(component==mTerrainTexList)
            {
                mTerrainTex[mTerrainTexPart] = mTerrainTexList->getSelectedLine();
                remakeTerrain();
            }
            else if(component==mObjectGroupList)loadObjectGroup();
            else if(component==mSelectTypeList)
            {
                if(mSelectObjectType != mSelectTypeList->getSelectedLine())
                {
                    mSelectObjectType = mSelectTypeList->getSelectedLine();
                    if(mSelectObjectType==0)mCollMgr->setMouseQueryMask(Collision::WORLDOBJECT_MASK);
                    else if(mSelectObjectType==1)mCollMgr->setMouseQueryMask(Collision::WORLDGATE_MASK);
                    else mCollMgr->setMouseQueryMask(Collision::WORLDWATER_MASK);
                    if(mCurrentObject)
                    {
                        mCurrentObject->highlight(false);
                        mCurrentObject = 0;
                    }
                    mSelectionHistory.clear();

                    if(mSelectObjectType==0)
                    {
                        mObjectIsLargeLabel->show();
                        mObjectIsLargeCheckBox->show(true);
                    }
                    else
                    {
                        mObjectIsLargeLabel->hide();
                        mObjectIsLargeCheckBox->show(false);
                    }
                }
            }
        }
        else if(component==mWaterField)
        {
            if(mCurrentObject && mCurrentObject->mIsWater)mCurrentObject->mObjectName = mWaterMat;
        }
        else if(component==mMiniMap)updateCameraToMiniMap();
        else if(component->isTextComponent() && static_cast<GuiTextComponent*>(component)->isEditable())
        {
            GuiTextField *text = static_cast<GuiTextField*>(component);
            unsigned short index = 0;
            if(text==mTerrainMapField)
            {
                if(SaveFile::resourceExists(mTerrainHeightMap))remakeTerrain();
                else
                {
                    mGui->showAlertBox("Cannot find "+mTerrainHeightMap+"!");
                    mTerrainHeightMap = "defaultTerrain.png";
                }
            }
            else if(text==mTerrainAlphaField)remakeTerrain();
            else if(text==mTerrainWidthField)
            {
                mTerrainSize.x = mTerrainSize.z = StringConverter::parseReal(mTerrainWidthField->getCaption());
                if(mTerrainSize.x==0)
                {
                    mTerrainSize.x = mTerrainSize.z = 1;
                    mTerrainWidthField->setCaption("1");
                }
                remakeTerrain();
            }
            else if(text==mTerrainHeightField)
            {
                mTerrainSize.y = StringConverter::parseReal(mTerrainHeightField->getCaption());
                remakeTerrain();
            }
            else if(text==mTerrainBoundaryField)
            {
                mTerrainBoundary = StringConverter::parseReal(mTerrainBoundaryField->getCaption());
            }
            else if(text==mSkyField)
            {
                if(mHasSky)
                {
                    SaveFile sf;
                    if(!sf.loadAnywhere(mSkyFile+".sky"))mGui->showAlertBox("Cannot find "+mSkyFile+"!");
                }
                updateSky();
            }
            else if(text==mWeatherField)
            {
                SaveFile sf;
                if(!sf.loadAnywhere(mWeatherFile+".cycle"))mGui->showAlertBox("Cannot find "+mWeatherFile+"!");
            }
            else if(text==mCeilingHeightField)updateSky();
            else if(text==mOceanHeightField)
            {
                mOceanCenter.y = StringConverter::parseReal(mOceanHeightField->getCaption());
                updateOcean();
            }
            else if(text==mOceanMatField)
            {
                updateOcean();
            }
            else if(text->nameIsIndex("MapMakerScreen/Pos",&index))
            {
                switch(index)
                {
                    case 0: mPosition.x = StringConverter::parseReal(text->getCaption()); break;
                    case 1: mPosition.y = StringConverter::parseReal(text->getCaption()); break;
                    case 2: mPosition.z = StringConverter::parseReal(text->getCaption()); break;
                    default: break;
                }
                if(mCurrentObject)updatePosition();
            }
            else if(text->nameIsIndex("MapMakerScreen/Scale",&index))
            {
                switch(index)
                {
                    case 0: mScale.x = StringConverter::parseReal(text->getCaption()); break;
                    case 1: mScale.y = StringConverter::parseReal(text->getCaption()); break;
                    case 2: mScale.z = StringConverter::parseReal(text->getCaption()); break;
                    default: break;
                }
                if(mCurrentObject)updateScale();
            }
            else if(text->nameIsIndex("MapMakerScreen/GatePos",&index))
            {
                if(mCurrentObject && mCurrentObject->mIsGate)
                {
                    Vector3 position = mCurrentObject->mGatePosition;
                    switch(index)
                    {
                        case 0: position.x = StringConverter::parseReal(text->getCaption()); break;
                        case 1: position.y = StringConverter::parseReal(text->getCaption()); break;
                        case 2: position.z = StringConverter::parseReal(text->getCaption()); break;
                        default: break;
                    }
                    mCurrentObject->mGatePosition = position;
                    updateGateFields();
                }
            }
            else if(text==mGateMapField)
            {
                if(mCurrentObject && mCurrentObject->mIsGate)
                {
                    const String mapName = mGateMapField->getCaption();
                    if(mapName=="")mGui->showAlertBox("Destination Map cannot be blank!");
                    else mCurrentObject->mObjectName = mapName;
                }
            }
            else if(text==mGateYawField)
            {
                if(mCurrentObject && mCurrentObject->mIsGate)
                {
                    mCurrentObject->mGateYaw = StringConverter::parseReal(mGateYawField->getCaption());
                    updateGateFields();
                }
            }
        }
    }
    void keyPressed(const OIS::KeyEvent &arg)
    {
        using namespace OIS;
        switch(arg.key)
        {
            case KC_LCONTROL:
            case KC_RCONTROL:
                mSpeedModifier = 0.04f;
                break;
            case KC_LSHIFT:
            case KC_RSHIFT:
                mSpeedModifier = 0.2f;
                break;
            case KC_LMENU:
            case KC_RMENU:
                mSpeedModifier = 4.0f;
                break;
            case KC_M: mMoveMode = (!mMoveMode && mCurrentObject); break;
            case KC_C: cloneObject(); break;
            case KC_DELETE: deleteObject(); break;
            case KC_X:
                if(mRandomizeClone)
                {
                    mScale = mRandomDefaultScale;
                    updateScale();
                }
                break;
            default: break;
        }
    }
    void keyReleased(const OIS::KeyEvent &arg)
    {
        using namespace OIS;
        switch(arg.key)
        {
            case KC_LCONTROL:
            case KC_RCONTROL:
            case KC_LSHIFT:
            case KC_RSHIFT:
            case KC_LMENU:
            case KC_RMENU:
                mSpeedModifier = 1.0f;
            default: break;
        }
    }
    void selectObject(MapMakerObject* object)
    {
        if(mCurrentObject)mCurrentObject->highlight(false);
        mCurrentObject = 0;
        mPosition = object->mNode->getPosition();
        mScale = object->getScale();
        updatePosition();
        updateScale();
        mCurrentObject = object;
        mCurrentObject->highlight(true);
        if(mRandomizeClone)mRandomDefaultScale = mScale;

        if(mCurrentObject->mIsGate)updateGateFields();
        else if(mCurrentObject->mIsWater)mWaterField->setCaption(mCurrentObject->mObjectName);

        if(!mCurrentObject->mIsGate && !mCurrentObject->mIsWater)
        {
            mObjectIsLargeLabel->show();
            mObjectIsLargeCheckBox->show(true);
            mObjectIsLargeCheckBox->setChecked(mCurrentObject->mIsLarge);
        }
        else
        {
            mObjectIsLargeLabel->hide();
            mObjectIsLargeCheckBox->show(false);
        }
    }
    void deleteObject()
    {
        if(!mCurrentObject)return;

        for(std::vector<MapMakerObject*>::iterator i=mObjectList.begin(); i!=mObjectList.end(); i++)
        {
            MapMakerObject *object = *i;
            if(mCurrentObject==object)
            {
                mObjectList.erase(i);
                break;
            }
        }
        mCurrentObject->highlight(false);
        delete mCurrentObject;
        mCurrentObject = 0;
        mSelectionHistory.clear();
    }
    void cloneObject()
    {
        if(!mCurrentObject)return;
        const Vector3 scale = mCurrentObject->getScale();
        const Quaternion orientation = mCurrentObject->mNode->getOrientation();
        const bool isLarge = mCurrentObject->mIsLarge;

        if(mCurrentObject->mIsWater)
        {
            mWaterField->setCaption(mCurrentObject->mObjectName);
            addNewWater();
        }
        else if(mCurrentObject->mIsGate)
        {
            updateGateFields();
            addNewGate();
        }
        else
        {
            const Vector3 randomDefaultScale = mRandomDefaultScale;
            addNewObject(mCurrentObject->mObjectName);
            if(mRandomizeClone)mRandomDefaultScale = randomDefaultScale;
            mCurrentObject->mIsLarge = isLarge;
            mObjectIsLargeCheckBox->setChecked(isLarge);
        }

        mScale = scale;
        if(mRandomizeClone)mScale *= Vector3(Math::RangeRandom(0.9f,1.1f),Math::RangeRandom(0.9f,1.1f),Math::RangeRandom(0.9f,1.1f));
        updateScale();
        if(!mCurrentObject->mIsWater)mCurrentObject->mNode->setOrientation(orientation);
        if(mRandomizeClone)mCurrentObject->mNode->yaw(Degree(Math::RangeRandom(0.1f,359.9f)),Node::TS_WORLD);
    }
    void alertBoxEvent(const String &name, const bool &flag)
    {
        if(!flag)return;

        if(name=="MapMaker/Overwrite")doSaveMap(true);
        else if(name=="MapMaker/OverwriteExport")doSaveMap(true,true);
        else if(name=="MapMaker/Load")doLoadMap(true);
    }
    void doLoadMap(const bool &load=false)
    {
        const String mapName = mMapList->getSelection();
        if(mapName=="")
        {
            mGui->showAlertBox("No map to load!");
            return;
        }
        if(!load)
        {
            mGui->showAlertBox("Load Map "+mapName+"?","MapMaker/Load",true);
            return;
        }
        loadMap(mapName);
    }
    void loadMap(const String &mapName)
    {
        //Load Terrain
        if(!validateTerrainFile(mapName+"/"+mapName+"Terrain.cfg"))
        {
            mGui->showAlertBox("Unable to load Terrain.cfg file for "+mapName+"!");
            return;
        }
        String buffer;
        SaveFile terrainFile(MAPMAKER_DIR+mapName+"/"+mapName+"Terrain.cfg");
        buffer = "";
        terrainFile.getSetting("Heightmap.image",buffer);
        mTerrainMapField->setCaption(buffer);
        buffer = "";
        terrainFile.getSetting("PageWorldX",buffer);
        mTerrainSize.x = mTerrainSize.z = StringConverter::parseReal(buffer);
        mTerrainWidthField->setCaption(buffer);
        buffer = "";
        terrainFile.getSetting("MaxHeight",buffer);
        mTerrainSize.y = StringConverter::parseReal(buffer);
        mTerrainHeightField->setCaption(buffer);

        //Load Terrain Material
        SaveFile terrainMatFile(MAPMAKER_DIR+mapName+"/"+mapName+"Material.cfg");
        for(int i=0;i<2;i++)terrainMatFile.getSetting("AlphaMap"+StringConverter::toString(i+1),mTerrainAlphaTex[i]);
        for(int i=0;i<6;i++)
        {
            buffer = "";
            terrainMatFile.getSetting("Splat"+StringConverter::toString(i+1),buffer);
            const short terrainTex = (buffer=="" ? 0 : mTerrainTexList->findLine(buffer));
            mTerrainTex[i] = (terrainTex<=0?0:terrainTex);
        }
        remakeTerrain();

        //Load Map
        SaveFile mapFile(MAPMAKER_DIR+mapName+".map");
        mMapField->setCaption(mapName);
        buffer = "";
        mapFile.getSetting("Name",buffer,"Map");
        mDisplayField->setCaption(buffer);
        if(mapFile.getSetting("StartPos",buffer,"MapMaker"))mCameraMgr->setPosition(StringConverter::parseVector3(buffer));
        buffer = "";
        mapFile.getSetting("Boundary",buffer,"Terrain");
        mTerrainBoundary = StringConverter::parseReal(buffer);
        mTerrainBoundaryField->setCaption(buffer);
        buffer = "";
        mapFile.getSetting("HasPortal",buffer,"Portal");
        const bool hasPortal = StringConverter::parseBool(buffer);
        mHasPortalCheckBox->setChecked(hasPortal);
        updateHasPortal();
        buffer = "";
        mapFile.getSetting("NoFlying",buffer,"Skills");
        const bool bNoFlying = StringConverter::parseBool(buffer);
        m_pNoFlyingCheckBox->setChecked(bNoFlying);

        if(hasPortal)
        {
            buffer = "";
            mapFile.getSetting("Position",buffer,"Portal");
            const Vector3 pos = StringConverter::parseVector3(buffer);
            for(int i=0;i<3;i++)mPortalPosField[i]->setCaption(StringConverter::toString(pos[i]));
            buffer = "";
            mapFile.getSetting("Yaw",buffer,"Portal");
            mPortalYawField->setCaption(buffer);
        }
        //Music
        buffer = "";
        mapFile.getSetting("Name",buffer,"Music");
        mMusicField->setCaption(buffer);
        //Sky
        buffer = "";
        mapFile.getSetting("HasSky",buffer,"Sky");
        mHasSky = StringConverter::parseBool(buffer);
        mSkyCheckBox->setChecked(mHasSky);
        buffer = "";
        if(mHasSky)
        {
            mapFile.getSetting("Filename",buffer,"Sky");
            mSkyFile = buffer;
        }
        else
        {
            mapFile.getSetting("CeilingMat",buffer,"Sky");
            mCeilingMat = buffer;
            buffer = "";
            mapFile.getSetting("CeilingHeight",buffer,"Sky");
            mCeilingHeightField->setCaption(buffer);
        }
        //Weather
        buffer = "";
        mapFile.getSetting("Filename",buffer,"Weather");
        mWeatherField->setCaption(buffer);
        updateSky();
        //Ocean
        buffer = "";
        mHasOcean = mapFile.getSetting("Ocean",buffer,"Water");
        mOceanCheckBox->setChecked(mHasOcean);
        if(mHasOcean)mOceanCenter = StringConverter::parseVector3(buffer);
        else mOceanCenter.y = 50;
        mOceanHeightField->setCaption(StringConverter::toString(mOceanCenter.y));
        buffer = "";
        mapFile.getSetting("OceanMat",buffer,"Water");
        mOceanMatField->setCaption(buffer);
        updateOcean();

        deleteData();
        //Load Objects, Gates and Water
        for(int i=0;i<3;i++)
        {
            SaveFile objectFile;
            String objectFilename = MAPMAKER_DIR+mapName+"/"+mapName;
            if(i==0)objectFilename += "Objects.cfg";
            else if(i==1)objectFilename += "Gates.cfg";
            else if(i==2)objectFilename += "Water.cfg";
            if(objectFile.load(objectFilename))
            {
                for(objectFile.beginSectionIterator(); objectFile.hasMoreSections(); objectFile.nextSection())
                {
                    const String objectName = objectFile.peekNextSectionName();
                    if(i==0)
                    {
                        const StringVector objectGroup = StringUtil::split(objectName,"/",1);
                        if(objectGroup.size()<2)continue;
                        mWorldObjectMgr->loadObjects(objectGroup[0],false);
                    }
                    for(objectFile.beginSettingIterator(); objectFile.hasMoreSettings(); objectFile.nextSetting())
                    {
                        const String settingValue = objectFile.getSettingValue();
                        const bool isLarge = (settingValue.length()>0?(settingValue[0]=='%'):false);
                        const StringVector part = StringUtil::split(settingValue,";%");

                        mCurrentObject = 0;
                        if(i==0)
                        {
                            addNewObject(objectName);
                        }
                        else if(i==1)
                        {
                            if(part.size()>5)
                            {
                                mGateMapField->setCaption(part[3]);
                                const Vector3 position = StringConverter::parseVector3(part[4]);
                                mGatePosField[0]->setCaption(StringConverter::toString(position.x));
                                mGatePosField[1]->setCaption(StringConverter::toString(position.y));
                                mGatePosField[2]->setCaption(StringConverter::toString(position.z));
                                mGateYawField->setCaption(part[5]);
                                mGateBlackMatCheckBox->setChecked(isLarge); //isGateBlackMat shares with isLarge
                                addNewGate();
                            }
                        }
                        else
                        {
                            mWaterMat = (part.size()>2?part[2]:"");
                            addNewWater();
                        }

                        if(mCurrentObject)
                        {
                            mPosition = (part.size()>0 ? StringConverter::parseVector3(part[0]) : Vector3::ZERO);
                            updatePosition();
                            if(i==0)
                            {
                                mScale = (part.size()>1 ? StringConverter::parseVector3(part[1]) : Vector3::UNIT_SCALE);
                                updateScale();
                                const Quaternion orientation = (part.size()>2 ? StringConverter::parseQuaternion(part[2]) : Quaternion::IDENTITY);
                                mCurrentObject->mNode->setOrientation(orientation);
                            }
                            else if(i==1)
                            {
                                const Real size = (part.size()>1 ? StringConverter::parseReal(part[1]) : 30);
                                mScale = Vector3(1,size,1);
                                updateScale();
                                const Quaternion orientation = (part.size()>2 ? StringConverter::parseQuaternion(part[2]) : Quaternion::IDENTITY);
                                mCurrentObject->mNode->setOrientation(orientation);
                            }
                            else
                            {
                                const Vector2 size = (part.size()>1 ? StringConverter::parseVector2(part[1]) : Vector2::UNIT_SCALE);
                                mScale = Vector3(size.x,1,size.y);
                                updateScale();
                            }
                            mCurrentObject->mIsLarge = isLarge;

                            mCurrentObject->highlight(false);
                        }
                    }
                }
            }
        }

        reset();
    }
    void doSaveMap(const bool &overwrite=false, const bool &doExport=false)
    {
        if(mMapName=="")
        {
            mGui->showAlertBox("Enter a Map name.");
            return;
        }
        if(!validateMapName())
        {
            mGui->showAlertBox("Invalid map name.");
            return;
        }
        if(mDisplayName=="")
        {
            mGui->showAlertBox("Enter a Map Display name.");
            return;
        }
        String mapName = mMapName;
        StringUtil::toLowerCase(mapName);
        if(mapName==MAPMAKER_TEMP || mapName=="default")
        {
            mGui->showAlertBox("Cannot save Map name as "+mMapName+"!");
            return;
        }
        SaveFile sf;

        if(!overwrite && sf.load(MAPMAKER_DIR+mMapName+".map"))
        {
            mGui->showAlertBox(mMapName+" exists, overwrite?","MapMaker/Overwrite"+String(doExport?"Export":""),true);
            return;
        }
        saveMap(mMapName);
        loadAllMaps();

        if(doExport)exportMap(mMapName);
    }
    const bool validateMapName()
    {
        if(mMapName.find_first_of("!@#$%^&*()+-={}[]<>?,./\\;':\"`~| ")!=string::npos)return false;
        if(StringUtil::startsWith(mMapName," ") || StringUtil::endsWith(mMapName," "))return false;
        return true;
    }
    void saveMap(const String &mapName)
    {
        //Create folder with mapname
        #if defined OIS_WIN32_PLATFORM
        const String dir = MAPMAKER_DIR+mapName;
        CreateDirectory(dir.c_str(),0);
        #elif defined OIS_LINUX_PLATFORM
        makedir(dir.c_str());
        #endif
        saveTerrainFile(mapName);
        saveTerrainMaterialFile(mapName);
        saveObjectsGatesWaterFile(mapName);
        saveMapFile(mapName);

        #if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
        ResourceGroupManager::getSingleton().addResourceLocation(String(macBundlePath()+"/"+MAPMAKER_DIR+mapName),"FileSystem","Terrains/"+mapName);
        #else
        ResourceGroupManager::getSingleton().addResourceLocation(MAPMAKER_DIR+mapName,"FileSystem","Terrains/"+mapName);
        #endif
        ResourceGroupManager::getSingleton().initialiseResourceGroup("Terrains/"+mapName);
    }
    void exportMap(const String &mapName)
    {
        std::vector<std::pair<String,String> > filenames;
        filenames.push_back(pair<String,String>(MAPMAKER_DIR+mapName+".map", mapName+".map"));
        filenames.push_back(pair<String,String>(MAPMAKER_DIR+mapName+"/"+mapName+"Terrain.cfg", mapName+"Terrain.cfg"));
        if(mTerrainHeightMap!="" && mTerrainHeightMap!="defaultTerrain.png")
        {
            const String filename = SaveFile::findDirectoryOf(mTerrainHeightMap);
            if(filename!="")filenames.push_back(pair<String,String>(filename, mTerrainHeightMap));
            else
            {
                LoggerManager::getSingleton().logMessage("ERROR::MapMaker:exportMap: Cannot find heightMap "+mTerrainHeightMap+" for map "+mapName+"!");
                mGui->queueAlertBox("Cannot find heightMap "+mTerrainHeightMap+" for map "+mapName+"!");
            }
        }
        for(int i=0;i<2;i++)
        {
            if(mTerrainAlphaTex[i]!="" && mTerrainAlphaTex[i]!="defaultMask.png")
            {
                const String filename = SaveFile::findDirectoryOf(mTerrainAlphaTex[i]);
                if(filename!="")filenames.push_back(pair<String,String>(filename, mTerrainAlphaTex[i]));
                else
                {
                    LoggerManager::getSingleton().logMessage("ERROR::MapMaker:exportMap: Cannot find maskTexture "+mTerrainAlphaTex[i]+" for map "+mapName+"!");
                    mGui->queueAlertBox("Cannot find maskTexture "+mTerrainAlphaTex[i]+" for map "+mapName+"!");
                }
            }
        }

        SaveFile objectsFile(MAPMAKER_DIR+mapName+"/"+mapName+"Objects.cfg");
        if(!objectsFile.isEmpty())filenames.push_back(pair<String,String>(MAPMAKER_DIR+mapName+"/"+mapName+"Objects.cfg", mapName+"Objects.cfg"));
        SaveFile gatesFile(MAPMAKER_DIR+mapName+"/"+mapName+"Water.cfg");
        if(!gatesFile.isEmpty())filenames.push_back(pair<String,String>(MAPMAKER_DIR+mapName+"/"+mapName+"Water.cfg", mapName+"Water.cfg"));
        SaveFile waterFile(MAPMAKER_DIR+mapName+"/"+mapName+"Gates.cfg");
        if(!waterFile.isEmpty())filenames.push_back(pair<String,String>(MAPMAKER_DIR+mapName+"/"+mapName+"Gates.cfg", mapName+"Gates.cfg"));

        filenames.push_back(pair<String,String>(MAPMAKER_DIR+mapName+"/"+mapName+"Terrain.material", mapName+"Terrain.material"));

        //Include only textures that are not default
        std::vector<String> defaultTexList;
        SaveFile sf(String(MAPMAKER_DIR)+"textures/defaultTex.cfg");
        for(sf.beginSectionIterator(); sf.hasMoreSections(); sf.nextSection())
        for(sf.beginSettingIterator(); sf.hasMoreSettings(); sf.nextSetting())
        {
            const String value = sf.getSettingValue();
            defaultTexList.push_back(value);
        }
        for(int i=0;i<6;i++)
        {
            if(mTerrainTex[i]!=0)
            {
                const String texName = mTerrainTexList->getLine(mTerrainTex[i]);
                if(texName!="")
                {
                    bool isDefault = false;
                    for(std::vector<String>::iterator it=defaultTexList.begin(); it!=defaultTexList.end(); it++)
                    {
                        const String defaultTex = *it;
                        if(texName==defaultTex)
                        {
                            isDefault = true;
                            break;
                        }
                    }
                    if(isDefault)continue;
                    filenames.push_back(pair<String,String>(String(MAPMAKER_DIR)+"textures/"+texName, texName));
                }
            }
        }

        const bool success = ZipManager::getSingletonPtr()->fhz(filenames, EXPORT_DIR+mapName+".fhm");

        if(success)
        {
            //Export object groups
            std::vector<String> objectGroups;
            for(std::vector<MapMakerObject*>::iterator it=mObjectList.begin(); it!=mObjectList.end(); it++)
            {
                MapMakerObject *object = *it;
                if(object->mIsWater || object->mIsGate)continue;
                const String objectName = object->mObjectName;
                const size_t pos = objectName.find_first_of('/');
                const String objectGroup = objectName.substr(0,pos);
                if(objectGroup!="")
                {
                    bool exists = false;
                    for(std::vector<String>::iterator jt=objectGroups.begin(); jt!=objectGroups.end(); jt++)
                    {
                        String cmpName = *jt;
                        String cmpName2 = objectGroup;
                        StringUtil::toLowerCase(cmpName);
                        StringUtil::toLowerCase(cmpName2);
                        if(cmpName==cmpName2)
                        {
                            exists = true;
                            break;
                        }
                    }
                    if(!exists)objectGroups.push_back(objectGroup);
                }
            }

            ObjectMaker objectMaker;
            objectMaker.init(mSceneMgr,false);
            for(std::vector<String>::iterator jt=objectGroups.begin(); jt!=objectGroups.end(); jt++)
            {
                const String objectGroup = *jt;
                objectMaker.exportGroup(objectGroup);
            }
            objectMaker.deleteGui();

            //Export sky and weather
            SkyMaker skyMaker;
            skyMaker.init(mSceneMgr,false);
            if(mSkyFile!="")skyMaker.exportSky(mSkyFile,true);
            if(mWeatherFile!="")skyMaker.exportCycle(mWeatherFile,true);
            skyMaker.deleteGui();
        }

        if(success)mGui->queueAlertBox("Exported map as "+String(EXPORT_DIR)+mapName+".fhm");
        else mGui->queueAlertBox("Failed to export map "+mapName+"!");
    }
    const bool validateTerrainFile(const String &filename)
    {
        SaveFile sf;
        if(!sf.load(MAPMAKER_DIR+filename))return false;
        String value = "";
        if(!sf.getSetting("Heightmap.image",value))return false;
        if(!SaveFile::resourceExists(value))return false;
        return true;
    }
    void saveTerrainFile(const String &mapName)
    {
        SaveFile sf(MAPMAKER_DIR+mapName+"/"+mapName+"Terrain.cfg");

        sf.setSetting("PageSource","Heightmap");
        sf.setSetting("Heightmap.image",mTerrainHeightMap);

        Image img;
        String heightmapGroup = "";
        if(!SaveFile::resourceExists(mTerrainHeightMap,&heightmapGroup))
        {
            mGui->showAlertBox("Error caching "+mapName+"!");
            return;
        }
        img.load(mTerrainHeightMap,heightmapGroup);
        const unsigned short imgSize = img.getWidth();

        sf.setSetting("PageSize",StringConverter::toString(imgSize));
        sf.setSetting("TileSize",StringConverter::toString(65));
        sf.setSetting("MaxPixelError",StringConverter::toString(3));

        sf.setSetting("PageWorldX",StringConverter::toString(mTerrainSize.x));
        sf.setSetting("PageWorldZ",StringConverter::toString(mTerrainSize.z));
        sf.setSetting("MaxHeight",StringConverter::toString(mTerrainSize.y));

        sf.setSetting("MaxMipMapLevel",StringConverter::toString(5));
        sf.setSetting("VertexNormals","yes");
        sf.setSetting("VertexProgramMorph","no");
        sf.setSetting("LODMorphStart",StringConverter::toString(0.2f));

        sf.setSetting("CustomMaterialName","Terrain/"+mapName);

        sf.save();
    }
    void saveTerrainMaterialFile(const String &mapName)
    {
        const String filename = MAPMAKER_DIR+mapName+"/"+mapName+"Terrain.material";
        const bool useAlpha2 = (mTerrainTex[3]||mTerrainTex[4]||mTerrainTex[5]);
        const String refMatName = (useAlpha2?"AlphaSplatTerrain2":"AlphaSplatTerrain1");
        ofstream outFile(filename.c_str(),ios::out);

        String buffer = "import "+refMatName+" from \"terrain.material\"\n"; outFile.write(buffer.c_str(),buffer.length());
        buffer = "material Terrain/"+mapName+" : "+refMatName+"\n"; outFile.write(buffer.c_str(),buffer.length());
        buffer = "{\n"; outFile.write(buffer.c_str(),buffer.length());
        buffer = "technique\n"; outFile.write(buffer.c_str(),buffer.length());
        buffer = "{\n"; outFile.write(buffer.c_str(),buffer.length());
        buffer = "pass\n"; outFile.write(buffer.c_str(),buffer.length());
        buffer = "{\n"; outFile.write(buffer.c_str(),buffer.length());
        buffer = "vertex_program_ref AlphaSplatTerrain/VP\n"; outFile.write(buffer.c_str(),buffer.length());
        buffer = "{\n"; outFile.write(buffer.c_str(),buffer.length());
        buffer = "param_named pageSize float "+StringConverter::toString(mTerrainSize.x)+"\n"; outFile.write(buffer.c_str(),buffer.length());
        buffer = "param_named textureTileSize float "+StringConverter::toString(mTerrainSize.x>0?50000/mTerrainSize.x:10)+"\n"; outFile.write(buffer.c_str(),buffer.length());
        buffer = "}\n"; outFile.write(buffer.c_str(),buffer.length());
        buffer = "fragment_program_ref "+refMatName+"/FP\n"; outFile.write(buffer.c_str(),buffer.length());
        buffer = "{\n"; outFile.write(buffer.c_str(),buffer.length());
        buffer = "param_named alpha0Mask float4";
        for(int i=0;i<3;i++)buffer += mTerrainTex[i]==0?" 0":" 1";
        buffer += " 0\n"; outFile.write(buffer.c_str(),buffer.length());
        if(useAlpha2)
        {
            buffer = "param_named alpha1Mask float4";
            for(int i=3;i<6;i++)buffer += mTerrainTex[i]==0?" 0":" 1";
            buffer += " 0\n"; outFile.write(buffer.c_str(),buffer.length());
        }
        buffer = "}\n"; outFile.write(buffer.c_str(),buffer.length());
        buffer = "}\n"; outFile.write(buffer.c_str(),buffer.length());
        buffer = "}\n"; outFile.write(buffer.c_str(),buffer.length());
        buffer = "set_texture_alias AlphaMap1 "+(mTerrainAlphaTex[0]==""?"black.png":mTerrainAlphaTex[0])+"\n"; outFile.write(buffer.c_str(),buffer.length());
        if(useAlpha2){buffer = "set_texture_alias AlphaMap2 "+(mTerrainAlphaTex[1]==""?"black.png":mTerrainAlphaTex[1])+"\n"; outFile.write(buffer.c_str(),buffer.length());}
        for(int i=0;i<6;i++)
        if(mTerrainTex[i]!=0)
        {
            const String texName = mTerrainTexList->getLine(mTerrainTex[i]);
            if(texName!="")
            {
                buffer = "set_texture_alias Splat"+StringConverter::toString(i+1)+" "+texName+"\n";
                outFile.write(buffer.c_str(),buffer.length());
            }
        }
        //buffer = "set_texture_alias Detail detail.jpg\n"; outFile.write(buffer.c_str(),buffer.length());
        buffer = "}\n"; outFile.write(buffer.c_str(),buffer.length());

        outFile.close();

        //Settings file for loading
        SaveFile sf(MAPMAKER_DIR+mapName+"/"+mapName+"Material.cfg");
        sf.clear();

        for(int i=0;i<2;i++)sf.setSetting("AlphaMap"+StringConverter::toString(i+1),mTerrainAlphaTex[i]);
        for(int i=0;i<6;i++)sf.setSetting("Splat"+StringConverter::toString(i+1),(mTerrainTex[i]?mTerrainTexList->getLine(mTerrainTex[i]):""));

        sf.save();
    }
    void saveObjectsGatesWaterFile(const String &mapName)
    {
        SaveFile objectFile(MAPMAKER_DIR+mapName+"/"+mapName+"Objects.cfg");
        SaveFile gateFile(MAPMAKER_DIR+mapName+"/"+mapName+"Gates.cfg");
        SaveFile waterFile(MAPMAKER_DIR+mapName+"/"+mapName+"Water.cfg");
        objectFile.clear();
        gateFile.clear();
        waterFile.clear();
        for(std::vector<MapMakerObject*>::iterator it=mObjectList.begin(); it!=mObjectList.end(); it++)
        {
            MapMakerObject *object = *it;
            SceneNode *node = object->mNode;

            String info = (object->mIsLarge||object->mIsGateBlackMat ? "%" : "");
            info += StringConverter::toString(node->getPosition());

            if(object->mIsWater)
            {
                const Vector3 scale = object->getScale();
                info += ";" + StringConverter::toString(Vector2(scale.x,scale.z));
                if(object->mObjectName!="")info += ";" + object->mObjectName;
            }
            else if(object->mIsGate)
            {
                const Vector3 scale = object->getScale();
                info += ";" + StringConverter::toString(scale.y);
                info += ";" + StringConverter::toString(node->getOrientation());
                info += ";" + object->mObjectName;
                info += ";" + StringConverter::toString(object->mGatePosition);
                info += ";" + StringConverter::toString(object->mGateYaw);
            }
            else
            {
                if(node->getScale()!=Vector3::UNIT_SCALE || node->getOrientation()!=Quaternion::IDENTITY)info += ";" + StringConverter::toString(node->getScale());
                if(node->getOrientation()!=Quaternion::IDENTITY)info += ";" + StringConverter::toString(node->getOrientation());
            }

            if(object->mIsWater)waterFile.setSetting("",info);
            else if(object->mIsGate)gateFile.setSetting("",info);
            else objectFile.setSetting("",info,object->mObjectName);
        }

        objectFile.save();
        gateFile.save();
        waterFile.save();
    }
    const bool saveMapFile(const String &mapName)
    {
        SaveFile sf;
        const bool exists = sf.load(MAPMAKER_DIR+mapName+".map");

        if(mDisplayName!="")sf.setSetting("Name",mDisplayName,"Map");

        //Generate seed
        String seed = "";
        if(!sf.getSetting("Seed",seed,"Map"))
        {
            for(int i=0;i<10;i++)
            {
                switch(rand()%3)
                {
                    case 0: seed.push_back(char(48+rand()%10)); break;
                    case 1: seed.push_back(char(65+rand()%26)); break;
                    default: seed.push_back(char(97+rand()%26)); break;
                }
            }
            sf.setSetting("Seed",seed,"Map");
        }
        //Generate official hash
        if(MAPMAKER_OFFICIALIZER)sf.setSetting("Hash",md5(mapName+seed),"Map");

        sf.setSetting("Filename",mapName+"Terrain.cfg","Terrain");
        sf.setSetting("Size",StringConverter::toString(Vector2(mTerrainSize.x,mTerrainSize.z)),"Terrain");
        sf.setSetting("Boundary",StringConverter::toString(mTerrainBoundary),"Terrain");

        sf.clearSection("Sky");
        sf.setSetting("HasSky",StringConverter::toString(mHasSky),"Sky");
        if(mHasSky)
        {
            if(mSkyFile!="")sf.setSetting("Filename",mSkyFile,"Sky");
        }
        else
        {
            if(mCeilingMat!="")sf.setSetting("CeilingMat",mCeilingMat,"Sky");
            sf.setSetting("CeilingHeight",StringConverter::toString(mCeilingHeight),"Sky");
        }

        if(mWeatherFile=="")sf.clearSection("Weather");
        else sf.setSetting("Filename",mWeatherFile,"Weather");

        sf.setSetting("Filename",mapName+"Objects.cfg","Objects");

        sf.setSetting("Filename",mapName+"Gates.cfg","Gates");

        sf.setSetting("Filename",mapName+"Water.cfg","Water");

        if(mHasOcean)
        {
            sf.setSetting("Ocean",StringConverter::toString(mOceanCenter),"Water");
            sf.setSetting("OceanSize",StringConverter::toString(mOceanSize),"Water");
            if(mOceanMat!="")sf.setSetting("OceanMat",mOceanMat,"Water");
            else sf.deleteSetting("OceanMat","Water");
        }
        else
        {
            sf.deleteSetting("Ocean","Water");
            sf.deleteSetting("OceanSize","Water");
            sf.deleteSetting("OceanMat","Water");
        }

        sf.setSetting("StartPos",StringConverter::toString(mCameraMgr->getPosition()),"MapMaker");

        sf.deleteSection("Portal");
        if(mHasPortalCheckBox->isChecked())
        {
            sf.setSetting("HasPortal","true","Portal");
            Vector3 pos = Vector3(2500,100,2500);
            for(int i=0;i<3;i++)pos[i] = StringConverter::parseReal(mPortalPosField[i]->getCaption());
            sf.setSetting("Position",StringConverter::toString(pos),"Portal");
            const Real yaw = StringConverter::parseReal(mPortalYawField->getCaption());
            sf.setSetting("Yaw",yaw>360?"360":StringConverter::toString(yaw),"Portal");
        }

        sf.deleteSection("Skills");
        if(m_pNoFlyingCheckBox->isChecked())
        {
            sf.setSetting("NoFlying","true","Skills");
        }

        if(mMusic=="")sf.deleteSection("Music");
        else sf.setSetting("Name",mMusic,"Music");

        sf.save();
        return exists;
    }
    void saveTemp()
    {
        saveMap("temp");
        mWasTesting = true;
    }
    void updateMiniMap()
    {
        const Vector3 pos = mCameraMgr->getPosition();
        if(mTerrainSize.x>0&&mTerrainSize.z>0)mMiniMap->setMarkerPosition(pos.x/mTerrainSize.x,pos.z/mTerrainSize.z);
        mMiniMap->setMarkerRotation(mCameraMgr->getYaw());
    }
    void updateCameraToMiniMap()
    {
        const pair<Real,Real> pos = mMiniMap->getMarkerPosition();
        if(mTerrainSize.x>0&&mTerrainSize.z>0)mCameraMgr->setPosition(Vector3(pos.first*mTerrainSize.x,mCameraMgr->getPosition().y,pos.second*mTerrainSize.z));
    }
    void updateMiniMapTexture()
    {
        mMiniMap->setMaterialTexture(mTerrainHeightMap);
    }
};

#endif
