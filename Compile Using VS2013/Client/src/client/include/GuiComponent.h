#ifndef _GUICOMPONENT_H_
#define _GUICOMPONENT_H_

class GuiComponent
{
protected:
    String mName;
    OverlayContainer *mComponent;
public:
    GuiComponent()
    {
        mName = "";
        mComponent = 0;
    }
    virtual ~GuiComponent()
    {
    }
    const String getName()
    {
        return mName;
    }
    const bool nameIs(const String &name)
    {
        return (mName==name);
    }
    const bool nameIsIndex(const String &name, unsigned short *index=0)
    {
        if(StringUtil::startsWith(mName,name,false))
        {
            if(index)
            {
                String indexStr = mName;
                indexStr.erase(0,(int)name.size());
                *index = StringConverter::parseInt(indexStr);
            }
            return true;
        }
        else return false;
    }
    virtual void show(const bool &flag)
    {
        if(flag)mComponent->show();
        else mComponent->hide();
    }
    const bool isVisible()
    {
        return mComponent->isVisible();
    }
    const bool isInArea(const Real &x, const Real &y)
    {
        if(!mComponent->isVisible() || !mComponent->getParent() || !mComponent->getParent()->isVisible())return false;
        const Real left = mComponent->_getDerivedLeft();
        const Real top = mComponent->_getDerivedTop();
        if(x>=left && x<=left+mComponent->getWidth() && y>=top && y<=top+mComponent->getHeight())return true;
        return false;
    }
    virtual const bool isButton()
    {
        return false;
    }
    virtual const bool isTextComponent()
    {
        return false;
    }
    virtual const bool isSlider()
    {
        return false;
    }
    virtual const bool isList()
    {
        return false;
    }
    virtual const bool isTab()
    {
        return false;
    }
    virtual const bool isSizer()
    {
        return false;
    }
    virtual const bool isMap()
    {
        return false;
    }
    virtual const bool isMover()
    {
        return false;
    }
    OverlayContainer* getParent()
    {
        return mComponent->getParent();
    }
    virtual void hover()
    {
    }
    virtual void pressed()
    {
    }
    virtual void unhover()
    {
    }
    virtual const bool isPressed()
    {
        return false;
    }
    virtual void setCaption(const String &caption)
    {
        mComponent->setCaption(caption);
    }
    virtual void setValue(const Real &value)
    {
    }
    virtual const Real getValue()
    {
        return 0;
    }
    OverlayContainer* getOverlayContainer()
    {
        return mComponent;
    }
    void setLeft(const Real &left)
    {
        mComponent->setLeft(left);
    }
    const Real getLeft()
    {
        return mComponent->getLeft();
    }
    virtual void setWidth(const Real &width)
    {
        mComponent->setWidth(width);
    }
    const Real getWidth()
    {
        return mComponent->getWidth();
    }
};

#endif
