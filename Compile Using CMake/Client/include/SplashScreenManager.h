#ifndef _SPLASHSCREENMANAGER_H_
#define _SPLASHSCREENMANAGER_H_

#include "SaveFile.h"
#include "KeyboardManager.h"

class SplashScreenManager : public Singleton<SplashScreenManager>
{
private:
    Overlay *mSplashOverlay;
    OverlayElement *mLoadingScreen;
    OverlayElement *mFlavourText;
    unsigned short mNumLoadingScreens;
    unsigned short mNumFlavourTexts;
public:
    SplashScreenManager()
    {
        mSplashOverlay = OverlayManager::getSingleton().getByName("SplashScreen");
        mLoadingScreen = OverlayManager::getSingleton().getOverlayElement("SplashScreen/Loading");
        mFlavourText = OverlayManager::getSingleton().getOverlayElement("SplashScreen/FlavourText");

        //Get number of loading screens
        unsigned short i = 0;
        mNumLoadingScreens = 0;
        bool hasIndex = MaterialManager::getSingletonPtr()->resourceExists("SplashScreenMat/Loading"+StringConverter::toString(i));
        while(hasIndex)
        {
            mNumLoadingScreens++;
            hasIndex = MaterialManager::getSingletonPtr()->resourceExists("SplashScreenMat/Loading"+StringConverter::toString(++i));
        }

        //Get number of flavour texts
        SaveFile sf("flavourText.cfg");
        i = 0;
        mNumFlavourTexts = 0;
        String buffer = "";
        hasIndex = sf.getSetting(StringConverter::toString(i),buffer);
        while(hasIndex)
        {
            mNumFlavourTexts++;
            hasIndex = sf.getSetting(StringConverter::toString(++i),buffer);
        }
    }
    ~SplashScreenManager()
    {
    }
    static SplashScreenManager* getSingletonPtr();
    static SplashScreenManager& getSingleton();
    void show(const bool &flag, const bool &randomScreen=true)
    {
        if(flag)
        {
            mSplashOverlay->show();
            if(randomScreen)
            {
                setLoadingMat(rand()%mNumLoadingScreens);
                setFlavourText(rand()%mNumFlavourTexts);
            }
        }
        else mSplashOverlay->hide();
    }
    const bool isVisible()
    {
        return mSplashOverlay->isVisible();
    }
    void setLoadingMat(const unsigned char &index)
    {
        mLoadingScreen->setMaterialName("SplashScreenMat/Loading"+StringConverter::toString(index));
    }
    void setFlavourText(const unsigned char &index)
    {
        SaveFile sf("flavourText.cfg");
        String buffer = "";
        sf.getSetting(StringConverter::toString(index),buffer);
        replaceKeysTex(buffer);
        Font *font = dynamic_cast<Font*>(FontManager::getSingleton().getByName(mFlavourText->getParameter("font_name")).getPointer());
        const Real charHeight = StringConverter::parseReal(mFlavourText->getParameter("char_height"));
        GuiTextComponent::wrapCaption(buffer,font,charHeight,mFlavourText->getWidth());
        mFlavourText->setCaption(buffer);
    }
    void replaceKeysTex(String &caption)
    {
        String result = "";
        String replaceIndex = "";
        bool readingIndex = false;
        for(String::iterator it=caption.begin(); it!=caption.end(); it++)
        {
            const char c = *it;
            if(c=='%')
            {
                if(readingIndex)
                {
                    KeyboardManager *keyboardMgr = KeyboardManager::getSingletonPtr();
                    const unsigned short index = StringConverter::parseInt(replaceIndex);
                    result += keyboardMgr->getKeyName(keyboardMgr->getKey(index));
                    replaceIndex = "";
                }
                readingIndex = !readingIndex;
            }
            else
            {
                if(readingIndex)replaceIndex.push_back(c);
                else result.push_back(c);
            }
        }
        caption = result;
    }
};

template<> SplashScreenManager* Singleton<SplashScreenManager>::ms_Singleton = 0;

SplashScreenManager* SplashScreenManager::getSingletonPtr()
{
	return ms_Singleton;
}

SplashScreenManager& SplashScreenManager::getSingleton()
{
	assert(ms_Singleton);
	return *ms_Singleton;
}

#endif

