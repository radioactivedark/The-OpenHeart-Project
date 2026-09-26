#ifndef _MOVIEMAKERSTATE_H_
#define _MOVIEMAKERSTATE_H_

#include <GameState.h>
#include <ControlManager.h>
#include <MovieMaker.h>
#include <MovieMakerPanelManager.h>

class MovieMakerState : public GameState
{
protected:
    ControlManager *mControlMgr;
    GridManager *mGridMgr;
    MapManager *mMapMgr;
    MiniMapManager *mMiniMapMgr;
    MovieMaker *mMovieMaker;
    MovieMakerPanelManager *mMovieMakerPanelMgr;
    MovieSceneManager *mMovieSceneMgr;
    PlayerData mPlayerData;
    bool mIsCreating;
    bool mHasDeleted;
    bool mEntered;

    Vector3 mCamPosition;
    Real mCamPitch;
    Real mCamYaw;
public:
    MovieMakerState()
    {
        mControlMgr = ControlManager::getSingletonPtr();
        mGridMgr = GridManager::getSingletonPtr();
        mMapMgr = MapManager::getSingletonPtr();
        mMiniMapMgr = MiniMapManager::getSingletonPtr();
        mMovieMaker = MovieMaker::getSingletonPtr();
        mMovieSceneMgr = new MovieSceneManager();
        mMovieMakerPanelMgr = new MovieMakerPanelManager();
        mPlayerData = PlayerData();
        mIsCreating = false;
        mHasDeleted = false;
        mEntered = false;

        mCamPosition = Vector3::ZERO;
        mCamPitch = 0;
        mCamYaw = 0;
    }
    ~MovieMakerState()
    {
        exit();
        delete mMovieMakerPanelMgr;
        delete mMovieSceneMgr;
    }
    void enter()
    {
        init();
        mGui->hideCursorOnRightClick(true);

        mMapMgr->init(mSceneMgr,"WaterMill");
        mMiniMapMgr->init();
        mMovieMaker->init();
        mMovieSceneMgr->init();
        mMovieMakerPanelMgr->init(mMovieSceneMgr);
        mControlMgr->init(0,0);

        if(!mEntered)
        {
            mCameraMgr->setPosition(Vector3(4855,146,5346));
        }
        else
        {
            mCameraMgr->setPosition(mCamPosition);
            mCameraMgr->setPitchYaw(mCamPitch,mCamYaw);
        }

        if(mHasDeleted)
        {
        }

        if(mIsCreating)
        {
            mMovieSceneMgr->remakeAllUnits();
            mMovieSceneMgr->addUnit(mPlayerData);
        }

        mEntered = true;
    }
    void exit()
    {
        mMovieSceneMgr->clear();
        mMovieMaker->clear();
        mMiniMapMgr->reset();
        mControlMgr->reset();
        mMapMgr->reset();
        uninit();
    }
    void update(const Real &timeElapsed)
    {
        mControlMgr->update(timeElapsed);
        mUnitMgr->update(timeElapsed);
        mControlMgr->updateCamera();
        mCameraMgr->update(timeElapsed);
        mGridMgr->update(mCameraMgr->getPosition());
        mSkyMgr->update(timeElapsed);
        mWaterMgr->update(mCameraMgr->getPosition());
        mSoundMgr->update(mCameraMgr->getPosition(),mCameraMgr->getDirection());
        mMovieMaker->update(timeElapsed);
        mMiniMapMgr->update(timeElapsed);
        //mGui->getPanelByName("DebugScreen")->getComponentByName("DebugScreen/Debug")->setCaption(mSkyMgr->getWeatherManager()->mDebug);

        mRoot->renderOneFrame();
    }
    const bool keyPressed(const char &index)
    {
        using namespace KeyboardIndex;
        switch(index)
        {
            case KB_OPTIONS: mGui->showAlertBox("Exit Movie Maker?","MovieMaker/Exit",true); break;
            default: break;
        }
        mControlMgr->keyPressed(index);
        return true;
    }
    const bool keyReleased(const char &index)
    {
        mControlMgr->keyReleased(index);
        return true;
    }
    bool buttonClicked(GuiComponent *button)
    {
        if(StringUtil::startsWith(button->getName(),"MovieMakerButtons",false))mMovieMakerPanelMgr->buttonClicked(button);
        else if(StringUtil::startsWith(button->getName(),"MovieMakerMain",false))
        {
            const unsigned char result = mMovieSceneMgr->buttonClicked(button);
            switch(result)
            {
                case MovieSceneManager::MOVIEMAKER_NEWCHAR:
                    {
                        changeState = true;
                        CharCreateState *state = new CharCreateState();
                        state->setCreatingPtr(&mIsCreating);
                        state->setDeletingPtr(&mHasDeleted);
                        state->setOfflineMode(true);
                        state->setReturnedPlayerDataPtr(&mPlayerData);
                        mNextState = state;

                        mCamPosition = mCameraMgr->getPosition();
                        mCamPitch = mCameraMgr->getPitch();
                        mCamYaw = mCameraMgr->getYaw();
                    }
                    break;
                case MovieSceneManager::MOVIEMAKER_EDITCHAR:
                    break;
            }
        }
        else mMovieMaker->buttonClicked(button);
        return true;
    }
    bool mouseMoved(const OIS::MouseEvent &arg)
    {
        if(arg.state.buttonDown(OIS::MB_Right))
        {
            mCameraMgr->yaw(-0.75f*arg.state.X.rel);
            mCameraMgr->pitch(0.75f*arg.state.Y.rel);
        }
        else if(arg.state.buttonDown(OIS::MB_Middle) && !mCameraMgr->isFreeCamera())mCameraMgr->zoom(0.005f*arg.state.Y.rel);

        return true;
    }
    bool changeEvent(GuiComponent *component, const unsigned char &event)
    {
        if(StringUtil::startsWith(component->getName(),"minimap"))mMiniMapMgr->changeEvent(component,event);
        else if(StringUtil::startsWith(component->getName(),"movie"))mMovieMaker->changeEvent(component,event);
        return true;
    }
    bool alertBoxEvent(const String &name, const bool &flag)
    {
        if(name=="MovieMaker/Exit")
        {
            if(flag)changeState = true;
        }
        else mMovieMaker->alertBoxEvent(name,flag);
        return true;
    }
};

#endif

