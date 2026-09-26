#ifndef _MAPMANAGER_H_
#define _MAPMANAGER_H_

#define MAP_TEMP "temp"
#define MAP_DIR "media/terrains/"
#define MAP_DIR_OFFICIAL "media/terrains/fhmaps/"
#define MAP_DEFAULT "fhLonelyCave"
#define MAP_CUSTOMMAPSGATEWAY "fhCapeOfWorlds"

#include <GridManager.h>
#include <CollisionManager.h>
#include <WorldObjectManager.h>
#include <WaterManager.h>
#include <SkyManager.h>
#include <ZipManager.h>
#include "md5.h"

struct MapInfo
{
    String mTerrainFile;
    String mSkyFile;
    String mWeatherFile;
    String mObjectsFile;
    String mGatesFile;
    String mWaterFile;
    Vector2 mTerrainSize;
    Real mTerrainBoundary;
    bool mHasOcean;
    Vector3 mOceanCenter;
    Vector2 mOceanSize;
    String mOceanMat;
    bool mHasSky;
    Real mCeilingHeight;
    bool mHasPortal;
    Vector3 mPortalPos;
    Real mPortalYaw;
    String mMusic;
    bool m_bNoFlying;
    MapInfo()
    {
        mTerrainFile = "";
        mSkyFile = "";
        mWeatherFile = "";
        mObjectsFile = "";
        mGatesFile = "";
        mWaterFile = "";
        mTerrainSize = Vector2::ZERO;
        mTerrainBoundary = 0;
        mHasSky = false;
        mCeilingHeight = 0;
        mHasOcean = false;
        mOceanCenter = Vector3::ZERO;
        mOceanSize = Vector2::ZERO;
        mOceanMat = "";
        mHasPortal = false;
        mPortalPos = Vector3::ZERO;
        mPortalYaw = 0;
        mMusic = "";
        m_bNoFlying = false;
    }
};

class MapManager : public Singleton<MapManager>
{
private:
    SceneManager *mSceneMgr;
    GridManager *mGridMgr;
    CollisionManager *mCollMgr;
    WorldObjectManager *mWorldObjectMgr;
    WaterManager *mWaterMgr;
    LightManager *mLightMgr;
    SkyManager *mSkyMgr;
    String mMapName;
    bool mIsMapOfficial;
    String mTerrainHeightmap;
    Vector2 mTerrainSize;
    bool m_bNoFlying;
    bool m_bIsDefault;
public:
    MapManager()
    {
        mGridMgr = GridManager::getSingletonPtr();
        mCollMgr = CollisionManager::getSingletonPtr();
        mWorldObjectMgr = WorldObjectManager::getSingletonPtr();
        mWaterMgr = WaterManager::getSingletonPtr();
        mLightMgr = LightManager::getSingletonPtr();
        mSkyMgr = SkyManager::getSingletonPtr();
        mSceneMgr = 0;
    }
    ~MapManager()
    {
    }
    void init(SceneManager *sceneMgr, const String &worldName, const bool &changeMusic=true, const bool &useFhz=true)
    {
        mSceneMgr = sceneMgr;
        mTerrainHeightmap = "";
        mTerrainSize = Vector2(5000,5000);
        if(worldName=="")createDefaultMap();
        else loadMap(worldName,changeMusic,useFhz);
    }
    void createDefaultMap()
    {
        mGridMgr->init(mSceneMgr,Vector2(5000,5000));
        setWorldGeometry("defaultTerrain.cfg");
        mSkyMgr->createSky(mLightMgr->createLight());
    }
    void loadTempObject()
    {
        SaveFile sf(String(MAP_DIR)+"temp/tempObjects.cfg");
        sf.clear();
        sf.setSetting("","2500 0 2500","temp/temp");
        sf.save();
        mWorldObjectMgr->loadObjects("temp",false);
        loadObjectsFile("temp","tempObjects.cfg",false);
        mGridMgr->buildAllGeometries();
    }
    void loadMap(const String &name, const bool &changeMusic, const bool &useFhz=true)
    {
        MapInfo mapInfo;
        mIsMapOfficial = false;
        mMapName = "";
        loadMapFile(name,&mapInfo,useFhz);

        if(mapInfo.mTerrainFile!="")
        {
            setWorldGeometry(mapInfo.mTerrainFile);
            mGridMgr->init(mSceneMgr,mapInfo.mTerrainSize);
            mCollMgr->setMapBounds(mapInfo.mTerrainBoundary,mapInfo.mTerrainSize.x-mapInfo.mTerrainBoundary);

            setTerrainHeightmap(name,mapInfo.mTerrainFile);
            mTerrainSize = mapInfo.mTerrainSize;
        }
        if(mapInfo.mHasSky)
        {
            mSkyMgr->createSky(mLightMgr->createLight());
            mSkyMgr->loadSky(mapInfo.mSkyFile,useFhz);
        }
        else
        {
            mSkyMgr->createCeiling(Vector3(mapInfo.mTerrainSize.x,mapInfo.mCeilingHeight,mapInfo.mTerrainSize.y),(mapInfo.mSkyFile!=""?mapInfo.mSkyFile:"CeilingMat"));
            mCollMgr->setCeilingHeight(mapInfo.mCeilingHeight);
        }
        if(mapInfo.mWeatherFile!="")mSkyMgr->createWeather(mapInfo.mWeatherFile,useFhz);
        if(mapInfo.mObjectsFile!="")loadObjectsFile(name,mapInfo.mObjectsFile,useFhz);
        if(mapInfo.mGatesFile!="")loadGatesFile(name,mapInfo.mGatesFile);
        if(mapInfo.mWaterFile!="")loadWaterFile(name,mapInfo.mWaterFile);

        if(name==MAP_CUSTOMMAPSGATEWAY)createGatesToCustomMaps();
        if(mapInfo.mHasPortal)createGateToGateway(mapInfo.mPortalPos,mapInfo.mPortalYaw);
        mGridMgr->buildAllGeometries();

        if(mapInfo.mHasOcean)mWaterMgr->createOcean(mapInfo.mOceanCenter,mapInfo.mOceanSize,mapInfo.mOceanMat);

        m_bNoFlying = mapInfo.m_bNoFlying;
        m_bIsDefault = (name==MAP_DEFAULT);

        if(changeMusic)
        {
            if(mapInfo.mMusic!="")
            {
                if(!SoundManager::getSingletonPtr()->initMusic(mapInfo.mMusic))SoundManager::getSingletonPtr()->initSingleMusic(mapInfo.mMusic);
            }
            else
            {
                if(mapInfo.mHasSky)SoundManager::getSingletonPtr()->initMusic(MUSIC_PLAINS);
                else SoundManager::getSingletonPtr()->initMusic(MUSIC_CAVES);
            }
        }
    }
    void setWorldGeometry(const String &name)
    {
        mSceneMgr->setWorldGeometry(name);
        //Extend world's bounds to ensure rayqueries work
        AxisAlignedBox box;
        const Vector3 max(100000, 10000, 100000);
        box.setExtents(-max, max);
        mSceneMgr->setOption("Size", &box);
    }
    void loadMapFile(const String &mapName, MapInfo *mapInfo, const bool &useFhz=true)
    {
        SaveFile sf;
        /*
        if(!ZipManager::getSingletonPtr()->defhz(MAP_DIR_OFFICIAL+mapName) && !ZipManager::getSingletonPtr()->defhz(MAP_DIR+mapName))
        {
            if(!sf.load(MAP_DIR+mapName+".map"))return;
        }
        else if(!sf.loadZip(mapName+".map",ZIP_TEMP_DIR+mapName+".tmp"))return;
        */
        if(useFhz)
        {
            if(!ZipManager::getSingletonPtr()->loadByAnyMeans(&sf,mapName,".fhm",".map"))
            {
                LoggerManager::getSingleton().logMessage("ERROR::Map: failed to load Map "+mapName);
                return;
            }
        }
        else
        {
            if(!sf.load(MAP_DIR+mapName+".map"))
            {
                LoggerManager::getSingleton().logMessage("ERROR::Map: failed to load Map "+mapName);
                return;
            }
        }

        String mapSeed = "";
        for(sf.beginSectionIterator(); sf.hasMoreSections(); sf.nextSection())
        {
            const String section = sf.peekNextSectionName();

            for(sf.beginSettingIterator(); sf.hasMoreSettings(); sf.nextSetting())
            {
                const String tag = sf.getSettingTag();

                //Map
                if(section=="Map")
                {
                    if(tag=="Name")mMapName = sf.getSettingValue();
                    else if(tag=="Seed")mapSeed = sf.getSettingValue();
                    else if(tag=="Hash")mIsMapOfficial = (sf.getSettingValue() == md5(mapName+mapSeed));
                }
                //Portal
                else if(section=="Portal")
                {
                    if(tag=="HasPortal")mapInfo->mHasPortal = StringConverter::parseBool(sf.getSettingValue());
                    else if(tag=="Position")mapInfo->mPortalPos = StringConverter::parseVector3(sf.getSettingValue());
                    else if(tag=="Yaw")mapInfo->mPortalYaw = StringConverter::parseReal(sf.getSettingValue());
                }
                //Skills
                else if(section=="Skills")
                {
                    if(tag=="NoFlying")mapInfo->m_bNoFlying = StringConverter::parseBool(sf.getSettingValue());
                }
                //Music
                else if(section=="Music")
                {
                    if(tag=="Name")mapInfo->mMusic = sf.getSettingValue();
                }
                //Terrain
                else if(section=="Terrain")
                {
                    if(tag=="Filename")mapInfo->mTerrainFile = sf.getSettingValue();
                    else if(tag=="Size")mapInfo->mTerrainSize = StringConverter::parseVector2(sf.getSettingValue());
                    else if(tag=="Boundary")mapInfo->mTerrainBoundary = StringConverter::parseReal(sf.getSettingValue());
                }
                //Sky
                else if(section=="Sky")
                {
                    if(tag=="HasSky")mapInfo->mHasSky = StringConverter::parseBool(sf.getSettingValue());
                    if(tag=="Filename" || tag=="CeilingMat")mapInfo->mSkyFile = sf.getSettingValue();
                    if(tag=="CeilingHeight")mapInfo->mCeilingHeight = StringConverter::parseReal(sf.getSettingValue());
                }
                //Weather
                else if(section=="Weather")
                {
                    if(tag=="Filename")mapInfo->mWeatherFile = sf.getSettingValue();
                }
                //Objects
                else if(section=="Objects")
                {
                    if(tag=="Filename")mapInfo->mObjectsFile = sf.getSettingValue();
                }
                //Gates
                else if(section=="Gates")
                {
                    if(tag=="Filename")mapInfo->mGatesFile = sf.getSettingValue();
                }
                //Water
                else if(section=="Water")
                {
                    if(tag=="Filename")mapInfo->mWaterFile = sf.getSettingValue();
                    else if(tag=="Ocean")
                    {
                        mapInfo->mHasOcean = true;
                        mapInfo->mOceanCenter = StringConverter::parseVector3(sf.getSettingValue());
                    }
                    else if(tag=="OceanSize")mapInfo->mOceanSize = StringConverter::parseVector2(sf.getSettingValue());
                    else if(tag=="OceanMat")mapInfo->mOceanMat = sf.getSettingValue();
                }
            }
        }
    }
    void loadObjectsFile(const String &mapName, const String &filename, const bool &useFhz=true)
    {
        SaveFile sf;
        if(!sf.load(MAP_DIR+mapName+"/"+filename) && !sf.loadZip(filename,ZIP_TEMP_DIR+mapName+".fhm"+String(ZIP_TEMP_EXTENSION)))return;

        for(sf.beginSectionIterator(); sf.hasMoreSections(); sf.nextSection())
        {
            const String objectName = sf.peekNextSectionName();

            const StringVector objectGroup = StringUtil::split(objectName,"/",1);
            if(objectGroup.size()<2)continue;
            mWorldObjectMgr->loadObjects(objectGroup[0],useFhz);
            WorldObject *object = mWorldObjectMgr->getWorldObject(objectName);
            if(!object)continue;

            unsigned int objectCounter = 0;

            for(sf.beginSettingIterator(); sf.hasMoreSettings(); sf.nextSetting())
            {
                const String settingValue = sf.getSettingValue();
                const bool isLarge = (settingValue.length()>0?(settingValue[0]=='%'):false);
                const StringVector part = StringUtil::split(settingValue,";%");
                const Vector3 center = (part.size()>0 ? StringConverter::parseVector3(part[0]) : Vector3::ZERO);
                const Vector3 scale = (part.size()>1 ? StringConverter::parseVector3(part[1]) : Vector3::UNIT_SCALE);
                const Quaternion orientation = (part.size()>2 ? StringConverter::parseQuaternion(part[2]) : Quaternion::IDENTITY);

                Grid* grid = mGridMgr->getGridContaining(center.x,center.z);
                if(!grid)continue;

                if(object->mSound!="")grid->addSound(object->mSound,center);

                unsigned int counter = 0;
                for(std::vector<Collision*>::iterator j=object->mCollList.begin(); j!=object->mCollList.end(); j++)grid->addCollision(mCollMgr->createCollision(*j,center,scale,orientation));
                for(std::vector<WorldSubMesh*>::iterator j=object->mSubMeshList.begin(); j!=object->mSubMeshList.end(); j++)
                {
                    WorldSubMesh *subMesh = *j;
                    Entity *ent = mSceneMgr->createEntity(objectName+"_"+StringConverter::toString(objectCounter)+"_"+StringConverter::toString(counter++),subMesh->mMesh);
                    if(ent)
                    {
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
                        if(subMesh->mIsCollidable || isLarge)
                        {
                            if(subMesh->mIsCollidable)ent->setQueryFlags(Collision::MESH_COLL_MASK);
                            grid->addMovableObject(ent,center+orientation*(subMesh->mOffset*scale),orientation*subMesh->mOrientation,subMesh->mScale*scale,isLarge);
                        }
                        else grid->addEntity(ent,center+orientation*(subMesh->mOffset*scale),orientation*subMesh->mOrientation,subMesh->mScale*scale);
                    }
                }
                counter = 0;
                for(std::vector<WorldBillboard*>::iterator j=object->mBillboardList.begin(); j!=object->mBillboardList.end(); j++)
                {
                    WorldBillboard *billboardInfo = *j;
                    BillboardSet *set = mSceneMgr->createBillboardSet(objectName+"_bb"+StringConverter::toString(objectCounter)+"_"+StringConverter::toString(counter++),1);
                    set->setMaterialName(billboardInfo->mMaterial);
                    set->setDefaultDimensions(billboardInfo->mSize.x,billboardInfo->mSize.y);
                    set->createBillboard(0,0,0);
                    grid->addMovableObject(set,center+orientation*(billboardInfo->mOffset*scale),Quaternion::IDENTITY,scale);
                }
                counter = 0;
                for(std::vector<WorldParticle*>::iterator j=object->mParticleList.begin(); j!=object->mParticleList.end(); j++)
                {
                    WorldParticle *particleInfo = *j;
                    if(!ParticleSystemManager::getSingletonPtr()->getTemplate(particleInfo->mParticle))
                    {
                        LoggerManager::getSingleton().logMessage("MapManager::loadObjectsFile: ParticleSystem not found: " + particleInfo->mParticle);
                        continue;
                    }
                    ParticleSystem *ps = mSceneMgr->createParticleSystem(objectName+"_ps"+StringConverter::toString(objectCounter)+"_"+StringConverter::toString(counter++),particleInfo->mParticle);
                    if(ps)grid->addMovableObject(ps,center+orientation*(particleInfo->mOffset*scale),particleInfo->mOrientation*orientation,particleInfo->mScale*scale);
                }
                counter = 0;
                for(std::vector<WorldLight*>::iterator j=object->mLightList.begin(); j!=object->mLightList.end(); j++)
                {
                    WorldLight *lightInfo = *j;
                    Light *lt = mLightMgr->createLight();
                    if(lt)
                    {
                        lt->setPosition(center+orientation*(lightInfo->mOffset*scale));
                        lt->setDiffuseColour(lightInfo->mColour);
                        lt->setAttenuation(1000,1.0f,0.01f,0);
                    }
                }
                objectCounter++;
            }
        }
    }
    void loadGatesFile(const String &mapName, const String &filename)
    {
        SaveFile sf;
        if(!sf.load(MAP_DIR+mapName+"/"+filename) && !sf.loadZip(filename,ZIP_TEMP_DIR+mapName+".fhm"+String(ZIP_TEMP_EXTENSION)))return;

        for(sf.beginSectionIterator(); sf.hasMoreSections(); sf.nextSection())
        for(sf.beginSettingIterator(); sf.hasMoreSettings(); sf.nextSetting())
        {
            const String info = sf.getSettingValue();
            const bool isBlackMat = (info.length()>0?(info[0]=='%'):false);
            const StringVector part = StringUtil::split(info,";%");
            if(part.size()<6)continue;
            const Vector3 center = StringConverter::parseVector3(part[0]);
            const Real size = StringConverter::parseReal(part[1]);
            const Quaternion orientation = StringConverter::parseQuaternion(part[2]);
            GateInfo gateInfo;
            gateInfo.mMap = part[3];
            gateInfo.mPosition = StringConverter::parseVector3(part[4]);
            gateInfo.mYaw = StringConverter::parseReal(part[5]);

            Grid* grid = mGridMgr->getGridContaining(center.x,center.z);
            if(!grid)continue;

            GateCollision copy(gateInfo,Vector3::ZERO,size,orientation.zAxis());
            grid->addCollision(mCollMgr->createCollision(&copy,center,Vector3::UNIT_SCALE,orientation, (isBlackMat?"GateMat/Black":"GateMat/Gate") ));
        }
    }
    void loadWaterFile(const String &mapName, const String &filename)
    {
        SaveFile sf;
        if(!sf.load(MAP_DIR+mapName+"/"+filename) && !sf.loadZip(filename,ZIP_TEMP_DIR+mapName+".fhm"+String(ZIP_TEMP_EXTENSION)))return;

        //unsigned int counter = 0;

        for(sf.beginSectionIterator(); sf.hasMoreSections(); sf.nextSection())
        for(sf.beginSettingIterator(); sf.hasMoreSettings(); sf.nextSetting())
        {
            const String info = sf.getSettingValue();
            const StringVector part = StringUtil::split(info,";");
            const Vector3 center = (part.size()>0 ? StringConverter::parseVector3(part[0]) : Vector3::ZERO);
            const Vector2 size = (part.size()>1 ? StringConverter::parseVector2(part[1]) : Vector2::UNIT_SCALE);

            Grid* grid = mGridMgr->getGridContaining(center.x,center.z);
            if(!grid)continue;

            WaterPlane *waterPlane = mWaterMgr->createWaterPlane(center,size,part.size()>2?part[2]:WATER_REFLECTMAT);
            grid->addWaterPlane(waterPlane);
            //Entity *ent = mSceneMgr->createEntity("WaterPlane"+StringConverter::toString(counter++),"WaterPlaneMesh");
            //ent->setMaterialName(part.size()>2?part[2]:WATER_DEFAULTMAT);
            grid->addMovableObject(waterPlane->mEnt,center,Quaternion::IDENTITY,Vector3(size.x/WATER_SIZE,1,size.y/WATER_SIZE));
        }
    }
    void reset()
    {
        mGridMgr->clear();
        mWorldObjectMgr->clear();
    }
    const String getMapSeed(const String &mapName, String *displayName=0, const bool &isGoHome=false)
    {
        SaveFile sf;
        /*
        if(!ZipManager::getSingletonPtr()->defhz(MAP_DIR_OFFICIAL+mapName) && !ZipManager::getSingletonPtr()->defhz(MAP_DIR+mapName))
        {
            if(!sf.load(MAP_DIR+mapName+".map"))return "";
        }
        else if(!sf.loadZip(mapName+".map",ZIP_TEMP_DIR+mapName+".tmp"))return "";
        */
        if(!ZipManager::getSingletonPtr()->loadByAnyMeans(&sf,mapName,".fhm",".map"))return "";

        if(displayName)
        {
            String buffer = "";
            sf.getSetting("Name",buffer,"Map");
            *displayName = buffer;
        }
        String seed = "";
        if(!sf.getSetting("Seed",seed,"Map"))return "";
        String buffer = "";
        //Unofficial maps cannot access official maps
        if(!mIsMapOfficial && mapName!=MAP_CUSTOMMAPSGATEWAY && !isGoHome && sf.getSetting("Hash",buffer,"Map"))
        {
            if(md5(mapName+seed)==buffer)return "";
        }

        return seed;
    }
    void createGatesToCustomMaps()
    {
        std::vector<GateInfo> gateList;
        //Check all .map files for portal
        const StringVector list = SaveFile::findResourceNames("UnofficialTerrains","*.map");
        for(StringVector::const_iterator it = list.begin(); it!=list.end(); it++)
        {
            String line = *it;
            line.erase((int)line.length()-4,4);
            if(line!=MAP_TEMP)
            {
                SaveFile sf;
                if(!sf.loadAnywhere(line+".map"))continue;
                String buffer = "";
                sf.getSetting("HasPortal",buffer,"Portal");
                if(!StringConverter::parseBool(buffer))continue;

                GateInfo gateInfo;
                gateInfo.mMap = line;
                buffer = "";
                sf.getSetting("Position",buffer,"Portal");
                gateInfo.mPosition = StringConverter::parseVector3(buffer);
                buffer = "";
                sf.getSetting("Yaw",buffer,"Portal");
                gateInfo.mYaw = StringConverter::parseReal(buffer);

                gateList.push_back(gateInfo);
            }
        }
        //Check all .fhm files in Unofficial Terrains for portal
        const StringVector fhzlist = SaveFile::findResourceNames("UnofficialTerrains","*.fhm");
        for(StringVector::const_iterator it = fhzlist.begin(); it!=fhzlist.end(); it++)
        {
            String line = *it;
            line.erase((int)line.length()-4,4);
            SaveFile sf;
            const String fhmFilename = SaveFile::findDirectoryOf(line+".fhm");
            if(fhmFilename=="")continue;
            if(!ZipManager::getSingletonPtr()->defhz(fhmFilename))continue;
            if(!sf.loadZip(line+".map",ZIP_TEMP_DIR+line+".fhm"+String(ZIP_TEMP_EXTENSION)))continue;
            String buffer = "";
            sf.getSetting("HasPortal",buffer,"Portal");
            if(!StringConverter::parseBool(buffer))continue;

            GateInfo gateInfo;
            gateInfo.mMap = line;
            buffer = "";
            sf.getSetting("Position",buffer,"Portal");
            gateInfo.mPosition = StringConverter::parseVector3(buffer);
            buffer = "";
            sf.getSetting("Yaw",buffer,"Portal");
            gateInfo.mYaw = StringConverter::parseReal(buffer);

            gateList.push_back(gateInfo);
        }
        unsigned short i = 0;
        const Real sqrtPoint5 = Math::Sqrt(0.5f);
        for(std::vector<GateInfo>::iterator it=gateList.begin(); it!=gateList.end(); it++)
        {
            const GateInfo gateInfo = *it;

            const Vector3 center = Vector3(7870-(i%2?0:570),775+(i/2)*60,4510+(i/2)*500);
            const Quaternion orientation = Quaternion(sqrtPoint5,0,(i%2?sqrtPoint5:-sqrtPoint5),0);
            Grid* grid = mGridMgr->getGridContaining(center.x,center.z);
            if(!grid)continue;

            GateCollision copy(gateInfo,Vector3::ZERO,70.0f,orientation.zAxis());
            grid->addCollision(mCollMgr->createCollision(&copy,center,Vector3::UNIT_SCALE,orientation,"GateMat/Portal"));

            i++;
        }
    }
    void createGateToGateway(const Vector3 &portalPos, const Real &portalYaw)
    {
        GateInfo gateInfo;
        gateInfo.mMap = MAP_CUSTOMMAPSGATEWAY;
        gateInfo.mPosition = Vector3(7575,595,3430);
        gateInfo.mYaw = 177;

        const Quaternion orientation = Quaternion(Degree(portalYaw),Vector3::UNIT_Y)*Quaternion(Degree(0),Vector3::UNIT_X)*Quaternion(Degree(0),Vector3::UNIT_Z);
        const Vector3 center = portalPos+orientation*Vector3(0,70,-100);
        Grid* grid = mGridMgr->getGridContaining(center.x,center.z);
        if(!grid)return;

        GateCollision copy(gateInfo,Vector3::ZERO,70.0f,orientation.zAxis());
        grid->addCollision(mCollMgr->createCollision(&copy,center,Vector3::UNIT_SCALE,orientation,"GateMat/Portal"));
    }
    static MapManager* getSingletonPtr();
    static MapManager& getSingleton();
    const String getMapName()
    {
        return mMapName;
    }
    void setTerrainHeightmap(const String &mapName, const String &filename)
    {
        SaveFile sf;
        if(!sf.load(MAP_DIR+mapName+"/"+filename) && !sf.loadZip(filename,ZIP_TEMP_DIR+mapName+".fhm"+String(ZIP_TEMP_EXTENSION)))return;
        sf.getSetting("Heightmap.image",mTerrainHeightmap);
    }
    const String getTerrainHeightmap()
    {
        return mTerrainHeightmap;
    }
    const Vector2 getTerrainSize()
    {
        return mTerrainSize;
    }
    const bool getNoFlying()
    {
        return m_bNoFlying;
    }
    const bool isDefaultMap()
    {
        return m_bIsDefault;
    }
};

template<> MapManager* Singleton<MapManager>::ms_Singleton = 0;

MapManager* MapManager::getSingletonPtr()
{
	return ms_Singleton;
}

MapManager& MapManager::getSingleton()
{
	assert(ms_Singleton);
	return *ms_Singleton;
}

#endif
