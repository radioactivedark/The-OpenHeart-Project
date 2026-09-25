#ifndef _UNITMANAGER_H_
#define _UNITMANAGER_H_

#define EARSHOT_SQUARED 4000000
#define UNIT_FAR_SQUARED 9000000
#define PRESET_DIR "presets/"

#include <Unit.h>
#include <CollisionManager.h>
#include <WaterManager.h>
#include <NameTagManager.h>
#include <ZipManager.h>

class PlayerData
{
public:
    String mName;
    char mSpecies;
    std::vector<unsigned char> mMesh;
    std::vector<unsigned char> mMat;
    char mScale[NUM_SCALABLES];
    unsigned char mEmote;
    unsigned char mColour[NUM_COLOURABLES][3];
    unsigned char mPreset;
    unsigned char m_nEquip;
    String m_szItems;
    PlayerData()
    {
        mName = "";
        mSpecies = 'f';
        for(int i=0;i<NUM_SCALABLES;i++)mScale[i] = 0;
        mEmote = 0;
        mPreset = 0;
        m_nEquip = 0;
        m_szItems = "";
    }
    PlayerData(Unit *unit)
    {
        fromUnit(unit);
    }
    ~PlayerData()
    {
        mMesh.clear();
        mMat.clear();
    }
    const String getMeshName(const unsigned char &part)
    {
        if(part>(int)mMesh.size() || part>=MAX_MESHES)return "";
        const String mesh[] = {"body","tail","mane","ears","tuft"};
        String name = "";
        name.push_back(mSpecies);
        name += mesh[part];
        name += StringConverter::toString(mMesh[part]);
        name += ".mesh";
        return name;
    }
    const Vector4 getColours(const unsigned char &part)
    {
        if(part>=NUM_COLOURABLES)return Vector4();
        return Vector4(float(mColour[part][0])/255,float(mColour[part][1])/255,float(mColour[part][2])/255,1);
    }
    void fromUnit(Unit *unit)
    {
        mName = unit->mName;
        mMesh.clear();
        mMat.clear();
        const std::vector<Entity*> ent = unit->getEntities();
        for(int i=0;i<5;i++)
        {
            if((int)ent.size()>i && ent[i])
            {
                String mesh = ent[i]->getMesh()->getName();
                if(i==0&&mesh.length()>0)mSpecies = mesh[0];
                if(mesh.length()>10)
                {
                    mesh.erase(0,5);
                    mesh.erase(mesh.length()-5,5);
                    mMesh.push_back(StringConverter::parseInt(mesh));
                }
            }
            else mMesh.push_back(0);
        }
        mMat = unit->getMarkings();
        const std::vector<char> scales = unit->getScales();
        if(!scales.empty())
        {
            for(int i=0;i<NUM_SCALABLES;i++)mScale[i] = scales[i];
        }
        mEmote = unit->getDefaultEmote();
        if(!mMat.empty())
        {
            const std::vector<Vector4> colours = unit->getColours(mMat[1],mMat[2],mMat[3]);
            if(!colours.empty())
            {
                for(int i=0;i<NUM_COLOURABLES;i++)
                    for(int j=0;j<3;j++)mColour[i][j] = int(colours[i][j]*255);
            }
        }
        mPreset = unit->getPreset();
        m_nEquip = 0;
        m_szItems = unit->getItems();
    }
    void copyData(const PlayerData &copy)
    {
        mName = copy.mName;
        mSpecies = copy.mSpecies;
        mMesh = copy.mMesh;
        mMat = copy.mMat;
        for(int i=0;i<NUM_SCALABLES;i++)mScale[i] = copy.mScale[i];
        mEmote = copy.mEmote;
        for(int i=0;i<NUM_COLOURABLES;i++)
            for(int j=0;j<3;j++)mColour[i][j] = copy.mColour[i][j];
        mPreset = copy.mPreset;
        m_nEquip = copy.m_nEquip;
        m_szItems = copy.m_szItems;
    }
};

class UnitManager : public Singleton<UnitManager>, public ObjectListener
{
private:
    SceneManager *mSceneMgr;
    CameraManager *mCameraMgr;
    CollisionManager *mCollMgr;
    WaterManager *mWaterMgr;
    NameTagManager *mNameTagMgr;
    std::vector<Unit*> mUnits;
    Unit *mPlayer;

    void deleteUnit(Unit *unit)
    {
        mNameTagMgr->deleteNameTagsOf(unit);
        delete unit;
    }
public:
    UnitManager()
    {
        reset();
    }
    ~UnitManager()
    {
        reset();
    }
    static UnitManager* getSingletonPtr();
    static UnitManager& getSingleton();
    void reset()
    {
        while(!mUnits.empty())
        {
            Unit *unit = mUnits.back();
            mUnits.pop_back();
            if(unit)deleteUnit(unit);
        }
        mCollMgr = 0;
        mPlayer = 0;
    }
    void init(SceneManager *sMgr)
    {
        mSceneMgr = sMgr;
        mCameraMgr = CameraManager::getSingletonPtr();
        mCollMgr = CollisionManager::getSingletonPtr();
        mWaterMgr = WaterManager::getSingletonPtr();
        mNameTagMgr = NameTagManager::getSingletonPtr();
    }
    void update(const Real &timeElapsed)
    {
        const Vector3 camPos = mCameraMgr->getPosition();
        for(std::vector<Unit*>::iterator i=mUnits.begin(); i!=mUnits.end(); i++)
        {
            Unit *unit = *i;
            if(unit)
            {
                unit->preUpdate(timeElapsed);
                if(unit->mIsLocal)
                {
                    mCollMgr->updateCollision(unit,true);
                    mCollMgr->updateGroundHeights(unit);
                    mWaterMgr->updateWaterCollision(unit);
                }
                else
                {
                    const Real fRemoteHeight = unit->getRemoteGroundHeight();
                    //Update actual ground height if in earshot
                    if(fRemoteHeight>=0.0f)
                    {
                        if(unit->mIsInEarshot)
                        {
                            const Vector3 cPos = unit->getPosition();

                            unit->setGroundHeight(mCollMgr->getGroundHeight(Vector3(cPos.x,fRemoteHeight,cPos.z),true,fRemoteHeight+unit->getHeadHeight(),false,1.0f));
                        }
                        //Use remotely provided groundheight for far units
                        else
                        {
                            unit->setGroundHeight(fRemoteHeight);
                        }
                    }

                    mCollMgr->updateNSGroundHeights(unit, unit->mIsInEarshot);
                    const Real average = (unit->getGroundHeight(0)+unit->getGroundHeight(1))*0.5f;
                    //Keep unit higher than groundheight provided remotely
                    if(unit->getGroundHeight()<average)unit->setGroundHeight(average);
                }
                unit->update(timeElapsed);

                //Update distances
                if(!unit->mIsInvisible)
                {
                    const Real distFromCamSq = camPos.squaredDistance(unit->getPosition());
                    unit->mIsInEarshot = (distFromCamSq<EARSHOT_SQUARED);
                    unit->setVisible(distFromCamSq<UNIT_FAR_SQUARED);
                }
            }
        }
    }
    Unit* createUnit(PlayerData &data, const String &username="")
    {
        /*unsigned short index = 0;
        bool found = false;
        for(std::vector<Unit*>::iterator i=mUnits.begin(); i!=mUnits.end(); i++)
        {
            Unit *unit = *i;
            if(!unit)
            {
                found = true;
                break;
            }
            index++;
        }
        if(!found)index = 0;*/

        Unit *unit = new Unit();
        unit->create(mSceneMgr,/*index?index:(int)mUnits.size(),*/true);
        initUnit(unit,data,username);
        unit->addListener(this);
        /*if(index)mUnits[index] = unit;
        else*/ mUnits.push_back(unit);
        return unit;
    }
    void initUnit(Unit *unit, PlayerData &data, String username="")
    {
        unit->mName = data.mName;
        unit->mUsername = username;
        for(int i=0;i<(int)data.mMesh.size();i++)
        {
            const String meshName = (data.mMesh[i]>0?data.getMeshName(i):"");
            const bool hasMesh = (meshName!="" && ResourceGroupManager::getSingletonPtr()->resourceExists("General",meshName));
            unit->addMesh(hasMesh?meshName:"");
        }
        unit->setDefaultEmote(data.mEmote);
        unit->initUnit();
        unit->setScales(data.mScale);
        unit->setEquip(data.m_nEquip);
        bool hasPreset = false;
        if(data.mPreset>0 && username!="")
        {
            //Replace username illegal script symbols
            fixUsernameForPreset(username);
            if(!unit->setPreset(data.mPreset,username))
            {
                if(ZipManager::getSingletonPtr()->defhz(PRESET_DIR+username+"_"+StringConverter::toString(data.mPreset)+".fhp") && unit->setPreset(data.mPreset,username))hasPreset = true;
            }
            else hasPreset = true;
        }
        if(!hasPreset && !data.mMat.empty())
        {
            String speciesStr = "";
            speciesStr.push_back(data.mSpecies);
            StringUtil::toUpperCase(speciesStr);
            unit->setMaterial(speciesStr,data.mMat[0]);
            for(int i=0;i<3;i++)unit->setMarkings(speciesStr,data.mMat[1+i],i);
            for(int i=0;i<NUM_COLOURABLES;i++)unit->setColour(i,data.getColours(i),(i==8&&data.mMat[1]),(i==8&&data.mMat[2]),(i==8&&data.mMat[3]));
        }
        switch(data.mSpecies)
        {
            default:
            case 'f':
                unit->setFeetDistances(5,5);
                unit->setCallType(Unit::CALL_ROAR);
                break;
            case 'c':
                unit->setFeetDistances(5,5);
                unit->setCallType(Unit::CALL_HOWL);
                break;
        }
        if(unit->mName!="")mNameTagMgr->createNameTag(unit,unit->mName);
        if(username!="")mNameTagMgr->createNameTag(unit,username,0.9f,1.0f);
        unit->addItems(data.m_szItems,data.mSpecies);

        unit->setQueryFlags(Collision::UNIT_MASK);
    }
    void initUnitPosition(Unit *unit, const bool &clampToGround=false)
    {
        if(unit->mIsLocal)mCollMgr->updateCollision(unit);
        mCollMgr->updateGroundHeights(unit);
        if(clampToGround)
        {
            const Vector3 pos = unit->getPosition();
            unit->setPosition(Vector3(pos.x,unit->getGroundHeight(),pos.z));
        }
        unit->updateMovement(0);
    }
    void remakeUnit(Unit* unit, PlayerData &data)
    {
        const Real animTime = unit->getPrimaryAnimTime();
        unit->removeAllItems();
        unit->uninitUnit();
        unit->resetUnit();
        unit->clearAnims();
        unit->removeEquip();
        unit->removeMeshes();
        initUnit(unit,data);
        unit->setPrimaryAnimTime(animTime);
    }
    void destroyUnit(Unit *unit)
    {
        for(std::vector<Unit*>::iterator it=mUnits.begin(); it!=mUnits.end(); it++)
        {
            Unit *u = *it;
            if(u && u==unit)
            {
                deleteUnit(unit);
                mUnits.erase(it);
                return;
            }
        }
    }
    void destroyAllExceptPlayer()
    {
        for(std::vector<Unit*>::iterator it=mUnits.begin(); it!=mUnits.end(); )
        {
            Unit *unit = *it;
            if(unit && unit!=mPlayer)
            {
                deleteUnit(unit);
                it = mUnits.erase(it);
                continue;
            }
            it++;
        }
    }
    /*Controllable* getUnit(const unsigned short &index)
    {
        if(index>=mUnits.size() || !mUnits[index])return 0;
        return mUnits[index];
    }*/
    Unit* getUnitByCharID(const unsigned int &id)
    {
        for(std::vector<Unit*>::iterator i=mUnits.begin(); i!=mUnits.end(); i++)
        {
            Unit *unit = *i;
            if(unit&&unit->mCharID==id)return unit;
        }
        return 0;
    }
    Unit* getUnitByUserID(const unsigned int &id)
    {
        for(std::vector<Unit*>::iterator i=mUnits.begin(); i!=mUnits.end(); i++)
        {
            Unit *unit = *i;
            if(unit&&unit->mUserID==id)return unit;
        }
        return 0;
    }
    Unit* getUnitByUsername(const String &name)
    {
        for(std::vector<Unit*>::iterator i=mUnits.begin(); i!=mUnits.end(); i++)
        {
            Unit *unit = *i;
            if(unit&&unit->mUsername==name)return unit;
        }
        return 0;
    }
    Unit* getUnitBySceneNode(SceneNode *node)
    {
        for(std::vector<Unit*>::iterator i=mUnits.begin(); i!=mUnits.end(); i++)
        {
            Unit *unit = *i;
            if(unit&&unit->getSceneNode()==node)return unit;
        }
        return 0;
    }
    const bool objectEvent(ObjectListenerObject *object, const unsigned char &event)
    {
        if(object->getType()!=Object::OBJECTTYPE_UNIT)return false;
        Unit *unit = static_cast<Unit*>(object);

        switch(event)
        {
            case ObjectListener::OBJECT_INVISIBLE:
                unit->setQueryFlags(0);
                break;
            case ObjectListener::OBJECT_VISIBLE:
                unit->setQueryFlags(Collision::UNIT_MASK);
                break;
            default: break;
        }

        return true;
    }
    const bool controllableEvent(ObjectListenerObject *object, const unsigned char &event)
    {
        if(object->getType()!=Object::OBJECTTYPE_UNIT)return false;
        Unit *unit = static_cast<Unit*>(object);

        switch(event)
        {
            case ObjectListener::CONTROLLABLE_STARTED:
                unit->mIsMoving = true;
                break;
            case ObjectListener::CONTROLLABLE_STOPPED:
                unit->mIsMoving = false;
                break;
            default: break;
        }

        return true;
    }
    const bool unitEvent(ObjectListenerObject *object, const unsigned char &event)
    {
        if(object->getType()!=Object::OBJECTTYPE_UNIT)return false;
        return true;
    }
    void setPlayer(Unit *unit)
    {
        mPlayer = unit;
    }
    void fixUsernameForPreset(String &username)
    {
        for(int i=0;i<(int)username.length();i++)
        {
            if(username[i]==' '||username[i]=='{'||username[i]=='}'||username[i]=='('||username[i]==')'||username[i]=='/')username[i] = '_';
        }
    }
};

template<> UnitManager* Singleton<UnitManager>::ms_Singleton = 0;

UnitManager* UnitManager::getSingletonPtr()
{
	return ms_Singleton;
}

UnitManager& UnitManager::getSingleton()
{
	assert(ms_Singleton);
	return *ms_Singleton;
}

#endif
