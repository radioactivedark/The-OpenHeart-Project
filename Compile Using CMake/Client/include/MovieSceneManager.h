#ifndef _MOVIESCENEMANAGER_H_
#define _MOVIESCENEMANAGER_H_

#include <UnitManager.h>

class MovieMakerUnit
{
public:
    PlayerData mPlayerData;
    Vector3 mPosition;
    Real mYaw;
    Unit *mUnit;
    MovieMakerUnit(Unit *unit)
    {
        mPlayerData.fromUnit(unit);
        mPosition = unit->getPosition();
        mYaw = unit->getYaw();
        mUnit = unit;
    }
    ~MovieMakerUnit()
    {
    }
    void savePosition()
    {
        if(!mUnit)return;
        mPosition = mUnit->getPosition();
        mYaw = mUnit->getYaw();
    }
};

class MovieSceneManager
{
private:
    Gui *mGui;
    GuiPanel *mPanel;
    GuiButton *mEditCharButton;

    UnitManager *mUnitMgr;
    CameraManager *mCameraMgr;
    ControlManager *mControlMgr;
    MiniMapManager *mMiniMapMgr;

    Unit *mCurrentUnit;
    MovieMakerUnit *mCurrentMovieUnit;
    std::vector<MovieMakerUnit*> mUnits;
public:
    MovieSceneManager()
    {
        clear();
    }
    ~MovieSceneManager()
    {
        clear();
        clearAll();
    }
    enum
    {
        MOVIEMAKER_NONE,
        MOVIEMAKER_NEWCHAR,
        MOVIEMAKER_EDITCHAR
    };
    void init()
    {
        mGui = Gui::getSingletonPtr();
        mUnitMgr = UnitManager::getSingletonPtr();
        mCameraMgr = CameraManager::getSingletonPtr();
        mControlMgr = ControlManager::getSingletonPtr();
        mMiniMapMgr = MiniMapManager::getSingletonPtr();

        mPanel = mGui->addMultiPanel("MovieMakerMainScreen");

        mEditCharButton = mPanel->getButton("MovieMakerMainScreen/EditChar");
        mEditCharButton->show(false);

        mPanel->show(false);
    }
    void clear()
    {
        mCurrentUnit = 0;

        //Save unit positions
        for(std::vector<MovieMakerUnit*>::iterator it=mUnits.begin(); it!=mUnits.end(); it++)
        {
            MovieMakerUnit *movieUnit = *it;
            movieUnit->savePosition();
            movieUnit->mUnit = 0;
        }
    }
    void clearAll()
    {
        while(!mUnits.empty())
        {
            MovieMakerUnit *unit = mUnits.back();
            mUnits.pop_back();
            delete unit;
        }
    }
    void addUnit(PlayerData &data)
    {
        Unit *unit = mUnitMgr->createUnit(data, data.mName);

        if(data.mPreset && data.mName=="")unit->setPreset(data.mPreset);
        unit->setPosition(mCameraMgr->getPosition());
        mUnitMgr->initUnitPosition(unit);
        setCurrentUnit(unit);

        MovieMakerUnit *movieUnit = new MovieMakerUnit(unit);
        mUnits.push_back(movieUnit);
    }
    void setCurrentUnit(Unit *unit)
    {
        if(mCurrentUnit==unit)return;
        mCurrentUnit = unit;

        mUnitMgr->setPlayer(unit);
        mControlMgr->setPlayer(unit);
        mControlMgr->setControlMode(ControlManager::CONTROL_FIXED);
        mControlMgr->updateCameraScale();
        mMiniMapMgr->setPlayer(unit);
    }
    void remakeAllUnits()
    {
        for(std::vector<MovieMakerUnit*>::iterator it=mUnits.begin(); it!=mUnits.end(); it++)
        {
            MovieMakerUnit *movieUnit = *it;
            Unit *unit = mUnitMgr->createUnit(movieUnit->mPlayerData, movieUnit->mPlayerData.mName);
            if(movieUnit->mPlayerData.mPreset && movieUnit->mPlayerData.mName=="")unit->setPreset(movieUnit->mPlayerData.mPreset);
            unit->setPosition(movieUnit->mPosition + Vector3(0,10,0));
            unit->setYaw(movieUnit->mYaw);
            mUnitMgr->initUnitPosition(unit,true);

            movieUnit->mUnit = unit;
        }
    }
    const unsigned char buttonClicked(GuiComponent *button)
    {
        if(button==mEditCharButton)return MOVIEMAKER_EDITCHAR;
        else if(button->nameIs("MovieMakerMainScreen/NewChar"))return MOVIEMAKER_NEWCHAR;

        return MOVIEMAKER_NONE;
    }
    GuiPanel* getPanel()
    {
        return mPanel;
    }
};

#endif
