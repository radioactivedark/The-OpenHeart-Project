#ifndef _GUIPANEL_H_
#define _GUIPANEL_H_

#include <GuiButton.h>
#include <GuiMaskedTextField.h>
#include <GuiTextArea.h>
#include <GuiSlider.h>
#include <GuiSizer.h>
#include <GuiMap.h>
#include <GuiMover.h>
#include <GuiList.h>
#include <GuiTab.h>

class GuiPanel;
class GuiPanelListener
{
public:
    enum
    {
        GUIPANEL_OPENED,
        GUIPANEL_CLOSED
    };
    GuiPanelListener(){}
    virtual ~GuiPanelListener(){}
    virtual const bool guiPanelEvent(GuiPanel *panel, const unsigned char &event) = 0;
};

class GuiPanel
{
protected:
    String mName;
    Overlay *mOverlay;
    OverlayContainer *mPanel;
    bool mAnchored;
    const unsigned int *mWindowWidth;
    const unsigned int *mWindowHeight;
    GuiPanelListener *mListener;
public:
    bool mVisible;  //whether this panel is SUPPOSED to be visible or not, used for temp hiding
    std::vector<GuiComponent*> mComponents;
    GuiPanel(Overlay* overlay, const bool &anchored, const unsigned int *windowWidth, const unsigned int *windowHeight)
    : mWindowWidth(windowWidth), mWindowHeight(windowHeight)
    {
        mOverlay = overlay;
        mAnchored = anchored;
        mName = overlay->getName();
        mPanel = overlay->getChild(mName+"/Panel");
        mVisible = true;
        mListener = 0;

        registerComponentsOfPanel(mPanel);

        if(!anchored)createCloseButton();
    }
    GuiPanel(const unsigned int *windowWidth, const unsigned int *windowHeight)
    : mWindowWidth(windowWidth), mWindowHeight(windowHeight)
    {
        mListener = 0;
    }
    virtual ~GuiPanel()
    {
        while(!mComponents.empty())
        {
            GuiComponent *component = mComponents.back();
            mComponents.pop_back();
            delete component;
        }
        if(!mAnchored)
        {
            //Destroy auto created button
            const String childName = mName+"/CloseButton";
            mPanel->removeChild(childName);
            OverlayManager::getSingleton().destroyOverlayElement(childName);
        }
        show(false);
    }
    void registerComponentsOfPanel(OverlayContainer *panel)
    {
        OverlayContainer::ChildContainerIterator it = panel->getChildContainerIterator();
        while(it.hasMoreElements())
        {
            OverlayContainer *container = it.getNext();
            registerComponent(container);
        }
    }
    void registerComponent(OverlayContainer *container)
    {
        String name = container->getName();
        if(StringUtil::endsWith(name,"button"))
        {
            name.erase(name.end()-6,name.end());
            mComponents.push_back(new GuiButton(container,name));
        }
        else if(StringUtil::endsWith(name,"textcomponent"))
        {
            name.erase(name.end()-13,name.end());
            mComponents.push_back(new GuiTextComponent(container,name,mWindowWidth,mWindowHeight));
        }
        else if(StringUtil::endsWith(name,"textfield"))
        {
            name.erase(name.end()-9,name.end());
            mComponents.push_back(new GuiTextField(container,name,mWindowWidth,mWindowHeight));
        }
        else if(StringUtil::endsWith(name,"textfieldmask"))
        {
            name.erase(name.end()-13,name.end());
            mComponents.push_back(new GuiMaskedTextField(container,name,mWindowWidth,mWindowHeight));
        }
        else if(StringUtil::endsWith(name,"textarea"))
        {
            name.erase(name.end()-8,name.end());
            GuiTextArea *guiArea = new GuiTextArea(container,name,mWindowWidth,mWindowHeight);
            mComponents.push_back(guiArea);
            mComponents.push_back(new GuiSlider(guiArea->getSlider(),name+"LS",guiArea));
        }
        else if(StringUtil::endsWith(name,"slider"))
        {
            name.erase(name.end()-6,name.end());
            mComponents.push_back(new GuiSlider(container,name));
        }
        else if(StringUtil::endsWith(name,"list"))
        {
            GuiList *guiList = 0;
            if(StringUtil::endsWith(name,"highlightlist"))
            {
                name.erase(name.end()-13,name.end());
                guiList = new GuiHighlightList(container,name);
            }
            else if(StringUtil::endsWith(name,"highlightdoublelist"))
            {
                name.erase(name.end()-19,name.end());
                guiList = new GuiHighlightDoubleList(container,name);
            }
            else
            {
                name.erase(name.end()-4,name.end());
                guiList = new GuiList(container,name);
            }
            mComponents.push_back(guiList);
            mComponents.push_back(new GuiSlider(guiList->getSlider(),name+"LS",guiList));
        }
        else if(StringUtil::endsWith(name,"checkbox"))
        {
            name.erase(name.end()-8,name.end());
            mComponents.push_back(new GuiCheckBox(container,name));
        }
        else if(StringUtil::endsWith(name,"sizer"))
        {
            name.erase(name.end()-5,name.end());
            unsigned char dir = 0;
            if(StringUtil::endsWith(name,"north"))
            {
                dir = 0;
                name.erase(name.end()-5,name.end());
            }
            else if(StringUtil::endsWith(name,"east"))
            {
                dir = 1;
                name.erase(name.end()-4,name.end());
            }
            else if(StringUtil::endsWith(name,"south"))
            {
                dir = 2;
                name.erase(name.end()-5,name.end());
            }
            else if(StringUtil::endsWith(name,"west"))
            {
                dir = 3;
                name.erase(name.end()-4,name.end());
            }
            mComponents.push_back(new GuiSizer(container,name,dir));
        }
        else if(StringUtil::endsWith(name,"map"))
        {
            name.erase(name.end()-3,name.end());
            mComponents.push_back(new GuiMap(container,name,mWindowWidth,mWindowHeight));
        }
        else if(StringUtil::endsWith(name,"mover"))
        {
            name.erase(name.end()-5,name.end());
            mComponents.push_back(new GuiMover(container,name));
        }
    }
    void createCloseButton()
    {
        OverlayContainer *closeButton = static_cast<OverlayContainer*>(OverlayManager::getSingleton().createOverlayElementFromTemplate("GuiTemplate/Button","Panel",mName+"/CloseButton"));
        closeButton->setDimensions(0.03f,0.02f);
        closeButton->setPosition(mPanel->getWidth()-0.04f,0.01f);
        mPanel->addChild(closeButton);
        GuiButton *button = new GuiButton(closeButton,"Close");
        button->setOwnMaterials("GuiMat/ButtonCloseUp","GuiMat/ButtonCloseOver","GuiMat/ButtonCloseDown");
        mComponents.push_back(button);
    }
    void addComponent(GuiComponent *component)
    {
        mPanel->addChild(component->getOverlayContainer());
        mComponents.push_back(component);
    }
    void removeComponent(GuiComponent *component)
    {
        for(std::vector<GuiComponent*>::iterator it=mComponents.begin();it!=mComponents.end();it++)
        {
            GuiComponent *comp = *it;
            if(comp==component)
            {
                mComponents.erase(it);
                mPanel->removeChild(component->getOverlayContainer()->getName());
                return;
            }
        }
    }
    void show(const bool &flag, const bool &temp=false)
    {
        if(flag)
        {
            mOverlay->show();
            mPanel->show();
            if(mListener)mListener->guiPanelEvent(this,GuiPanelListener::GUIPANEL_OPENED);
        }
        else
        {
            mOverlay->hide();
            mPanel->hide();
            if(mListener)mListener->guiPanelEvent(this,GuiPanelListener::GUIPANEL_CLOSED);
        }
        if(!temp)mVisible = flag;
    }
    void toggleVisibility()
    {
        if(!mOverlay->isVisible() || !mPanel->isVisible())
        {
            mOverlay->show();
            mPanel->show();
            mVisible = true;
            if(mListener)mListener->guiPanelEvent(this,GuiPanelListener::GUIPANEL_OPENED);
        }
        else
        {
            mOverlay->hide();
            mPanel->hide();
            mVisible = false;
            if(mListener)mListener->guiPanelEvent(this,GuiPanelListener::GUIPANEL_CLOSED);
        }
    }
    const bool isVisible()
    {
        return (mOverlay->isVisible() && mPanel->isVisible());
    }
    const bool isInArea(const Real &x, const Real &y)
    {
        if(!mOverlay->isVisible() || !mPanel->isVisible())return false;
        const Real left = mPanel->getLeft();
        const Real top = mPanel->getTop();
        if(x>=left && x<=left+mPanel->getWidth() && y>=top && y<=top+mPanel->getHeight())return true;
        return false;
    }
    void setZOrder(const unsigned short &z)
    {
        mOverlay->setZOrder(z);
    }
    const unsigned short getZOrder()
    {
        return mOverlay->getZOrder();
    }
    void swapZOrder(GuiPanel *panel)
    {
        const unsigned short z = getZOrder();
        setZOrder(panel->getZOrder());
        panel->setZOrder(z);
    }
    const String getName()
    {
        return mName;
    }
    GuiComponent* getComponentByName(const String &name)
    {
        for(std::vector<GuiComponent*>::iterator it=mComponents.begin(); it!=mComponents.end(); it++)
        {
            GuiComponent *component = *it;
            if(component->getName()==name)return component;
        }
        LoggerManager::getSingleton().logMessage("ERROR::GuiPanel:getComponentByName: No GuiComponent by name '"+name+"'");
        throw(Exception(9,"No GuiComponent by name '"+name+"'",""));
        return 0;
    }
    GuiButton* getButton(const String &name)
    {
        GuiComponent *component = getComponentByName(name);
        if(!component)return 0;
        if(!component->isButton())
        {
            LoggerManager::getSingleton().logMessage("ERROR::GuiPanel:getButton: GuiComponent '"+name+"' is not a GuiButton");
            throw(Exception(9,"GuiComponent '"+name+"' is not a GuiButton",""));
            return 0;
        }
        return static_cast<GuiButton*>(component);
    }
    GuiCheckBox* getCheckBox(const String &name)
    {
        GuiComponent *component = getComponentByName(name);
        if(!component)return 0;
        if(component->isButton())
        {
            GuiButton *button = static_cast<GuiButton*>(component);
            if(button->isCheckBox())return static_cast<GuiCheckBox*>(button);
        }
        LoggerManager::getSingleton().logMessage("ERROR::GuiPanel:getCheckBox: GuiComponent '"+name+"' is not a GuiCheckBox");
        throw(Exception(9,"GuiComponent '"+name+"' is not a GuiCheckBox",""));
        return 0;
    }
    GuiTextComponent* getTextComponent(const String &name)
    {
        GuiComponent *component = getComponentByName(name);
        if(!component)return 0;
        if(!component->isTextComponent())
        {
            LoggerManager::getSingleton().logMessage("ERROR::GuiPanel:getTextComponent: GuiComponent '"+name+"' is not a GuiTextComponent");
            throw(Exception(9,"GuiComponent '"+name+"' is not a GuiTextComponent",""));
            return 0;
        }
        return static_cast<GuiTextComponent*>(component);
    }
    GuiTextField* getTextField(const String &name)
    {
        GuiComponent *component = getComponentByName(name);
        if(!component)return 0;
        if(component->isTextComponent())
        {
            GuiTextComponent *text = static_cast<GuiTextComponent*>(component);
            if(!text->isArea() && !text->isMasked())return static_cast<GuiTextField*>(text);
        }
        LoggerManager::getSingleton().logMessage("ERROR::GuiPanel:getTextField: GuiComponent '"+name+"' is not a GuiTextField");
        throw(Exception(9,"GuiComponent '"+name+"' is not a GuiTextField",""));
        return 0;
    }
    GuiMaskedTextField* getMaskedTextField(const String &name)
    {
        GuiComponent *component = getComponentByName(name);
        if(!component)return 0;
        if(component->isTextComponent())
        {
            GuiTextComponent *text = static_cast<GuiTextComponent*>(component);
            if(!text->isArea() && text->isMasked())return static_cast<GuiMaskedTextField*>(text);
        }
        LoggerManager::getSingleton().logMessage("ERROR::GuiPanel:getMaskedTextField: GuiComponent '"+name+"' is not a GuiMaskedTextField");
        throw(Exception(9,"GuiComponent '"+name+"' is not a GuiMaskedTextField",""));
        return 0;
    }
    GuiTextArea* getTextArea(const String &name)
    {
        GuiComponent *component = getComponentByName(name);
        if(!component)return 0;
        if(component->isTextComponent())
        {
            GuiTextComponent *text = static_cast<GuiTextComponent*>(component);
            if(text->isArea())return static_cast<GuiTextArea*>(text);
        }
        LoggerManager::getSingleton().logMessage("ERROR::GuiPanel:getTextArea: GuiComponent '"+name+"' is not a GuiTextArea");
        throw(Exception(9,"GuiComponent '"+name+"' is not a GuiTextArea",""));
        return 0;
    }
    GuiList* getList(const String &name)
    {
        GuiComponent *component = getComponentByName(name);
        if(!component)return 0;
        if(!component->isList())
        {
            LoggerManager::getSingleton().logMessage("ERROR::GuiPanel:getList: GuiComponent '"+name+"' is not a GuiList");
            throw(Exception(9,"GuiComponent '"+name+"' is not a GuiList",""));
            return 0;
        }
        return static_cast<GuiList*>(component);
    }
    GuiHighlightList* getHighlightList(const String &name)
    {
        GuiComponent *component = getComponentByName(name);
        if(!component)return 0;
        if(component->isList())
        {
            GuiList *list = static_cast<GuiList*>(component);
            if(list->isHighlightList())return static_cast<GuiHighlightList*>(list);
        }
        LoggerManager::getSingleton().logMessage("ERROR::GuiPanel:getHighlightList: GuiComponent '"+name+"' is not a GuiHighlightList");
        throw(Exception(9,"GuiComponent '"+name+"' is not a GuiHighlightList",""));
        return 0;
    }
    GuiHighlightDoubleList* getHighlightDoubleList(const String &name)
    {
        GuiComponent *component = getComponentByName(name);
        if(!component)return 0;
        if(component->isList())
        {
            GuiList *list = static_cast<GuiList*>(component);
            if(list->isHighlightList() && static_cast<GuiHighlightList*>(list)->isDoubleList())return static_cast<GuiHighlightDoubleList*>(list);
        }
        LoggerManager::getSingleton().logMessage("ERROR::GuiPanel:getHighlightDoubleList: GuiComponent '"+name+"' is not a GuiHighlightDoubleList");
        throw(Exception(9,"GuiComponent '"+name+"' is not a GuiHighlightDoubleList",""));
        return 0;
    }
    GuiSlider* getSlider(const String &name)
    {
        GuiComponent *component = getComponentByName(name);
        if(!component)return 0;
        if(!component->isSlider())
        {
            LoggerManager::getSingleton().logMessage("ERROR::GuiPanel:getSlider: GuiComponent '"+name+"' is not a GuiSlider");
            throw(Exception(9,"GuiComponent '"+name+"' is not a GuiSlider",""));
            return 0;
        }
        return static_cast<GuiSlider*>(component);
    }
    GuiSizer* getSizer(const String &name)
    {
        GuiComponent *component = getComponentByName(name);
        if(!component)return 0;
        if(!component->isSizer())
        {
            LoggerManager::getSingleton().logMessage("ERROR::GuiPanel:getSizer: GuiComponent '"+name+"' is not a GuiSizer");
            throw(Exception(9,"GuiComponent '"+name+"' is not a GuiSizer",""));
            return 0;
        }
        return static_cast<GuiSizer*>(component);
    }
    GuiMover* getMover(const String &name)
    {
        GuiComponent *component = getComponentByName(name);
        if(!component)return 0;
        if(!component->isMover())
        {
            LoggerManager::getSingleton().logMessage("ERROR::GuiPanel:getMover: GuiComponent '"+name+"' is not a GuiMover");
            throw(Exception(9,"GuiComponent '"+name+"' is not a GuiMover",""));
            return 0;
        }
        return static_cast<GuiMover*>(component);
    }
    GuiTab* getTab(const String &name)
    {
        if(!isMultiPanel())
        {
            LoggerManager::getSingleton().logMessage("ERROR::GuiPanel:getTab: GuiPanel '"+mName+"' is not a MultiPanel and has no Tabs");
            throw(Exception(9,"GuiPanel '"+mName+"' is not a MultiPanel and has no Tabs",""));
            return 0;
        }
        GuiComponent *component = getComponentByName(name);
        if(!component)return 0;
        if(!component->isTab())
        {
            LoggerManager::getSingleton().logMessage("ERROR::GuiPanel:getTab: GuiComponent '"+name+"' is not a GuiTab");
            throw(Exception(9,"GuiComponent '"+name+"' is not a GuiTab",""));
            return 0;
        }
        return static_cast<GuiTab*>(component);
    }
    GuiMap* getMap(const String &name)
    {
        GuiComponent *component = getComponentByName(name);
        if(!component)return 0;
        if(!component->isMap())
        {
            LoggerManager::getSingleton().logMessage("ERROR::GuiPanel:getMap: GuiComponent '"+name+"' is not a GuiMap");
            throw(Exception(9,"GuiComponent '"+name+"' is not a GuiMap",""));
            return 0;
        }
        return static_cast<GuiMap*>(component);
    }
    virtual const bool isMultiPanel()
    {
        return false;
    }
    OverlayContainer* getOverlayContainer()
    {
        return mPanel;
    }
    void setPosition(const Real &x, const Real &y)
    {
        mPanel->setLeft(x);
        mPanel->setTop(y);
    }
    void setHeight(const Real &value)
    {
        mPanel->setHeight(value);
    }
    const Real getHeight()
    {
        return mPanel->getHeight();
    }
    void setLeft(const Real &value)
    {
        mPanel->setLeft(value);
    }
    void setWidth(const Real &value)
    {
        mPanel->setWidth(value);
    }
    const Real getWidth()
    {
        return mPanel->getWidth();
    }
    void windowResized()
    {
        for(std::vector<GuiComponent*>::iterator it=mComponents.begin();it!=mComponents.end();it++)
        {
            GuiComponent *component = *it;
            if(component->isTextComponent())static_cast<GuiTextComponent*>(component)->windowResized();
            else if(component->isMap())static_cast<GuiMap*>(component)->windowResized();
        }
    }
    void setListener(GuiPanelListener *listener)
    {
        mListener = listener;
    }
};

#endif
