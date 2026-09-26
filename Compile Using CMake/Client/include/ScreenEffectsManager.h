#ifndef _SCREENEFFECTSMANAGER_H_
#define _SCREENEFFECTSMANAGER_H_

#include "CapabilitiesManager.h"

#define MAPPEEK_DURATION 0.25f
#define MAPPEEK_STAYDURATION 1.0f
#define MAPTITLE_STAYDURATION 2.0f
#define MAPTITLE_OUTDURATION 1.0f

class ScreenEffectsManager : public Singleton<ScreenEffectsManager>
{
private:
    Overlay *mFaderOverlay;
    OverlayElement *mFader;
    bool mFadeIn;
    bool mFadeOut;
    Real mFadeTimer;
    Real mFadeDuration;
    ColourValue mFadeColour;

    Overlay *mUnderwaterOverlay;
    OverlayElement *mUnderwater;

    Overlay *mMapPeekOverlay;
    OverlayElement *mMapPeek;
    OverlayElement *mMapPeekText;
    String mMapPeekName;
    Real mMapPeekTimer;
    bool mMapPeekOut;

    Overlay *mMapTitleOverlay;
    OverlayElement *mMapTitleText;
    String mMapTitleName;
    Real mMapTitleTimer;
    bool mMapTitleOut;

    bool mSkipOneUpdate;
public:
    ScreenEffectsManager()
    {
        mFaderOverlay = OverlayManager::getSingleton().getByName("Fader");
        mFader = OverlayManager::getSingleton().getOverlayElement("Fader");

        mUnderwaterOverlay = OverlayManager::getSingleton().getByName("UnderwaterFader");
        mUnderwater = OverlayManager::getSingleton().getOverlayElement("UnderwaterFader");
        setUnderwaterColour(ColourValue(0.3f,0.7f,0.74f));

        mMapPeekOverlay = OverlayManager::getSingleton().getByName("MapPeek");
        mMapPeek = OverlayManager::getSingleton().getOverlayElement("MapPeek");
        mMapPeekText = OverlayManager::getSingleton().getOverlayElement("MapPeek/Text");

        mMapTitleOverlay = OverlayManager::getSingleton().getByName("MapTitle");
        mMapTitleText = OverlayManager::getSingleton().getOverlayElement("MapTitle/Text");


        reset();
    }
    ~ScreenEffectsManager()
    {
    }
    static ScreenEffectsManager* getSingletonPtr();
    static ScreenEffectsManager& getSingleton();
    void reset()
    {
        stopFading();
        setUnderwater(false);
        showMapPeek(false);
        showMapTitle(false);
        mSkipOneUpdate = false;
    }
    void update(const Real &timeElapsed)
    {
        if(mSkipOneUpdate)
        {
            mSkipOneUpdate = false;
            return;
        }
        updateFader(timeElapsed);
        updateMapPeek(timeElapsed);
        updateMapTitle(timeElapsed);
    }
    void updateFader(const Real &timeElapsed)
    {
        if(mFadeIn)
        {
            mFadeTimer -= timeElapsed;
            if(mFadeTimer<=0)
            {
                stopFading();
                return;
            }
        }
        else if(mFadeOut)
        {
            mFadeTimer += timeElapsed;
            if(mFadeTimer>=mFadeDuration)
            {
                mFadeOut = false;
                setFaderRatio(1);
                return;
            }
        }
        else return;
        setFaderRatio(mFadeTimer/mFadeDuration);
    }
    void setFaderRatio(const Real &ratio)
    {
        mFader->setCustomParameter(1,Vector4(mFadeColour.r,mFadeColour.g,mFadeColour.b,ratio));
        CapabilitiesManager::getSingletonPtr()->assertCustomColour(mFader,1);
    }
    void doFadeIn(const Real &duration=2.0f, const ColourValue &colour=ColourValue::White)
    {
        mFadeIn = true;
        mFadeOut = false;
        mFadeTimer = mFadeDuration = duration;
        mFadeColour = colour;
        setFaderRatio(1);
        mFaderOverlay->show();
        mFader->show();
    }
    void doFadeOut(const Real &duration=2.0f, const ColourValue &colour=ColourValue::White)
    {
        mFadeOut = true;
        mFadeIn = false;
        mFadeTimer = 0;
        mFadeDuration = duration;
        mFadeColour = colour;
        setFaderRatio(0);
        mFaderOverlay->show();
        mFader->show();
    }
    void stopFading()
    {
        mFadeIn = false;
        mFadeOut = false;
        mFaderOverlay->hide();
        mFader->hide();
    }
    void skipOneUpdate()
    {
        mSkipOneUpdate = true;
    }
    void setUnderwater(const bool &flag)
    {
        if(flag)mUnderwaterOverlay->show();
        else mUnderwaterOverlay->hide();
    }
    void setUnderwaterColour(const ColourValue &colour)
    {
        mUnderwater->setCustomParameter(1,Vector4(colour.r,colour.g,colour.b,0.2f));
        CapabilitiesManager::getSingletonPtr()->assertCustomColour(mUnderwater,1);
    }
    void showMapPeek(const bool &flag, const String &name="")
    {
        if(flag)
        {
            //Already visible
            if(mMapPeekTimer>0)
            {
                //Same name, keep visible
                if(mMapPeekName==name)
                {
                    if(mMapPeekOut)
                    {
                        mMapPeekOut = false;
                        mMapPeekTimer = MAPPEEK_DURATION+MAPPEEK_STAYDURATION - mMapPeekTimer;
                    }
                }
                //Transtion out
                else
                {
                    if(mMapPeekTimer>MAPPEEK_DURATION)mMapPeekTimer = MAPPEEK_DURATION;
                    mMapPeekOut = true;
                }
            }
            //Transition in
            else
            {
                mMapPeekTimer = MAPPEEK_DURATION+MAPPEEK_STAYDURATION;
                mMapPeekOut = false;
                mMapPeekName = name;
                mMapPeekText->setCaption(name);
                mMapPeek->setTop(-0.03f);
                mMapPeekOverlay->show();
            }
        }
        else
        {
            mMapPeekTimer = 0;
            mMapPeekOut = false;
            mMapPeekName = "";
            mMapPeekOverlay->hide();
        }
    }
    void updateMapPeek(const Real &timeElapsed)
    {
        if(mMapPeekTimer<=0)return;

        mMapPeekTimer -= timeElapsed;

        Real ratio = 1;
        if(mMapPeekOut)
        {
            if(mMapPeekTimer<=0)
            {
                showMapPeek(false);
                return;
            }
            ratio = 1 - mMapPeekTimer/MAPPEEK_DURATION;
        }
        else
        {
            if(mMapPeekTimer<=0)
            {
                mMapPeekOut = true;
                mMapPeekTimer = MAPPEEK_DURATION;
                ratio = 0;
            }
            else
            {
                const Real diff = mMapPeekTimer-MAPPEEK_STAYDURATION;
                if(diff>0)ratio = diff/MAPPEEK_DURATION;
                else ratio = 0;
            }
        }

        mMapPeek->setTop(-ratio*0.03f);
    }
    void showMapTitle(const bool &flag, const String &name="")
    {
        if(flag)
        {
            //Already visible
            if(mMapTitleTimer>0)
            {
                //Same name, keep visible
                if(mMapTitleName==name)
                {
                    if(mMapTitleOut)
                    {
                        mMapTitleOut = false;
                        mMapTitleTimer = MAPTITLE_STAYDURATION - mMapTitleTimer;
                    }
                }
                //Transtion out
                else if(!mMapTitleOut)
                {
                    mMapTitleTimer = MAPTITLE_OUTDURATION;
                    mMapTitleOut = true;
                }
            }
            //Appear and stay
            else
            {
                mMapTitleTimer = MAPTITLE_STAYDURATION;
                mMapTitleOut = false;
                mMapTitleName = name;
                mMapTitleText->setCaption(name);
                mMapTitleOverlay->show();
            }
        }
        else
        {
            mMapTitleTimer = 0;
            mMapTitleOut = false;
            mMapTitleName = "";
            mMapTitleOverlay->hide();
        }
    }
    void updateMapTitle(const Real &timeElapsed)
    {
        if(mMapTitleTimer<=0)return;

        mMapTitleTimer -= timeElapsed;

        Real ratio = 1;
        if(mMapTitleOut)
        {
            if(mMapTitleTimer<=0)
            {
                showMapTitle(false);
                return;
            }
            ratio = 1 - mMapTitleTimer/MAPTITLE_OUTDURATION;
        }
        else
        {
            if(mMapTitleTimer<=0)
            {
                mMapTitleOut = true;
                mMapTitleTimer = MAPTITLE_OUTDURATION;
            }
            ratio = 0;
        }

        Pass *pass = mMapTitleText->getMaterial()->getTechnique(0)->getPass(0);
        pass->getTextureUnitState(0)->setAlphaOperation(Ogre::LBX_SUBTRACT, Ogre::LBS_TEXTURE, Ogre::LBS_MANUAL, 0, ratio);
    }
};

template<> ScreenEffectsManager* Singleton<ScreenEffectsManager>::ms_Singleton = 0;

ScreenEffectsManager* ScreenEffectsManager::getSingletonPtr()
{
	return ms_Singleton;
}

ScreenEffectsManager& ScreenEffectsManager::getSingleton()
{
	assert(ms_Singleton);
	return *ms_Singleton;
}

#endif

