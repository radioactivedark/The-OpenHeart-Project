#ifndef _GUI_H_
#define _GUI_H_

#define ZORDER_MAX 490
#define ZORDER_QUOTA 5

#include <GuiMultiPanel.h>

struct GuiCursor
{
    OverlayElement *mCursor;
    Real mX;
    Real mY;
    bool mShowCursor;
    bool mHideOnRightClick;
    bool mShallowHide;          //Hide but don't clamp
    GuiCursor(OverlayElement *cursor)
    {
        mCursor = cursor;
        mX = mY = 0.5;
        mShowCursor = true;
        mHideOnRightClick = false;
        mShallowHide = false;
    }
    void show(const bool &flag)
    {
        mShowCursor = flag;
        if(!mShallowHide)
        {
            if(flag)mCursor->show();
            else mCursor->hide();
        }
    }
    void shallowHide(const bool &flag)
    {
        mShallowHide = flag;
        if(flag)mCursor->hide();
        else if(mShowCursor)mCursor->show();
    }
};

class Gui : public Singleton<Gui>
{
private:
    GuiPanel *mComboBox;
    GuiHighlightList *mComboList;
    String mComboBoxName;
    GuiPanel *mAlertBox;
    String mAlertBoxName;
    std::list<String> mAlertBoxQueue;
    GuiTextComponent *mAlertBoxText;
    OverlayElement *mAlertBoxOK;
    OverlayElement *mAlertBoxCancel;
    GuiPanel **mActivePanel;
    GuiCursor *mCursor;
    bool mHidden;
public:
    std::vector<GuiPanel*> mPanels;
    //bool showCursor;
    //bool hideCursorOnRightClick;
    //Real *mCursorX;
    //Real *mCursorY;
    const unsigned int *mWindowWidth;
    const unsigned int *mWindowHeight;
    String mAvgFPS;
    String mCurFPS;
    String mBestFPS;
    String mWorstFPS;
    String mTriCount;
    String mBatchCount;

    Gui(GuiCursor *cursor, GuiPanel **activePanel, const unsigned int *windowWidth, const unsigned int *windowHeight)
    : mWindowWidth(windowWidth), mWindowHeight(windowHeight)
    {
        mCursor = cursor;
        mActivePanel = activePanel;

        mComboBox = new GuiPanel(OverlayManager::getSingleton().getByName("ComboBox"),true,mWindowWidth,mWindowHeight);
        mComboList = mComboBox->getHighlightList("ComboBox/Combo");
        mComboBoxName = "";

        mAlertBox = new GuiPanel(OverlayManager::getSingleton().getByName("AlertBox"),true,mWindowWidth,mWindowHeight);
        mAlertBoxText = mAlertBox->getTextComponent("AlertBox/Alert");
        mAlertBoxText->setAlignment(GuiTextComponent::ALIGN_CENTER);
        mAlertBoxText->mClickable = false;
        mAlertBoxOK = OverlayManager::getSingleton().getOverlayElement("AlertBox/OKButton");
        mAlertBoxCancel = OverlayManager::getSingleton().getOverlayElement("AlertBox/CancelButton");
        mAlertBoxName = "";

        clear();
    }
    ~Gui()
    {
        clear();
        delete mComboBox;
        delete mAlertBox;
    }
    void windowResized()
    {
        mAlertBoxText->windowResized();
        for(std::vector<GuiPanel*>::iterator it = mPanels.begin(); it!=mPanels.end(); it++)
        {
            GuiPanel *panel = *it;
            panel->windowResized();
        }
    }
    void clear()
    {
        mAlertBoxQueue.clear();
        mComboBox->show(false);
        mAlertBox->show(false);
        while(!mPanels.empty())
        {
            GuiPanel *panel = mPanels.back();
            mPanels.pop_back();
            delete panel;
        }
        mCursor->show(true);
        mCursor->mHideOnRightClick = false;
        mHidden = false;
    }
    GuiPanel* addPanel(const String &name, const bool &anchored=false)
    {
        Overlay *overlay = OverlayManager::getSingleton().getByName(name);
        if(!overlay)return 0;
        Overlay::Overlay2DElementsIterator it = overlay->get2DElementsIterator();
        if(!it.hasMoreElements())return 0;
        GuiPanel *panel = StringUtil::endsWith(it.getNext()->getName(),"multipanel")
                        ? new GuiMultiPanel(overlay,anchored,mWindowWidth,mWindowHeight)
                        : new GuiPanel(overlay,anchored,mWindowWidth,mWindowHeight);
        panel->setZOrder(ZORDER_MAX-(int)mPanels.size()*ZORDER_QUOTA);
        mPanels.push_back(panel);
        panel->show(true);

        return panel;
    }
    GuiMultiPanel* addMultiPanel(const String &name, const bool &anchored=false)
    {
        GuiPanel *panel = addPanel(name,anchored);
        if(!panel)return 0;
        if(!panel->isMultiPanel())
        {
            throw(Exception(9,"GuiPanel '"+name+"' is not a MultiPanel",""));
            return 0;
        }
        return static_cast<GuiMultiPanel*>(panel);
    }
    void deletePanel(GuiPanel *panel)
    {
        for(std::vector<GuiPanel*>::iterator it=mPanels.begin(); it!=mPanels.end(); it++)
        {
            GuiPanel *p = *it;
            if(p == panel)
            {
                mPanels.erase(it);
                delete panel;
                return;
            }
        }
    }
    static Gui* getSingletonPtr();
    static Gui& getSingleton();
    GuiPanel* getPanelAt(const Real &x, const Real &y, int *pos=0)
    {
        if(mAlertBox->isInArea(x,y))return mAlertBox;
        for(std::vector<GuiPanel*>::iterator it = mPanels.begin(); it!=mPanels.end(); it++)
        {
            GuiPanel *panel = *it;
            if(panel->isInArea(x,y))
            {
                return panel;
            }
            if(pos)*pos += 1;
        }
        return 0;
    }
    GuiPanel* getPanelByName(const String &name, int *pos=0, const bool &throwException=true)
    {
        for(std::vector<GuiPanel*>::iterator it = mPanels.begin(); it!=mPanels.end(); it++)
        {
            GuiPanel *panel = *it;
            if(panel->getName()==name)return panel;
            if(pos)*pos += 1;
        }
        if(throwException)throw(Exception(9,"No GuiPanel by name '"+name+"'",""));
        return 0;
    }
    GuiPanel* getMultiPanelByName(const String &name, int *pos=0, const bool &throwException=true)
    {
        GuiPanel *panel = getPanelByName(name,pos,throwException);
        if(!panel)return 0;
        if(!panel->isMultiPanel())
        {
            throw(Exception(9,"GuiPanel '"+name+"' is not a MultiPanel",""));
            return 0;
        }
        return static_cast<GuiMultiPanel*>(panel);
    }
    void bringPanelToFront(const int &pos)
    {
        if(mPanels.empty() || pos<0 || pos>=(int)mPanels.size())return;
        GuiPanel *a = mPanels.front();
        GuiPanel *b = mPanels[pos];
        mPanels.at(0) = b;
        mPanels.at(pos) = a;
    }
    void setActivePanel(GuiPanel *panel, int pos=-1)
    {
        if(*mActivePanel)(*mActivePanel)->swapZOrder(panel);
        *mActivePanel = panel;
        if(pos==-1)
        {
            pos = 0;
            getPanelByName(panel->getName(),&pos);
        }
        bringPanelToFront(pos);
    }
    void showComboBox(const String &name, const std::vector<String> &list)
    {
        mComboBoxName = name;
        mComboList->setVisibleLines(list.size());
        mComboList->clear();
        bool doubleWidth = false;
        for(std::vector<String>::const_iterator it=list.begin(); it!=list.end(); it++)
        {
            const String caption = *it;
            mComboList->pushLine(caption);
            if(caption.length()>10)doubleWidth = true;
        }
        mComboList->setWidth(doubleWidth?0.2f:0.1f);
        mComboBox->setWidth(doubleWidth?0.2f:0.1f);
        mComboBox->setHeight(mComboList->getHeight());
        mComboBox->setPosition((mCursor->mX-mComboBox->getWidth()<0?0:mCursor->mX-mComboBox->getWidth()),(mCursor->mY+mComboBox->getHeight()>1?mCursor->mY-mComboBox->getHeight():mCursor->mY));
        mComboList->update(0);
        mComboBox->show(true);
    }
    void hideComboBox()
    {
        mComboBox->show(false);
    }
    const bool isComboBoxOpen()
    {
        return mComboBox->isVisible();
    }
    GuiPanel* getComboBox()
    {
        return mComboBox;
    }
    GuiHighlightList* getComboBoxList()
    {
        return mComboList;
    }
    const String getComboBoxName()
    {
        return mComboBoxName;
    }
    void showAlertBox(String caption, const String &name="", const bool &hasCancel=false, const bool &hasOK=true)
    {
        mAlertBoxName = name;
        mAlertBoxText->wrapCaption(caption);
        mAlertBoxText->setCaption(caption);
        if(!hasOK)
        {
            mAlertBoxOK->hide();
            mAlertBoxCancel->hide();
            mAlertBoxText->getOverlayContainer()->setTop(0.04f);
        }
        else
        {
            mAlertBoxOK->show();
            mAlertBoxText->getOverlayContainer()->setTop(0);
            if(hasCancel)
            {
                mAlertBoxOK->setLeft(0.02f);
                mAlertBoxCancel->show();
            }
            else
            {
                mAlertBoxOK->setLeft(0.09f);
                mAlertBoxCancel->hide();
            }
        }
        mAlertBox->show(true);
    }
    void queueAlertBox(const String &caption)
    {
        if(mAlertBox->isVisible())
        {
            mAlertBoxQueue.push_back(caption);
        }
        else showAlertBox(caption);
    }
    void dequeueAlertBox()
    {
        if(mAlertBoxQueue.empty())return;
        const String caption = mAlertBoxQueue.front();
        mAlertBoxQueue.pop_front();
        showAlertBox(caption);
    }
    void hideAlertBox()
    {
        mAlertBox->show(false);
    }
    GuiPanel* getAlertBox()
    {
        return mAlertBox;
    }
    const String getAlertBoxName()
    {
        return mAlertBoxName;
    }
    void updateStats()
    {
        GuiPanel *panel = getPanelByName("DebugScreen",0,false);
        if(!panel)return;
        panel->getComponentByName("DebugScreen/AvgFPS")->setCaption(mAvgFPS);
        panel->getComponentByName("DebugScreen/CurFPS")->setCaption(mCurFPS);
        panel->getComponentByName("DebugScreen/BestFPS")->setCaption(mBestFPS);
        panel->getComponentByName("DebugScreen/WorstFPS")->setCaption(mWorstFPS);
        panel->getComponentByName("DebugScreen/TriCount")->setCaption(mTriCount);
        panel->getComponentByName("DebugScreen/BatchCount")->setCaption(mBatchCount);
    }
    void showAll(const bool &flag)
    {
        mHidden = !flag;
        mCursor->shallowHide(!flag);
        if(!flag)
        {
            mAlertBox->show(false,true);
            mComboBox->show(false,true);
        }
        else
        {
            if(mAlertBox->mVisible)mAlertBox->show(true,true);
            if(mComboBox->mVisible)mComboBox->show(true,true);
        }
        for(std::vector<GuiPanel*>::iterator it = mPanels.begin(); it!=mPanels.end(); it++)
        {
            GuiPanel *panel = *it;
            if(!flag)panel->show(false,true);
            else if(panel->mVisible)panel->show(true,true);
        }
    }
    const bool isVisible()
    {
        return !mHidden;
    }
    void showCursor(const bool &flag)
    {
        mCursor->show(flag);
    }
    void hideCursorOnRightClick(const bool &flag)
    {
        mCursor->mHideOnRightClick = flag;
    }
    const Real getCursorX()
    {
        return mCursor->mX;
    }
    const Real getCursorY()
    {
        return mCursor->mY;
    }
};

template<> Gui* Singleton<Gui>::ms_Singleton = 0;

Gui* Gui::getSingletonPtr()
{
	return ms_Singleton;
}

Gui& Gui::getSingleton()
{
	assert(ms_Singleton);
	return *ms_Singleton;
}

#endif
