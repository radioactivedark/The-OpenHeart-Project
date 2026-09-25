#ifndef _GUISLIDERLIST_H_
#define _GUISLIDERLIST_H_

#include <GuiComponent.h>

class GuiSliderList
{
protected:
    GuiComponent *mSliderRef;
    OverlayContainer *mSlider;
public:
    GuiSliderList(OverlayContainer *list=0, const String &name="")
    {
        mSliderRef = 0;

        //Created slider element will be deleted in respective GuiSlider
        mSlider = static_cast<OverlayContainer*>(OverlayManager::getSingleton().createOverlayElementFromTemplate("GuiTemplate/Slider","Panel",name+"LS"));
        list->getParent()->addChild(mSlider);
        mSlider->setTop(list->getTop());
        mSlider->setLeft(list->getLeft()+list->getWidth());
        mSlider->setHeight(list->getHeight());
        mSlider->hide();
    }
    void setSliderRef(GuiComponent *slider)
    {
        mSliderRef = slider;
    }
    GuiComponent* getSliderRef()
    {
        return mSliderRef;
    }
    OverlayContainer* getSlider()
    {
        return mSlider;
    }
    virtual void update(const unsigned short &firstLine) = 0;
    virtual const unsigned short getVisibleLines() = 0;
    virtual const unsigned short getSelectedLine() = 0;
    virtual const unsigned short getNumLines() = 0;
};

#endif
