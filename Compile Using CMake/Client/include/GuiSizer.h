#ifndef _GUISIZER_H_
#define _GUISIZER_H_

#define SIZER_WIDTH 0.012f
#define SIZER_WIDTH_VERT 0.01f

#include <GuiComponent.h>

class GuiSizer : public GuiComponent
{
private:
    OverlayContainer *mParent;
    unsigned char mDirection;
    Real mStartPos;
    Real mMinSize;
public:
    GuiSizer(OverlayContainer* sizer, const String &name, const unsigned char &direction)
    {
        mComponent = sizer;
        mParent = mComponent->getParent();
        mName = name;
        mDirection = direction;
        mComponent->setPosition( direction==1?mParent->getWidth()-SIZER_WIDTH_VERT:0 , direction==2?mParent->getHeight()-SIZER_WIDTH:0 );
        updateSize();
        mMinSize = SIZER_WIDTH;
    }
    ~GuiSizer()
    {
    }
    void updateSize()
    {
        mComponent->setWidth( mDirection==0||mDirection==2?mParent->getWidth():SIZER_WIDTH_VERT );
        mComponent->setHeight( mDirection==1||mDirection==3?mParent->getHeight():SIZER_WIDTH );
    }
    const bool isSizer()
    {
        return true;
    }
    void setStartPos(const Real &x, const Real &y)
    {
        mStartPos = (mDirection==0||mDirection==2?y:x);
    }
    void setMinSize(const Real &size)
    {
        mMinSize = size;
    }
    const bool updateByCursor(const Real &x, const Real &y)
    {
        const Real offset = (mDirection==0||mDirection==2?y:x) - mStartPos;
        if(offset==0)return false;
        Real newSize = 0;
        switch(mDirection)
        {
            case 0:
                newSize = mParent->getHeight()-offset;
                break;
            case 1:
                newSize = mParent->getWidth()+offset;
                break;
            case 2:
                newSize = mParent->getHeight()+offset;
                break;
            case 3:
                newSize = mParent->getWidth()-offset;
                break;
        }
        if(newSize<=mMinSize)return false;
        switch(mDirection)
        {
            case 0:
                mParent->setTop(mParent->getTop()+offset);
                mParent->setHeight(newSize);
                break;
            case 1:
                mParent->setWidth(newSize);
                mComponent->setPosition(mParent->getWidth()-SIZER_WIDTH_VERT,0);
                break;
            case 2:
                mParent->setHeight(newSize);
                mComponent->setPosition( 0,mParent->getHeight()-SIZER_WIDTH);
                break;
            case 3:
                mParent->setLeft(mParent->getLeft()+offset);
                mParent->setWidth(newSize);
                break;
        }
        setStartPos(x,y);
        return true;
    }
};

#endif
