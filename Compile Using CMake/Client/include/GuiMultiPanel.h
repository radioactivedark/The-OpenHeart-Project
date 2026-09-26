#ifndef _GUIMULTIPANEL_H_
#define _GUIMULTIPANEL_H_

#include <GuiPanel.h>
#include <GuiTab.h>

class GuiMultiPanel : public GuiPanel
{
protected:
    std::vector<OverlayContainer*> mSubPanels;
    GuiTab *mActiveTab;
public:
    GuiMultiPanel(Overlay* overlay, const bool &anchored, const unsigned int *windowWidth, const unsigned int *windowHeight)
    : GuiPanel(windowWidth,windowHeight)
    {
        mActiveTab = 0;
        mOverlay = overlay;
        mAnchored = anchored;
        mName = overlay->getName();
        mPanel = overlay->getChild(mName+"/MultiPanel");

        //Register sub panels and other elements
        OverlayContainer::ChildContainerIterator it = mPanel->getChildContainerIterator();
        while(it.hasMoreElements())
        {
            OverlayContainer *container = it.getNext();
            const String name = container->getName();
            if(StringUtil::endsWith(name,"panel"))
            {
                mSubPanels.push_back(container);
                registerComponentsOfPanel(container);
                container->hide();
            }
            else registerComponent(container);
        }
        const Real width = mPanel->getWidth();
        const int numPanels = mSubPanels.size();
        const Real tabWidth = numPanels>0 ? (width/numPanels) : width;
        int i = 0;
        for(std::vector<OverlayContainer*>::iterator it=mSubPanels.begin(); it!=mSubPanels.end(); it++)
        {
            OverlayContainer *panel = *it;
            String name = panel->getName();
            name.erase(name.end()-5,name.end());

            OverlayContainer *tabButton = static_cast<OverlayContainer*>(OverlayManager::getSingleton().createOverlayElementFromTemplate("GuiTemplate/Tab","Panel",name+"Tab"));
            tabButton->setDimensions(tabWidth,0.05f);
            tabButton->setPosition(i*tabWidth,anchored?0:0.04f);
            OverlayElement *label = OverlayManager::getSingleton().createOverlayElementFromTemplate("GuiTemplate/CenteredText","TextArea",name+"TabLabel");
            //const size_t delim = name.find_first_of('/');
            //if(delim!=std::string::npos)name.erase(0,delim+1);
            label->setCaption(panel->getCaption());
            tabButton->addChild(label);
            mPanel->addChild(tabButton);
            GuiTab *tab = new GuiTab(tabButton,name/*+"Tab"*/,i);
            mComponents.push_back(tab);
            if(i==0)
            {
                tab->pressed();
                mActiveTab = tab;
            }
            i++;
        }

        if(!anchored)createCloseButton();

        if(!mSubPanels.empty())mSubPanels[0]->show();
    }
    ~GuiMultiPanel()
    {
        for(std::vector<OverlayContainer*>::iterator it=mSubPanels.begin(); it!=mSubPanels.end(); it++)
        {
            OverlayContainer *panel = *it;
            String name = panel->getName();
            name.erase(name.end()-5,name.end());

            const String childName = name+"Tab";
            mPanel->removeChild(childName);
            OverlayManager::getSingleton().destroyOverlayElement(childName);
            OverlayManager::getSingleton().destroyOverlayElement(childName+"Label");
        }
    }
    const bool isMultiPanel()
    {
        return true;
    }
    void selectTab(GuiTab *tab)
    {
        if(tab->getIndex()>=mSubPanels.size())return;
        if(!mActiveTab || mActiveTab==tab)return;
        mActiveTab->deactivate();
        mSubPanels[mActiveTab->getIndex()]->hide();
        mActiveTab = tab;
        mActiveTab->pressed();
        mSubPanels[mActiveTab->getIndex()]->show();
    }
};

#endif
