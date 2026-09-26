#ifndef _OPTIONSMANAGER_H_
#define _OPTIONSMANAGER_H_

//#define USE_OGRE_161

#include <Gui.h>
#include <SaveFile.h>
#include <SoundManager.h>
#include <KeyboardManager.h>

class OptionsManager : public Singleton<OptionsManager>
{
private:
    SceneManager *mSceneMgr;
    Gui *mGui;
    SoundManager *mSoundMgr;
    WaterManager *mWaterMgr;
    ChatManager *mChatMgr;
    NameTagManager *mNameTagMgr;
    GridManager *mGridMgr;
    KeyboardManager *mKeyboardMgr;
    ControlManager *mControlMgr;
    GuiPanel *mPanel;

    GuiHighlightList *mInterfaceList;
    GuiSlider *mInterfaceSlider;
    GuiHighlightList *mInterfaceSelectList;
    GuiSlider *mInterfaceSelectListSlider;
    OverlayElement *mInterfaceSliderLabel;
    GuiCheckBox *mInterfaceCheckBox;
    OverlayElement *mInterfaceCheckBoxLabel;
    Real mMouseSensitivity;

    GuiHighlightList *mVideoList;
    GuiHighlightList *mVideoSelectList;
    GuiSlider *mVideoSelectListSlider;
    GuiCheckBox *mVideoCheckBox;
    OverlayElement *mVideoCheckBoxLabel;
    GuiSlider *mVideoSlider;
    OverlayElement *mVideoSliderLabel;
    bool mIsOpenGL;
    unsigned char mVideoMode;
    bool mFullScreen;
    unsigned char mAntiAliasing;
    bool mShadows;
    Real mShadowDetail;

    GuiHighlightList *mSoundList;
    GuiSlider *mSoundSlider;
    OverlayElement *mSoundLabel;

    GuiHighlightDoubleList *mControlList;
    bool mRestartRequired;
public:
    OptionsManager()
    {
        mGui = Gui::getSingletonPtr();
        mSoundMgr = SoundManager::getSingletonPtr();
        mWaterMgr = WaterManager::getSingletonPtr();
        mChatMgr = ChatManager::getSingletonPtr();
        mNameTagMgr = NameTagManager::getSingletonPtr();
        mGridMgr = GridManager::getSingletonPtr();
        mKeyboardMgr = KeyboardManager::getSingletonPtr();
        mControlMgr = ControlManager::getSingletonPtr();
        mRestartRequired = false;

        defaultInterface(false);
        defaultVideo(false);
        defaultSound(false);
        load();
    }
    virtual ~OptionsManager()
    {
        save();
    }
    void init(SceneManager *sceneMgr)
    {
        mSceneMgr = sceneMgr;
        mPanel = mGui->addPanel("OptionsScreen");
        mPanel->show(false);

        //Interface
        mInterfaceList = mPanel->getHighlightList("OptionsScreen/Interfaces");
        mInterfaceList->pushLine("Mouse Sensitivity");
        mInterfaceList->pushLine("Chat Font Size");
        mInterfaceList->pushLine("Name Tag Size");
        mInterfaceList->pushLine("Running Mode");
        mInterfaceList->pushLine("Arrival Chat Name Type");
        mInterfaceList->pushLine("Local Chat Name Type");
        mInterfaceList->pushLine("General Chat Name Type");
        mInterfaceList->pushLine("Caret Offset");
        mInterfaceList->update(0);
        mInterfaceList->setSelection(0);
        mInterfaceSelectList = mPanel->getHighlightList("OptionsScreen/InterfaceSelect");
        mInterfaceSelectList->update(0);
        mInterfaceSelectListSlider = mPanel->getSlider("OptionsScreen/InterfaceSelectLS");
        mInterfaceCheckBox = mPanel->getCheckBox("OptionsScreen/InterfaceCB");
        mInterfaceCheckBoxLabel = OverlayManager::getSingleton().getOverlayElement("OptionsScreen/InterfaceCBLabel");
        mInterfaceSlider = mPanel->getSlider("OptionsScreen/InterfaceSl");
        mInterfaceSliderLabel = OverlayManager::getSingleton().getOverlayElement("OptionsScreen/InterfaceSlLabel");

        updateInterfaceOption();

        //Video
        mVideoList = mPanel->getHighlightList("OptionsScreen/Videos");
        mVideoList->pushLine("Rendering Subsystem");
        mVideoList->pushLine("Video Mode");
        mVideoList->pushLine("Full Screen");
        mVideoList->pushLine("Anti-aliasing");
        //mVideoList->pushLine("Shadows");
        mVideoList->pushLine("Water Reflection");
        mVideoList->pushLine("View Distance");
        mVideoList->update(0);
        mVideoList->setSelection(0);
        mVideoSelectList = mPanel->getHighlightList("OptionsScreen/VideoSelect");
        mVideoSelectList->update(0);
        mVideoSelectListSlider = mPanel->getSlider("OptionsScreen/VideoSelectLS");
        mVideoCheckBox = mPanel->getCheckBox("OptionsScreen/VideoCB");
        mVideoCheckBoxLabel = OverlayManager::getSingleton().getOverlayElement("OptionsScreen/VideoCBLabel");
        mVideoSlider = mPanel->getSlider("OptionsScreen/VideoSl");
        mVideoSliderLabel = OverlayManager::getSingleton().getOverlayElement("OptionsScreen/VideoSlLabel");

        updateVideoOption();
        /*if(mShadows)
        {
            mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_MODULATIVE);
            mSceneMgr->setShadowTextureSize((unsigned short)Math::Pow(2,int(8+mShadowDetail*4)));
        }*/

        //Sound
        mSoundList = mPanel->getHighlightList("OptionsScreen/Sounds");
        mSoundList->pushLine("Effects Volume");
        mSoundList->pushLine("Music Volume");
        mSoundList->pushLine("GUI Volume");
        mSoundList->update(0);
        mSoundList->setSelection(0);
        mSoundSlider = mPanel->getSlider("OptionsScreen/SetSound");
        mSoundLabel = OverlayManager::getSingleton().getOverlayElement("OptionsScreen/SoundValueLabel");

        updateSoundSlider();

        //Controls
        mControlList = mPanel->getHighlightDoubleList("OptionsScreen/Controls");
        mKeyboardMgr->setList(mControlList);
    }
    void reset()
    {
        mKeyboardMgr->setList(0);
    }
    void save()
    {
        SaveFile sf("settings.cfg");

        //Interface
        sf.setSetting("MouseSensitivity",StringConverter::toString(mMouseSensitivity),"Interface");
        sf.setSetting("ChatFontSize",StringConverter::toString(mChatMgr->getChatFontSize()),"Interface");
        sf.setSetting("NameTagSize",StringConverter::toString(mNameTagMgr->getFontSize()),"Interface");
        sf.setSetting("NameTagShow",StringConverter::toString(mNameTagMgr->getShowNameTags()),"Interface");
        sf.setSetting("RelativeRunningMode",StringConverter::toString(mControlMgr->getRelativeMode()),"Interface");
        sf.setSetting("ArrivalChatNameType",StringConverter::toString(mChatMgr->mSystemChatNameType),"Interface");
        sf.setSetting("LocalChatNameType",StringConverter::toString(mChatMgr->mLocalChatNameType),"Interface");
        sf.setSetting("GeneralChatNameType",StringConverter::toString(mChatMgr->mGeneralChatNameType),"Interface");
        sf.setSetting("CaretOffset",StringConverter::toString(GuiTextHandler::getSingletonPtr()->m_fCaretOffset),"Interface");
        //Video
        sf.setSetting("Shadows",StringConverter::toString(mShadows),"Video");
        sf.setSetting("ShadowDetail",StringConverter::toString(mShadowDetail),"Video");
        sf.setSetting("WaterReflection",StringConverter::toString(mWaterMgr->getDoReflection()),"Video");
        sf.setSetting("ViewDistance",StringConverter::toString(mGridMgr->getViewDistanceRatio()),"Video");
        //Sound
        sf.setSetting("Effects",StringConverter::toString(mSoundMgr->getVolume(0)),"Sound");
        sf.setSetting("Music",StringConverter::toString(mSoundMgr->getVolume(1)),"Sound");
        sf.setSetting("GUI",StringConverter::toString(mSoundMgr->getVolume(2)),"Sound");

        sf.save();
    }
    const bool load()
    {
        SaveFile sf;
        if(!sf.load("settings.cfg"))return false;

        String buffer = "";
        //Interface
        if(sf.getSetting("MouseSensitivity",buffer,"Interface"))mMouseSensitivity = StringConverter::parseReal(buffer);
        buffer = "";
        if(sf.getSetting("ChatFontSize",buffer,"Interface"))mChatMgr->setChatFontSize(StringConverter::parseInt(buffer));
        buffer = "";
        if(sf.getSetting("NameTagSize",buffer,"Interface"))mNameTagMgr->setFontSize(StringConverter::parseInt(buffer));
        buffer = "";
        if(sf.getSetting("NameTagShow",buffer,"Interface"))mNameTagMgr->showNameTags(StringConverter::parseBool(buffer));
        buffer = "";
        if(sf.getSetting("RelativeRunningMode",buffer,"Interface"))mControlMgr->setRelativeMode(StringConverter::parseBool(buffer));
        buffer = "";
        if(sf.getSetting("ArrivalChatNameType",buffer,"Interface"))mChatMgr->mSystemChatNameType = StringConverter::parseInt(buffer);
        buffer = "";
        if(sf.getSetting("LocalChatNameType",buffer,"Interface"))mChatMgr->mLocalChatNameType = StringConverter::parseInt(buffer);
        buffer = "";
        if(sf.getSetting("GeneralChatNameType",buffer,"Interface"))mChatMgr->mGeneralChatNameType = StringConverter::parseInt(buffer);
        buffer = "";
        if(sf.getSetting("CaretOffset",buffer,"Interface"))GuiTextHandler::getSingletonPtr()->m_fCaretOffset = StringConverter::parseReal(buffer);

        //Sound
        buffer = "";
        if(sf.getSetting("Effects",buffer,"Sound"))mSoundMgr->setEffectsVolume(StringConverter::parseReal(buffer));
        buffer = "";
        if(sf.getSetting("Music",buffer,"Sound"))mSoundMgr->setMusicVolume(StringConverter::parseReal(buffer));
        buffer = "";
        if(sf.getSetting("GUI",buffer,"Sound"))mSoundMgr->setGUIVolume(StringConverter::parseReal(buffer));

        //Video
        buffer = "";
        if(sf.getSetting("Shadows",buffer,"Video"))mShadows = StringConverter::parseBool(buffer);
        buffer = "";
        if(sf.getSetting("ShadowDetail",buffer,"Video"))mShadowDetail = StringConverter::parseReal(buffer);
        buffer = "";
        if(sf.getSetting("WaterReflection",buffer,"Video"))mWaterMgr->doReflection(StringConverter::parseBool(buffer));
        buffer = "";
        if(sf.getSetting("ViewDistance",buffer,"Video"))mGridMgr->setViewDistanceRatio(StringConverter::parseReal(buffer));
        SaveFile ogreCfg;
        if(!ogreCfg.load("ogre.cfg"))return true;

        buffer = "";
        if(ogreCfg.getSetting("Render System",buffer))mIsOpenGL = (buffer=="OpenGL Rendering Subsystem");
        buffer = "";
        if(ogreCfg.getSetting("Video Mode",buffer,mIsOpenGL?"OpenGL Rendering Subsystem":"Direct3D9 Rendering Subsystem"))
            for(int i=0;i<5;i++)if(getVideoMode(i)==buffer){mVideoMode = i; break;}
        buffer = "";
        if(ogreCfg.getSetting("Full Screen",buffer,mIsOpenGL?"OpenGL Rendering Subsystem":"Direct3D9 Rendering Subsystem"))
            mFullScreen = StringConverter::parseBool(buffer);
        buffer = "";
        if(ogreCfg.getSetting(mIsOpenGL?"FSAA":"Anti aliasing",buffer,mIsOpenGL?"OpenGL Rendering Subsystem":"Direct3D9 Rendering Subsystem"))
            for(int i=0;i<7;i++)if(getAntiAliasing(i)==buffer){mAntiAliasing = i; break;}

        return true;
    }
    void show(const bool &flag)
    {
        mPanel->show(flag);
        if(flag)mGui->setActivePanel(mPanel);
    }
    void toggleShow()
    {
        show(!mPanel->isVisible());
    }
    static OptionsManager* getSingletonPtr();
    static OptionsManager& getSingleton();
    void updateInterfaceOption()
    {
        mInterfaceSelectList->show(false);
        mInterfaceSelectListSlider->show(false);
        mInterfaceCheckBox->show(false);
        mInterfaceCheckBoxLabel->hide();
        mInterfaceSlider->show(false);
        mInterfaceSliderLabel->hide();
        const unsigned short selection = mInterfaceList->getSelectedLine();
        if(selection==0)
        {
            const Real value = mMouseSensitivity-0.5f;
            mInterfaceSlider->show(true);
            mInterfaceSliderLabel->show();
            mInterfaceSlider->setValue(value);
            mInterfaceSliderLabel->setCaption(StringConverter::toString(value<0.01f?0:value,3));
        }
        else if(selection==1)
        {
            const Real value = Real(mChatMgr->getChatFontSize()-14)/6;
            mInterfaceSlider->show(true);
            mInterfaceSliderLabel->show();
            mInterfaceSlider->setValue(value);
            mInterfaceSliderLabel->setCaption(StringConverter::toString(mChatMgr->getChatFontSize()));
        }
        else if(selection==2)
        {
            mInterfaceCheckBox->show(true);
            mInterfaceCheckBoxLabel->show();
            mInterfaceCheckBox->setChecked(mNameTagMgr->getShowNameTags());
            mInterfaceCheckBoxLabel->setCaption("Show Nametags:");
            const Real value = Real(mNameTagMgr->getFontSize()-12)/8;
            mInterfaceSlider->show(true);
            mInterfaceSliderLabel->show();
            mInterfaceSlider->setValue(value);
            mInterfaceSliderLabel->setCaption(StringConverter::toString(mNameTagMgr->getFontSize()));
        }
        else if(selection==3)
        {
            mInterfaceCheckBox->show(true);
            mInterfaceCheckBoxLabel->show();
            mInterfaceCheckBox->setChecked(mControlMgr->getRelativeMode());
            mInterfaceCheckBoxLabel->setCaption("Relative Mode:");
        }
        else if(selection==4 || selection==5 || selection==6)
        {
            mInterfaceSelectList->clear();
            mInterfaceSelectList->pushLine("Character Name");
            mInterfaceSelectList->pushLine("User Name");
            mInterfaceSelectList->pushLine("Both");
            if(selection==4)mInterfaceSelectList->pushLine("Don't Show");
            mInterfaceSelectList->setSelection(selection==4 ? mChatMgr->mSystemChatNameType: (selection==5 ? mChatMgr->mLocalChatNameType : mChatMgr->mGeneralChatNameType));
            mInterfaceSelectListSlider->updateSliderToListSelection();
            mInterfaceSelectList->show(true);
        }
        else if(selection==7)
        {
            const Real fOffset = GuiTextHandler::getSingletonPtr()->m_fCaretOffset;
            const Real value = (fOffset + 0.01f)*50;
            mInterfaceSlider->show(true);
            mInterfaceSliderLabel->show();
            mInterfaceSlider->setValue(value);
            mInterfaceSliderLabel->setCaption(StringConverter::toString(Math::Abs(fOffset)<0.0001f?0.0f:fOffset,1));
        }
    }
    void updateInterfaceSetting()
    {
        const unsigned short selection = mInterfaceList->getSelectedLine();
        if(selection==0)mMouseSensitivity = mInterfaceSlider->getValue()+0.5f;
        else if(selection==1)
        {
            mChatMgr->setChatFontSize(int(mInterfaceSlider->getValue()*6)+14);
            mInterfaceSliderLabel->setCaption(StringConverter::toString(mChatMgr->getChatFontSize()));
        }
        else if(selection==2)
        {
            mNameTagMgr->showNameTags(mInterfaceCheckBox->isChecked());
            mNameTagMgr->setFontSize(int(mInterfaceSlider->getValue()*8)+12);
            mInterfaceSliderLabel->setCaption(StringConverter::toString(mNameTagMgr->getFontSize()));
        }
        else if(selection==3)
        {
            mControlMgr->setRelativeMode(mInterfaceCheckBox->isChecked());
        }
        else if(selection==4)
        {
            mChatMgr->mSystemChatNameType = mInterfaceSelectList->getSelectedLine();
        }
        else if(selection==5)
        {
            mChatMgr->mLocalChatNameType = mInterfaceSelectList->getSelectedLine();
        }
        else if(selection==6)
        {
            mChatMgr->mGeneralChatNameType = mInterfaceSelectList->getSelectedLine();
        }
        else if(selection==7)
        {
            GuiTextHandler::getSingletonPtr()->m_fCaretOffset = mInterfaceSlider->getValue()/50-0.01f;
            const Real fOffset = GuiTextHandler::getSingletonPtr()->m_fCaretOffset;
            mInterfaceSliderLabel->setCaption(StringConverter::toString(Math::Abs(fOffset)<0.0001f?0.0f:fOffset,1));
        }
    }
    void defaultInterface(const bool &update=true)
    {
        mMouseSensitivity = 1.0f;
        mChatMgr->setChatFontSize(14);
        mControlMgr->setRelativeMode(false);
        mChatMgr->mSystemChatNameType = 1;
        mChatMgr->mLocalChatNameType = 0;
        mChatMgr->mGeneralChatNameType = 1;
        GuiTextHandler::getSingletonPtr()->m_fCaretOffset = -0.004f;
        if(update)updateInterfaceOption();
    }
    void updateSoundOption()
    {
        const unsigned short selection = mSoundList->getSelectedLine();
        switch(selection)
        {
            case 0: mSoundMgr->setEffectsVolume(mSoundSlider->getValue()); break;
            case 1: mSoundMgr->setMusicVolume(mSoundSlider->getValue()); break;
            case 2: mSoundMgr->setGUIVolume(mSoundSlider->getValue()); break;
            default: break;
        }
    }
    void updateSoundSlider()
    {
        const Real volume = mSoundMgr->getVolume(mSoundList->getSelectedLine());
        mSoundSlider->setValue(volume);
        mSoundLabel->setCaption(StringConverter::toString(volume,3));
    }
    void defaultSound(const bool &update=true)
    {
        mSoundMgr->setEffectsVolume(1.0f);
        mSoundMgr->setMusicVolume(0.8f);
        mSoundMgr->setGUIVolume(0.6f);
        if(update)updateSoundSlider();
    }
    void updateVideoOption()
    {
        mVideoSelectList->show(false);
        mVideoSelectListSlider->show(false);
        mVideoCheckBox->show(false);
        mVideoCheckBoxLabel->hide();
        mVideoSlider->show(false);
        mVideoSliderLabel->hide();
        const unsigned short selection = mVideoList->getSelectedLine();
        if(selection==0)
        {
            mVideoSelectList->clear();
            mVideoSelectList->pushLine("Direct3D9");
            mVideoSelectList->pushLine("OpenGL");
            mVideoSelectList->setSelection(mIsOpenGL?1:0);
            mVideoSelectListSlider->updateSliderToListSelection();
            mVideoSelectList->show(true);
        }
        else if(selection==1)
        {
            mVideoSelectList->clear();
            for(int i=0;i<4;i++)
            {
                String buffer = getVideoMode(i);
                if(!mIsOpenGL)buffer.erase((int)buffer.length()-16,16);
                mVideoSelectList->pushLine(buffer);
            }
            mVideoSelectList->setSelection(mVideoMode);
            mVideoSelectListSlider->updateSliderToListSelection();
            mVideoSelectList->show(true);
        }
        else if(selection==2)
        {
            mVideoCheckBox->setChecked(mFullScreen);
            mVideoCheckBox->show(true);
            mVideoCheckBoxLabel->setCaption("Full Screen:");
            mVideoCheckBoxLabel->show();
        }
        else if(selection==3)
        {
            mVideoSelectList->clear();
            for(int i=0;i<(mIsOpenGL?5:7);i++)mVideoSelectList->pushLine(getAntiAliasing(i));
            mVideoSelectList->setSelection(mAntiAliasing);
            mVideoSelectListSlider->updateSliderToListSelection();
            mVideoSelectList->show(true);
        }
        /*else if(selection==4)
        {
            mVideoCheckBox->setChecked(mShadows);
            mVideoCheckBox->show(true);
            mVideoCheckBoxLabel->setCaption("Shadows:");
            mVideoCheckBoxLabel->show();
            mVideoSlider->setValue(mShadowDetail);
            mVideoSlider->show(true);
            mVideoSliderLabel->setCaption(StringConverter::toString(mShadowDetail<0.01f?0:mShadowDetail,3));
            mVideoSliderLabel->show();
        }*/
        else if(selection==4)//5)
        {
            mVideoCheckBox->setChecked(mWaterMgr->getDoReflection());
            mVideoCheckBox->show(true);
            mVideoCheckBoxLabel->setCaption("Reflection:");
            mVideoCheckBoxLabel->show();
        }
        else if(selection==5)//6)
        {
            mVideoCheckBoxLabel->setCaption("View Distance:");
            mVideoCheckBoxLabel->show();
            const Real viewRatio = mGridMgr->getViewDistanceRatio();
            mVideoSlider->setValue(viewRatio);
            mVideoSlider->show(true);
            mVideoSliderLabel->setCaption(StringConverter::toString(viewRatio<0.01f?0:viewRatio,3));
            mVideoSliderLabel->show();
        }
    }
    void updateVideoSetting()
    {
        const unsigned short option = mVideoList->getSelectedLine();
        const unsigned short setting = mVideoSelectList->getSelectedLine();

        if(option==0)
        {
            mIsOpenGL = (setting==1);
            SaveFile sf("ogre.cfg");
            sf.setSetting("Render System",mIsOpenGL?"OpenGL Rendering Subsystem":"Direct3D9 Rendering Subsystem");
            sf.save();
        }
        else if(option==1)
        {
            mVideoMode = setting;
            SaveFile sf("ogre.cfg");
            sf.setSetting("Video Mode",getVideoMode(mVideoMode),mIsOpenGL?"OpenGL Rendering Subsystem":"Direct3D9 Rendering Subsystem");
            sf.save();
        }
        else if(option==2)
        {
            mFullScreen = mVideoCheckBox->isChecked();
            SaveFile sf("ogre.cfg");
            sf.setSetting("Full Screen",mFullScreen?"Yes":"No",mIsOpenGL?"OpenGL Rendering Subsystem":"Direct3D9 Rendering Subsystem");
            sf.save();
        }
        else if(option==3)
        {
            mAntiAliasing = setting;
            SaveFile sf("ogre.cfg");
            sf.setSetting("FSAA",getAntiAliasing(mAntiAliasing),mIsOpenGL?"OpenGL Rendering Subsystem":"Direct3D9 Rendering Subsystem");
            sf.save();
        }
        /*else if(option==4)
        {
            mShadows = mVideoCheckBox->isChecked();
            mShadowDetail = mVideoSlider->getValue();
            if(mShadows)
            {
                mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_MODULATIVE);
                mSceneMgr->setShadowTextureSize((unsigned short)Math::Pow(2,int(8+mShadowDetail*4)));
            }
            else mSceneMgr->setShadowTechnique(SHADOWTYPE_NONE);
        }*/
        else if(option==4)//5)
        {
            mWaterMgr->doReflection(mVideoCheckBox->isChecked());
        }
        else if(option==5)//6)
        {
            mGridMgr->setViewDistanceRatio(mVideoSlider->getValue());
        }

        if(option<=3)
        {
            mGui->showAlertBox("Restart game for settings to take effect.");
            mRestartRequired = true;
        }
    }
    const bool isRestartRequired()
    {
        return mRestartRequired;
    }
    const String getVideoMode(const unsigned char &i)
    {
        if(mIsOpenGL)
        {
            switch(i)
            {
                case 0: return "640 x 480";
                case 1: return "800 x 600";
                case 2: return "1024 x 768";
                case 3: return "1280 x 800";
                default: break;
            }
        }
        else
        {
            switch(i)
            {
                case 0: return "640 x 480 @ 32-bit colour";
                //case 1: return "640 x 400 @ 32-bit colour";
                case 1: return "800 x 600 @ 32-bit colour";
                case 2: return "1024 x 768 @ 32-bit colour";
                case 3: return "1280 x 800 @ 32-bit colour";
                default: break;
            }
        }
        return "";
    }
    const String getAntiAliasing(const unsigned char &i)
    {
        if(mIsOpenGL)
        {
            switch(i)
            {
                case 0: return "0";
                case 1: return "2";
                case 2: return "4";
                case 3: return "8";
                case 4: return "16";
                default: break;
            }
        }
        else
        {
            switch(i)
            {
                case 0: return "None";
                case 1: return "NonMaskable 1";
                case 2: return "NonMaskable 2";
                case 3: return "NonMaskable 3";
                case 4: return "NonMaskable 4";
                case 5: return "Level 2";
                case 6: return "Level 4";
                default: break;
            }
        }
        return "";
    }
    void defaultVideo(const bool &update=true)
    {
        mIsOpenGL = true;
        mVideoMode = 1;
        mFullScreen = false;
        mAntiAliasing = 0;
        mShadows = false;
        mShadowDetail = 0;
        mGridMgr->setViewDistanceRatio(0.667f);
        if(update)updateVideoOption();
    }
    static void checkAndcreateDefaultVideoConfig(const bool &toggle=false, const bool &fillBlanks=false)
    {
        SaveFile sf;
        String buffer = "";
        if(sf.load("ogre.cfg"))
        {
            sf.getSetting("Render System",buffer);
            if(!toggle && !fillBlanks && (buffer=="OpenGL Rendering Subsystem" || buffer=="Direct3D9 Rendering Subsystem"))return;
        }

        if(!fillBlanks)
        {
            if(toggle && buffer=="OpenGL Rendering Subsystem")sf.setSetting("Render System","Direct3D9 Rendering Subsystem");
            else sf.setSetting("Render System","OpenGL Rendering Subsystem");
        }

        if(!sf.getSetting("Allow NVPerfHUD",buffer,"Direct3D9 Rendering Subsystem"))sf.setSetting("Allow NVPerfHUD","No","Direct3D9 Rendering Subsystem");
#ifndef USE_OGRE_161
        if(!sf.getSetting("FSAA",buffer,"Direct3D9 Rendering Subsystem"))sf.setSetting("FSAA","None","Direct3D9 Rendering Subsystem");
#else
        if(!sf.getSetting("Anti aliasing",buffer,"Direct3D9 Rendering Subsystem"))sf.setSetting("Anti aliasing","None","Direct3D9 Rendering Subsystem");
#endif
        if(!sf.getSetting("Floating-point mode",buffer,"Direct3D9 Rendering Subsystem"))sf.setSetting("Floating-point mode","Fastest","Direct3D9 Rendering Subsystem");
        if(!sf.getSetting("Full Screen",buffer,"Direct3D9 Rendering Subsystem"))sf.setSetting("Full Screen","No","Direct3D9 Rendering Subsystem");
        if(!sf.getSetting("VSync",buffer,"Direct3D9 Rendering Subsystem"))sf.setSetting("VSync","No","Direct3D9 Rendering Subsystem");
        if(!sf.getSetting("Video Mode",buffer,"Direct3D9 Rendering Subsystem"))sf.setSetting("Video Mode","800 x 600 @ 32-bit colour","Direct3D9 Rendering Subsystem");
        if(!sf.getSetting("sRGB Gamma Conversion",buffer,"Direct3D9 Rendering Subsystem"))sf.setSetting("sRGB Gamma Conversion","No","Direct3D9 Rendering Subsystem");

        if(!sf.getSetting("Colour Depth",buffer,"OpenGL Rendering Subsystem"))sf.setSetting("Colour Depth","32","OpenGL Rendering Subsystem");
        if(!sf.getSetting("Display Frequency",buffer,"OpenGL Rendering Subsystem"))sf.setSetting("Display Frequency","N/A","OpenGL Rendering Subsystem");
        if(!sf.getSetting("FSAA",buffer,"OpenGL Rendering Subsystem"))sf.setSetting("FSAA","0","OpenGL Rendering Subsystem");
        if(!sf.getSetting("Full Screen",buffer,"OpenGL Rendering Subsystem"))sf.setSetting("Full Screen","No","OpenGL Rendering Subsystem");
        if(!sf.getSetting("RTT Preferred Mode",buffer,"OpenGL Rendering Subsystem"))sf.setSetting("RTT Preferred Mode","FBO","OpenGL Rendering Subsystem");
        if(!sf.getSetting("VSync",buffer,"OpenGL Rendering Subsystem"))sf.setSetting("VSync","No","OpenGL Rendering Subsystem");
#ifndef USE_OGRE_161
        if(!sf.getSetting("VSync Interval",buffer,"OpenGL Rendering Subsystem"))sf.setSetting("VSync Interval","1","OpenGL Rendering Subsystem");
#endif
        if(!sf.getSetting("Video Mode",buffer,"OpenGL Rendering Subsystem"))sf.setSetting("Video Mode","800 x 600","OpenGL Rendering Subsystem");
        if(!sf.getSetting("sRGB Gamma Conversion",buffer,"OpenGL Rendering Subsystem"))sf.setSetting("sRGB Gamma Conversion","No","OpenGL Rendering Subsystem");

        sf.save();
    }
    void defaultControls()
    {
        mKeyboardMgr->setDefaults();
        mKeyboardMgr->save();
        mKeyboardMgr->updateList();
    }
    void keyPressed(const char &index)
    {
        if(!mPanel->isVisible())return;
    }
    void buttonClicked(GuiComponent *button)
    {
        if(!mPanel->isVisible())return;

        if(button==mInterfaceCheckBox)updateInterfaceSetting();
        else if(button==mVideoCheckBox)updateVideoSetting();
        else if(button->nameIs("OptionsScreen/InterfaceDefault"))mGui->showAlertBox("Restore defaults?","Options/InterfaceDefault",true);
        else if(button->nameIs("OptionsScreen/VideoDefault"))mGui->showAlertBox("Restore defaults?","Options/VideoDefault",true);
        else if(button->nameIs("OptionsScreen/SoundDefault"))mGui->showAlertBox("Restore defaults?","Options/SoundDefault",true);
        else if(button->nameIs("OptionsScreen/ControlDefault"))mGui->showAlertBox("Restore defaults?","Options/ControlDefault",true);
        else if(button->nameIs("OptionsScreen/ControlChange"))
        {
            const short selection = mControlList->getSelectedLine();
            if(selection>=0)
            {
                mControlList->changeSecondLine(selection,"_");
                mControlList->update();
                mKeyboardMgr->doSetKey(true,selection);
            }
        }
    }
    void changeEvent(GuiComponent *component, const unsigned char &event)
    {
        if(!mPanel->isVisible())return;
        if(component->isSlider())
        {
            if(component==mInterfaceSlider)
            {
                const Real value = component->getValue();
                mInterfaceSliderLabel->setCaption(StringConverter::toString(value<0.01f?0:value,3));
                updateInterfaceSetting();
            }
            else if(component==mVideoSlider)
            {
                const Real value = component->getValue();
                mVideoSliderLabel->setCaption(StringConverter::toString(value<0.01f?0:value,3));
                updateVideoSetting();
            }
            else if(component==mSoundSlider)
            {
                const Real value = component->getValue();
                mSoundLabel->setCaption(StringConverter::toString(value<0.01f?0:value,3));
                updateSoundOption();
            }
        }
        else if(component->isList())
        {
            if(component==mInterfaceList)updateInterfaceOption();
            else if(component==mInterfaceSelectList)updateInterfaceSetting();
            else if(component==mSoundList)updateSoundSlider();
            else if(component==mVideoList)updateVideoOption();
            else if(component==mVideoSelectList)updateVideoSetting();
        }
    }
    void alertBoxEvent(const String &name, const bool &flag)
    {
        if(!mPanel->isVisible())return;
        if(flag)
        {
            if(name=="Options/InterfaceDefault")defaultInterface();
            else if(name=="Options/VideoDefault")
            {
                defaultVideo();
                mGui->showAlertBox("Restart game for settings to take effect.");
            }
            else if(name=="Options/SoundDefault")defaultSound();
            else if(name=="Options/ControlDefault")defaultControls();
        }
    }
    const bool getFullScreen()
    {
        return mFullScreen;
    }
    const Real getMouseSensitivity()
    {
        return mMouseSensitivity;
    }
    Real* getMouseSensitivityPtr()
    {
        return &mMouseSensitivity;
    }
    const String getSavedUsername()
    {
        SaveFile sf;
        if(!sf.load("settings.cfg"))return "";
        String buffer = "";
        if(sf.getSetting("Username",buffer,"Login"))return buffer;
        return "";
    }
    void saveUsername(const String &name)
    {
        SaveFile sf("settings.cfg");
        if(name=="")sf.deleteSetting("Username","Login");
        else sf.setSetting("Username",name,"Login");
        sf.save();
    }
};

template<> OptionsManager* Singleton<OptionsManager>::ms_Singleton = 0;

OptionsManager* OptionsManager::getSingletonPtr()
{
	return ms_Singleton;
}

OptionsManager& OptionsManager::getSingleton()
{
	assert(ms_Singleton);
	return *ms_Singleton;
}

#endif
