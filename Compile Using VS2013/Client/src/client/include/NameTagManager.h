#ifndef _NAMETAGMANAGER_H_
#define _NAMETAGMANAGER_H_

#include <Gui.h>
#include <CameraManager.h>
#include <Object.h>
#include <OgreFontManager.h>

class NameTag
{
private:
    Object *mOwner;
    Overlay *mOverlay;
    OverlayContainer *mBox;
    OverlayElement *mCaption;
    Real mSize;
    Real mYOffset;  //Offsets by ratio of box's height
    bool mHidden;
public:
    NameTag(const unsigned int &i, Object *owner, const unsigned short &fontSize, const Real &relSize=1.0f, const Real &yOffset=0)
    {
        mOwner = owner;
        mSize = relSize;
        mYOffset = yOffset;
        mHidden = false;

        const String name = "NameTag"+StringConverter::toString(i);
        OverlayManager *overlayMgr = OverlayManager::getSingletonPtr();
        mOverlay = overlayMgr->create(name);
        mBox = static_cast<OverlayContainer*>(overlayMgr->createOverlayElementFromTemplate("GuiTemplate/NameTag","Panel",name+"Box"));
        mCaption = overlayMgr->createOverlayElementFromTemplate("GuiTemplate/NameTagText","TextArea",name+"Text");

        setFontSize(fontSize);
        mOverlay->add2D(mBox);
        mBox->addChild(mCaption);
        mOverlay->hide();
    }
    ~NameTag()
    {
        mOverlay->hide();
        OverlayManager *overlayMgr = OverlayManager::getSingletonPtr();
        mBox->removeChild(mCaption->getName());
        mOverlay->remove2D(mBox);
        overlayMgr->destroyOverlayElement(mCaption);
        overlayMgr->destroyOverlayElement(mBox);
        overlayMgr->destroy(mOverlay);
    }
    const Object* getOwner()
    {
        return mOwner;
    }
    void setCaption(const String &caption)
    {
        mCaption->setCaption(caption);
        //Resize box
        Font *font = dynamic_cast<Font*>(FontManager::getSingleton().getByName(mCaption->getParameter("font_name")).getPointer());
        const Real charHeight = StringConverter::parseReal(mCaption->getParameter("char_height")) / *(Gui::getSingletonPtr()->mWindowHeight);
        Real width = 0;
        for(int i=0;i<(int)caption.length();i++)
        {
            if(caption[i]==' ')width += font->getGlyphAspectRatio(0x0030);
            else width += font->getGlyphAspectRatio(caption[i]);
        }
        mBox->setWidth(width*charHeight);
        mBox->setHeight(charHeight);
    }
    void setFontSize(const unsigned short &size)
    {
        if(mSize==1.0f)
        {
            mCaption->setParameter("char_height",StringConverter::toString(size));
            mCaption->setTop(Real(size)*-0.4285f);
        }
        else
        {
            const unsigned short relSize = int(size*mSize);
            mCaption->setParameter("char_height",StringConverter::toString(relSize));
            mCaption->setTop(Real(relSize)*-0.4285f);
        }
        //Resize boxes
        setCaption(mCaption->getCaption());
    }
    void update(const Plane &camPlane, Camera *camera, const bool placeAtTop=false)
    {
        if(mHidden)return;
        if(!mOwner->mIsInEarshot)
        {
            mOverlay->hide();
            return;
        }
        /*const MovableObject* movable = mOwner->getMainMovableObject();
        if(!movable || !movable->isInScene())return;

        const AxisAlignedBox AABB = movable->getWorldBoundingBox(true);
        const Vector3 cornersOfAABB[4] =
            {placeAtTop ? AABB.getCorner(AxisAlignedBox::FAR_LEFT_TOP) : AABB.getCorner(AxisAlignedBox::FAR_LEFT_BOTTOM),
            placeAtTop ? AABB.getCorner(AxisAlignedBox::FAR_RIGHT_TOP) : AABB.getCorner(AxisAlignedBox::FAR_RIGHT_BOTTOM),
            placeAtTop ? AABB.getCorner(AxisAlignedBox::NEAR_LEFT_TOP) : AABB.getCorner(AxisAlignedBox::NEAR_LEFT_BOTTOM),
            placeAtTop ? AABB.getCorner(AxisAlignedBox::NEAR_RIGHT_TOP) : AABB.getCorner(AxisAlignedBox::NEAR_RIGHT_BOTTOM)};

        Real minX = 0,minY = 0,maxX = 0,maxY = 0;
        Real x[4],y[4];
        for(int i=0;i<4;i++)
        {
            x[i] = 0;
            y[i] = 0;
            getScreenCoords(camera,cornersOfAABB[i],x[i],y[i]);
            if(camPlane.getSide(cornersOfAABB[i]) == Plane::NEGATIVE_SIDE)
            {
                if(i==0)
                {
                    minX = x[i];
                    minY = y[i];
                    maxX = x[i];
                    maxY = y[i];
                }
                else
                {
                    if(minX > x[i])minX = x[i];
                    if(minY > y[i])minY = y[i];
                    if(maxX < x[i])maxX = x[i];
                    if(maxY < y[i])maxY = y[i];
                }
            }
            else
            {
                minX = 0;
                minY = 0;
                maxX = 0;
                maxY = 0;
                break;
            }
        const Real boxWidth = mBox->getWidth();
        const Real boxHeight = mBox->getHeight();
        const Real finalX = 1-(minX + maxX + boxWidth)*0.5f;
        const Real finalY = 1-maxY-(placeAtTop?boxHeight:0) + mYOffset;
        if(finalX+boxWidth>0 && finalX<1 && finalY+boxHeight>0 && finalY<1)
        {
            mBox->setPosition(finalX,finalY);
            mOverlay->show();
        }
        else mOverlay->hide();
        }*/
        const Vector3 position = mOwner->getPosition();
        if(camPlane.getSide(position) == Plane::NEGATIVE_SIDE)
        {
            Real x=0,y=0;
            getScreenCoords(camera,position,x,y);
            const Real boxWidth = mBox->getWidth();
            const Real boxHeight = mBox->getHeight();
            const Real finalX = 1-(x+boxWidth*0.5f);
            const Real finalY = 1-y-(placeAtTop?boxHeight:0) + mYOffset*boxHeight;
            mBox->setPosition(finalX,finalY);
            mOverlay->show();
        }
        else mOverlay->hide();
    }
    void getScreenCoords(Camera *cam, const Vector3 &position, Real &x, Real &y)
    {
        const Vector3 hcsPosition = cam->getProjectionMatrix() * (cam->getViewMatrix() * position);

        x = 1.0f - ((hcsPosition.x * 0.5f) + 0.5f);
        y = ((hcsPosition.y * 0.5f) + 0.5f);
    }
    void show(const bool &flag)
    {
        mHidden = !flag;
        if(mHidden)mOverlay->hide();
    }
};

class NameTagManager : public Singleton<NameTagManager>
{
private:
    CameraManager *mCameraMgr;
    std::vector<NameTag*> mNameTags;
    unsigned int mNameTagIndex;
    unsigned short mFontSize;
    bool mHideNameTags;
public:
    NameTagManager()
    {
        mNameTagIndex = 0;
        mFontSize = 14;
        mHideNameTags = false;
    }
    ~NameTagManager()
    {
        clear();
    }
    static NameTagManager* getSingletonPtr();
    static NameTagManager& getSingleton();
    void init()
    {
        mCameraMgr = CameraManager::getSingletonPtr();
    }
    void update(const Real &timeElapsed)
    {
        const Vector3 camDir = mCameraMgr->getDirection();
        const Plane camPlane = Plane(camDir,mCameraMgr->getPosition());
        Camera *camera = mCameraMgr->getCamera();
        for(std::vector<NameTag*>::iterator i=mNameTags.begin(); i!=mNameTags.end(); i++)
        {
            NameTag* nametag = *i;
            nametag->update(camPlane,camera);
        }
    }
    void clear()
    {
        while(!mNameTags.empty())
        {
            NameTag *nametag = mNameTags.back();
            mNameTags.pop_back();
            delete nametag;
        }
    }
    void createNameTag(Object *object, const String &name, const Real &relSize=1.0f, const Real &yOffset=0)
    {
        NameTag *nametag = new NameTag(mNameTagIndex++,object,mFontSize,relSize,yOffset);
        nametag->setCaption(name);
        nametag->show(!mHideNameTags);

        mNameTags.push_back(nametag);
    }
    void deleteNameTagsOf(Object *object)
    {
        std::vector<NameTag*>::iterator it=mNameTags.begin();
        while(it!=mNameTags.end())
        {
            NameTag *nametag = *it;
            if(nametag->getOwner()==object)
            {
                delete nametag;
                it = mNameTags.erase(it);
            }
            else it++;
        }
    }
    void setFontSize(const unsigned short &size)
    {
        mFontSize = size;
        for(std::vector<NameTag*>::iterator i=mNameTags.begin(); i!=mNameTags.end(); i++)
        {
            NameTag* nametag = *i;
            nametag->setFontSize(size);
        }
    }
    const unsigned short getFontSize()
    {
        return mFontSize;
    }
    void showNameTags(const bool &flag, const bool &temporary=false)
    {
        if(!temporary)mHideNameTags = !flag;
        for(std::vector<NameTag*>::iterator i=mNameTags.begin(); i!=mNameTags.end(); i++)
        {
            NameTag* nametag = *i;
            nametag->show(flag);
        }
    }
    const bool getShowNameTags()
    {
        return (!mHideNameTags);
    }
};

template<> NameTagManager* Singleton<NameTagManager>::ms_Singleton = 0;

NameTagManager* NameTagManager::getSingletonPtr()
{
	return ms_Singleton;
}

NameTagManager& NameTagManager::getSingleton()
{
	assert(ms_Singleton);
	return *ms_Singleton;
}

#endif
