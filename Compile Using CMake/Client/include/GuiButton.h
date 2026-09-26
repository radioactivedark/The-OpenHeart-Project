#ifndef _GUIBUTTON_H_
#define _GUIBUTTON_H_

#include <GuiComponent.h>

class GuiButton : public GuiComponent
{
protected:
    OverlayElement *mCaption;
    bool mPressed;
    bool mHoldable;
    bool mIsHeld;
    bool mHasOwnMat;
    String mNormalMat;
    String mHoverMat;
    String mPressedMat;
    String mHighlightMat;
    bool mHighlighted;
public:
    GuiButton(OverlayContainer* button, const String &name)
    {
        mComponent = button;
        OverlayContainer::ChildIterator it = button->getChildIterator();
        if(it.hasMoreElements())mCaption = it.getNext();
        else mCaption = 0;
        mName = name;
        mPressed = false;
        mHoldable = false;
        mIsHeld = false;
        mHasOwnMat = false;
        mHighlighted = false;
    }
    ~GuiButton()
    {
    }
    void setCaption(const String &caption)
    {
        if(mCaption)mCaption->setCaption(caption);
    }
    void setOwnMaterials(const String &normal, const String &hover, const String &pressed, const String &highlight="")
    {
        mHasOwnMat = true;
        mNormalMat = normal;
        mHoverMat = hover;
        mPressedMat = pressed;
        mHighlightMat = highlight;
        mComponent->setMaterialName(mNormalMat);
    }
    void highlight(const bool &flag)
    {
        if(mHighlighted==flag)return;
        mHighlighted = flag;
        if(flag)mComponent->setMaterialName(mHighlightMat);
        else mComponent->setMaterialName(mNormalMat);
    }
    virtual void hover()
    {
        mComponent->setMaterialName(mHasOwnMat?mHoverMat:"GuiMat/ButtonOver");
        mPressed = false;
    }
    virtual void pressed()
    {
        mComponent->setMaterialName(mHasOwnMat?mPressedMat:"GuiMat/ButtonDown");
        mPressed = true;
    }
    virtual void unhover()
    {
        if(mHighlighted)mComponent->setMaterialName(mHasOwnMat?mHighlightMat:"GuiMat/ButtonHighlight");
        else mComponent->setMaterialName(mHasOwnMat?mNormalMat:"GuiMat/ButtonUp");
        mPressed = false;
    }
    virtual const bool isPressed()
    {
        return mPressed;
    }
    const bool isButton()
    {
        return true;
    }
    virtual const bool isCheckBox()
    {
        return false;
    }
    void setHoldable(const bool &flag)
    {
        mHoldable = flag;
    }
    const bool isHoldable()
    {
        return mHoldable;
    }
    const bool isHeld()
    {
        return mIsHeld;
    }
    void setIsHeld(const bool &flag)
    {
        mIsHeld = flag;
    }
};

class GuiCheckBox : public GuiButton
{
private:
    bool mChecked;
public:
    GuiCheckBox(OverlayContainer* button, const String &name) : GuiButton(button,name)
    {
        mChecked = false;
        mComponent->setMaterialName("GuiMat/CheckBoxUpFalse");
    }
    ~GuiCheckBox()
    {
    }
    void hover()
    {
        mPressed = false;
    }
    void pressed()
    {
        mComponent->setMaterialName("GuiMat/CheckBoxDown");
        mPressed = true;
    }
    void unhover()
    {
        if(mChecked)mComponent->setMaterialName("GuiMat/CheckBoxUpTrue");
        else mComponent->setMaterialName("GuiMat/CheckBoxUpFalse");
        mPressed = false;
    }
    const bool isPressed()
    {
        return mPressed;
    }
    const bool isCheckBox()
    {
        return true;
    }
    void toggleChecked()
    {
        mChecked = !mChecked;
    }
    const bool isChecked()
    {
        return mChecked;
    }
    void setChecked(const bool &flag)
    {
        mChecked = flag;
        unhover();
    }

};

#endif
