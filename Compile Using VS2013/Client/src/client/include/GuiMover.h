#ifndef _GUIMOVER_H_
#define _GUIMOVER_H_

#include <GuiComponent.h>

class GuiMover : public GuiComponent
{
private:
    OverlayContainer *mParent;
    Vector2 mStartPos;
    OverlayElement *mCaption;
public:
    GuiMover(OverlayContainer* mover, const String &name)
    {
        mComponent = mover;
        mParent = mComponent->getParent();
        mName = name;
        mComponent->setWidth(mParent->getWidth()-0.04f);
        OverlayContainer::ChildIterator it = mComponent->getChildIterator();
        if(it.hasMoreElements())mCaption = it.getNext();
        else mCaption = 0;
    }
    ~GuiMover()
    {
    }
    const bool isMover()
    {
        return true;
    }
    void setStartPos(const Real &x, const Real &y)
    {
        mStartPos = Vector2(x,y);
    }
    const bool updateByCursor(const Real &x, const Real &y)
    {
        const Vector2 offset = Vector2(x,y) - mStartPos;
        if(offset==Vector2::ZERO)return false;
        const Real left = mParent->getLeft()+offset.x;
        const Real top = mParent->getTop()+offset.y;
        mParent->setPosition(left>0.98f?0.98f:left,top>0.96f?0.96f:(top<0?0:top));
        setStartPos(x,y);
        return true;
    }
    void setCaption(const String &caption)
    {
        if(mCaption)mCaption->setCaption(caption);
    }
};

#endif

