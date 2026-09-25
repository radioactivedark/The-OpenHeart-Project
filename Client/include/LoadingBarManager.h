#ifndef _LOADINGBARMANAGER_H_
#define _LOADINGBARMANAGER_H_

class LoadingBarManager : public Singleton<LoadingBarManager>
{
private:
    Root *mRoot;
    Gui *mGui;
    GuiPanel *mPanel;
    GuiTextComponent *mTitleText;
    OverlayElement *mBar;
    OverlayElement *mBarMax;
    Real mBarMaxLength;
public:
    LoadingBarManager()
    {
        mPanel = 0;
    }
    ~LoadingBarManager()
    {
        if(mPanel)
        {
            delete mPanel;
        }
    }
    static LoadingBarManager* getSingletonPtr();
    static LoadingBarManager& getSingleton();
    void init()
    {
        mRoot = Root::getSingletonPtr();
        mGui = Gui::getSingletonPtr();
        if(!mPanel)mPanel = new GuiPanel(OverlayManager::getSingleton().getByName("LoadingBarScreen"),true,mGui->mWindowWidth,mGui->mWindowHeight);
        mPanel->show(false);

        mTitleText = mPanel->getTextComponent("LoadingBarScreen/Title");
        mTitleText->setAlignment(GuiTextComponent::ALIGN_CENTER);
        mTitleText->mClickable = false;
        mBar = OverlayManager::getSingleton().getOverlayElement("LoadingBarScreen/Bar");
        mBarMax = OverlayManager::getSingleton().getOverlayElement("LoadingBarScreen/BarMax");
        mBarMaxLength = mBarMax->getWidth();
    }
    void setTitle(const String &title)
    {
        mTitleText->setCaption(title);

        if(mPanel->isVisible())mRoot->renderOneFrame();
    }
    void show()
    {
        mPanel->show(true);

        mRoot->renderOneFrame();
    }
    void progress(const Real &ratio)
    {
        mBar->setWidth(ratio * mBarMaxLength);

        mRoot->renderOneFrame();
    }
    void hide()
    {
        mPanel->show(false);

        mRoot->renderOneFrame();
    }
};

template<> LoadingBarManager* Singleton<LoadingBarManager>::ms_Singleton = 0;

LoadingBarManager* LoadingBarManager::getSingletonPtr()
{
	return ms_Singleton;
}

LoadingBarManager& LoadingBarManager::getSingleton()
{
	assert(ms_Singleton);
	return *ms_Singleton;
}

#endif
