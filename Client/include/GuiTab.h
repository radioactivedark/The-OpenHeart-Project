#ifndef _GUITAB_H_
#define _GUITAB_H_

#include <GuiComponent.h>

class GuiTab : public GuiComponent
{
private:
    unsigned short mIndex;
    bool mActive;
public:
    GuiTab(OverlayContainer* button, const String &name, const unsigned short &index)
    {
        mComponent = button;
        mName = name;
        mIndex = index;
        mActive = false;
    }
    ~GuiTab()
    {
    }
    void hover()
    {
        mComponent->setMaterialName("GuiMat/TabOver");
    }
    void pressed()
    {
        mComponent->setMaterialName("GuiMat/TabUp");
        mActive = true;
    }
    void deactivate()
    {
        mComponent->setMaterialName("GuiMat/TabDown");
        mActive = false;
    }
    void unhover()
    {
        if(mActive)mComponent->setMaterialName("GuiMat/TabUp");
        else mComponent->setMaterialName("GuiMat/TabDown");
    }
    const bool isTab()
    {
        return true;
    }
    const unsigned short getIndex()
    {
        return mIndex;
    }
    void setCaption(const String &caption)
    {
        if(!mComponent)return;
        OverlayContainer::ChildIterator it = mComponent->getChildIterator();
        if(!it.hasMoreElements())return;
        OverlayElement *label = it.getNext();
        label->setCaption(caption);
    }
};

#endif

