#ifndef _GUISLIDER_H_
#define _GUISLIDER_H_

#include <GuiSliderList.h>

#define MARKER_WIDTH 0.04f
#define MARKER_WIDTH_HALVED 0.02f

class GuiSlider : public GuiComponent
{
protected:
    OverlayElement *mMarker;
    bool mIsVertical;
    Real mValue;
    Real mWidth;
    GuiSliderList *mBoundList;
    bool mIsHeld;
public:
    GuiSlider(OverlayContainer* slider, const String &name, GuiSliderList *list=0)
    {
        mComponent = slider;
        mName = name;
        mIsVertical = (slider->getHeight()>slider->getWidth());
        mWidth = (mIsVertical?slider->getHeight():slider->getWidth())-MARKER_WIDTH;
        mMarker = OverlayManager::getSingleton().createOverlayElementFromTemplate("GuiTemplate/SliderMarker","Panel",mName+"Mark");
        slider->addChild(mMarker);
        mMarker->setDimensions((mIsVertical?slider->getWidth():MARKER_WIDTH),(mIsVertical?MARKER_WIDTH:slider->getHeight()));
        mMarker->setPosition(0,0);

        mValue = 0;
        mIsHeld = false;
        mBoundList = list;
        if(mBoundList)mBoundList->setSliderRef(this);
    }
    ~GuiSlider()
    {
        const String childName = mMarker->getName();
        mComponent->removeChild(childName);
        OverlayManager::getSingleton().destroyOverlayElement(childName);
        //Destroy overlay container if this is auto created
        if(mBoundList)
        {
            mBoundList->setSliderRef(0);
            const String name = mComponent->getName();
            mComponent->getParent()->removeChild(name);
            OverlayManager::getSingleton().destroyOverlayElement(name);
        }
    }
    void setValue(const Real &value)
    {
        mValue = value;
        updateMarker();
        if(mBoundList)mBoundList->update(getFirstLineOf(mBoundList->getNumLines(),mBoundList->getVisibleLines()));
    }
    const bool updateByCursor(const Real &x, const Real &y, const bool &isClick)
    {
        if(isClick)
        {
            //Clicked outside button
            if(mIsVertical ? y<mMarker->_getDerivedTop()||y>mMarker->_getDerivedTop()+MARKER_WIDTH : x<mMarker->_getDerivedLeft()||x>mMarker->_getDerivedLeft()+MARKER_WIDTH)
            {
                const bool isDecrement = (mIsVertical ? (y<mMarker->_getDerivedTop()) : (x<mMarker->_getDerivedLeft()));
                Real value = mValue;
                if(mBoundList && mBoundList->getNumLines()>mBoundList->getVisibleLines())value += Real(mBoundList->getVisibleLines())/(mBoundList->getNumLines()-mBoundList->getVisibleLines()) * (isDecrement?-1:1);
                else value += 0.1 * (isDecrement?-1:1);
                if(value<0)value = 0;
                else if(value>1)value = 1;
                if(mValue!=value)
                {
                    setValue(value);
                    return true;
                }
                return false;
            }
            //Clicked button
            else mIsHeld = true;
        }
        Real value = ((mIsVertical?y-mComponent->_getDerivedTop():x-mComponent->_getDerivedLeft()) - MARKER_WIDTH_HALVED)/mWidth;
        if(value<0)value = 0;
        else if(value>1)value = 1;
        if(mValue!=value)
        {
            setValue(value);
            return true;
        }
        return false;
    }
    void updateMarker()
    {
        if(mIsVertical)mMarker->setTop(mValue*mWidth);
        else mMarker->setLeft(mValue*mWidth);
    }
    const Real getValue()
    {
        return mValue;
    }
    const unsigned short getFirstLineOf(const unsigned short &numLines, const unsigned short &visibleLines)
    {
        return numLines>visibleLines ? (unsigned short)((numLines-visibleLines+(mValue!=1))*mValue) : 0;
    }
    void updateSliderToListSelection()
    {
        if(!mBoundList)return;
        const unsigned short visibleLines = mBoundList->getVisibleLines();
        const unsigned short selectedLine = mBoundList->getSelectedLine();
        const unsigned short numLines = mBoundList->getNumLines() - visibleLines;
        if(selectedLine<visibleLines || numLines==0)mValue = 0;
        else mValue = Real(selectedLine-visibleLines+1)/numLines;
        updateMarker();
        mBoundList->update(getFirstLineOf(mBoundList->getNumLines(),visibleLines));
    }
    void hover()
    {
        mMarker->setMaterialName("GuiMat/SliderMarkerOver");
    }
    void pressed()
    {
        mMarker->setMaterialName("GuiMat/SliderMarkerDown");
    }
    void unhover()
    {
        mMarker->setMaterialName("GuiMat/SliderMarkerUp");
    }
    const bool isPressed()
    {
        return (mMarker->getMaterialName()=="GuiMat/SliderMarkerDown");
    }
    const bool isSlider()
    {
        return true;
    }
    void setWidth(const Real &value)
    {
        if(mIsVertical)mComponent->setHeight(value);
        else mComponent->setWidth(value);
        mWidth = value-MARKER_WIDTH;
        updateMarker();
    }
    const bool isHeld()
    {
        return mIsHeld;
    }
    void setIsHeld(const bool &held)
    {
        mIsHeld = held;
    }
};

#endif
