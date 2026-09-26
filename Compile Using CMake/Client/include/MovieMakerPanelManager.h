#ifndef _MOVIEMAKERPANELMANAGER_H_
#define _MOVIEMAKERPANELMANAGER_H_

#include <MovieMaker.h>
#include <MiniMapManager.h>
#include <MovieSceneManager.h>

class MovieMakerPanelManager
{
private:
    Gui *mGui;
    GuiPanel *mMovieMakerPanel;
    GuiPanel *mMoviePanel;
    GuiButton *mMovieButton;
    GuiPanel *mMiniMapPanel;
    GuiButton *mMiniMapButton;
    GuiPanel *mScenePanel;
    GuiButton *mSceneButton;
public:
    MovieMakerPanelManager()
    {
    }
    ~MovieMakerPanelManager()
    {
    }
    void init(MovieSceneManager *movieSceneMgr)
    {
        mGui = Gui::getSingletonPtr();
        mMovieMakerPanel = mGui->addPanel("MovieMakerButtonsScreen",true);
        mMovieButton = mMovieMakerPanel->getButton("MovieMakerButtonsScreen/Movie");
        mMovieButton->setOwnMaterials("GuiMat/ButtonMovieUp","GuiMat/ButtonMovieOver","GuiMat/ButtonMovieDown");
        mMiniMapButton = mMovieMakerPanel->getButton("MovieMakerButtonsScreen/MiniMap");
        mMiniMapButton->setOwnMaterials("GuiMat/ButtonMiniMapUp","GuiMat/ButtonMiniMapOver","GuiMat/ButtonMiniMapDown","GuiMat/ButtonMiniMapHighlight");
        mSceneButton = mMovieMakerPanel->getButton("MovieMakerButtonsScreen/Scene");
        mSceneButton->setOwnMaterials("GuiMat/ButtonInfoUp","GuiMat/ButtonInfoOver","GuiMat/ButtonInfoDown");

        mMoviePanel = MovieMaker::getSingletonPtr()->getPanel();
        mMiniMapPanel = MiniMapManager::getSingletonPtr()->getPanel();
        mScenePanel = movieSceneMgr->getPanel();
    }
    void buttonClicked(GuiComponent *button)
    {
        if(button==mMovieButton)mMoviePanel->toggleVisibility();
        else if(button==mMiniMapButton)MiniMapManager::getSingletonPtr()->toggleMode();
        else if(button==mSceneButton)mScenePanel->toggleVisibility();
    }
};

#endif

