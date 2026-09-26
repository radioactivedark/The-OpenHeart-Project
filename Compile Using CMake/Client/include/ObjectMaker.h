#ifndef _OBJECTMAKER_H_
#define _OBJECTMAKER_H_

#define OBJECTMAKER_COLL_DEFAULT_SIZE 30
#define OBJECTMAKER_DEFAULT_DIR "media/objects/"
#define OBJECTMAKER_TEMP_GROUP "temp"
#define OBJECTMAKER_CENTER Vector3(2500,0,2500)

#include <Gui.h>
#include <CameraManager.h>
#include <LightManager.h>
#include <SaveFile.h>

struct ObjectMakerMesh
{
    SceneNode *mNode;
    Entity *mMesh;
    String mMeshName;
    bool mNewMat;
    bool mHidden;
    bool mIsCollidable;
    ObjectMakerMesh()
    {
        mNode = 0;
        mMesh = 0;
        mMeshName = "";
        mNewMat = false;
        mHidden = false;
        mIsCollidable = false;
    }
    void setVisible(const bool &flag)
    {
        mNode->setVisible(flag);
        mHidden = !flag;
        if(!flag)highlight(false);
        mMesh->setQueryFlags(flag ? Collision::WORLDOBJECT_MASK : 0);
    }
    void highlight(const bool &flag)
    {
        mNode->showBoundingBox(flag);
    }
    void setPosition(const Vector3 &position)
    {
        mNode->setPosition(position);
    }
    const Vector3 getPosition()
    {
        return mNode->getPosition();
    }
};

struct ObjectMakerColl
{
    SceneNode *mNode;
    Entity *mEnt;
    String mCollType;   //b = box, s = sphere, wb = water box, ws = water sphere
    bool mHidden;
    ObjectMakerColl()
    {
        mNode = 0;
        mEnt = 0;
        mCollType = "";
        mHidden = false;
    }
    void setVisible(const bool &flag)
    {
        mNode->setVisible(flag);
        mHidden = !flag;
        if(!flag)highlight(false);
        mEnt->setQueryFlags(flag ? Collision::WORLDCOLLISION_MASK : 0);
    }
    void highlight(const bool &flag)
    {
        mNode->showBoundingBox(flag);
    }
    void setPosition(const Vector3 &position)
    {
        mNode->setPosition(position);
    }
    const Vector3 getPosition()
    {
        return mNode->getPosition();
    }
};

struct ObjectMakerBillboard
{
    SceneNode *mNode;
    BillboardSet *mSet;
    String mMatName;
    bool mHidden;
    SceneNode *mSelectNode;
    Entity *mSelectBox;
    ObjectMakerBillboard()
    {
        mNode = 0;
        mSet = 0;
        mMatName = "";
        mHidden = false;
        mSelectNode = 0;
        mSelectBox = 0;
    }
    void setVisible(const bool &flag)
    {
        mNode->setVisible(flag);
        mHidden = !flag;
        if(!flag)highlight(false);
        mSelectBox->setQueryFlags(flag ? Collision::WORLDBILLBOARD_MASK : 0);
    }
    void highlight(const bool &flag)
    {
        mSelectNode->showBoundingBox(flag);
    }
    void setPosition(const Vector3 &position)
    {
        mNode->setPosition(position);
    }
    const Vector3 getPosition()
    {
        return mNode->getPosition();
    }
};

struct ObjectMakerParticle
{
    SceneNode *mNode;
    ParticleSystem *mSys;
    String mTemplateName;
    bool mHidden;
    SceneNode *mSelectNode;
    Entity *mSelectBox;
    ObjectMakerParticle()
    {
        mNode = 0;
        mSys = 0;
        mTemplateName = "";
        mHidden = false;
        mSelectNode = 0;
        mSelectBox = 0;
    }
    void setVisible(const bool &flag)
    {
        mNode->setVisible(flag);
        mHidden = !flag;
        if(!flag)highlight(false);
        mSelectBox->setQueryFlags(flag ? Collision::WORLDPARTICLE_MASK : 0);
    }
    void highlight(const bool &flag)
    {
        mSelectNode->showBoundingBox(flag);
    }
    void setPosition(const Vector3 &position)
    {
        mNode->setPosition(position);
    }
    const Vector3 getPosition()
    {
        return mNode->getPosition();
    }
};

struct ObjectMakerLight
{
    Light *mLight;
    SceneNode *mSelectNode;
    Entity *mSelectBox;
    ObjectMakerLight()
    {
        mLight = 0;
        mSelectNode = 0;
        mSelectBox = 0;
    }
    void highlight(const bool &flag)
    {
        mSelectNode->showBoundingBox(flag);
    }
    void setPosition(const Vector3 &position)
    {
        mLight->setPosition(position+OBJECTMAKER_CENTER);
        mSelectNode->setPosition(position);
    }
    const Vector3 getPosition()
    {
        return mSelectNode->getPosition();
    }
};

class ObjectMaker
{
private:
    SceneManager *mSceneMgr;
    Gui *mGui;
    CameraManager *mCameraMgr;
    CollisionManager *mCollMgr;
    LightManager *mLightMgr;
    SceneNode *mRootNode;
    GuiMultiPanel *mPanel;
    GuiMultiPanel *mEffectPanel;

    GuiTextField *mObjectField;
    GuiTextField *mGroupField;
    String mObjectName;
    String mGroupName;
    GuiHighlightList *mGroupList;
    GuiSlider *mGroupListSlider;
    GuiHighlightList *mObjectList;
    GuiSlider *mObjectListSlider;
    GuiTextField *mSoundField;
    String mSoundFilename;

    std::vector<ObjectMakerMesh*> mMeshList;
    ObjectMakerMesh *mCurrentMesh;
    unsigned short mMeshIndex;
    String mMeshName;
    String mMatName;
    Vector3 mMeshPosition;
    Vector3 mMeshScale;
    GuiTextField *mMeshField;
    GuiTextField *mMatField;
    GuiTextField *mMeshPosField[3];
    GuiTextField *mMeshScaleField[3];
    unsigned short mMatPart;
    OverlayElement *mMatPartLabel;
    GuiButton *mMatPartUp;
    GuiButton *mMatPartDn;
    GuiCheckBox *mMeshOwnCollision;

    std::vector<ObjectMakerColl*> mCollList;
    ObjectMakerColl *mCurrentColl;
    unsigned short mCollIndex;
    Vector3 mCollPosition;
    Vector3 mCollScale;
    GuiHighlightList *mCollSelectList;
    GuiTextField *mCollPosField[3];
    GuiTextField *mCollScaleField[3];
    GuiButton *mCollScaleUp[3];
    GuiButton *mCollScaleDn[3];

    std::vector<ObjectMakerBillboard*> mBillboardList;
    ObjectMakerBillboard *mCurrentBillboard;
    unsigned short mBillboardIndex;
    String mBillboardName;
    Vector3 mBillboardPosition;
    Vector2 mBillboardScale;
    GuiTextField *mBillboardField;
    GuiTextField *mBillboardPosField[3];
    GuiTextField *mBillboardScaleField[2];

    std::vector<ObjectMakerParticle*> mParticleList;
    ObjectMakerParticle *mCurrentParticle;
    unsigned short mParticleIndex;
    String mParticleName;
    Vector3 mParticlePosition;
    Vector3 mParticleScale;
    GuiTextField *mParticleField;
    GuiTextField *mParticlePosField[3];
    GuiTextField *mParticleScaleField[3];

    std::vector<ObjectMakerLight*> mLightList;
    ObjectMakerLight *mCurrentLight;
    unsigned short mLightIndex;
    Vector3 mLightPosition;
    ColourValue mLightColour;
    GuiTextField *mLightPosField[3];
    GuiSlider *mLightColourSlider[3];
    GuiTextField *mLightColourField[3];
    OverlayElement *mLightColourBox;

    unsigned short mHeldType;
    bool mHeldDirection;
    Vector3 mHeldValue;
    bool mCollMode; //false = MeshMode; true = CollMode
    bool mBillboardMode;
    bool mParticleMode;
    bool mLightMode;
    Real mSpeedModifier;
    bool mWasTesting;
public:
    bool mShowCollisions;
    ObjectMaker()
    {
        mSceneMgr = 0;
        mCurrentMesh = 0;
        mCurrentColl = 0;
        mCurrentBillboard = 0;
        mCurrentParticle = 0;
        mCurrentLight = 0;
        clear();
    }
    ~ObjectMaker()
    {
        deleteData();
    }
    enum
    {
        HELD_NONE,
        MESH_POSITION,
        MESH_SCALE,
        MESH_ROTATION,
        COLL_POSITION,
        COLL_SCALE,
        BILLBOARD_POSITION,
        BILLBOARD_SCALE,
        PARTICLE_POSITION,
        PARTICLE_SCALE,
        PARTICLE_ROTATION,
        LIGHT_POSITION
    };
    void init(SceneManager *sceneMgr, const bool &load=true)
    {
        mSceneMgr = sceneMgr;
        mGui = Gui::getSingletonPtr();
        mCameraMgr = CameraManager::getSingletonPtr();
        mCollMgr = CollisionManager::getSingletonPtr();
        mCollMgr->setMouseQueryMask(Collision::WORLDOBJECT_MASK);
        mLightMgr = LightManager::getSingletonPtr();

        mRootNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();
        mRootNode->setPosition(OBJECTMAKER_CENTER);

        mPanel = mGui->addMultiPanel("ObjectMakerScreen",true);
        mObjectField = mPanel->getTextField("ObjectMakerScreen/Name");
        mGroupField = mPanel->getTextField("ObjectMakerScreen/Group");
        mObjectField->bindString(&mObjectName);
        mObjectField->mMaxLength = 32;
        mGroupField->bindString(&mGroupName);
        mGroupField->mMaxLength = 32;
        mGroupList = mPanel->getHighlightList("ObjectMakerScreen/GroupSel");
        mGroupListSlider = mPanel->getSlider("ObjectMakerScreen/GroupSelLS");
        mObjectList = mPanel->getHighlightList("ObjectMakerScreen/NameSel");
        mObjectListSlider = mPanel->getSlider("ObjectMakerScreen/NameSelLS");
        mPanel->getCheckBox("ObjectMakerScreen/ShowColl")->setChecked(mShowCollisions);
        mSoundField= mPanel->getTextField("ObjectMakerScreen/Sound");
        mSoundField->bindString(&mSoundFilename);
        mSoundField->mMaxLength = 32;

        if(load)loadAllGroups();

        //Mesh page
        mMeshField = mPanel->getTextField("ObjectMakerScreen/Mesh");
        mMeshField->bindString(&mMeshName);
        mMeshField->mMaxLength = 32;
        mMatField = mPanel->getTextField("ObjectMakerScreen/Material");
        mMatField->bindString(&mMatName);
        mMatField->mMaxLength = 32;
        for(int i=0;i<3;i++)
        {
            mPanel->getButton("ObjectMakerScreen/MeshPosUp"+StringConverter::toString(i))->setHoldable(true);
            mPanel->getButton("ObjectMakerScreen/MeshPosDn"+StringConverter::toString(i))->setHoldable(true);
            mPanel->getButton("ObjectMakerScreen/MeshScaleUp"+StringConverter::toString(i))->setHoldable(true);
            mPanel->getButton("ObjectMakerScreen/MeshScaleDn"+StringConverter::toString(i))->setHoldable(true);
            mPanel->getButton("ObjectMakerScreen/MeshRotUp"+StringConverter::toString(i))->setHoldable(true);
            mPanel->getButton("ObjectMakerScreen/MeshRotDn"+StringConverter::toString(i))->setHoldable(true);

            mMeshPosField[i] = mPanel->getTextField("ObjectMakerScreen/MeshPos"+StringConverter::toString(i));
            mMeshPosField[i]->mMaxLength = 5;
            mMeshScaleField[i] = mPanel->getTextField("ObjectMakerScreen/MeshScale"+StringConverter::toString(i));
            mMeshScaleField[i]->mMaxLength = 5;
        }
        mMatPartLabel = OverlayManager::getSingleton().getOverlayElement("ObjectMakerScreen/MatPartLabel");
        mMatPartUp = mPanel->getButton("ObjectMakerScreen/MatPartUp");
        mMatPartDn = mPanel->getButton("ObjectMakerScreen/MatPartDn");
        mMeshOwnCollision = mPanel->getCheckBox("ObjectMakerScreen/OwnColl");

        //Collision page
        mCollSelectList = mPanel->getHighlightList("ObjectMakerScreen/Coll");
        mCollSelectList->pushLine("Box");
        mCollSelectList->pushLine("Sphere");
        mCollSelectList->pushLine("Water Box");
        mCollSelectList->pushLine("Water Sphere");
        mCollSelectList->update(0);
        for(int i=0;i<3;i++)
        {
            mPanel->getButton("ObjectMakerScreen/CollPosUp"+StringConverter::toString(i))->setHoldable(true);
            mPanel->getButton("ObjectMakerScreen/CollPosDn"+StringConverter::toString(i))->setHoldable(true);
            mCollScaleUp[i] = mPanel->getButton("ObjectMakerScreen/CollScaleUp"+StringConverter::toString(i));
            mCollScaleUp[i]->setHoldable(true);
            mCollScaleDn[i] = mPanel->getButton("ObjectMakerScreen/CollScaleDn"+StringConverter::toString(i));
            mCollScaleDn[i]->setHoldable(true);

            mCollPosField[i] = mPanel->getTextField("ObjectMakerScreen/CollPos"+StringConverter::toString(i));
            mCollPosField[i]->mMaxLength = 5;
            mCollScaleField[i] = mPanel->getTextField("ObjectMakerScreen/CollScale"+StringConverter::toString(i));
            mCollScaleField[i]->mMaxLength = 5;
        }

        mEffectPanel = mGui->addMultiPanel("EffectMakerScreen",true);

        //Billboard page
        mBillboardField = mEffectPanel->getTextField("EffectMakerScreen/Billboard");
        mBillboardField->bindString(&mBillboardName);
        mBillboardField->mMaxLength = 32;
        for(int i=0;i<3;i++)
        {
            mEffectPanel->getButton("EffectMakerScreen/BBPosUp"+StringConverter::toString(i))->setHoldable(true);
            mEffectPanel->getButton("EffectMakerScreen/BBPosDn"+StringConverter::toString(i))->setHoldable(true);
            mBillboardPosField[i] = mEffectPanel->getTextField("EffectMakerScreen/BBPos"+StringConverter::toString(i));
            mBillboardPosField[i]->mMaxLength = 5;
            if(i<2)
            {
                mEffectPanel->getButton("EffectMakerScreen/BBScaleUp"+StringConverter::toString(i))->setHoldable(true);
                mEffectPanel->getButton("EffectMakerScreen/BBScaleDn"+StringConverter::toString(i))->setHoldable(true);
                mBillboardScaleField[i] = mEffectPanel->getTextField("EffectMakerScreen/BBScale"+StringConverter::toString(i));
                mBillboardScaleField[i]->mMaxLength = 5;
            }
        }

        //Particle page
        mParticleField = mEffectPanel->getTextField("EffectMakerScreen/Particle");
        mParticleField->bindString(&mParticleName);
        mParticleField->mMaxLength = 32;
        for(int i=0;i<3;i++)
        {
            mEffectPanel->getButton("EffectMakerScreen/PSPosUp"+StringConverter::toString(i))->setHoldable(true);
            mEffectPanel->getButton("EffectMakerScreen/PSPosDn"+StringConverter::toString(i))->setHoldable(true);
            mEffectPanel->getButton("EffectMakerScreen/PSScaleUp"+StringConverter::toString(i))->setHoldable(true);
            mEffectPanel->getButton("EffectMakerScreen/PSScaleDn"+StringConverter::toString(i))->setHoldable(true);
            mEffectPanel->getButton("EffectMakerScreen/PSRotUp"+StringConverter::toString(i))->setHoldable(true);
            mEffectPanel->getButton("EffectMakerScreen/PSRotDn"+StringConverter::toString(i))->setHoldable(true);

            mParticlePosField[i] = mEffectPanel->getTextField("EffectMakerScreen/PSPos"+StringConverter::toString(i));
            mParticlePosField[i]->mMaxLength = 5;
            mParticleScaleField[i] = mEffectPanel->getTextField("EffectMakerScreen/PSScale"+StringConverter::toString(i));
            mParticleScaleField[i]->mMaxLength = 5;
        }

        //Light page
        mLightColourBox = OverlayManager::getSingleton().getOverlayElement("EffectMakerScreen/LTColourBox");
        for(int i=0;i<3;i++)
        {
            mEffectPanel->getButton("EffectMakerScreen/LTPosUp"+StringConverter::toString(i))->setHoldable(true);
            mEffectPanel->getButton("EffectMakerScreen/LTPosDn"+StringConverter::toString(i))->setHoldable(true);

            mLightPosField[i] = mEffectPanel->getTextField("EffectMakerScreen/LTPos"+StringConverter::toString(i));
            mLightPosField[i]->mMaxLength = 5;
            mLightColourSlider[i] = mEffectPanel->getSlider("EffectMakerScreen/LTColour"+StringConverter::toString(i));
            mLightColourField[i] = mEffectPanel->getTextField("EffectMakerScreen/LTColourF"+StringConverter::toString(i));
            mLightColourField[i]->mMaxLength = 5;
        }

        if(mWasTesting)
        {
            loadObject(OBJECTMAKER_TEMP_GROUP,OBJECTMAKER_TEMP_GROUP);
            mObjectField->setCaption(mObjectName);
            mGroupField->setCaption(mGroupName);
            mWasTesting = false;
        }
        else reset();
    }
    void clear()
    {
        mObjectName = "";
        mGroupName = "";
        mSoundFilename = "";
        mWasTesting = false;
        mShowCollisions = false;
    }
    void deleteData()
    {
        while(!mMeshList.empty())
        {
            ObjectMakerMesh *mesh = mMeshList.back();
            mMeshList.pop_back();
            delete mesh;
        }
        while(!mCollList.empty())
        {
            ObjectMakerColl *coll = mCollList.back();
            mCollList.pop_back();
            delete coll;
        }
        while(!mBillboardList.empty())
        {
            ObjectMakerBillboard *bb = mBillboardList.back();
            mBillboardList.pop_back();
            delete bb;
        }
        while(!mParticleList.empty())
        {
            ObjectMakerParticle *ps = mParticleList.back();
            mParticleList.pop_back();
            delete ps;
        }
        while(!mLightList.empty())
        {
            ObjectMakerLight *lt = mLightList.back();
            mLightList.pop_back();
            delete lt;
        }
        mCurrentMesh = 0;
        mCurrentColl = 0;
        mCurrentBillboard = 0;
        mCurrentParticle = 0;
        mCurrentLight = 0;
    }
    void deleteGui()
    {
        mGui->deletePanel(mPanel);
        mGui->deletePanel(mEffectPanel);
    }
    void clearScene()
    {
        for(std::vector<ObjectMakerMesh*>::iterator it=mMeshList.begin(); it!=mMeshList.end(); it++)
        {
            ObjectMakerMesh *mesh = *it;
            mesh->mNode->detachAllObjects();
            mSceneMgr->destroyEntity(mesh->mMesh);
            mesh->mMesh = 0;
        }
        for(std::vector<ObjectMakerColl*>::iterator it=mCollList.begin(); it!=mCollList.end(); it++)
        {
            ObjectMakerColl *coll = *it;
            coll->mNode->detachAllObjects();
            mSceneMgr->destroyEntity(coll->mEnt);
            coll->mEnt = 0;
        }
        for(std::vector<ObjectMakerBillboard*>::iterator it=mBillboardList.begin(); it!=mBillboardList.end(); it++)
        {
            ObjectMakerBillboard *bb = *it;
            bb->mNode->detachAllObjects();
            mSceneMgr->destroyBillboardSet(bb->mSet);
            bb->mSet = 0;
            bb->mSelectNode->detachAllObjects();
            mSceneMgr->destroyEntity(bb->mSelectBox);
            bb->mSelectBox = 0;
        }
        for(std::vector<ObjectMakerParticle*>::iterator it=mParticleList.begin(); it!=mParticleList.end(); it++)
        {
            ObjectMakerParticle *ps = *it;
            ps->mNode->detachAllObjects();
            mSceneMgr->destroyParticleSystem(ps->mSys);
            ps->mSys = 0;
            ps->mSelectNode->detachAllObjects();
            mSceneMgr->destroyEntity(ps->mSelectBox);
            ps->mSelectBox = 0;
        }
        for(std::vector<ObjectMakerLight*>::iterator it=mLightList.begin(); it!=mLightList.end(); it++)
        {
            ObjectMakerLight *lt = *it;
            mLightMgr->destroyLight(lt->mLight);
            lt->mLight = 0;
            lt->mSelectNode->detachAllObjects();
            mSceneMgr->destroyEntity(lt->mSelectBox);
            lt->mSelectBox = 0;
        }
        mMeshIndex = 0;
        mCollIndex = 0;
        mBillboardIndex = 0;
        mParticleIndex = 0;
        mLightIndex = 0;
        deleteData();
    }
    void reset()
    {
        if(mCurrentMesh)mCurrentMesh->highlight(false);
        mCurrentMesh = 0;
        mMeshPosition = Vector3::ZERO;
        mMeshScale = Vector3::UNIT_SCALE;
        updateMeshPosition();
        updateMeshScale();
        updateMaterialPart();
        mMeshField->setCaption("");
        mMatField->setCaption("");

        if(mCurrentColl)mCurrentColl->highlight(false);
        mCurrentColl = 0;
        mCollPosition = Vector3::ZERO;
        mCollScale = Vector3(OBJECTMAKER_COLL_DEFAULT_SIZE,OBJECTMAKER_COLL_DEFAULT_SIZE,OBJECTMAKER_COLL_DEFAULT_SIZE);
        updateCollPosition();
        updateCollScale();
        mCollSelectList->setSelection(0);

        if(mCurrentBillboard)mCurrentBillboard->highlight(false);
        mCurrentBillboard = 0;
        mBillboardPosition = Vector3::ZERO;
        mBillboardScale = Vector2(OBJECTMAKER_COLL_DEFAULT_SIZE,OBJECTMAKER_COLL_DEFAULT_SIZE);
        updateBillboardPosition();
        updateBillboardScale();
        mBillboardField->setCaption("");

        if(mCurrentParticle)mCurrentParticle->highlight(false);
        mCurrentParticle = 0;
        mParticlePosition = Vector3::ZERO;
        mParticleScale = Vector3::UNIT_SCALE;
        updateParticlePosition();
        updateParticleScale();
        mParticleField->setCaption("");

        if(mCurrentLight)mCurrentLight->highlight(false);
        mCurrentLight = 0;
        mLightPosition = Vector3::ZERO;
        mLightColour = ColourValue(1,1,1,1);
        updateLightPosition();
        updateLightColour();

        mHeldType = HELD_NONE;
        mHeldDirection = false;
        mHeldValue = Vector3::ZERO;
        mCollMode = false;
        mBillboardMode = false;
        mParticleMode = false;
        mLightMode = false;
        mSpeedModifier = 1.0f;

        mPanel->show(true);
        mEffectPanel->show(false);
    }
    void update(const Real &timeElapsed)
    {
        switch(mHeldType)
        {
            case MESH_POSITION:
                mMeshPosition += timeElapsed*50*mHeldValue*(mHeldDirection?1:-1)*mSpeedModifier;
                updateMeshPosition();
                break;
            case MESH_SCALE:
                mMeshScale += timeElapsed*1*mHeldValue*(mHeldDirection?1:-1)*mSpeedModifier;
                updateMeshScale();
                break;
            case MESH_ROTATION: updateMeshRotation(timeElapsed*90*mHeldValue*(mHeldDirection?1:-1)*mSpeedModifier); break;
            case COLL_POSITION:
                mCollPosition += timeElapsed*50*mHeldValue*(mHeldDirection?1:-1)*mSpeedModifier;
                updateCollPosition();
                break;
            case COLL_SCALE:
                mCollScale += timeElapsed*50*mHeldValue*(mHeldDirection?1:-1)*mSpeedModifier;
                updateCollScale();
                break;
            case BILLBOARD_POSITION:
                mBillboardPosition += timeElapsed*50*mHeldValue*(mHeldDirection?1:-1)*mSpeedModifier;
                updateBillboardPosition();
                break;
            case BILLBOARD_SCALE:
                mBillboardScale += timeElapsed*50*Vector2(mHeldValue.x,mHeldValue.y)*(mHeldDirection?1:-1)*mSpeedModifier;
                updateBillboardScale();
                break;
            case PARTICLE_POSITION:
                mParticlePosition += timeElapsed*50*mHeldValue*(mHeldDirection?1:-1)*mSpeedModifier;
                updateParticlePosition();
                break;
            case PARTICLE_SCALE:
                mParticleScale += timeElapsed*1*mHeldValue*(mHeldDirection?1:-1)*mSpeedModifier;
                updateParticleScale();
                break;
            case PARTICLE_ROTATION: updateParticleRotation(timeElapsed*90*mHeldValue*(mHeldDirection?1:-1)*mSpeedModifier); break;
            case LIGHT_POSITION:
                mLightPosition += timeElapsed*50*mHeldValue*(mHeldDirection?1:-1)*mSpeedModifier;
                updateLightPosition();
                break;
            default: break;
        }
    }
    void addNewMesh()
    {
        if(!SaveFile::resourceExists(mMeshName))
        {
            mGui->showAlertBox(mMeshName+" cannot be found!");
            return;
        }

        ObjectMakerMesh *mesh = new ObjectMakerMesh();

        mesh->mMesh = mSceneMgr->createEntity("ObjectMakerMesh"+StringConverter::toString(mMeshIndex++),mMeshName);
        mesh->mMesh->setQueryFlags(Collision::WORLDOBJECT_MASK);
        mesh->mMeshName = mMeshName;
        mesh->mNode = mRootNode->createChildSceneNode();
        mesh->mNode->attachObject(mesh->mMesh);
        mesh->mNode->showBoundingBox(true);

        mMeshList.push_back(mesh);
        if(mCurrentMesh)mCurrentMesh->highlight(false);
        mCurrentMesh = mesh;

        updateMeshPosition();
        updateMeshScale();
        updateMaterialPart();
        mMeshOwnCollision->setChecked(false);
    }
    void updateMeshPosition()
    {
        for(int i=0;i<3;i++)mMeshPosField[i]->setCaption(StringConverter::toString(i==0?mMeshPosition.x:(i==1?mMeshPosition.y:mMeshPosition.z),4));
        if(!mCurrentMesh)return;
        mCurrentMesh->setPosition(mMeshPosition);
    }
    void updateMeshScale()
    {
        for(int i=0;i<3;i++)mMeshScaleField[i]->setCaption(StringConverter::toString(i==0?mMeshScale.x:(i==1?mMeshScale.y:mMeshScale.z),4));
        if(!mCurrentMesh)return;
        mCurrentMesh->mNode->setScale(mMeshScale);
    }
    void updateMeshRotation(const Vector3 &pyr)
    {
        if(!mCurrentMesh)return;
        if(pyr.x!=0)mCurrentMesh->mNode->pitch(Degree(pyr.x),Node::TS_WORLD);
        if(pyr.y!=0)mCurrentMesh->mNode->yaw(Degree(pyr.y),Node::TS_WORLD);
        if(pyr.z!=0)mCurrentMesh->mNode->roll(Degree(pyr.z),Node::TS_WORLD);
    }
    void updateMaterialPart()
    {
        if(!mCurrentMesh)
        {
            mMatPart = 0;
            mMatPartLabel->setCaption("");
            mMatPartUp->show(false);
            mMatPartDn->show(false);
            return;
        }
        mMatPartLabel->setCaption(StringConverter::toString(mMatPart+1));
        mMatPartDn->show(mMatPart>0);
        mMatPartUp->show(mMatPart+1<(int)mCurrentMesh->mMesh->getNumSubEntities());
        if(mMatPart<(int)mCurrentMesh->mMesh->getNumSubEntities())mMatField->setCaption(mCurrentMesh->mMesh->getSubEntity(mMatPart)->getMaterialName());
    }
    void updateMaterial()
    {
        if(!mCurrentMesh || mMatName=="")return;

        if(mMatPart>=(int)mCurrentMesh->mMesh->getNumSubEntities())return;
        if(mCurrentMesh->mMesh->getSubEntity(mMatPart)->getMaterialName()==mMatName)return;
        if(!MaterialManager::getSingletonPtr()->resourceExists(mMatName))
        {
            mGui->showAlertBox(mMatName+" cannot be found!");
            return;
        }

        mCurrentMesh->mMesh->getSubEntity(mMatPart)->setMaterialName(mMatName);
        mCurrentMesh->mNewMat = true;
    }
    void addNewCollision()
    {
        ObjectMakerColl *coll = new ObjectMakerColl();

        const String collMesh[4] = {"CollBox","CollSphere","CollBox","CollSphere"};
        const String collType[4] = {"b","s","wb","ws"};
        const unsigned char selection = (mCollSelectList->getSelectedLine()<4?mCollSelectList->getSelectedLine():0);
        coll->mEnt = mSceneMgr->createEntity("ObjectMakerColl"+StringConverter::toString(mCollIndex++),collMesh[selection]);
        coll->mEnt->setQueryFlags(Collision::WORLDCOLLISION_MASK);
        coll->mEnt->setMaterialName(selection<2?"Collision/Highlight":"Collision/HighlightBlue");
        coll->mNode = mRootNode->createChildSceneNode();
        coll->mNode->attachObject(coll->mEnt);
        coll->mNode->showBoundingBox(true);
        coll->mCollType = collType[selection];

        mCollList.push_back(coll);
        if(mCurrentColl)mCurrentColl->highlight(false);
        mCurrentColl = coll;

        updateCollPosition();
        updateCollScale();
    }
    void updateCollPosition()
    {
        for(int i=0;i<3;i++)mCollPosField[i]->setCaption(StringConverter::toString(i==0?mCollPosition.x:(i==1?mCollPosition.y:mCollPosition.z),4));
        if(!mCurrentColl)return;
        mCurrentColl->setPosition(mCollPosition);
    }
    void updateCollScale()
    {
        for(int i=0;i<3;i++)mCollScaleField[i]->setCaption(StringConverter::toString(i==0?mCollScale.x:(i==1?mCollScale.y:mCollScale.z),4));
        if(!mCurrentColl)return;
        if(mCurrentColl->mCollType=="s" || mCurrentColl->mCollType=="ws")mCurrentColl->mNode->setScale(Vector3(mCollScale.x,mCollScale.x,mCollScale.x));
        else mCurrentColl->mNode->setScale(mCollScale);
    }
    void updateCollScaleButtons()
    {
        const unsigned short selection = mCollSelectList->getSelectedLine();
        const bool isBox = (selection==0 || selection==2);
        for(int i=1;i<3;i++)
        {
            mCollScaleField[i]->show(isBox);
            mCollScaleUp[i]->show(isBox);
            mCollScaleDn[i]->show(isBox);
        }
    }
    const bool updateCollSelectList(const String &selection)
    {
        if(selection=="b"){mCollSelectList->setSelection(0); return true;}
        else if(selection=="s"){mCollSelectList->setSelection(1); return true;}
        else if(selection=="wb"){mCollSelectList->setSelection(2); return true;}
        else if(selection=="ws"){mCollSelectList->setSelection(3); return true;}
        return false;
    }
    void addNewBillboard()
    {
        if(!MaterialManager::getSingletonPtr()->resourceExists(mBillboardName))
        {
            mGui->showAlertBox("Material "+mBillboardName+" cannot be found!");
            return;
        }

        ObjectMakerBillboard *bb = new ObjectMakerBillboard();

        bb->mSet = mSceneMgr->createBillboardSet("ObjectMakerBB"+StringConverter::toString(mBillboardIndex++),1);
        bb->mSet->setMaterialName(mBillboardName);
        bb->mSet->createBillboard(0,0,0);
        bb->mMatName = mBillboardName;
        bb->mNode = mRootNode->createChildSceneNode();
        bb->mNode->attachObject(bb->mSet);

        bb->mSelectBox = mSceneMgr->createEntity("ObjectMakerBBBox"+StringConverter::toString(mBillboardIndex-1),"CollSphere");
        bb->mSelectBox->setQueryFlags(Collision::WORLDBILLBOARD_MASK);
        bb->mSelectBox->setMaterialName("GuiMat/Blank");
        bb->mSelectNode = bb->mNode->createChildSceneNode();
        bb->mSelectNode->attachObject(bb->mSelectBox);


        mBillboardList.push_back(bb);
        if(mCurrentBillboard)mCurrentBillboard->highlight(false);
        mCurrentBillboard = bb;
        mCurrentBillboard->highlight(true);

        updateBillboardPosition();
        updateBillboardScale();
    }
    void updateBillboardPosition()
    {
        for(int i=0;i<3;i++)mBillboardPosField[i]->setCaption(StringConverter::toString(i==0?mBillboardPosition.x:(i==1?mBillboardPosition.y:mBillboardPosition.z),4));
        if(!mCurrentBillboard)return;
        mCurrentBillboard->setPosition(mBillboardPosition);
    }
    void updateBillboardScale()
    {
        for(int i=0;i<2;i++)mBillboardScaleField[i]->setCaption(StringConverter::toString(i==0?mBillboardScale.x:mBillboardScale.y,4));
        if(!mCurrentBillboard)return;
        mCurrentBillboard->mSet->setDefaultDimensions(mBillboardScale.x,mBillboardScale.y);
        mCurrentBillboard->mSelectNode->setScale(0.5f*Vector3(mBillboardScale.x,mBillboardScale.y,mBillboardScale.x));
    }
    void addNewParticle()
    {
        if(!ParticleSystemManager::getSingletonPtr()->getTemplate(mParticleName))
        {
            mGui->showAlertBox(mParticleName+" cannot be found!");
            return;
        }

        ObjectMakerParticle *ps = new ObjectMakerParticle();

        ps->mSys = mSceneMgr->createParticleSystem("ObjectMakerPS"+StringConverter::toString(mParticleIndex++),mParticleName);
        ps->mTemplateName = mParticleName;
        ps->mNode = mRootNode->createChildSceneNode();
        ps->mNode->attachObject(ps->mSys);

        ps->mSelectBox = mSceneMgr->createEntity("ObjectMakerPSBox"+StringConverter::toString(mParticleIndex-1),"CollSphere");
        ps->mSelectBox->setQueryFlags(Collision::WORLDPARTICLE_MASK);
        ps->mSelectBox->setMaterialName("GuiMat/Blank");
        ps->mSelectNode = ps->mNode->createChildSceneNode();
        ps->mSelectNode->attachObject(ps->mSelectBox);
        ps->mSelectNode->setScale(0.5f*OBJECTMAKER_COLL_DEFAULT_SIZE*Vector3::UNIT_SCALE);

        mParticleList.push_back(ps);
        if(mCurrentParticle)mCurrentParticle->highlight(false);
        mCurrentParticle = ps;
        mCurrentParticle->highlight(true);

        updateParticlePosition();
        updateParticleScale();
    }
    void updateParticlePosition()
    {
        for(int i=0;i<3;i++)mParticlePosField[i]->setCaption(StringConverter::toString(i==0?mParticlePosition.x:(i==1?mParticlePosition.y:mParticlePosition.z),4));
        if(!mCurrentParticle)return;
        mCurrentParticle->setPosition(mParticlePosition);
    }
    void updateParticleScale()
    {
        for(int i=0;i<3;i++)mParticleScaleField[i]->setCaption(StringConverter::toString(i==0?mParticleScale.x:(i==1?mParticleScale.y:mParticleScale.z),4));
        if(!mCurrentParticle)return;
        mCurrentParticle->mNode->setScale(mParticleScale);
    }
    void updateParticleRotation(const Vector3 &pyr)
    {
        if(!mCurrentParticle)return;
        if(pyr.x!=0)mCurrentParticle->mNode->pitch(Degree(pyr.x),Node::TS_WORLD);
        if(pyr.y!=0)mCurrentParticle->mNode->yaw(Degree(pyr.y),Node::TS_WORLD);
        if(pyr.z!=0)mCurrentParticle->mNode->roll(Degree(pyr.z),Node::TS_WORLD);
    }
    void addNewLight()
    {
        Light *l = mLightMgr->createLight();
        if(!l)
        {
            mGui->showAlertBox("Cannot create more than 3 lights!");
            return;
        }
        ObjectMakerLight *lt = new ObjectMakerLight();

        lt->mLight = l;
        l->setType(Light::LT_POINT);
        l->setAttenuation(1000,1.0f,0.01f,0);

        lt->mSelectBox = mSceneMgr->createEntity("ObjectMakerLTBox"+StringConverter::toString(mLightIndex++),"CollSphere");
        lt->mSelectBox->setQueryFlags(Collision::WORLDLIGHT_MASK);
        lt->mSelectBox->setMaterialName("Collision/Highlight");
        lt->mSelectNode = mRootNode->createChildSceneNode();
        lt->mSelectNode->attachObject(lt->mSelectBox);
        lt->mSelectNode->setScale(0.5f*OBJECTMAKER_COLL_DEFAULT_SIZE*Vector3::UNIT_SCALE);

        mLightList.push_back(lt);
        if(mCurrentLight)mCurrentLight->highlight(false);
        mCurrentLight = lt;
        mCurrentLight->highlight(true);

        updateLightPosition();
        updateLightColour();
    }
    void updateLightPosition()
    {
        for(int i=0;i<3;i++)mLightPosField[i]->setCaption(StringConverter::toString(i==0?mLightPosition.x:(i==1?mLightPosition.y:mLightPosition.z),4));
        if(!mCurrentLight)return;
        mCurrentLight->setPosition(mLightPosition);
    }
    void updateLightColour()
    {
        for(int i=0;i<3;i++)
        {
            mLightColourSlider[i]->setValue(i==0?mLightColour.r:(i==1?mLightColour.g:mLightColour.b));
            mLightColourField[i]->setCaption(StringConverter::toString(int((i==0?mLightColour.r:(i==1?mLightColour.g:mLightColour.b))*255.0f),3));
        }
        mLightColourBox->setCustomParameter(1,Vector4(mLightColour.r,mLightColour.g,mLightColour.b,mLightColour.a));
        CapabilitiesManager::getSingletonPtr()->assertCustomColour(mLightColourBox,1);
        if(!mCurrentLight)return;
        mCurrentLight->mLight->setDiffuseColour(mLightColour);
    }
    void buttonClicked(GuiComponent *button)
    {
        if(button->nameIs("ObjectMakerScreen/Save"))saveObject();
        else if(button->nameIs("ObjectMakerScreen/Export"))saveObject(false,true);
        else if(button->nameIs("ObjectMakerScreen/ShowColl"))mShowCollisions = static_cast<GuiCheckBox*>(button)->isChecked();
        else if(button->nameIs("ObjectMakerScreen/Load"))loadObject();
        else if(button->nameIs("ObjectMakerScreen/Delete"))deleteObject();
        else if(button->nameIs("ObjectMakerScreen/Clear"))mGui->showAlertBox("Clear scene?","ObjectMaker/Clear",true);
        else if(button->nameIs("ObjectMakerScreen/EffectMaker"))
        {
            mCollMode = false;
            mBillboardMode = true;
            mPanel->show(false);
            mEffectPanel->show(true);
            mCollMgr->setMouseQueryMask(Collision::WORLDBILLBOARD_MASK);
            if(mCurrentMesh)mCurrentMesh->highlight((!mCollMode && !mBillboardMode && !mParticleMode && !mLightMode));
            if(mCurrentColl)mCurrentColl->highlight(mCollMode);
            if(mCurrentBillboard)mCurrentBillboard->highlight(mBillboardMode);
            if(mCurrentParticle)mCurrentParticle->highlight(mParticleMode);
            if(mCurrentLight)mCurrentLight->highlight(mLightMode);
        }
        else if(button==mMeshOwnCollision)
        {
            if(mCurrentMesh)mCurrentMesh->mIsCollidable = mMeshOwnCollision->isChecked();
        }
        else if(button->nameIs("ObjectMakerScreen/MeshNew"))
        {
            if(mCurrentMesh && mCurrentMesh->mNode->getPosition()==Vector3::ZERO && mCurrentMesh->mNode->getScale()==Vector3::UNIT_SCALE)
                mGui->showAlertBox("Create new Mesh?","ObjectMaker/MeshNew",true);
            else
            {
                mMeshPosition = Vector3::ZERO;
                mMeshScale = Vector3::UNIT_SCALE;
                mMatPart = 0;
                addNewMesh();
            }
        }
        else if(button->nameIs("ObjectMakerScreen/MatPartUp"))
        {
            mMatPart++;
            updateMaterialPart();
        }
        else if(button->nameIs("ObjectMakerScreen/MatPartDn"))
        {
            mMatPart--;
            updateMaterialPart();
        }
        else if(button->nameIs("ObjectMakerScreen/MeshRotReset"))
        {
            if(mCurrentMesh)mCurrentMesh->mNode->resetOrientation();
        }
        else if(button->nameIs("ObjectMakerScreen/MeshClone"))
        {
            if(mCurrentMesh)mGui->showAlertBox("Clone current Mesh?","ObjectMaker/MeshClone",true);
            else mGui->showAlertBox("Select a Mesh to clone.");
        }
        else if(button->nameIs("ObjectMakerScreen/MeshUpdate"))
        {
            if(mCurrentMesh)
            {
                if(mCurrentMesh->mMeshName!=mMeshName)
                {
                    if(!ResourceGroupManager::getSingletonPtr()->resourceExists("General",mMeshName))mGui->showAlertBox(mMeshName+" cannot be found!");
                    else
                    {
                        mCurrentMesh->mNode->detachAllObjects();
                        mSceneMgr->destroyEntity(mCurrentMesh->mMesh);
                        mCurrentMesh->mMesh = mSceneMgr->createEntity("ObjectMakerMesh"+StringConverter::toString(mMeshIndex++),mMeshName);
                        mCurrentMesh->mMesh->setQueryFlags(Collision::WORLDOBJECT_MASK);
                        mCurrentMesh->mNode->attachObject(mCurrentMesh->mMesh);
                        mCurrentMesh->mMeshName = mMeshName;
                        updateMaterialPart();
                    }
                }
                else updateMaterial();
            }
            updateMeshPosition();
            updateMeshScale();
        }
        else if(button->nameIs("ObjectMakerScreen/MeshHide"))
        {
            if(mCurrentMesh)
            {
                mCurrentMesh->setVisible(false);
                mCurrentMesh = 0;
            }
        }
        else if(button->nameIs("ObjectMakerScreen/MeshShowAll"))
        {
            for(std::vector<ObjectMakerMesh*>::iterator i=mMeshList.begin(); i!=mMeshList.end(); i++)
            {
                ObjectMakerMesh *mesh = *i;
                mesh->setVisible(true);
            }
        }
        else if(button->nameIs("ObjectMakerScreen/MeshDelete"))
        {
            if(mCurrentMesh)
            {
                mCurrentMesh->highlight(false);
                mCurrentMesh->mNode->detachAllObjects();
                mSceneMgr->destroyEntity(mCurrentMesh->mMesh);
                mCurrentMesh->mMesh = 0;
                for(std::vector<ObjectMakerMesh*>::iterator i=mMeshList.begin(); i!=mMeshList.end(); i++)
                {
                    ObjectMakerMesh *mesh = *i;
                    if(mesh==mCurrentMesh)
                    {
                        mMeshList.erase(i);
                        break;
                    }
                }
                delete mCurrentMesh;
                mCurrentMesh = 0;
            }
            updateMaterialPart();
        }
        else if(button->nameIs("ObjectMakerScreen/CollNew"))
        {
            mCollPosition = Vector3::ZERO;
            mCollScale = Vector3::UNIT_SCALE*OBJECTMAKER_COLL_DEFAULT_SIZE;
            addNewCollision();
        }
        else if(button->nameIs("ObjectMakerScreen/CollClone"))
        {
            if(mCurrentColl)
            {
                mCollPosition = mCurrentColl->mNode->getPosition();
                mCollScale = mCurrentColl->mNode->getScale();
                updateCollSelectList(mCurrentColl->mCollType);
                addNewCollision();
            }
        }
        else if(button->nameIs("ObjectMakerScreen/CollUpdate"))
        {
            if(mCurrentColl)
            {
                mCurrentColl->mNode->detachAllObjects();
                mSceneMgr->destroyEntity(mCurrentColl->mEnt);

                const String collMesh[4] = {"CollBox","CollSphere","CollBox","CollSphere"};
                const String collType[4] = {"b","s","wb","ws"};
                const unsigned char selection = (mCollSelectList->getSelectedLine()<4?mCollSelectList->getSelectedLine():0);
                mCurrentColl->mEnt = mSceneMgr->createEntity("ObjectMakerColl"+StringConverter::toString(mCollIndex++),collMesh[selection]);
                mCurrentColl->mEnt->setQueryFlags(Collision::WORLDCOLLISION_MASK);
                mCurrentColl->mEnt->setMaterialName(selection<2?"Collision/Highlight":"Collision/HighlightBlue");
                mCurrentColl->mNode->attachObject(mCurrentColl->mEnt);
                mCurrentColl->mCollType = collType[selection];
            }
            updateCollPosition();
            updateCollScale();
        }
        else if(button->nameIs("ObjectMakerScreen/CollHide"))
        {
            if(mCurrentColl)
            {
                mCurrentColl->setVisible(false);
                mCurrentColl = 0;
            }
        }
        else if(button->nameIs("ObjectMakerScreen/CollShowAll"))
        {
            for(std::vector<ObjectMakerColl*>::iterator i=mCollList.begin(); i!=mCollList.end(); i++)
            {
                ObjectMakerColl *coll = *i;
                coll->setVisible(true);
            }
        }
        else if(button->nameIs("ObjectMakerScreen/CollDelete"))
        {
            if(mCurrentColl)
            {
                mCurrentColl->highlight(false);
                mCurrentColl->mNode->detachAllObjects();
                mSceneMgr->destroyEntity(mCurrentColl->mEnt);
                mCurrentColl->mEnt = 0;
                for(std::vector<ObjectMakerColl*>::iterator i=mCollList.begin(); i!=mCollList.end(); i++)
                {
                    ObjectMakerColl *coll = *i;
                    if(coll==mCurrentColl)
                    {
                        mCollList.erase(i);
                        break;
                    }
                }
                delete mCurrentColl;
                mCurrentColl = 0;
            }
        }
        else if(button->nameIs("EffectMakerScreen/ObjectMaker"))
        {
            mBillboardMode = false;
            mPanel->show(true);
            mEffectPanel->show(false);
            mCollMgr->setMouseQueryMask(Collision::WORLDOBJECT_MASK);
            if(mCurrentMesh)mCurrentMesh->highlight((!mCollMode && !mBillboardMode && !mParticleMode && !mLightMode));
            if(mCurrentColl)mCurrentColl->highlight(mCollMode);
            if(mCurrentBillboard)mCurrentBillboard->highlight(mBillboardMode);
            if(mCurrentParticle)mCurrentParticle->highlight(mParticleMode);
            if(mCurrentLight)mCurrentLight->highlight(mLightMode);
        }
        else if(button->nameIs("EffectMakerScreen/BBNew"))
        {
            mBillboardPosition = Vector3::ZERO;
            mBillboardScale = Vector2(OBJECTMAKER_COLL_DEFAULT_SIZE,OBJECTMAKER_COLL_DEFAULT_SIZE);
            addNewBillboard();
        }
        else if(button->nameIs("EffectMakerScreen/BBClone"))
        {
            if(mCurrentBillboard)mGui->showAlertBox("Clone current Billboard?","EffectMaker/BBClone",true);
            else mGui->showAlertBox("Select a Billboard to clone.");
        }
        else if(button->nameIs("EffectMakerScreen/BBUpdate"))
        {
            if(mCurrentBillboard)
            {
                if(!MaterialManager::getSingletonPtr()->resourceExists(mBillboardName))mGui->showAlertBox("Material "+mBillboardName+" cannot be found!");
                else
                {
                    mCurrentBillboard->mSet->setMaterialName(mBillboardName);
                    mCurrentBillboard->mMatName = mBillboardName;
                }
            }
            updateBillboardPosition();
            updateBillboardScale();
        }
        else if(button->nameIs("EffectMakerScreen/BBHide"))
        {
            if(mCurrentBillboard)
            {
                mCurrentBillboard->setVisible(false);
                mCurrentBillboard = 0;
            }
        }
        else if(button->nameIs("EffectMakerScreen/BBShowAll"))
        {
            for(std::vector<ObjectMakerBillboard*>::iterator i=mBillboardList.begin(); i!=mBillboardList.end(); i++)
            {
                ObjectMakerBillboard *bb = *i;
                bb->setVisible(true);
            }
        }
        else if(button->nameIs("EffectMakerScreen/BBDelete"))
        {
            if(mCurrentBillboard)
            {
                mCurrentBillboard->highlight(false);
                mCurrentBillboard->mNode->detachAllObjects();
                mSceneMgr->destroyBillboardSet(mCurrentBillboard->mSet);
                mCurrentBillboard->mSet = 0;
                mCurrentBillboard->mSelectNode->detachAllObjects();
                mSceneMgr->destroyEntity(mCurrentBillboard->mSelectBox);
                mCurrentBillboard->mSelectBox = 0;
                for(std::vector<ObjectMakerBillboard*>::iterator i=mBillboardList.begin(); i!=mBillboardList.end(); i++)
                {
                    ObjectMakerBillboard *bb = *i;
                    if(bb==mCurrentBillboard)
                    {
                        mBillboardList.erase(i);
                        break;
                    }
                }
                delete mCurrentBillboard;
                mCurrentBillboard = 0;
            }
        }
        else if(button->nameIs("EffectMakerScreen/PSNew"))
        {
            mParticlePosition = Vector3::ZERO;
            mParticleScale = Vector3::UNIT_SCALE;
            addNewParticle();
        }
        else if(button->nameIs("EffectMakerScreen/PSRotReset"))
        {
            if(mCurrentParticle)mCurrentParticle->mNode->resetOrientation();
        }
        else if(button->nameIs("EffectMakerScreen/PSClone"))
        {
            if(mCurrentParticle)mGui->showAlertBox("Clone current Particle?","EffectMaker/PSClone",true);
            else mGui->showAlertBox("Select a Particle to clone.");
        }
        else if(button->nameIs("EffectMakerScreen/PSUpdate"))
        {
            if(mCurrentParticle && mCurrentParticle->mTemplateName!=mParticleName)
            {
                if(!ParticleSystemManager::getSingletonPtr()->getTemplate(mParticleName))mGui->showAlertBox(mParticleName+" cannot be found!");
                else
                {
                    mCurrentParticle->mNode->detachAllObjects();
                    mSceneMgr->destroyParticleSystem(mCurrentParticle->mSys);
                    mCurrentParticle->mSys = mSceneMgr->createParticleSystem("ObjectMakerPS"+StringConverter::toString(mParticleIndex++),mParticleName);
                    mCurrentParticle->mSys->setQueryFlags(Collision::WORLDPARTICLE_MASK);
                    mCurrentParticle->mNode->attachObject(mCurrentParticle->mSys);
                    mCurrentParticle->mTemplateName = mParticleName;
                }
            }
            updateParticlePosition();
            updateParticleScale();
        }
        else if(button->nameIs("EffectMakerScreen/PSHide"))
        {
            if(mCurrentParticle)
            {
                mCurrentParticle->setVisible(false);
                mCurrentParticle = 0;
            }
        }
        else if(button->nameIs("EffectMakerScreen/PSShowAll"))
        {
            for(std::vector<ObjectMakerParticle*>::iterator i=mParticleList.begin(); i!=mParticleList.end(); i++)
            {
                ObjectMakerParticle *ps = *i;
                ps->setVisible(true);
            }
        }
        else if(button->nameIs("EffectMakerScreen/PSDelete"))
        {
            if(mCurrentParticle)
            {
                mCurrentParticle->highlight(false);
                mCurrentParticle->mNode->detachAllObjects();
                mSceneMgr->destroyParticleSystem(mCurrentParticle->mSys);
                mCurrentParticle->mSys = 0;
                mCurrentParticle->mSelectNode->detachAllObjects();
                mSceneMgr->destroyEntity(mCurrentParticle->mSelectBox);
                mCurrentParticle->mSelectBox = 0;
                for(std::vector<ObjectMakerParticle*>::iterator i=mParticleList.begin(); i!=mParticleList.end(); i++)
                {
                    ObjectMakerParticle *ps = *i;
                    if(ps==mCurrentParticle)
                    {
                        mParticleList.erase(i);
                        break;
                    }
                }
                delete mCurrentParticle;
                mCurrentParticle = 0;
            }
        }
        else if(button->nameIs("EffectMakerScreen/LTNew"))
        {
            mLightPosition = Vector3::ZERO;
            addNewLight();
        }
        else if(button->nameIs("EffectMakerScreen/LTDelete"))
        {
            if(mCurrentLight)
            {
                mCurrentLight->highlight(false);
                mLightMgr->destroyLight(mCurrentLight->mLight);
                mCurrentLight->mLight = 0;
                mCurrentLight->mSelectNode->detachAllObjects();
                mSceneMgr->destroyEntity(mCurrentLight->mSelectBox);
                mCurrentLight->mSelectBox = 0;
                for(std::vector<ObjectMakerLight*>::iterator i=mLightList.begin(); i!=mLightList.end(); i++)
                {
                    ObjectMakerLight *lt = *i;
                    if(lt==mCurrentLight)
                    {
                        mLightList.erase(i);
                        break;
                    }
                }
                delete mCurrentLight;
                mCurrentLight = 0;
            }
        }
    }
    void buttonHeld(GuiComponent *button)
    {
        unsigned short index = 0;
        bool held = false;
        if(button->nameIsIndex("ObjectMakerScreen/MeshPosUp",&index))
        {
            mHeldDirection = true;
            mHeldType = MESH_POSITION;
            held = true;
        }
        else if(button->nameIsIndex("ObjectMakerScreen/MeshPosDn",&index))
        {
            mHeldDirection = false;
            mHeldType = MESH_POSITION;
            held = true;
        }
        else if(button->nameIsIndex("ObjectMakerScreen/MeshScaleUp",&index))
        {
            mHeldDirection = true;
            mHeldType = MESH_SCALE;
            held = true;
        }
        else if(button->nameIsIndex("ObjectMakerScreen/MeshScaleDn",&index))
        {
            mHeldDirection = false;
            mHeldType = MESH_SCALE;
            held = true;
        }
        else if(button->nameIsIndex("ObjectMakerScreen/MeshRotUp",&index))
        {
            mHeldDirection = true;
            mHeldType = MESH_ROTATION;
            held = true;
        }
        else if(button->nameIsIndex("ObjectMakerScreen/MeshRotDn",&index))
        {
            mHeldDirection = false;
            mHeldType = MESH_ROTATION;
            held = true;
        }
        else if(button->nameIsIndex("ObjectMakerScreen/CollPosUp",&index))
        {
            mHeldDirection = true;
            mHeldType = COLL_POSITION;
            held = true;
        }
        else if(button->nameIsIndex("ObjectMakerScreen/CollPosDn",&index))
        {
            mHeldDirection = false;
            mHeldType = COLL_POSITION;
            held = true;
        }
        else if(button->nameIsIndex("ObjectMakerScreen/CollScaleUp",&index))
        {
            mHeldDirection = true;
            mHeldType = COLL_SCALE;
            held = true;
        }
        else if(button->nameIsIndex("ObjectMakerScreen/CollScaleDn",&index))
        {
            mHeldDirection = false;
            mHeldType = COLL_SCALE;
            held = true;
        }
        else if(button->nameIsIndex("EffectMakerScreen/BBPosUp",&index))
        {
            mHeldDirection = true;
            mHeldType = BILLBOARD_POSITION;
            held = true;
        }
        else if(button->nameIsIndex("EffectMakerScreen/BBPosDn",&index))
        {
            mHeldDirection = false;
            mHeldType = BILLBOARD_POSITION;
            held = true;
        }
        else if(button->nameIsIndex("EffectMakerScreen/BBScaleUp",&index))
        {
            mHeldDirection = true;
            mHeldType = BILLBOARD_SCALE;
            held = true;
        }
        else if(button->nameIsIndex("EffectMakerScreen/BBScaleDn",&index))
        {
            mHeldDirection = false;
            mHeldType = BILLBOARD_SCALE;
            held = true;
        }
        else if(button->nameIsIndex("EffectMakerScreen/PSPosUp",&index))
        {
            mHeldDirection = true;
            mHeldType = PARTICLE_POSITION;
            held = true;
        }
        else if(button->nameIsIndex("EffectMakerScreen/PSPosDn",&index))
        {
            mHeldDirection = false;
            mHeldType = PARTICLE_POSITION;
            held = true;
        }
        else if(button->nameIsIndex("EffectMakerScreen/PSScaleUp",&index))
        {
            mHeldDirection = true;
            mHeldType = PARTICLE_SCALE;
            held = true;
        }
        else if(button->nameIsIndex("EffectMakerScreen/PSScaleDn",&index))
        {
            mHeldDirection = false;
            mHeldType = PARTICLE_SCALE;
            held = true;
        }
        else if(button->nameIsIndex("EffectMakerScreen/PSRotUp",&index))
        {
            mHeldDirection = true;
            mHeldType = PARTICLE_ROTATION;
            held = true;
        }
        else if(button->nameIsIndex("EffectMakerScreen/PSRotDn",&index))
        {
            mHeldDirection = false;
            mHeldType = PARTICLE_ROTATION;
            held = true;
        }
        else if(button->nameIsIndex("EffectMakerScreen/LTPosUp",&index))
        {
            mHeldDirection = true;
            mHeldType = LIGHT_POSITION;
            held = true;
        }
        else if(button->nameIsIndex("EffectMakerScreen/LTPosDn",&index))
        {
            mHeldDirection = false;
            mHeldType = LIGHT_POSITION;
            held = true;
        }

        if(held)mHeldValue = Vector3(index==0?1:0,index==1?1:0,index==2?1:0);
    }
    void buttonReleased(GuiComponent *button)
    {
        mHeldType = HELD_NONE;
        mHeldValue = Vector3::ZERO;
    }
    void changeEvent(GuiComponent *component, const unsigned char &event)
    {
        if(component->isList())
        {
            if(component==mCollSelectList)updateCollScaleButtons();
            else if(component==mGroupList)loadGroup();
        }
        else if(component->isTextComponent() && static_cast<GuiTextComponent*>(component)->isEditable())
        {
            GuiTextField *text = static_cast<GuiTextField*>(component);
            unsigned short index = 0;
            if(text->nameIs("ObjectMakerScreen/Material"))updateMaterial();
            else if(text->nameIsIndex("ObjectMakerScreen/MeshPos",&index))
            {
                switch(index)
                {
                    case 0: mMeshPosition.x = StringConverter::parseReal(text->getCaption()); break;
                    case 1: mMeshPosition.y = StringConverter::parseReal(text->getCaption()); break;
                    case 2: mMeshPosition.z = StringConverter::parseReal(text->getCaption()); break;
                    default: break;
                }
                if(mCurrentMesh)updateMeshPosition();
            }
            else if(text->nameIsIndex("ObjectMakerScreen/MeshScale",&index))
            {
                switch(index)
                {
                    case 0: mMeshScale.x = StringConverter::parseReal(text->getCaption()); break;
                    case 1: mMeshScale.y = StringConverter::parseReal(text->getCaption()); break;
                    case 2: mMeshScale.z = StringConverter::parseReal(text->getCaption()); break;
                    default: break;
                }
                if(mCurrentMesh)updateMeshScale();
            }
            else if(text->nameIsIndex("ObjectMakerScreen/CollPos",&index))
            {
                switch(index)
                {
                    case 0: mCollPosition.x = StringConverter::parseReal(text->getCaption()); break;
                    case 1: mCollPosition.y = StringConverter::parseReal(text->getCaption()); break;
                    case 2: mCollPosition.z = StringConverter::parseReal(text->getCaption()); break;
                    default: break;
                }
                if(mCurrentColl)updateCollPosition();
            }
            else if(text->nameIsIndex("ObjectMakerScreen/CollScale",&index))
            {
                switch(index)
                {
                    case 0: mCollScale.x = StringConverter::parseReal(text->getCaption()); break;
                    case 1: mCollScale.y = StringConverter::parseReal(text->getCaption()); break;
                    case 2: mCollScale.z = StringConverter::parseReal(text->getCaption()); break;
                    default: break;
                }
                if(mCurrentColl)updateCollScale();
            }
            else if(text->nameIsIndex("EffectMakerScreen/BBPos",&index))
            {
                switch(index)
                {
                    case 0: mBillboardPosition.x = StringConverter::parseReal(text->getCaption()); break;
                    case 1: mBillboardPosition.y = StringConverter::parseReal(text->getCaption()); break;
                    case 2: mBillboardPosition.z = StringConverter::parseReal(text->getCaption()); break;
                    default: break;
                }
                if(mCurrentBillboard)updateBillboardPosition();
            }
            else if(text->nameIsIndex("EffectMakerScreen/BBScale",&index))
            {
                switch(index)
                {
                    case 0: mBillboardScale.x = StringConverter::parseReal(text->getCaption()); break;
                    case 1: mBillboardScale.y = StringConverter::parseReal(text->getCaption()); break;
                    default: break;
                }
                if(mCurrentBillboard)updateBillboardScale();
            }
            else if(text->nameIsIndex("EffectMakerScreen/PSPos",&index))
            {
                switch(index)
                {
                    case 0: mParticlePosition.x = StringConverter::parseReal(text->getCaption()); break;
                    case 1: mParticlePosition.y = StringConverter::parseReal(text->getCaption()); break;
                    case 2: mParticlePosition.z = StringConverter::parseReal(text->getCaption()); break;
                    default: break;
                }
                if(mCurrentParticle)updateParticlePosition();
            }
            else if(text->nameIsIndex("EffectMakerScreen/PSScale",&index))
            {
                switch(index)
                {
                    case 0: mParticleScale.x = StringConverter::parseReal(text->getCaption()); break;
                    case 1: mParticleScale.y = StringConverter::parseReal(text->getCaption()); break;
                    case 2: mParticleScale.z = StringConverter::parseReal(text->getCaption()); break;
                    default: break;
                }
                if(mCurrentParticle)updateParticleScale();
            }
            else if(text->nameIsIndex("EffectMakerScreen/LTPos",&index))
            {
                switch(index)
                {
                    case 0: mLightPosition.x = StringConverter::parseReal(text->getCaption()); break;
                    case 1: mLightPosition.y = StringConverter::parseReal(text->getCaption()); break;
                    case 2: mLightPosition.z = StringConverter::parseReal(text->getCaption()); break;
                    default: break;
                }
                if(mCurrentLight)updateLightPosition();
            }
            else if(text->nameIsIndex("EffectMakerScreen/LTColourF",&index))
            {
                switch(index)
                {
                    case 0: mLightColour.r = float(StringConverter::parseInt(text->getCaption()))/255.0f; break;
                    case 1: mLightColour.g = float(StringConverter::parseInt(text->getCaption()))/255.0f; break;
                    case 2: mLightColour.b = float(StringConverter::parseInt(text->getCaption()))/255.0f; break;
                    default: break;
                }
                updateLightColour();
            }
        }
        else if(component->isTab())
        {
            if(component->nameIs("ObjectMakerScreen/Page2"))
            {
                mCollMode = false;
                mCollMgr->setMouseQueryMask(Collision::WORLDOBJECT_MASK);
            }
            else if(component->nameIs("ObjectMakerScreen/Page3"))
            {
                mCollMode = true;
                mCollMgr->setMouseQueryMask(Collision::WORLDCOLLISION_MASK);
            }
            else if(component->nameIs("EffectMakerScreen/Page1"))
            {
                mBillboardMode = true;
                mParticleMode = false;
                mLightMode = false;
                mCollMgr->setMouseQueryMask(Collision::WORLDBILLBOARD_MASK);
            }
            else if(component->nameIs("EffectMakerScreen/Page2"))
            {
                mBillboardMode = false;
                mParticleMode = true;
                mLightMode = false;
                mCollMgr->setMouseQueryMask(Collision::WORLDPARTICLE_MASK);
            }
            else if(component->nameIs("EffectMakerScreen/Page3"))
            {
                mBillboardMode = false;
                mParticleMode = false;
                mLightMode = true;
                mCollMgr->setMouseQueryMask(Collision::WORLDLIGHT_MASK);
            }

            if(mCurrentMesh)mCurrentMesh->highlight((!mCollMode && !mBillboardMode && !mParticleMode && !mLightMode));
            if(mCurrentColl)mCurrentColl->highlight(mCollMode);
            if(mCurrentBillboard)mCurrentBillboard->highlight(mBillboardMode);
            if(mCurrentParticle)mCurrentParticle->highlight(mParticleMode);
            if(mCurrentLight)mCurrentLight->highlight(mLightMode);
        }
        else if(component->isSlider())
        {
            GuiSlider *slider = static_cast<GuiSlider*>(component);
            unsigned short index = 0;
            if(slider->nameIsIndex("EffectMakerScreen/LTColour",&index))
            {
                switch(index)
                {
                    case 0: mLightColour.r = slider->getValue(); break;
                    case 1: mLightColour.g = slider->getValue(); break;
                    case 2: mLightColour.b = slider->getValue(); break;
                    default: break;
                }
                updateLightColour();
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
            case KC_TAB:
                selectNext();
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
    void doMousePicking(const Real &x, const Real &y)
    {
        SceneNode *node = 0;
        const pair<bool,Vector3> result = mCollMgr->doMousePicking(mCameraMgr->getCamera(),x,y,&node);
        //Mesh
        if(!mCollMode && !mBillboardMode && !mParticleMode && !mLightMode)
        {
            if(node)
            {
                if(mCurrentMesh && mCurrentMesh->mNode==node)return;

                for(std::vector<ObjectMakerMesh*>::iterator i=mMeshList.begin(); i!=mMeshList.end(); i++)
                {
                    ObjectMakerMesh *mesh = *i;
                    if(mesh->mNode==node)
                    {
                        selectMesh(mesh);
                        break;
                    }
                }
            }
            else if(mCurrentMesh)
            {
                mCurrentMesh->highlight(false);
                mCurrentMesh = 0;
            }
        }
        //Collision
        else if(mCollMode)
        {
            if(node)
            {
                if(mCurrentColl && mCurrentColl->mNode==node)return;

                for(std::vector<ObjectMakerColl*>::iterator i=mCollList.begin(); i!=mCollList.end(); i++)
                {
                    ObjectMakerColl *coll = *i;
                    if(coll->mNode==node)
                    {
                        selectCollision(coll);
                        break;
                    }
                }
            }
            else if(mCurrentColl)
            {
                mCurrentColl->highlight(false);
                mCurrentColl = 0;
            }
        }
        //Billboard
        else if(mBillboardMode)
        {
            if(node)
            {
                if(mCurrentBillboard && mCurrentBillboard->mSelectNode==node)return;

                for(std::vector<ObjectMakerBillboard*>::iterator i=mBillboardList.begin(); i!=mBillboardList.end(); i++)
                {
                    ObjectMakerBillboard *bb = *i;
                    if(bb->mSelectNode==node)
                    {
                        selectBillboard(bb);
                        break;
                    }
                }
            }
            else if(mCurrentBillboard)
            {
                mCurrentBillboard->highlight(false);
                mCurrentBillboard = 0;
            }
        }
        //Particle
        else if(mParticleMode)
        {
            if(node)
            {
                if(mCurrentParticle && mCurrentParticle->mSelectNode==node)return;

                for(std::vector<ObjectMakerParticle*>::iterator i=mParticleList.begin(); i!=mParticleList.end(); i++)
                {
                    ObjectMakerParticle *ps = *i;
                    if(ps->mSelectNode==node)
                    {
                        selectParticle(ps);
                        break;
                    }
                }
            }
            else if(mCurrentParticle)
            {
                mCurrentParticle->highlight(false);
                mCurrentParticle = 0;
            }
        }
        //Light
        //Particle
        else if(mLightMode)
        {
            if(node)
            {
                if(mCurrentLight && mCurrentLight->mSelectNode==node)return;

                for(std::vector<ObjectMakerLight*>::iterator i=mLightList.begin(); i!=mLightList.end(); i++)
                {
                    ObjectMakerLight *lt = *i;
                    if(lt->mSelectNode==node)
                    {
                        selectLight(lt);
                        break;
                    }
                }
            }
            else if(mCurrentLight)
            {
                mCurrentLight->highlight(false);
                mCurrentLight = 0;
            }
        }
    }
    void selectNext()
    {
        //Mesh
        if(!mCollMode && !mBillboardMode && !mParticleMode && !mLightMode)
        {
            if(!mCurrentMesh && !mMeshList.empty())
            {
                selectMesh(mMeshList.back());
                return;
            }
            if(mCurrentMesh && mMeshList.size()==1)
            {
                mCurrentMesh->highlight(false);
                mCurrentMesh = 0;
                return;
            }
            for(std::vector<ObjectMakerMesh*>::iterator i=mMeshList.begin(); i!=mMeshList.end(); i++)
            {
                ObjectMakerMesh *mesh = *i;
                if(mCurrentMesh==mesh)
                {
                    i++;
                    if(i==mMeshList.end())selectMesh(mMeshList.front());
                    else selectMesh(*i);
                    return;
                }
            }
        }
        //Collision
        else if(mCollMode)
        {
            if(!mCurrentColl && !mCollList.empty())
            {
                selectCollision(mCollList.back());
                return;
            }
            if(mCurrentColl && mCollList.size()==1)
            {
                mCurrentColl->highlight(false);
                mCurrentColl = 0;
                return;
            }
            for(std::vector<ObjectMakerColl*>::iterator i=mCollList.begin(); i!=mCollList.end(); i++)
            {
                ObjectMakerColl *coll = *i;
                if(mCurrentColl==coll)
                {
                    i++;
                    if(i==mCollList.end())selectCollision(mCollList.front());
                    else selectCollision(*i);
                    return;
                }
            }
        }
        //Billabord
        else if(mBillboardMode)
        {
            if(!mCurrentBillboard && !mBillboardList.empty())
            {
                selectBillboard(mBillboardList.back());
                return;
            }
            if(mCurrentBillboard && mBillboardList.size()==1)
            {
                mCurrentBillboard->highlight(false);
                mCurrentBillboard = 0;
                return;
            }
            for(std::vector<ObjectMakerBillboard*>::iterator i=mBillboardList.begin(); i!=mBillboardList.end(); i++)
            {
                ObjectMakerBillboard *bb = *i;
                if(mCurrentBillboard==bb)
                {
                    i++;
                    if(i==mBillboardList.end())selectBillboard(mBillboardList.front());
                    else selectBillboard(*i);
                    return;
                }
            }
        }
        //Particle
        else if(mParticleMode)
        {
            if(!mCurrentParticle && !mParticleList.empty())
            {
                selectParticle(mParticleList.back());
                return;
            }
            if(mCurrentParticle && mParticleList.size()==1)
            {
                mCurrentParticle->highlight(false);
                mCurrentParticle = 0;
                return;
            }
            for(std::vector<ObjectMakerParticle*>::iterator i=mParticleList.begin(); i!=mParticleList.end(); i++)
            {
                ObjectMakerParticle *ps = *i;
                if(mCurrentParticle==ps)
                {
                    i++;
                    if(i==mParticleList.end())selectParticle(mParticleList.front());
                    else selectParticle(*i);
                    return;
                }
            }
        }
        //Light
        else if(mLightMode)
        {
            if(!mCurrentLight && !mLightList.empty())
            {
                selectLight(mLightList.back());
                return;
            }
            if(mCurrentLight && mLightList.size()==1)
            {
                mCurrentLight->highlight(false);
                mCurrentLight = 0;
                return;
            }
            for(std::vector<ObjectMakerLight*>::iterator i=mLightList.begin(); i!=mLightList.end(); i++)
            {
                ObjectMakerLight *lt = *i;
                if(mCurrentLight==lt)
                {
                    i++;
                    if(i==mLightList.end())selectLight(mLightList.front());
                    else selectLight(*i);
                    return;
                }
            }
        }
    }
    void selectMesh(ObjectMakerMesh *mesh)
    {
        if(mCurrentMesh)mCurrentMesh->highlight(false);
        mCurrentMesh = 0;
        mMeshPosition = mesh->getPosition();
        mMeshScale = mesh->mNode->getScale();
        updateMeshPosition();
        updateMeshScale();
        mCurrentMesh = mesh;
        mCurrentMesh->highlight(true);
        mMatPart = 0;
        updateMaterialPart();
        mMeshField->setCaption(mCurrentMesh->mMeshName);
        mMeshOwnCollision->setChecked(mCurrentMesh->mIsCollidable);
    }
    void selectCollision(ObjectMakerColl *coll)
    {
        if(mCurrentColl)mCurrentColl->highlight(false);
        mCurrentColl = 0;
        mCollPosition = coll->getPosition();
        mCollScale = coll->mNode->getScale();
        updateCollPosition();
        updateCollScale();
        mCurrentColl = coll;
        mCurrentColl->highlight(true);
        if(mCurrentColl->mCollType=="b")mCollSelectList->setSelection(0);
        else if(mCurrentColl->mCollType=="s")mCollSelectList->setSelection(1);
        updateCollScaleButtons();
    }
    void selectBillboard(ObjectMakerBillboard *bb)
    {
        if(mCurrentBillboard)mCurrentBillboard->highlight(false);
        mCurrentBillboard = 0;
        mBillboardPosition = bb->getPosition();
        mBillboardScale = Vector2(bb->mSet->getDefaultWidth(),bb->mSet->getDefaultHeight());
        updateBillboardPosition();
        updateBillboardScale();
        mCurrentBillboard = bb;
        mCurrentBillboard->highlight(true);
        mBillboardField->setCaption(mCurrentBillboard->mMatName);
    }
    void selectParticle(ObjectMakerParticle *ps)
    {
        if(mCurrentParticle)mCurrentParticle->highlight(false);
        mCurrentParticle = 0;
        mParticlePosition = ps->getPosition();
        mParticleScale = ps->mNode->getScale();
        updateParticlePosition();
        updateParticleScale();
        mCurrentParticle = ps;
        mCurrentParticle->highlight(true);
        mParticleField->setCaption(mCurrentParticle->mTemplateName);
    }
    void selectLight(ObjectMakerLight *lt)
    {
        if(mCurrentLight)mCurrentLight->highlight(false);
        mCurrentLight = 0;
        mLightPosition = lt->getPosition();
        mLightColour = lt->mLight->getDiffuseColour();
        updateLightPosition();
        updateLightColour();
        mCurrentLight = lt;
        mCurrentLight->highlight(true);
    }
    void alertBoxEvent(const String &name, const bool &flag)
    {
        if(name=="ObjectMaker/SaveDefault" || name=="ObjectMaker/SaveDefaultExport")
        {
            if(flag)
            {
                mGroupField->setCaption("Default");
                saveObject(false, name=="ObjectMaker/SaveDefaultExport");
            }
        }
        else if(name=="ObjectMaker/SaveOverwrite" || name=="ObjectMaker/SaveOverwriteExport")
        {
            if(flag)saveObject(true, name=="ObjectMaker/SaveOverwriteExport");
        }
        else if(name=="ObjectMaker/Load")
        {
            if(flag)loadObject(true);
        }
        else if(name=="ObjectMaker/Delete")
        {
            if(flag)deleteObject(true);
        }
        else if(name=="ObjectMaker/Clear")
        {
            if(flag)
            {
                reset();
                clearScene();
            }
        }
        else if(name=="ObjectMaker/MeshNew")
        {
            if(flag)addNewMesh();
        }
        else if(name=="ObjectMaker/MeshClone")
        {
            if(flag && mCurrentMesh)
            {
                mMeshPosition = mCurrentMesh->getPosition();
                mMeshScale = mCurrentMesh->mNode->getScale();
                mMeshField->setCaption(mCurrentMesh->mMeshName);
                ObjectMakerMesh *prevMesh = mCurrentMesh;
                addNewMesh();
                if(mCurrentMesh)
                {
                    mCurrentMesh->mNode->setOrientation(prevMesh->mNode->getOrientation());
                    if(prevMesh->mNewMat)
                    {
                        mCurrentMesh->mNewMat= true;
                        for(int j=0; j<(int)prevMesh->mMesh->getNumSubEntities(); j++)
                        {
                            const String matString = prevMesh->mMesh->getSubEntity(j)->getMaterialName();
                            mCurrentMesh->mMesh->getSubEntity(j)->setMaterialName(matString);
                        }
                        updateMaterialPart();
                    }
                    mCurrentMesh->mIsCollidable = prevMesh->mIsCollidable;
                    mMeshOwnCollision->setChecked(mCurrentMesh->mIsCollidable);
                }
            }
        }
        else if(name=="EffectMaker/BBClone")
        {
            if(flag && mCurrentBillboard)
            {
                mBillboardPosition = mCurrentBillboard->getPosition();
                mBillboardScale = Vector2(mCurrentBillboard->mSet->getDefaultWidth(),mCurrentBillboard->mSet->getDefaultHeight());
                mBillboardField->setCaption(mCurrentBillboard->mMatName);
                addNewBillboard();
            }
        }
        else if(name=="EffectMaker/PSClone")
        {
            if(flag && mCurrentParticle)
            {
                mParticlePosition = mCurrentParticle->getPosition();
                mParticleScale = mCurrentParticle->mNode->getScale();
                mParticleField->setCaption(mCurrentParticle->mTemplateName);
                ObjectMakerParticle *prevParticle = mCurrentParticle;
                addNewParticle();
                if(mCurrentParticle)
                {
                    mCurrentParticle->mNode->setOrientation(prevParticle->mNode->getOrientation());
                }
            }
        }
    }
    void saveObject(const bool &overwrite=false, const bool &doExport=false)
    {
        if(mObjectName=="")
        {
            mGui->showAlertBox("Enter Object name.");
            return;
        }
        if(mGroupName=="")
        {
            mGui->showAlertBox("No Group entered, save in Default Group?","ObjectMaker/SaveDefault"+String(doExport?"Export":""),true);
            return;
        }
        if(mGroupName.find('/')!=string::npos)
        {
            mGui->showAlertBox("Group name cannot contain /");
            return;
        }
        String groupName = mGroupName;
        StringUtil::toLowerCase(groupName);
        if(groupName==OBJECTMAKER_TEMP_GROUP)
        {
            mGui->showAlertBox("Cannot save in Group "+mGroupName+"!");
            return;
        }

        if(!saveObject(mObjectName,mGroupName,overwrite))
        {
            mGui->showAlertBox(mObjectName+" already exists, overwrite?","ObjectMaker/SaveOverwrite"+String(doExport?"Export":""),true);
        }
        else
        {
            if(mGroupName==mGroupList->getSelection())loadGroup();
            else loadAllGroups();

            if(doExport)exportGroup(mGroupName);
        }
    }
    const bool saveObject(const String &objectName, const String &groupName, const bool &overwrite)
    {
        SaveFile sf;
        if(!sf.loadAnywhere(groupName+".object"))sf.load(OBJECTMAKER_DEFAULT_DIR+groupName+".object");
        //Object exists
        if(sf.hasSection(objectName))
        {
            if(!overwrite)
            {
                return false;
            }
            sf.clearSection(objectName);
        }

        //Write sound
        if(mSoundFilename!="")sf.setSetting("sound",mSoundFilename,objectName);
        else sf.deleteSetting("sound",objectName);

        //Write meshes
        unsigned short i = 0;
        for(std::vector<ObjectMakerMesh*>::iterator it=mMeshList.begin(); it!=mMeshList.end(); it++)
        {
            ObjectMakerMesh *mesh = *it;
            String meshString = (mesh->mIsCollidable?"%":"") + mesh->mMeshName + ";";
            meshString += StringConverter::toString(mesh->getPosition()) + ";";
            meshString += StringConverter::toString(mesh->mNode->getScale()) + ";";
            meshString += StringConverter::toString(mesh->mNode->getOrientation()) + ";";
            if(mesh->mNewMat)
            {
                for(int j=0; j<(int)mesh->mMesh->getNumSubEntities(); j++)
                {
                    const String matString = mesh->mMesh->getSubEntity(j)->getMaterialName();
                    meshString += matString + ";";
                }
            }
            sf.setSetting("mesh"+StringConverter::toString(i),meshString,objectName);
            i++;
        }
        //Write collisions
        i = 0;
        for(std::vector<ObjectMakerColl*>::iterator it=mCollList.begin(); it!=mCollList.end(); it++)
        {
            ObjectMakerColl *coll = *it;
            String collString = coll->mCollType + ";";
            collString += StringConverter::toString(coll->getPosition()) + ";";
            if(coll->mCollType=="s" || coll->mCollType=="ws")collString += StringConverter::toString(coll->mNode->getScale().x,4) + ";";
            else collString += StringConverter::toString(coll->mNode->getScale()) + ";";
            sf.setSetting("coll"+StringConverter::toString(i),collString,objectName);
            i++;
        }
        //Write billboards
        i = 0;
        for(std::vector<ObjectMakerBillboard*>::iterator it=mBillboardList.begin(); it!=mBillboardList.end(); it++)
        {
            ObjectMakerBillboard *bb = *it;
            String bbString = bb->mMatName + ";";
            bbString += StringConverter::toString(bb->getPosition()) + ";";
            bbString += StringConverter::toString(bb->mSet->getDefaultWidth()) + " ";
            bbString += StringConverter::toString(bb->mSet->getDefaultHeight());
            sf.setSetting("billboard"+StringConverter::toString(i),bbString,objectName);
            i++;
        }
        //Write particles
        i = 0;
        for(std::vector<ObjectMakerParticle*>::iterator it=mParticleList.begin(); it!=mParticleList.end(); it++)
        {
            ObjectMakerParticle *ps = *it;
            String psString = ps->mTemplateName + ";";
            psString += StringConverter::toString(ps->getPosition()) + ";";
            psString += StringConverter::toString(ps->mNode->getScale()) + ";";
            psString += StringConverter::toString(ps->mNode->getOrientation());
            sf.setSetting("particle"+StringConverter::toString(i),psString,objectName);
            i++;
        }
        //Write lights
        i = 0;
        for(std::vector<ObjectMakerLight*>::iterator it=mLightList.begin(); it!=mLightList.end(); it++)
        {
            ObjectMakerLight *lt = *it;
            String ltString = StringConverter::toString(lt->getPosition()) + ";";
            ltString += StringConverter::toString(lt->mLight->getDiffuseColour());
            sf.setSetting("light"+StringConverter::toString(i),ltString,objectName);
            i++;
        }

        sf.save();
        return true;
    }
    void exportGroup(const String &groupName)
    {
        const String origObjectName = mObjectName;
        const String origGroupName = mGroupName;

        std::vector<pair<String,String> > filenames;
        std::vector<String> meshNames;
        std::vector<String> texNames;
        std::vector<String> materialNames;
        std::vector<String> particleNames;
        std::vector<String> particleScriptNames;
        std::vector<String> soundNames;

        SaveFile sf;
        if(!sf.loadAnywhere(groupName+".object"))
        {
            mGui->queueAlertBox("Cannot find Object Group "+groupName+"!");
            return;
        }
        filenames.push_back(pair<String,String>(SaveFile::findDirectoryOf(groupName+".object"), groupName+".object"));

        //Export each object in group
        for(sf.beginSectionIterator(); sf.hasMoreSections(); sf.nextSection())
        {
            const String objectName = sf.peekNextSectionName();
            if(!loadObject(objectName,groupName))continue;

            //Mesh
            for(std::vector<ObjectMakerMesh*>::iterator it=mMeshList.begin(); it!=mMeshList.end(); it++)
            {
                ObjectMakerMesh *mesh = *it;
                //.mesh
                bool exists = false;
                for(std::vector<String>::iterator jt=meshNames.begin(); jt!=meshNames.end(); jt++)
                {
                    String cmpName = *jt;
                    String cmpName2 = mesh->mMeshName;
                    StringUtil::toLowerCase(cmpName);
                    StringUtil::toLowerCase(cmpName2);
                    if(cmpName==cmpName2)
                    {
                        exists = true;
                        break;
                    }
                }
                if(!exists)meshNames.push_back(mesh->mMeshName);
                //materials/textures
                for(int j=0; j<(int)mesh->mMesh->getNumSubEntities(); j++)
                {
                    MaterialPtr mat = mesh->mMesh->getSubEntity(j)->getMaterial();
                    if(mat.isNull())continue;
                    ZipManager::salvageMaterialAndTextures(mat,&materialNames,&texNames);
                }
            }
            //Billboards
            for(std::vector<ObjectMakerBillboard*>::iterator it=mBillboardList.begin(); it!=mBillboardList.end(); it++)
            {
                ObjectMakerBillboard *bb = *it;
                MaterialPtr mat = bb->mSet->getMaterial();
                if(mat.isNull())continue;
                ZipManager::salvageMaterialAndTextures(mat,&materialNames,&texNames);
            }
            //Particles
            for(std::vector<ObjectMakerParticle*>::iterator it=mParticleList.begin(); it!=mParticleList.end(); it++)
            {
                ObjectMakerParticle *ps = *it;
                //Particle template
                const String particleName = ps->mTemplateName;
                ParticleSystem *pSys = ParticleSystemManager::getSingletonPtr()->getTemplate(particleName);
                if(!pSys)continue;
                bool exists = false;
                for(std::vector<String>::iterator jt=particleNames.begin(); jt!=particleNames.end(); jt++)
                {
                    const String cmpName = *jt;
                    if(cmpName==particleName)
                    {
                        exists = true;
                        break;
                    }
                }
                if(exists)continue;
                particleNames.push_back(particleName);
                //material/textures
                if(MaterialManager::getSingletonPtr()->resourceExists(ps->mSys->getMaterialName()))
                {
                    MaterialPtr mat = MaterialManager::getSingletonPtr()->getByName(ps->mSys->getMaterialName());
                    if(mat.isNull())continue;
                    ZipManager::salvageMaterialAndTextures(mat,&materialNames,&texNames);
                }
                //Script file
                const String scriptName = pSys->getOrigin();
                exists = false;
                for(std::vector<String>::iterator jt=particleScriptNames.begin(); jt!=particleScriptNames.end(); jt++)
                {
                    String cmpName = *jt;
                    String cmpName2 = scriptName;
                    StringUtil::toLowerCase(cmpName);
                    StringUtil::toLowerCase(cmpName2);
                    if(cmpName==cmpName2)
                    {
                        exists = true;
                        break;
                    }
                }
                if(exists)continue;
                particleScriptNames.push_back(scriptName);
            }
            //Sound
            if(mSoundFilename!="")
            {
                bool exists = false;
                for(std::vector<String>::iterator jt=soundNames.begin(); jt!=soundNames.end(); jt++)
                {
                    String cmpName = *jt;
                    String cmpName2 = mSoundFilename;
                    StringUtil::toLowerCase(cmpName);
                    StringUtil::toLowerCase(cmpName2);
                    if(cmpName==cmpName2)
                    {
                        exists = true;
                        break;
                    }
                }
                if(!exists)soundNames.push_back(mSoundFilename);
            }
        }
        //Add to list
        for(std::vector<String>::iterator it=meshNames.begin(); it!=meshNames.end(); it++)
        {
            const String meshName = *it;
            const String filename = SaveFile::findDirectoryOf(meshName);
            if(filename!="")filenames.push_back(pair<String,String>(filename, meshName));
            else
            {
                LoggerManager::getSingleton().logMessage("ERROR::ObjectMaker:exportGroup: Cannot find mesh "+meshName+" for "+groupName+"!");
                mGui->queueAlertBox("Cannot find mesh "+meshName+" for "+groupName+"!");
            }
        }
        for(std::vector<String>::iterator it=texNames.begin(); it!=texNames.end(); it++)
        {
            const String texName = *it;
            const String filename = SaveFile::findDirectoryOf(texName);
            if(filename!="")filenames.push_back(pair<String,String>(filename, texName));
            else
            {
                LoggerManager::getSingleton().logMessage("ERROR::ObjectMaker:exportGroup: Cannot find texture "+texName+" for "+groupName+"!");
                mGui->queueAlertBox("Cannot find texture "+texName+" for "+groupName+"!");
            }
        }
        for(std::vector<String>::iterator it=materialNames.begin(); it!=materialNames.end(); it++)
        {
            const String materialName = *it;
            const String filename = SaveFile::findDirectoryOf(materialName);
            if(filename!="")filenames.push_back(pair<String,String>(filename, materialName));
            else
            {
                LoggerManager::getSingleton().logMessage("ERROR::ObjectMaker:exportGroup: Cannot find material "+materialName+" for "+groupName+"!");
                mGui->queueAlertBox("Cannot find material "+materialName+" for "+groupName+"!");
            }
        }
        std::vector<pair<String,String> > particleScriptlets;
        for(std::vector<String>::iterator it=particleScriptNames.begin(); it!=particleScriptNames.end(); it++)
        {
            String particleScriptName = *it;
            String filename = SaveFile::findDirectoryOf(particleScriptName);
            if(filename=="")
            {
                LoggerManager::getSingleton().logMessage("ERROR::ObjectMaker:exportGroup: Cannot find particleScript "+particleScriptName+" for "+groupName+"!");
                mGui->queueAlertBox("Cannot find particleScript "+particleScriptName+" for "+groupName+"!");
                continue;
            }
            ByteArray bArray;
            bArray.writeFromFile(filename);
            filename.erase((int)filename.length()-9,9);
            particleScriptName.erase((int)particleScriptName.length()-9,9);

            particleScriptlets = bArray.splitScript(filename,".particle","particle_system");
            unsigned short scriptCount = 0;
            for(std::vector<pair<String,String> >::const_iterator jt=particleScriptlets.begin(); jt!=particleScriptlets.end(); jt++)
            {
                const pair<String,String> scriptFilename = *jt;
                //Only include scriptlets that are required
                bool required = false;
                for(std::vector<String>::iterator kt=particleNames.begin(); kt!=particleNames.end(); kt++)
                {
                    const String particleName = *kt;
                    if(scriptFilename.first==particleName)
                    {
                        required = true;
                        particleNames.erase(kt);
                        break;
                    }
                }
                if(required)
                {
                    const String scriptShortname = particleScriptName+"_"+StringConverter::toString(scriptCount)+".particle";
                    filenames.push_back(pair<String,String>(scriptFilename.second, scriptShortname));
                }
                scriptCount++;
            }
        }
        for(std::vector<String>::iterator it=soundNames.begin(); it!=soundNames.end(); it++)
        {
            const String soundName = *it;
            const String filename = SaveFile::findDirectoryOf(soundName);
            if(filename!="")filenames.push_back(pair<String,String>(filename, soundName));
            else
            {
                LoggerManager::getSingleton().logMessage("ERROR::ObjectMaker:exportGroup: Cannot find sound "+soundName+" for "+groupName+"!");
                mGui->queueAlertBox("Cannot find sound "+soundName+" for "+groupName+"!");
            }
        }

        if(ZipManager::getSingletonPtr()->fhz(filenames, EXPORT_DIR+groupName+".fho"))
        {
            mGui->queueAlertBox("Exported object group as "+String(EXPORT_DIR)+groupName+".fho");
        }
        else
        {
            mGui->queueAlertBox("Failed to export object group "+groupName+"!");
        }

        while(!particleScriptlets.empty())
        {
            const pair<String,String> filename = particleScriptlets.back();
            particleScriptlets.pop_back();
            _unlink(filename.second.c_str());
        }

        //Load back current object
        loadObject(origObjectName,origGroupName);
    }
    void loadAllGroups()
    {
        mGroupList->clear();
        const std::vector<String> list = SaveFile::findResourceNames("*.object");
        for(std::vector<String>::const_iterator i = list.begin(); i!=list.end(); i++)
        {
            String line = *i;
            line.erase((int)line.length()-7,7);
            if(line!=OBJECTMAKER_TEMP_GROUP)mGroupList->pushLine(line);
        }
        mGroupList->update(0);
        mGroupListSlider->setValue(0);
    }
    void loadGroup()
    {
        loadGroup(mGroupList->getSelection());
    }
    void loadGroup(const String &groupName)
    {
        SaveFile sf;
        if(!sf.loadAnywhere(groupName+".object"))
        {
            mGui->showAlertBox("Cannot find Object Group "+groupName+"!");
            return;
        }
        mObjectList->clear();
        for(sf.beginSectionIterator(); sf.hasMoreSections(); sf.nextSection())
        {
            mObjectList->pushLine(sf.peekNextSectionName());
        }
        mObjectList->update(0);
        mObjectListSlider->setValue(0);
    }
    void loadObject(const bool &load=false)
    {
        const String objectName = mObjectList->getSelection();
        if(objectName=="")
        {
            mGui->showAlertBox("No object to load!");
            return;
        }
        if(!load)
        {
            mGui->showAlertBox("Load "+objectName+"?","ObjectMaker/Load",true);
            return;
        }

        const String groupName = mGroupList->getSelection();
        mObjectField->setCaption(objectName);
        mGroupField->setCaption(groupName);

        loadObject(objectName,groupName);
    }
    const bool loadObject(const String objectName, const String &groupName)
    {
        clearScene();

        SaveFile sf;
        if(!sf.loadAnywhere(groupName+".object"))return false;

        mSoundField->setCaption("");

        for(sf.beginSectionIterator(); sf.hasMoreSections(); sf.nextSection())
        {
            if(objectName==sf.peekNextSectionName())
            {
                for(sf.beginSettingIterator(); sf.hasMoreSettings(); sf.nextSetting())
                {
                    const String tag = sf.getSettingTag();
                    //Sound
                    if(StringUtil::startsWith(tag,"sound"))mSoundField->setCaption(sf.getSettingValue());
                    //Mesh
                    if(StringUtil::startsWith(tag,"mesh"))
                    {
                        const String dataStr = sf.getSettingValue();
                        const bool isCollidable = (dataStr.length()>0?(dataStr[0]=='%'):false);
                        const StringVector part = StringUtil::split(dataStr,";%");
                        unsigned short partIndex = 0;
                        bool cont = true;
                        for(StringVector::const_iterator it=part.begin(); cont && it!=part.end(); it++)
                        {
                            const String line = *it;
                            switch(partIndex)
                            {
                                case 0:
                                    mMeshName = line;
                                    addNewMesh();
                                    if(!mCurrentMesh)cont = false;
                                    else mCurrentMesh->mIsCollidable = isCollidable;
                                    break;
                                case 1: mCurrentMesh->setPosition(StringConverter::parseVector3(line)); break;
                                case 2: mCurrentMesh->mNode->setScale(StringConverter::parseVector3(line)); break;
                                case 3: mCurrentMesh->mNode->setOrientation(StringConverter::parseQuaternion(line)); break;
                                default:
                                    mCurrentMesh->mNewMat = true;
                                    if(partIndex-4<(int)mCurrentMesh->mMesh->getNumSubEntities())
                                        mCurrentMesh->mMesh->getSubEntity(partIndex-4)->setMaterialName(line);
                                    break;
                            }
                            partIndex++;
                        }
                    }
                    //Collision
                    else if(StringUtil::startsWith(tag,"coll"))
                    {
                        const StringVector part = StringUtil::split(sf.getSettingValue(),";");
                        unsigned short partIndex = 0;
                        for(StringVector::const_iterator it=part.begin(); it!=part.end(); it++)
                        {
                            const String line = *it;
                            switch(partIndex)
                            {
                                case 0:
                                    if(!updateCollSelectList(line))
                                    {
                                        it=part.end();
                                        break;
                                    }
                                    addNewCollision();
                                    break;
                                case 1: mCurrentColl->setPosition(StringConverter::parseVector3(line)); break;
                                case 2:
                                    if(mCurrentColl->mCollType=="s" || mCurrentColl->mCollType=="ws")mCurrentColl->mNode->setScale(Vector3::UNIT_SCALE*StringConverter::parseReal(line));
                                    else mCurrentColl->mNode->setScale(StringConverter::parseVector3(line));
                                    break;
                                default: break;
                            }
                            partIndex++;
                        }
                    }
                    //Billboard
                    else if(StringUtil::startsWith(tag,"billboard"))
                    {
                        const StringVector part = StringUtil::split(sf.getSettingValue(),";");
                        unsigned short partIndex = 0;
                        bool cont = true;
                        for(StringVector::const_iterator it=part.begin(); cont && it!=part.end(); it++)
                        {
                            const String line = *it;
                            switch(partIndex)
                            {
                                case 0:
                                    mBillboardName = line;
                                    addNewBillboard();
                                    if(!mCurrentBillboard)cont = false;
                                    break;
                                case 1: mCurrentBillboard->setPosition(StringConverter::parseVector3(line)); break;
                                case 2:
                                {
                                    const Vector2 scale = StringConverter::parseVector2(line);
                                    mCurrentBillboard->mSet->setDefaultDimensions(scale.x,scale.y);
                                }
                                break;
                                default: break;
                            }
                            partIndex++;
                        }
                    }
                    //Particle
                    else if(StringUtil::startsWith(tag,"particle"))
                    {
                        const StringVector part = StringUtil::split(sf.getSettingValue(),";");
                        unsigned short partIndex = 0;
                        bool cont = true;
                        for(StringVector::const_iterator it=part.begin(); cont && it!=part.end(); it++)
                        {
                            const String line = *it;
                            switch(partIndex)
                            {
                                case 0:
                                    mParticleName = line;
                                    addNewParticle();
                                    if(!mCurrentParticle)cont = false;
                                    break;
                                case 1: mCurrentParticle->setPosition(StringConverter::parseVector3(line)); break;
                                case 2: mCurrentParticle->mNode->setScale(StringConverter::parseVector3(line)); break;
                                case 3: mCurrentParticle->mNode->setOrientation(StringConverter::parseQuaternion(line)); break;
                                default: break;
                            }
                            partIndex++;
                        }
                    }
                    //Light
                    else if(StringUtil::startsWith(tag,"light"))
                    {
                        const StringVector part = StringUtil::split(sf.getSettingValue(),";");
                        unsigned short partIndex = 0;
                        for(StringVector::const_iterator it=part.begin(); it!=part.end(); it++)
                        {
                            const String line = *it;
                            switch(partIndex)
                            {
                                case 0:
                                    addNewLight();
                                    if(mCurrentLight)mCurrentLight->setPosition(StringConverter::parseVector3(line)); break;
                                case 1: if(mCurrentLight)mCurrentLight->mLight->setDiffuseColour(StringConverter::parseColourValue(line)); break;
                                default: break;
                            }
                            partIndex++;
                        }
                    }
                }
                break;
            }
        }
        reset();
        return true;
    }
    void deleteObject(const bool &flag=false)
    {
        const String objectName = mObjectList->getSelection();
        const String groupName = mGroupList->getSelection();
        if(objectName=="")
        {
            mGui->showAlertBox("No object to delete!");
            return;
        }
        if(!flag)
        {
            mGui->showAlertBox("Delete "+objectName+" from Group "+groupName+"?","ObjectMaker/Delete",true);
            return;
        }
        SaveFile sf;
        if(!sf.loadAnywhere(groupName+".object"))return;
        sf.deleteSection(objectName);
        sf.save();
        loadGroup();
    }
    void saveTemp()
    {
        saveObject(OBJECTMAKER_TEMP_GROUP,OBJECTMAKER_TEMP_GROUP,true);
        mWasTesting = true;
    }
};

#endif
