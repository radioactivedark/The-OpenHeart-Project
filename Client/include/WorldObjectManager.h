#ifndef _WORLDOBJECTMANAGER_H_
#define _WORLDOBJECTMANAGER_H_

#define OBJECT_DIR "media/objects/"

#include <Collision.h>
#include <SaveFile.h>
#include <ZipManager.h>

struct WorldSubMesh
{
    String mMesh;
    Vector3 mOffset;
    Vector3 mScale;
    Quaternion mOrientation;
    std::vector<String> mMaterial;
    bool mIsCollidable;
    WorldSubMesh()
    {
        mMesh = "";
        mOffset = Vector3::ZERO;
        mScale = Vector3::UNIT_SCALE;
        mOrientation = Quaternion::IDENTITY;
        mMaterial.clear();
        mIsCollidable = false;
    }
};

struct WorldBillboard
{
    String mMaterial;
    Vector3 mOffset;
    Vector2 mSize;
    WorldBillboard()
    {
        mMaterial = "";
        mOffset = Vector3::ZERO;
        mSize = Vector2::UNIT_SCALE;
    }
};

struct WorldParticle
{
    String mParticle;
    Vector3 mOffset;
    Vector3 mScale;
    Quaternion mOrientation;
    WorldParticle()
    {
        mParticle = "";
        mOffset = Vector3::ZERO;
        mScale = Vector3::UNIT_SCALE;
        mOrientation = Quaternion::IDENTITY;
    }
};

struct WorldLight
{
    Vector3 mOffset;
    ColourValue mColour;
    WorldLight()
    {
        mOffset = Vector3::ZERO;
        mColour = ColourValue(1,1,1,1);
    }
};

class WorldObject
{
public:
    String mName;
    std::vector<WorldSubMesh*> mSubMeshList;
    std::vector<Collision*> mCollList;
    std::vector<WorldBillboard*> mBillboardList;
    std::vector<WorldParticle*> mParticleList;
    std::vector<WorldLight*> mLightList;
    String mSound;
    WorldObject(const String &name)
    {
        mName = name;
        mSubMeshList.clear();
        mCollList.clear();
        mBillboardList.clear();
        mParticleList.clear();
        mLightList.clear();
        mSound = "";
    }
    ~WorldObject()
    {
        while(!mSubMeshList.empty())
        {
            WorldSubMesh *subMesh = mSubMeshList.back();
            mSubMeshList.pop_back();
            delete subMesh;
        }
        while(!mCollList.empty())
        {
            Collision *coll = mCollList.back();
            mCollList.pop_back();
            delete coll;
        }
        while(!mBillboardList.empty())
        {
            WorldBillboard *billboard = mBillboardList.back();
            mBillboardList.pop_back();
            delete billboard;
        }
        while(!mParticleList.empty())
        {
            WorldParticle *particle = mParticleList.back();
            mParticleList.pop_back();
            delete particle;
        }
        while(!mLightList.empty())
        {
            WorldLight *light = mLightList.back();
            mLightList.pop_back();
            delete light;
        }
    }
    void addSubMesh(const String &info)
    {
        WorldSubMesh *subMesh = new WorldSubMesh();
        subMesh->mIsCollidable = (info.length()>0?(info[0]=='%'):false);
        const StringVector part = StringUtil::split(info,";%");

        unsigned short i = 0;
        for(StringVector::const_iterator it=part.begin(); it!=part.end(); it++)
        {
            const String line = *it;
            switch(i)
            {
                case 0: subMesh->mMesh = line; break;
                case 1: subMesh->mOffset = StringConverter::parseVector3(line); break;
                case 2: subMesh->mScale = StringConverter::parseVector3(line); break;
                case 3: subMesh->mOrientation = StringConverter::parseQuaternion(line); break;
                default: subMesh->mMaterial.push_back(line); break;
            }
            i++;
        }

        mSubMeshList.push_back(subMesh);
    }
    void addCollision(const String &info)
    {
        const StringVector part = StringUtil::split(info,";");
        Collision *coll = 0;

        if(part.size()>=3)
        {
            if(part[0]=="b" || part[0]=="wb")coll = new CollBox(StringConverter::parseVector3(part[1]),StringConverter::parseVector3(part[2]));
            else if(part[0]=="s" || part[0]=="ws")coll = new CollSphere(StringConverter::parseVector3(part[1]),StringConverter::parseReal(part[2]));
            else if(part[0]=="c" && part.size()==4)coll = new FlatCircle(StringConverter::parseVector3(part[1]),StringConverter::parseReal(part[2]),StringConverter::parseVector3(part[3]));

            if(part[0]=="wb" || part[0]=="ws")coll->setIsWater(true);
        }

        if(coll)mCollList.push_back(coll);
    }
    void addBillboard(const String &info)
    {
        const StringVector part = StringUtil::split(info,";");
        WorldBillboard *billboard = new WorldBillboard();

        if(part.size()>0)billboard->mMaterial = part[0];
        if(part.size()>1)billboard->mOffset = StringConverter::parseVector3(part[1]);
        if(part.size()>2)billboard->mSize = StringConverter::parseVector2(part[2]);

        mBillboardList.push_back(billboard);
    }
    void addParticle(const String &info)
    {
        const StringVector part = StringUtil::split(info,";");
        WorldParticle *particle = new WorldParticle();

        if(part.size()>0)particle->mParticle = part[0];
        if(part.size()>1)particle->mOffset = StringConverter::parseVector3(part[1]);
        if(part.size()>2)particle->mScale = StringConverter::parseVector3(part[2]);
        if(part.size()>3)particle->mOrientation = StringConverter::parseQuaternion(part[3]);

        mParticleList.push_back(particle);
    }
    void addLight(const String &info)
    {
        const StringVector part = StringUtil::split(info,";");
        WorldLight *light = new WorldLight();

        if(part.size()>0)light->mOffset = StringConverter::parseVector3(part[0]);
        if(part.size()>1)light->mColour = StringConverter::parseColourValue(part[1]);

        mLightList.push_back(light);
    }
    void setSound(const String &sound)
    {
        mSound = sound;
    }
};

class WorldObjectManager : public Singleton<WorldObjectManager>
{
private:
    std::vector<WorldObject*> mWorldObjectList;
    std::vector<String> mLoadedObjectGroups;
public:
    WorldObjectManager()
    {
    }
    ~WorldObjectManager()
    {
        clear();
    }
    void clear()
    {
        clearObjects();
    }
    void clearObjects()
    {
        while(!mWorldObjectList.empty())
        {
            WorldObject *object = mWorldObjectList.back();
            mWorldObjectList.pop_back();
            delete object;
        }
        mLoadedObjectGroups.clear();
    }
    static WorldObjectManager* getSingletonPtr();
    static WorldObjectManager& getSingleton();
    void loadObjects(const String &groupName, const bool &useFhz=true)
    {
        for(std::vector<String>::iterator it=mLoadedObjectGroups.begin(); it!=mLoadedObjectGroups.end(); it++)
        {
            if(groupName==*it)return;
        }

        SaveFile sf;
        /*if(!sf.loadAnywhere(groupName+".object"))
        {
            if(!ZipManager::getSingletonPtr()->defhz(OBJECT_DIR+groupName) || !sf.loadZip(groupName+".object",ZIP_TEMP_DIR+groupName+".tmp"))
            {
                LoggerManager::getSingleton().logMessage("ERROR::WorldObject: failed to load ObjectGroup "+groupName);
                return;
            }
        }*/
        if(useFhz)
        {
            if(!ZipManager::getSingletonPtr()->loadByAnyMeans(&sf,groupName,".fho",".object"))
            {
                LoggerManager::getSingleton().logMessage("ERROR::WorldObject: failed to load ObjectGroup "+groupName);
                return;
            }
        }
        else
        {
            if(!sf.loadAnywhere(groupName+".object"))
            {
                LoggerManager::getSingleton().logMessage("ERROR::WorldObject: failed to load (non .fho) ObjectGroup "+groupName);
                return;
            }
        }

        for(sf.beginSectionIterator(); sf.hasMoreSections(); sf.nextSection())
        {
            WorldObject *object = new WorldObject(groupName + "/" + sf.peekNextSectionName());

            for(sf.beginSettingIterator(); sf.hasMoreSettings(); sf.nextSetting())
            {
                const String tag = sf.getSettingTag();
                if(StringUtil::startsWith(tag,"mesh"))object->addSubMesh(sf.getSettingValue());
                else if(StringUtil::startsWith(tag,"coll"))object->addCollision(sf.getSettingValue());
                else if(StringUtil::startsWith(tag,"billboard"))object->addBillboard(sf.getSettingValue());
                else if(StringUtil::startsWith(tag,"particle"))object->addParticle(sf.getSettingValue());
                else if(StringUtil::startsWith(tag,"light"))object->addLight(sf.getSettingValue());
                else if(StringUtil::startsWith(tag,"sound"))object->setSound(sf.getSettingValue());
            }

            mWorldObjectList.push_back(object);
        }
        mLoadedObjectGroups.push_back(groupName);
    }
    WorldObject* getWorldObject(const String &name)
    {
        for(int i=0;i<(int)mWorldObjectList.size();i++)
            if(mWorldObjectList[i]->mName==name)return mWorldObjectList[i];
        return 0;
    }
};

template<> WorldObjectManager* Singleton<WorldObjectManager>::ms_Singleton = 0;

WorldObjectManager* WorldObjectManager::getSingletonPtr()
{
	return ms_Singleton;
}

WorldObjectManager& WorldObjectManager::getSingleton()
{
	assert(ms_Singleton);
	return *ms_Singleton;
}

#endif
