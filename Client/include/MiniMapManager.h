#ifndef _MINIMAPMANAGER_H_
#define _MINIMAPMANAGER_H_

#define MINIMAP_FLASH_DURATION 2.0f

#include <MapManager.h>
#include <CameraManager.h>
#include <UnitManager.h>
#include <ControlManager.h>

class MiniMapManager : public Singleton<MiniMapManager>, public NetworkListener
{
private:
    Gui *mGui;
    MapManager *mMapMgr;
    CameraManager *mCameraMgr;
    UnitManager *mUnitMgr;
    ControlManager *mControlMgr;
    GuiPanel *mMiniMapPanel;
    GuiMap *mMiniMap;

    Unit *mPlayer;
    Vector2 mTerrainSize;
    std::vector<Unit*> mMarkerUnits;
    std::pair<Real,Real> mMapClickedPos;
    Real mMapScale;
    unsigned char mMapMode;

    GuiMapMarker *mFlashMarker;
    Real mFlashTimer;
    Real mFlashInterval;
public:
    MiniMapManager()
    {
        mMapMgr = MapManager::getSingletonPtr();
        mCameraMgr = CameraManager::getSingletonPtr();
        mUnitMgr = UnitManager::getSingletonPtr();
        mControlMgr = ControlManager::getSingletonPtr();
        mMapMode = 0;

        reset();
    }
    ~MiniMapManager()
    {
    }
    static MiniMapManager* getSingletonPtr();
    static MiniMapManager& getSingleton();
    void init()
    {
        mGui = Gui::getSingletonPtr();
        mMiniMapPanel = mGui->addPanel("MiniMapScreen",true);
        mMiniMap = mMiniMapPanel->getMap("MiniMapScreen/Mini");
        mMiniMap->addMarker(0,"GuiMat/MiniMapMarker");
        mMiniMap->setActiveMarker(0);

        const String terrainHeightmap = mMapMgr->getTerrainHeightmap();
        if(terrainHeightmap!="")
        {
            mMiniMap->setMaterial("GuiMat/MiniMap");
            mMiniMap->setMaterialTexture(terrainHeightmap);
            mMiniMap->setMaterial("GuiMat/MiniMap2");
            mMiniMap->setMaterialTexture(terrainHeightmap);
        }
        if(mMapMode==0)mMiniMap->setMaterial("GuiMat/MiniMap");
        else if(mMapMode==1)mMiniMap->setMaterial("GuiMat/MiniMap2");
        else mMiniMapPanel->show(false);

        mMapScale = 2.5f;
        mMiniMap->setMaterialScale(mMapScale);
        mTerrainSize = mMapMgr->getTerrainSize();
    }
    void reset()
    {
        mPlayer = 0;
        mMarkerUnits.clear();
        mFlashMarker = 0;
        mFlashTimer = 0;
        mFlashInterval = 0;
    }
    void update(const Real &timeElapsed)
    {
        if(!mMiniMapPanel->isVisible())return;

        const Vector3 pos = (mCameraMgr->isFreeCamera()||!mPlayer) ? mCameraMgr->getPosition() : mPlayer->getPosition();
        const Vector2 flatPos = convertPosition(pos);
        mMiniMap->setMarkerPosition(0.5f,0.5f);
        mMiniMap->setMaterialScroll(flatPos.x-0.5f,flatPos.y-0.5f);
        mMiniMap->setMarkerRotation(mCameraMgr->getYaw());

        for(std::vector<Unit*>::iterator it=mMarkerUnits.begin(); it!=mMarkerUnits.end(); it++)
        {
            Unit *u = *it;
            const Vector2 uPos = convertPosition(u->getPosition());
            mMiniMap->setMarkerPosition(0.5f+mMapScale*(uPos.x-flatPos.x),0.5f+mMapScale*(uPos.y-flatPos.y),u->mCharID);
        }

        updateFlashMarker(timeElapsed);
    }
    void updateFlashMarker(const Real &timeElapsed)
    {
        if(!mFlashMarker)return;
        mFlashTimer -= timeElapsed;
        if(mFlashTimer<=0)
        {
            mFlashMarker->mMarker->setMaterialName("GuiMat/MiniMapMarker2");
            mFlashMarker = 0;
            return;
        }
        mFlashInterval -= timeElapsed;
        if(mFlashInterval<=0)
        {
            mFlashMarker->mMarker->setMaterialName("GuiMat/MiniMapMarker3");
            mFlashInterval = 0.2f;
        }
        else if(mFlashInterval<=0.1f)
        {
            mFlashMarker->mMarker->setMaterialName("GuiMat/MiniMapMarker2");
        }
    }
    const Vector2 convertPosition(const Vector3 &pos)
    {
        if(mTerrainSize.x>0&&mTerrainSize.y>0)return Vector2(pos.x/mTerrainSize.x, pos.z/mTerrainSize.y);
        return Vector2::ZERO;
    }
    /*void updateCameraToMiniMap()
    {
        const pair<Real,Real> pos = mMiniMap->getMarkerPosition();
        if(mTerrainSize.x>0&&mTerrainSize.y>0)mCameraMgr->setPosition(Vector3(pos.first*mTerrainSize.x,mCameraMgr->getPosition().y,pos.second*mTerrainSize.y));
    }*/
    void setPlayer(Unit *player)
    {
        mPlayer = player;
    }
    GuiPanel* getPanel()
    {
        return mMiniMapPanel;
    }
    void changeEvent(GuiComponent *component, const unsigned char &event)
    {
        if(component==mMiniMap)
        {
            if(event==ChangeListener::EVENT_MOUSEPRESSED)mMapClickedPos = mMiniMap->getMarkerPosition();
            else if(event==ChangeListener::EVENT_MOUSEMOVED)
            {
                const pair<Real,Real> dragPos = mMiniMap->getMarkerPosition();
                const Real dX = (dragPos.first - mMapClickedPos.first)*5;
                const Real dY = (dragPos.second - mMapClickedPos.second)*5;
                mMapClickedPos = dragPos;
                scaleMap((Math::Abs(dX)>Math::Abs(dY)) ? dX : dY);
            }
        }
    }
    void scaleMap(const Real &change)
    {
        mMapScale += change*mMapScale;
        if(mMapScale<1.0f)mMapScale = 1.0f;
        else if(mMapScale>10.0f)mMapScale = 10.0f;
        mMiniMap->setMaterialScale(mMapScale);
    }
    void toggleMode()
    {
        mMapMode += 1;
        if(mMapMode>2)mMapMode = 0;

        if(mMapMode==0)
        {
            mMiniMap->setMaterial("GuiMat/MiniMap");
            mMiniMap->setMaterialScale(mMapScale);
            mMiniMapPanel->show(true);
        }
        else if(mMapMode==1)
        {
            mMiniMap->setMaterial("GuiMat/MiniMap2");
            mMiniMap->setMaterialScale(mMapScale);
        }
        else mMiniMapPanel->show(false);
    }
    void addUnitMarker(Unit *unit)
    {
        mMiniMap->addMarker(unit->mCharID,"GuiMat/MiniMapMarker2");
        mMarkerUnits.push_back(unit);
    }
    void removeUnitMarker(Unit *unit)
    {
        for(std::vector<Unit*>::iterator it=mMarkerUnits.begin(); it!=mMarkerUnits.end(); it++)
        {
            Unit *u = *it;
            if(u==unit)
            {
                if(mFlashMarker==mMiniMap->getMarker(unit->mCharID))mFlashMarker = 0;
                mMiniMap->removeMarker(unit->mCharID);
                mMarkerUnits.erase(it);
                return;
            }
        }
    }
    void friendRemove(const unsigned int &userID)
    {
        Unit *unit = mUnitMgr->getUnitByUserID(userID);
        if(unit)removeUnitMarker(unit);
    }
    void friendListInfo(const unsigned int &userID, const String &username, const bool &isOnline)
    {
        if(!isOnline)return;
        Unit *unit = mUnitMgr->getUnitByUserID(userID);
        if(unit)addUnitMarker(unit);
    }
    void findCharEvent(const String &name, const String &mapName)
    {
        Unit *unit = mUnitMgr->getUnitByUsername(name);
        if(unit)
        {
            if(mFlashMarker)
            {
                mFlashTimer = 0;
                updateFlashMarker(0);
            }
            mFlashMarker = mMiniMap->getMarker(unit->mCharID);
            if(mFlashMarker)
            {
                mFlashTimer = MINIMAP_FLASH_DURATION;
                mFlashInterval = 0;
            }
        }
    }
};

template<> MiniMapManager* Singleton<MiniMapManager>::ms_Singleton = 0;

MiniMapManager* MiniMapManager::getSingletonPtr()
{
	return ms_Singleton;
}

MiniMapManager& MiniMapManager::getSingleton()
{
	assert(ms_Singleton);
	return *ms_Singleton;
}

#endif
