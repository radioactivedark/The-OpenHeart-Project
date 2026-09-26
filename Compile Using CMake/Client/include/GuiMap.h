#ifndef _GUIMAP_H_
#define _GUIMAP_H_

#include <GuiComponent.h>

struct GuiMapMarker
{
    OverlayElement *mMarker;
    Real mX;
    Real mY;
    GuiMapMarker(OverlayElement *marker)
    {
        mX = mY = 0;
        mMarker = marker;
    }
};

class GuiMap : public GuiComponent
{
protected:
    std::map<unsigned short,GuiMapMarker*> mMarker;
    GuiMapMarker *mActiveMarker;
    const unsigned int *mWindowWidth;
    const unsigned int *mWindowHeight;
    Real mWidth;
    Real mHeight;
public:
    GuiMap(OverlayContainer* map, const String &name, const unsigned int *windowWidth, const unsigned int *windowHeight)
    : mWindowWidth(windowWidth), mWindowHeight(windowHeight)
    {
        mComponent = map;
        mName = name;
        mActiveMarker = 0;
        mWidth = map->getWidth();
        mHeight = map->getHeight();
        setHeight(mHeight);
    }
    ~GuiMap()
    {
        for(std::map<unsigned short,GuiMapMarker*>::iterator it=mMarker.begin(); it!=mMarker.end(); it++)
        {
            GuiMapMarker *marker = it->second;
            const String markerName = marker->mMarker->getName();
            mComponent->removeChild(markerName);
            OverlayManager::getSingleton().destroyOverlayElement(markerName);
            delete marker;
        }
        mMarker.clear();
    }
    const bool updateByCursor(const Real &x, const Real &y)
    {
        if(!mActiveMarker)return false;
        Real dX = mWidth>0 ? (x-mComponent->_getDerivedLeft())/mWidth : 0;
        Real dY = mHeight>0 ? (y-mComponent->_getDerivedTop())/mHeight : 0;
        if(dX<0)dX = 0;
        else if(dX>1)dX = 1;
        if(dY<0)dY = 0;
        else if(dY>1)dY = 1;
        if(dX==mActiveMarker->mX && dY==mActiveMarker->mY)return false;
        setMarkerPosition(dX,dY);
        return true;
    }
    GuiMapMarker* getMarker(const int &id=-1)
    {
        std::map<unsigned short,GuiMapMarker*>::iterator it = mMarker.find(id);
        return id==-1?mActiveMarker: (it==mMarker.end()?0:it->second);
    }
    void setMarkerPosition(const Real &x, const Real &y, const int &id=-1)
    {
        GuiMapMarker *marker = getMarker(id);
        if(!marker)return;
        OverlayElement *markerOv = marker->mMarker;
        if(x<0||x>1||y<0||y>1)markerOv->hide();
        else
        {
            markerOv->show();
            markerOv->setLeft(x*mWidth - markerOv->getWidth()*0.5f);
            markerOv->setTop(y*mHeight - markerOv->getHeight()*0.5f);
        }
        marker->mX = x;
        marker->mY = y;
    }
    void setMarkerRotation(const Real &angle, const int &id=-1)
    {
        GuiMapMarker *marker = getMarker(id);
        if(!marker)return;
        marker->mMarker->getTechnique()->getPass(0)->getTextureUnitState(0)->setTextureRotate(Degree(angle));
    }
    const std::pair<Real,Real> getMarkerPosition(const int &id=-1)
    {
        GuiMapMarker *marker = getMarker(id);
        if(!marker)return std::pair<Real,Real>(0,0);
        return std::pair<Real,Real>(marker->mX,marker->mY);
    }
    const bool isMap()
    {
        return true;
    }
    void setHeight(const Real &value)
    {
        const Real windowRatio = *mWindowWidth>0 ? Real(*mWindowHeight) / *mWindowWidth : 0.75f;
        mHeight = value;
        mWidth = value*windowRatio;
        mComponent->setHeight(mHeight);
        mComponent->setWidth(mWidth);
        for(std::map<unsigned short,GuiMapMarker*>::iterator it=mMarker.begin(); it!=mMarker.end(); it++)
        {
            GuiMapMarker *marker = it->second;
            marker->mMarker->setWidth(marker->mMarker->getHeight()*windowRatio);
            setMarkerPosition(marker->mX,marker->mY,it->first);
        }
    }
    void addMarker(const unsigned short &id, const String &material="", const Real &height=0.04f)
    {
        OverlayElement *markerOverlay = static_cast<OverlayContainer*>(OverlayManager::getSingleton().createOverlayElementFromTemplate("GuiTemplate/MapMarker","Panel",mName+"MM"+StringConverter::toString(id)));
        if(material!="")markerOverlay->setMaterialName(material);
        mComponent->addChild(markerOverlay);
        GuiMapMarker *marker = new GuiMapMarker(markerOverlay);
        mMarker[id] = marker;

        const Real windowRatio = *mWindowWidth>0 ? Real(*mWindowHeight) / *mWindowWidth : 0.75f;
        markerOverlay->setHeight(height);
        markerOverlay->setWidth(height*windowRatio);
    }
    void removeMarker(const unsigned short &id)
    {
        GuiMapMarker *marker = getMarker(id);
        if(!marker)return;
        const String markerName = marker->mMarker->getName();
        mComponent->removeChild(markerName);
        OverlayManager::getSingleton().destroyOverlayElement(markerName);
        delete marker;
        mMarker.erase(id);
    }
    void setActiveMarker(const int &id=-1)
    {
        mActiveMarker = getMarker(id);
    }
    void windowResized()
    {
        setHeight(mHeight);
    }
    void setMaterial(const String &material)
    {
        mComponent->setMaterialName(material);
    }
    void setMaterialTexture(const String &texture)
    {
        mComponent->getTechnique()->getPass(0)->getTextureUnitState(0)->setTextureName(texture);
    }
    void setMaterialScale(const Real &scale)
    {
        mComponent->getTechnique()->getPass(0)->getTextureUnitState(0)->setTextureScale(scale,scale);
    }
    void setMaterialScroll(const Real &x, const Real &y)
    {
        mComponent->getTechnique()->getPass(0)->getTextureUnitState(0)->setTextureScroll(x,y);
    }
};

#endif
