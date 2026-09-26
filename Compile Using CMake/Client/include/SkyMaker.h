#ifndef _SKYMAKER_H_
#define _SKYMAKER_H_

#include <Gui.h>
#include <SkyManager.h>
#include <SaveFile.h>

typedef pair<WeatherPeriod*, pair<OverlayElement*,OverlayElement*> > WeatherMakerPeriod;

class SkyMaker
{
private:
    SceneManager *mSceneMgr;
    Gui *mGui;
    SkyManager *mSkyMgr;
    WeatherManager *mWeatherMgr;
    GuiMultiPanel *mPanel;
    GuiPanel *mWeatherCyclePanel;

    GuiButton *mSkyTestButton;
    GuiTextField *mSkyField;
    GuiHighlightList *mSkyLoadList;
    GuiSlider *mSkyLoadListSlider;
    GuiHighlightList *mSkySelectList;
    GuiSlider *mSkyColourSlider[3];
    GuiTextField *mSkyColourField[3];
    OverlayElement *mSkyMatLabel;
    GuiTextField *mSkyMatField;
    String mSkyFile;
    SkyColours mSkyColours;
    String mSkyMat[2];

    GuiTextField *mWeatherField;
    GuiHighlightList *mWeatherLoadList;
    GuiSlider *mWeatherLoadListSlider;
    GuiHighlightList *mWeatherSelectList;
    GuiSlider *mWeatherColourSlider[3];
    GuiTextField *mWeatherColourField[3];
    GuiTextField *mWeatherPSField;
    GuiTextField *mWeatherRateField;
    GuiTextField *mWeatherPosField[3];
    Weather mWeather;

    GuiTextField *mCycleField;
    GuiHighlightList *mCycleLoadList;
    GuiSlider *mCycleLoadListSlider;
    GuiHighlightList *mCycleSelectList;
    GuiSlider *mCycleSelectListSlider;
    GuiSlider *mCycleSlider;
    OverlayElement *mTimeLabel;
    GuiCheckBox *mCycleFadeIn;
    GuiCheckBox *mCycleFadeOut;
    GuiTextField *mCycleDurationField;
    std::vector<WeatherMakerPeriod> mCycleList;
    String mCycleFile;
    unsigned short mCycleTime;
    WeatherMakerPeriod *mCurrentPeriod;
    unsigned short mIndicatorCount;

    bool mTestSky;
public:
    SkyMaker() : mWeather("")
    {
        mSceneMgr = 0;
    }
    ~SkyMaker()
    {
        clear();
    }
    void init(SceneManager *sceneMgr, const bool &load=true)
    {
        mSceneMgr = sceneMgr;
        mGui = Gui::getSingletonPtr();
        mSkyMgr = SkyManager::getSingletonPtr();
        mSkyMgr->freezeTime(true);
        mSkyMgr->createWeather("");
        mWeatherMgr = mSkyMgr->getWeatherManager();
        mPanel = mGui->addMultiPanel("SkyMakerScreen",true);

        //Sky Panel
        mSkyField = mPanel->getTextField("SkyMakerScreen/Name");
        mSkyFile = "";
        mSkyField->bindString(&mSkyFile);
        mSkyField->mMaxLength = 32;
        mSkyTestButton = mPanel->getButton("SkyMakerScreen/SkyTest");
        mSkyLoadList = mPanel->getHighlightList("SkyMakerScreen/Skies");
        mSkyLoadListSlider = mPanel->getSlider("SkyMakerScreen/SkiesLS");
        mSkySelectList = mPanel->getHighlightList("SkyMakerScreen/SkySelect");
        mSkySelectList->pushLine("Dawn (Top)");
        mSkySelectList->pushLine("Dawn (Bottom)");
        mSkySelectList->pushLine("Day (Top)");
        mSkySelectList->pushLine("Day (Bottom)");
        mSkySelectList->pushLine("Dusk (Top)");
        mSkySelectList->pushLine("Dusk (Bottom)");
        mSkySelectList->pushLine("Night (Top)");
        mSkySelectList->pushLine("Night (Bottom)");
        mSkySelectList->pushLine("Sunrise");
        mSkySelectList->pushLine("Sun");
        mSkySelectList->pushLine("Sunset");
        mSkySelectList->pushLine("Moon");
        mSkySelectList->pushLine("Dawn Light");
        mSkySelectList->pushLine("Day Light");
        mSkySelectList->pushLine("Dusk Light");
        mSkySelectList->pushLine("Night Light");
        mSkySelectList->pushLine("Dawn Clouds");
        mSkySelectList->pushLine("Day Clouds");
        mSkySelectList->pushLine("Dusk Clouds");
        mSkySelectList->pushLine("Night Clouds");
        mSkySelectList->update(0);
        mSkySelectList->setSelection(0);
        for(int i=0;i<3;i++)
        {
            mSkyColourSlider[i] = mPanel->getSlider("SkyMakerScreen/CX"+StringConverter::toString(i));
            mSkyColourField[i] = mPanel->getTextField("SkyMakerScreen/CT"+StringConverter::toString(i));
            mSkyColourField[i]->mMaxLength = 5;
        }
        mSkyMatLabel = OverlayManager::getSingleton().getOverlayElement("SkyMakerScreen/SkyMatLabel");
        mSkyMatField = mPanel->getTextField("SkyMakerScreen/SkyMat");
        for(int i=0;i<2;i++)mSkyMat[i] = "";
        mSkyMatField->bindString(&mSkyMat[0]);
        mSkyMatField->mMaxLength = 32;

        if(load)
        {
            loadAllSkies();
            updateSkySelection();
        }

        //Weather Panel
        mWeather = Weather("");
        mWeatherField = mPanel->getTextField("SkyMakerScreen/WeatherName");
        mWeatherField->bindString(&mWeather.mName);
        mWeatherField->mMaxLength = 32;
        mWeatherLoadList = mPanel->getHighlightList("SkyMakerScreen/Weathers");
        mWeatherLoadListSlider = mPanel->getSlider("SkyMakerScreen/WeathersLS");
        mWeatherSelectList = mPanel->getHighlightList("SkyMakerScreen/WeatherSelect");
        mWeatherSelectList->pushLine("Sky Top Colour");
        mWeatherSelectList->pushLine("Sky Bottom Colour");
        mWeatherSelectList->pushLine("Clouds Colour");
        mWeatherSelectList->update(0);
        mWeatherSelectList->setSelection(0);
        for(int i=0;i<3;i++)
        {
            mWeatherColourSlider[i] = mPanel->getSlider("SkyMakerScreen/WCX"+StringConverter::toString(i));
            mWeatherColourField[i] = mPanel->getTextField("SkyMakerScreen/WCT"+StringConverter::toString(i));
            mWeatherColourField[i]->mMaxLength = 5;
            mWeatherPosField[i] = mPanel->getTextField("SkyMakerScreen/WeatherPos"+StringConverter::toString(i));
            mWeatherPosField[i]->mMaxLength = 5;
        }
        mWeatherPSField = mPanel->getTextField("SkyMakerScreen/WeatherPS");
        mWeatherPSField->bindString(&mWeather.mParticle);
        mWeatherPSField->mMaxLength = 32;
        mWeatherRateField = mPanel->getTextField("SkyMakerScreen/WeatherRate");
        mWeatherRateField->mMaxLength = 5;

        mWeather.mHasSkyShader = true;
        updateWeather();
        updateWeatherColourFields();

        //Weather Cycle Panel
        mWeatherCyclePanel = mGui->addPanel("WeatherMakerScreen",true);
        mWeatherCyclePanel->show(false);

        mCycleField = mWeatherCyclePanel->getTextField("WeatherMakerScreen/Name");
        mCycleField->bindString(&mCycleFile);
        mCycleField->mMaxLength = 32;
        mCycleLoadList = mWeatherCyclePanel->getHighlightList("WeatherMakerScreen/Cycles");
        mCycleLoadListSlider = mWeatherCyclePanel->getSlider("WeatherMakerScreen/CyclesLS");
        mCycleSelectList = mWeatherCyclePanel->getHighlightList("WeatherMakerScreen/Weathers");
        mCycleSelectListSlider = mWeatherCyclePanel->getSlider("WeatherMakerScreen/WeathersLS");
        mCycleSlider = mWeatherCyclePanel->getSlider("WeatherMakerScreen/Time");
        mTimeLabel = OverlayManager::getSingleton().getOverlayElement("WeatherMakerScreen/TimeLabel");
        mCycleFadeIn = mWeatherCyclePanel->getCheckBox("WeatherMakerScreen/FadeIn");
        mCycleFadeOut = mWeatherCyclePanel->getCheckBox("WeatherMakerScreen/FadeOut");
        mCycleDurationField = mWeatherCyclePanel->getTextField("WeatherMakerScreen/Duration");
        mCycleDurationField->mMaxLength = 4;
        mCycleTime = 0;
        mCurrentPeriod = 0;
        mIndicatorCount = 0;

        if(load)
        {
            loadAllWeathers();
            loadAllCycles();
        }

        mTestSky = false;
    }
    void clear()
    {
        clearWeatherCycle();
    }
    void deleteGui()
    {
        mGui->deletePanel(mPanel);
        mGui->deletePanel(mWeatherCyclePanel);
    }
    void update(const Real &timeElapsed)
    {
        if(mTestSky)mWeatherMgr->setWeatherRatio(1);
    }
    void updateColourFields()
    {
        const ColourValue colour = mSkyColours.getColour(mSkySelectList->getSelectedLine());
        for(int i=0;i<3;i++)
        {
            mSkyColourSlider[i]->setValue(colour[i]);
            mSkyColourField[i]->setCaption(StringConverter::toString((unsigned char)(colour[i]*255)));
        }
    }
    void updateColours()
    {
        const ColourValue colour(mSkyColourSlider[0]->getValue(),mSkyColourSlider[1]->getValue(),mSkyColourSlider[2]->getValue());
        mSkyColours.setColour(mSkySelectList->getSelectedLine(),colour);
        mSkyMgr->setSkyColours(mSkyColours);
        mSkyMgr->updateSky(true);
        updateWeather();
    }
    void updateSkySelection()
    {
        const unsigned char &selection = mSkySelectList->getSelectedLine();
        Real time = 0;
        switch(selection)
        {
            case 0:
            case 1:
            case 8:
            case 12:
            case 16:
                time = 700;
                break;
            case 2:
            case 3:
            case 9:
            case 13:
            case 17:
                time = 1300;
                break;
            case 4:
            case 5:
            case 10:
            case 14:
            case 18:
                time = 1900;
                break;
            case 6:
            case 7:
            case 11:
            case 15:
            case 19:
                time = 100;
                break;
        }
        if(selection>=8 && selection<=11)
        {
            mSkyMatLabel->show();
            mSkyMatField->show(true);
            mSkyMatField->bindString(&mSkyMat[selection==11]);
        }
        else
        {
            mSkyMatLabel->hide();
            mSkyMatField->show(false);
        }
        mSkyMgr->setDayTime(time);
        mSkyMgr->updateSky(true);
        updateColourFields();
        updateWeather();
    }
    void updateWeatherColourFields()
    {
        const unsigned char selection = mWeatherSelectList->getSelectedLine();
        ColourValue colour;
        if(selection==0)colour = mWeather.mSkyTop;
        else if(selection==1)colour = mWeather.mSkyBot;
        else if(selection==2)colour = mWeather.mClouds;
        for(int i=0;i<3;i++)
        {
            mWeatherColourSlider[i]->setValue(colour[i]);
            mWeatherColourField[i]->setCaption(StringConverter::toString((unsigned char)(colour[i]*255)));
        }
    }
    void updateWeatherColours()
    {
        const ColourValue colour(mWeatherColourSlider[0]->getValue(),mWeatherColourSlider[1]->getValue(),mWeatherColourSlider[2]->getValue());
        const unsigned char selection = mWeatherSelectList->getSelectedLine();
        if(selection==0)mWeather.mSkyTop = colour;
        else if(selection==1)mWeather.mSkyBot = colour;
        else if(selection==2)mWeather.mClouds = colour;
        mWeather.mHasSkyShader = true;
        mWeatherMgr->setWeatherRatio(1);
    }
    void updateWeather()
    {
        mWeatherMgr->changeWeather(&mWeather,0);
        mWeatherMgr->setWeatherRatio(1);
    }
    void clearWeatherCycle()
    {
        while(!mCycleList.empty())
        {
            WeatherPeriod *period = mCycleList.back().first;
            pair<OverlayElement*,OverlayElement*> indicator = mCycleList.back().second;
            mCycleList.pop_back();
            delete period;
            if(indicator.first)
            {
                mWeatherCyclePanel->getOverlayContainer()->removeChild(indicator.first->getName());
                OverlayManager::getSingleton().destroyOverlayElement(indicator.first);
            }
            if(indicator.second)
            {
                mWeatherCyclePanel->getOverlayContainer()->removeChild(indicator.second->getName());
                OverlayManager::getSingleton().destroyOverlayElement(indicator.second);
            }
        }
        if(mCurrentPeriod)mWeatherMgr->exitCurrentPeriod();
        mCurrentPeriod = 0;
        mIndicatorCount = 0;
    }
    const bool createWeatherCyclePeriod(const unsigned short &time, const string &type, const unsigned short &duration, const bool &fadeIn, const bool &fadeOut)
    {
        if(type=="")
        {
            mGui->showAlertBox("Select a Weather to insert.");
            return false;
        }
        if(duration<=0)
        {
            mGui->showAlertBox("Duration must be more than 0.");
            return false;
        }
        if(getWeatherCyclePeriod(time))
        {
            mGui->showAlertBox("Start time clashes with another period!");
            return false;
        }
        if(getWeatherCyclePeriod((time+duration-1)%WEATHER_TIME_MAX))
        {
            mGui->showAlertBox("End time clashes with another period!");
            return false;
        }
        bool wrapped = false;
        WeatherPeriod *period = new WeatherPeriod;
        //Special case: full duration
        if(duration==WEATHER_TIME_MAX)
        {
            period->mPeriod.first = 0;
            period->mPeriod.second = duration;
        }
        else
        {
            period->mPeriod.first = time;
            period->mPeriod.second = time+duration;
            if(period->mPeriod.second>WEATHER_TIME_MAX)
            {
                period->mPeriod.second -= WEATHER_TIME_MAX;
                wrapped = true;
            }
        }
        period->mType = type;
        period->mFadeIn = fadeIn;
        period->mFadeOut = fadeOut;

        OverlayElement *indicator = OverlayManager::getSingleton().createOverlayElementFromTemplate("GuiTemplate/WeatherCycleIndicator","Panel","WeatherMaker"+StringConverter::toString(mIndicatorCount++));
        if(fadeIn)
        {
            if(wrapped)indicator->setMaterialName("GuiMat/WeatherCycleIndicatorInL");
            else indicator->setMaterialName("GuiMat/WeatherCycleIndicatorIn");
        }
        else if(fadeOut)
        {
            if(wrapped)indicator->setMaterialName("GuiMat/WeatherCycleIndicatorOutL");
            else indicator->setMaterialName("GuiMat/WeatherCycleIndicatorOut");
        }
        indicator->setLeft(float(period->mPeriod.first)/WEATHER_TIME_MAX * 0.84f + 0.03f);
        if(wrapped)indicator->setWidth(float(duration-period->mPeriod.second)/WEATHER_TIME_MAX * 0.84f);
        else indicator->setWidth(float(duration)/WEATHER_TIME_MAX * 0.84f);
        mWeatherCyclePanel->getOverlayContainer()->addChild(indicator);

        OverlayElement *indicator2 = 0;
        if(wrapped)
        {
            indicator2 = OverlayManager::getSingleton().createOverlayElementFromTemplate("GuiTemplate/WeatherCycleIndicator","Panel","WeatherMaker"+StringConverter::toString(mIndicatorCount++));
            if(fadeIn)indicator2->setMaterialName("GuiMat/WeatherCycleIndicatorInR");
            else if(fadeOut)indicator2->setMaterialName("GuiMat/WeatherCycleIndicatorOutR");
            indicator2->setLeft(0.03f);
            indicator2->setWidth(float(period->mPeriod.second)/WEATHER_TIME_MAX * 0.84f);
            mWeatherCyclePanel->getOverlayContainer()->addChild(indicator2);
        }

        mCycleList.push_back(WeatherMakerPeriod(period,pair<OverlayElement*,OverlayElement*>(indicator,indicator2)));
        return true;
    }
    WeatherMakerPeriod* getWeatherCyclePeriod(const unsigned short &time)
    {
        for(std::vector<WeatherMakerPeriod>::iterator it=mCycleList.begin(); it!=mCycleList.end(); it++)
        {
            WeatherMakerPeriod *period = &*it;
            if(period->first->isInPeriod(time))return period;
        }
        return 0;
    }
    void updateCurrentPeriod(WeatherMakerPeriod *newPeriod)
    {
        if(mCurrentPeriod==newPeriod)return;
        String prevType = "";
        if(mCurrentPeriod)
        {
            WeatherPeriod *period = mCurrentPeriod->first;
            OverlayElement *indicator = mCurrentPeriod->second.first;
            OverlayElement *indicator2 = mCurrentPeriod->second.second;
            if(period->mFadeIn)
            {
                if(indicator2)
                {
                    indicator->setMaterialName("GuiMat/WeatherCycleIndicatorInL");
                    indicator2->setMaterialName("GuiMat/WeatherCycleIndicatorInR");
                }
                else indicator->setMaterialName("GuiMat/WeatherCycleIndicatorIn");
            }
            else if(period->mFadeOut)
            {
                if(indicator2)
                {
                    indicator->setMaterialName("GuiMat/WeatherCycleIndicatorOutL");
                    indicator2->setMaterialName("GuiMat/WeatherCycleIndicatorOutR");
                }
                else indicator->setMaterialName("GuiMat/WeatherCycleIndicatorOut");
            }
            else
            {
                indicator->setMaterialName("GuiMat/WeatherCycleIndicator");
                if(indicator2)indicator2->setMaterialName("GuiMat/WeatherCycleIndicator");
            }

            if(!newPeriod)mWeatherMgr->exitCurrentPeriod();
        }
        mCurrentPeriod = newPeriod;
        if(mCurrentPeriod)
        {
            WeatherPeriod *period = mCurrentPeriod->first;
            OverlayElement *indicator = mCurrentPeriod->second.first;
            OverlayElement *indicator2 = mCurrentPeriod->second.second;
            if(period->mFadeIn)
            {
                if(indicator2)
                {
                    indicator->setMaterialName("GuiMat/WeatherCycleHighlightInL");
                    indicator2->setMaterialName("GuiMat/WeatherCycleHighlightInR");
                }
                else indicator->setMaterialName("GuiMat/WeatherCycleHighlightIn");
            }
            else if(period->mFadeOut)
            {
                if(indicator2)
                {
                    indicator->setMaterialName("GuiMat/WeatherCycleHighlightOutL");
                    indicator2->setMaterialName("GuiMat/WeatherCycleHighlightOutR");
                }
                else indicator->setMaterialName("GuiMat/WeatherCycleHighlightOut");
            }
            else
            {
                indicator->setMaterialName("GuiMat/WeatherCycleHighlight");
                if(indicator2)indicator2->setMaterialName("GuiMat/WeatherCycleHighlight");
            }
            const short line = mCycleSelectList->findLine(period->mType);
            if(line>=0)
            {
                mCycleSelectList->setSelection(line);
                mCycleSelectListSlider->updateSliderToListSelection();
            }

            mWeatherMgr->enterPeriod(period);
        }
    }
    void updateCycleTime(const bool &getfromSlider=true)
    {
        if(getfromSlider)mCycleTime = (unsigned short)(mCycleSlider->getValue()*(WEATHER_TIME_MAX-1));
        mTimeLabel->setCaption(StringConverter::toString(mCycleTime));
        updateCurrentPeriod(getWeatherCyclePeriod(mCycleTime));
        mWeatherMgr->setWeatherTime(mCycleTime);
    }
    void buttonClicked(GuiComponent *button)
    {
        if(button->nameIs("SkyMakerScreen/Save"))doSaveSky();
        else if(button->nameIs("SkyMakerScreen/Export"))doSaveSky(false,true);
        else if(button==mSkyTestButton)
        {
            mTestSky = !mTestSky;
            button->setCaption(mTestSky?"Stop Test":"Start Test");
            mSkyMgr->freezeTime(!mTestSky,false);
            mSkyMgr->setFastForward(mTestSky);
        }
        else if(button->nameIs("SkyMakerScreen/Load"))doLoadSky();
        else if(button->nameIs("SkyMakerScreen/WeatherSave"))doSaveWeather();
        else if(button->nameIs("SkyMakerScreen/WeatherReset"))doResetWeather();
        else if(button->nameIs("SkyMakerScreen/WeatherLoad"))doLoadWeather();
        else if(button->nameIs("WeatherMakerScreen/New"))
        {
            const unsigned short duration = StringConverter::parseInt(mCycleDurationField->getCaption());
            if(createWeatherCyclePeriod(mCycleTime,mCycleSelectList->getSelection(),duration,mCycleFadeIn->isChecked(),mCycleFadeOut->isChecked()))
            {
                mCycleTime += duration;
                if(mCycleTime>=WEATHER_TIME_MAX)mCycleTime -= WEATHER_TIME_MAX;
                mCycleSlider->setValue(float(mCycleTime)/(WEATHER_TIME_MAX-1));
                updateCycleTime(false);
            }
            //Place time after current period for continuity
            else if(mCurrentPeriod)
            {
                mCycleTime = mCurrentPeriod->first->mPeriod.second;
                mCycleSlider->setValue(float(mCycleTime)/(WEATHER_TIME_MAX-1));
                updateCycleTime(false);
            }
        }
        else if(button->nameIs("WeatherMakerScreen/Remove"))
        {
            if(mCurrentPeriod)
            {
                WeatherPeriod *period = mCurrentPeriod->first;
                pair<OverlayElement*,OverlayElement*> indicator = mCurrentPeriod->second;
                if(period)delete period;
                if(indicator.first)
                {
                    mWeatherCyclePanel->getOverlayContainer()->removeChild(indicator.first->getName());
                    OverlayManager::getSingleton().destroyOverlayElement(indicator.first);
                }
                if(indicator.second)
                {
                    mWeatherCyclePanel->getOverlayContainer()->removeChild(indicator.second->getName());
                    OverlayManager::getSingleton().destroyOverlayElement(indicator.second);
                }
                for(std::vector<WeatherMakerPeriod>::iterator it=mCycleList.begin(); it!=mCycleList.end(); it++)
                {
                    WeatherMakerPeriod *period = &*it;
                    if(mCurrentPeriod==period)
                    {
                        mCycleList.erase(it);
                        break;
                    }
                }
                mWeatherMgr->exitCurrentPeriod();
                mCurrentPeriod = 0;
            }
        }
        else if(button==mCycleFadeIn)mCycleFadeOut->setChecked(false);
        else if(button==mCycleFadeOut)mCycleFadeIn->setChecked(false);
        else if(button->nameIs("WeatherMakerScreen/Save"))doSaveCycle();
        else if(button->nameIs("WeatherMakerScreen/Export"))doSaveCycle(false,true);
        else if(button->nameIs("WeatherMakerScreen/Clear"))doClearCycle();
        else if(button->nameIs("WeatherMakerScreen/Load"))doLoadCycle();
        else if(button->nameIs("WeatherMakerScreen/Back"))
        {
            mPanel->show(true);
            mWeatherCyclePanel->show(false);
            GuiTab *tab = mPanel->getTab("SkyMakerScreen/Page2");
            tab->pressed();
            mPanel->selectTab(tab);
            if(mCurrentPeriod)
            {
                mWeatherMgr->exitCurrentPeriod();
                mCurrentPeriod = 0;
            }
            updateWeather();
        }
    }
    void changeEvent(GuiComponent *component, const unsigned char &event)
    {
        if(component->isTab())
        {
            if(component->nameIs("SkyMakerScreen/Page3"))
            {
                if(mTestSky)
                {
                    mTestSky = false;
                    mSkyTestButton->setCaption("Start Test");
                    mSkyMgr->freezeTime(true,false);
                    mSkyMgr->setFastForward(false);
                }
                mPanel->show(false);
                mWeatherCyclePanel->show(true);
                mWeatherMgr->exitCurrentPeriod();
                updateCycleTime(false);
            }
        }
        else if(component->isSlider())
        {
            unsigned short index = 0;
            if(component==mCycleSlider)updateCycleTime();
            else if(component->nameIsIndex("SkyMakerScreen/CX",&index))
            {
                mSkyColourField[index]->setCaption(StringConverter::toString((unsigned char)(static_cast<GuiSlider*>(component)->getValue()*255)));
                updateColours();
            }
            else if(component->nameIsIndex("SkyMakerScreen/WCX",&index))
            {
                mWeatherColourField[index]->setCaption(StringConverter::toString((unsigned char)(static_cast<GuiSlider*>(component)->getValue()*255)));
                updateWeatherColours();
            }
        }
        else if(component->isTextComponent())
        {
            unsigned short index = 0;
            if(component==mSkyMatField)
            {
                const unsigned char selection = (mSkySelectList->getSelectedLine()==11?1:0);
                if(selection>1)return;
                if(mSkyMat[selection]!="" && !MaterialManager::getSingletonPtr()->resourceExists(mSkyMat[selection]))mGui->showAlertBox(mSkyMat[selection]+" cannot be found!");
                else
                {
                    if(selection)mSkyMgr->setMoonMat(mSkyMat[selection]);
                    else mSkyMgr->setSunMat(mSkyMat[selection]);
                }
            }
            else if(component==mWeatherPSField)
            {
                const String ps = mWeatherPSField->getCaption();
                if(ps!="" && !ParticleSystemManager::getSingletonPtr()->getTemplate(ps))mGui->showAlertBox(ps+" cannot be found!");
                else
                {
                    mWeather.mParticle = ps;
                    updateWeather();
                }
            }
            else if(component==mWeatherRateField)
            {
                const Real rate = StringConverter::parseReal(mWeatherRateField->getCaption());
                mWeather.mEmissionRate = (rate<0?0:rate);
                component->setCaption(StringConverter::toString(mWeather.mEmissionRate));
                mWeather.mHasOwnRate = true;
                updateWeather();
            }
            else if(component==mCycleDurationField)
            {
                unsigned short duration = StringConverter::parseInt(mCycleDurationField->getCaption());
                if(duration>WEATHER_TIME_MAX)duration = WEATHER_TIME_MAX;
                mCycleDurationField->setCaption(StringConverter::toString(duration));
            }
            else if(component->nameIsIndex("SkyMakerScreen/WeatherPos",&index))
            {
                const Real pos = StringConverter::parseReal(static_cast<GuiTextField*>(component)->getCaption());
                if(index==0)mWeather.mOffset.x = pos;
                else if(index==0)mWeather.mOffset.y = pos;
                else if(index==0)mWeather.mOffset.z = pos;
                component->setCaption(StringConverter::toString(pos));
                updateWeather();
            }
            else if(component->nameIsIndex("SkyMakerScreen/CT",&index))
            {
                const unsigned char value = StringConverter::parseInt(static_cast<GuiTextField*>(component)->getCaption());
                mSkyColourSlider[index]->setValue(float(value)/255);
                component->setCaption(StringConverter::toString(value));
                updateColours();
            }
            else if(component->nameIsIndex("SkyMakerScreen/WCT",&index))
            {
                const unsigned char value = StringConverter::parseInt(static_cast<GuiTextField*>(component)->getCaption());
                mWeatherColourSlider[index]->setValue(float(value)/255);
                component->setCaption(StringConverter::toString(value));
                updateWeatherColours();
            }
        }
        else if(component->isList())
        {
            if(component==mSkySelectList)updateSkySelection();
            else if(component==mWeatherSelectList)updateWeatherColourFields();
            else if(component==mCycleSelectList)
            {
                if(mCurrentPeriod && mCurrentPeriod->first)
                {
                    mCurrentPeriod->first->mType = mCycleSelectList->getSelection();
                    mWeatherMgr->enterPeriod(mCurrentPeriod->first);
                }
            }
        }
    }
    void alertBoxEvent(const String &name, const bool &flag)
    {
        if(flag)
        {
            if(name=="SkyMaker/OverwriteSky")doSaveSky(true);
            else if(name=="SkyMaker/OverwriteSkyExport")doSaveSky(true,true);
            else if(name=="SkyMaker/LoadSky")doLoadSky(true);
            else if(name=="SkyMaker/OverwriteWeather")doSaveWeather(true);
            else if(name=="SkyMaker/ResetWeather")doResetWeather(true);
            else if(name=="SkyMaker/LoadWeather")doLoadWeather(true);
            else if(name=="SkyMaker/OverwriteCycle")doSaveCycle(true);
            else if(name=="SkyMaker/OverwriteCycleExport")doSaveCycle(true,true);
            else if(name=="SkyMaker/ClearCycle")doClearCycle(true);
            else if(name=="SkyMaker/LoadCycle")doLoadCycle(true);
        }
    }
    void doSaveSky(const bool &overwrite=false, const bool &doExport=false)
    {
        if(mSkyFile=="")
        {
            mGui->showAlertBox("Enter a filename.");
            return;
        }
        SaveFile sf;
        if(!overwrite && sf.load(SKYMAKER_DIR+mSkyFile+".sky"))
        {
            mGui->showAlertBox(mSkyFile+".sky exists, overwrite?","SkyMaker/OverwriteSky"+String(doExport?"Export":""),true);
            return;
        }
        saveSky(mSkyFile);
        if(doExport)exportSky(mSkyFile);
        loadAllSkies();
    }
    void saveSky(const String &filename)
    {
        SaveFile sf;
        if(!sf.loadAnywhere(filename+".sky"))sf.load(SKYMAKER_DIR+filename+".sky");
        for(int i=0;i<20;i++)sf.setSetting(StringConverter::toString(i),StringConverter::toString(mSkyColours.getColour(i)),"Colour");
        sf.deleteSection("Mat");
        for(int i=0;i<2;i++)
            if(mSkyMat[i]!="")sf.setSetting(StringConverter::toString(i),mSkyMat[i],"Mat");
        sf.save();
    }
    void doLoadSky(const bool &load=false)
    {
        const String fileName = mSkyLoadList->getSelection();
        if(fileName=="")
        {
            mGui->showAlertBox("No file to load!");
            return;
        }
        if(!load)
        {
            mGui->showAlertBox("Load file "+fileName+"?","SkyMaker/LoadSky",true);
            return;
        }
        loadSky(fileName);
    }
    const bool loadSky(const String &filename, const bool &updateSky=true)
    {
        SaveFile sf;
        if(!sf.loadAnywhere(filename+".sky"))
        {
            mGui->showAlertBox(filename+" not found!");
            return false;
        }
        mSkyField->setCaption(filename);
        String buffer;
        for(int i=0;i<20;i++)
        {
            buffer = "";
            sf.getSetting(StringConverter::toString(i),buffer,"Colour");
            mSkyColours.setColour(i,StringConverter::parseColourValue(buffer));
        }
        mSkyMgr->setSkyColours(mSkyColours);
        buffer = "";
        sf.getSetting("0",buffer,"Mat");
        mSkyMat[0] = buffer;
        mSkyMgr->setSunMat(buffer);
        buffer = "";
        sf.getSetting("1",buffer,"Mat");
        mSkyMat[1] = buffer;
        mSkyMgr->setMoonMat(buffer);

        if(updateSky)updateSkySelection();
        return true;
    }
    void loadAllSkies()
    {
        mSkyLoadList->clear();
        const std::vector<String> list = SaveFile::findResourceNames("*.sky");
        for(std::vector<String>::const_iterator i = list.begin(); i!=list.end(); i++)
        {
            String line = *i;
            line.erase((int)line.length()-4,4);
            mSkyLoadList->pushLine(line);
        }
        mSkyLoadList->update(0);
        mSkyLoadListSlider->setValue(0);
    }
    void doSaveWeather(const bool &overwrite=false)
    {
        if(mWeather.mName=="")
        {
            mGui->showAlertBox("Enter a Weather Name.");
            return;
        }
        SaveFile sf;
        if(!sf.loadAnywhere(mWeather.mName+".weather"))sf.load(SKYMAKER_DIR+mWeather.mName+".weather");
        if(!overwrite && sf.hasSection(mWeather.mName))
        {
            mGui->showAlertBox(mWeather.mName+" exists, overwrite?","SkyMaker/OverwriteWeather",true);
            return;
        }
        saveWeather();
        loadAllWeathers();
    }
    void saveWeather()
    {
        SaveFile sf;
        if(!sf.loadAnywhere(mWeather.mName+".weather"))sf.load(SKYMAKER_DIR+mWeather.mName+".weather");

        if(mWeather.mParticle!="")sf.setSetting("ps",mWeather.mParticle,mWeather.mName);
        else sf.deleteSetting("ps",mWeather.mName);
        if(mWeather.mOffset!=Vector3::ZERO)sf.setSetting("offset",StringConverter::toString(mWeather.mOffset),mWeather.mName);
        if(mWeather.mEmissionRate>0)sf.setSetting("rate",StringConverter::toString(mWeather.mEmissionRate),mWeather.mName);
        if(mWeather.mSkyTop!=ColourValue())sf.setSetting("skytop",StringConverter::toString(mWeather.mSkyTop),mWeather.mName);
        if(mWeather.mSkyBot!=ColourValue())sf.setSetting("skybot",StringConverter::toString(mWeather.mSkyBot),mWeather.mName);
        if(mWeather.mClouds!=ColourValue())sf.setSetting("clouds",StringConverter::toString(mWeather.mClouds),mWeather.mName);

        sf.save();
    }
    void doResetWeather(const bool &reset=false)
    {
        if(!reset)
        {
            mGui->showAlertBox("Reset Weather?","SkyMaker/ResetWeather",true);
            return;
        }
        mWeather = Weather(mWeather.mName);
        mWeatherPSField->setCaption("");
        mWeatherRateField->setCaption("");
        for(int i=0;i<3;i++)mWeatherPosField[i]->setCaption("");
        updateWeatherColourFields();
        updateWeather();
    }
    void doLoadWeather(const bool &load=false)
    {
        const String name = mWeatherLoadList->getSelection();
        if(name=="")
        {
            mGui->showAlertBox("No Weather to load!");
            return;
        }
        if(!load)
        {
            mGui->showAlertBox("Load Weather "+name+"?","SkyMaker/LoadWeather",true);
            return;
        }
        loadWeather(name);
    }
    void loadWeather(const String &name)
    {
        SaveFile sf;
        if(!sf.loadAnywhere(name+".weather"))
        {
            mGui->showAlertBox(name+" not found!");
            return;
        }
        mWeatherField->setCaption(name);
        String buffer = "";
        sf.getSetting("ps",buffer,name);
        mWeatherPSField->setCaption(buffer);
        buffer = "";
        sf.getSetting("offset",buffer,name);
        mWeather.mOffset = (buffer=="" ? Vector3::ZERO : StringConverter::parseVector3(buffer));
        mWeatherPosField[0]->setCaption(StringConverter::toString((mWeather.mOffset.x)));
        mWeatherPosField[1]->setCaption(StringConverter::toString((mWeather.mOffset.y)));
        mWeatherPosField[2]->setCaption(StringConverter::toString((mWeather.mOffset.z)));
        buffer = "";
        mWeather.mHasOwnRate = sf.getSetting("rate",buffer,name);
        mWeather.mEmissionRate = (buffer=="" ? 0 : StringConverter::parseReal(buffer));
        mWeatherRateField->setCaption(StringConverter::toString((mWeather.mEmissionRate)));
        buffer = "";
        const bool hasTop = sf.getSetting("skytop",buffer,name);
        mWeather.mSkyTop = (buffer=="" ? ColourValue() : StringConverter::parseColourValue(buffer));
        buffer = "";
        const bool hasBot = sf.getSetting("skybot",buffer,name);
        mWeather.mSkyBot = (buffer=="" ? ColourValue() : StringConverter::parseColourValue(buffer));
        buffer = "";
        const bool hasCloud = sf.getSetting("clouds",buffer,name);
        mWeather.mClouds = (buffer=="" ? ColourValue() : StringConverter::parseColourValue(buffer));
        mWeather.mHasSkyShader = (hasTop || hasBot || hasCloud);

        updateWeather();
        updateWeatherColourFields();
    }
    void loadAllWeathers()
    {
        mWeatherLoadList->clear();
        mCycleSelectList->clear();

        const std::vector<String> list = SaveFile::findResourceNames("*.weather");
        for(std::vector<String>::const_iterator i = list.begin(); i!=list.end(); i++)
        {
            String line = *i;
            line.erase((int)line.length()-8,8);
            mWeatherLoadList->pushLine(line);
            mCycleSelectList->pushLine(line);
        }

        mWeatherLoadList->update(0);
        mWeatherLoadListSlider->setValue(0);
        mCycleSelectList->update(0);
        mCycleSelectListSlider->setValue(0);

        mWeatherMgr->loadWeathers();
    }
    void doSaveCycle(const bool &overwrite=false, const bool &doExport=false)
    {
        if(mCycleFile=="")
        {
            mGui->showAlertBox("Enter a filename.");
            return;
        }
        SaveFile sf;
        if(!overwrite && sf.loadAnywhere(mCycleFile+".cycle"))
        {
            mGui->showAlertBox(mCycleFile+".cycle exists, overwrite?","SkyMaker/OverwriteCycle"+String(doExport?"Export":""),true);
            return;
        }
        saveCycle(mCycleFile);
        if(doExport)exportCycle(mCycleFile);
        loadAllCycles();
    }
    void saveCycle(const String &filename)
    {
        SaveFile sf;
        if(!sf.loadAnywhere(filename+".cycle"))sf.load(SKYMAKER_DIR+filename+".cycle");
        sf.clear();
        for(std::vector<WeatherMakerPeriod>::iterator it=mCycleList.begin(); it!=mCycleList.end(); it++)
        {
            WeatherMakerPeriod ref = *it;
            WeatherPeriod *period = ref.first;
            const String periodName = StringConverter::toString(period->mPeriod.first) + "-" + StringConverter::toString(period->mPeriod.second);
            sf.setSetting("type",period->mType,periodName);
            if(period->mFadeIn)sf.setSetting("fade","in",periodName);
            else if(period->mFadeOut)sf.setSetting("fade","out",periodName);
        }
        sf.save();
    }
    void doLoadCycle(const bool &load=false)
    {
        const String fileName = mCycleLoadList->getSelection();
        if(fileName=="")
        {
            mGui->showAlertBox("No file to load!");
            return;
        }
        if(!load)
        {
            mGui->showAlertBox("Load file "+fileName+"?","SkyMaker/LoadCycle",true);
            return;
        }
        loadCycle(fileName);
    }
    const bool loadCycle(const String &filename)
    {
        SaveFile sf;
        if(!sf.loadAnywhere(filename+".cycle"))
        {
            mGui->showAlertBox(filename+" not found!");
            return false;
        }
        mCycleField->setCaption(filename);
        clearWeatherCycle();
        for(sf.beginSectionIterator(); sf.hasMoreSections(); sf.nextSection())
        {
            const StringVector periodStr = StringUtil::split(sf.peekNextSectionName(),"-",1);
            if(periodStr.size()<2)continue;

            WeatherPeriod period;
            period.mPeriod.first = StringConverter::parseInt(periodStr[0]);
            period.mPeriod.second = StringConverter::parseInt(periodStr[1]);

            for(sf.beginSettingIterator(); sf.hasMoreSettings(); sf.nextSetting())
            {
                const String tag = sf.getSettingTag();
                if(tag=="type")period.mType = sf.getSettingValue();
                else if(tag=="fade")
                {
                    period.mFadeIn = (sf.getSettingValue()=="in");
                    period.mFadeOut = (sf.getSettingValue()=="out");
                }
            }

            int duration = period.mPeriod.second - period.mPeriod.first;
            if(duration<0)duration += WEATHER_TIME_MAX;
            createWeatherCyclePeriod(period.mPeriod.first,period.mType,duration,period.mFadeIn,period.mFadeOut);
        }
        updateCycleTime(false);
        return true;
    }
    void loadAllCycles()
    {
        mCycleLoadList->clear();

        SaveFile sf;
        const std::vector<String> list = SaveFile::findResourceNames("*.cycle");
        for(std::vector<String>::const_iterator i = list.begin(); i!=list.end(); i++)
        {
            String line = *i;
            line.erase((int)line.length()-6,6);
            mCycleLoadList->pushLine(line);
        }

        mCycleLoadList->update(0);
        mCycleLoadListSlider->setValue(0);
    }
    void doClearCycle(const bool &reset=false)
    {
        if(!reset)
        {
            mGui->showAlertBox("Clear Weather Cycle?","SkyMaker/ClearCycle",true);
            return;
        }
        clearWeatherCycle();
    }
    void exportSky(const String &skyFile, const bool &load=false)
    {
        std::vector<std::pair<String,String> > filenames;
        std::vector<String> texNames;
        std::vector<String> materialNames;

        if(load && !loadSky(skyFile,false))return;

        const String skyDirectory = SaveFile::findDirectoryOf(skyFile+".sky");
        if(skyDirectory=="")
        {
            mGui->queueAlertBox("Cannot find "+skyFile+".sky!");
            return;
        }
        filenames.push_back(pair<String,String>(skyDirectory, skyFile+".sky"));

        for(int i=0; i<2; i++)
        {
            if(mSkyMat[i]!="" && MaterialManager::getSingletonPtr()->resourceExists(mSkyMat[i]))
            {
                MaterialPtr mat = MaterialManager::getSingletonPtr()->getByName(mSkyMat[i]);
                if(mat.isNull())continue;
                ZipManager::salvageMaterialAndTextures(mat,&materialNames,&texNames);
            }
        }
        //Add to list
        for(std::vector<String>::iterator it=texNames.begin(); it!=texNames.end(); it++)
        {
            const String texName = *it;
            const String filename = SaveFile::findDirectoryOf(texName);
            if(filename!="")filenames.push_back(pair<String,String>(filename, texName));
            else
            {
                LoggerManager::getSingleton().logMessage("ERROR::SkyMaker:exportSky: Cannot find texture "+texName+" for sky "+skyFile+"!");
                mGui->queueAlertBox("Cannot find texture "+texName+" for sky "+skyFile+"!");
            }
        }
        for(std::vector<String>::iterator it=materialNames.begin(); it!=materialNames.end(); it++)
        {
            const String materialName = *it;
            const String filename = SaveFile::findDirectoryOf(materialName);
            if(filename!="")filenames.push_back(pair<String,String>(filename, materialName));
            else
            {
                LoggerManager::getSingleton().logMessage("ERROR::SkyMaker:exportSky: Cannot find material "+materialName+" for sky "+skyFile+"!");
                mGui->queueAlertBox("Cannot find material "+materialName+" for sky "+skyFile+"!");
            }
        }

        if(ZipManager::getSingletonPtr()->fhz(filenames, EXPORT_DIR+skyFile+".fhs"))
        {
            mGui->queueAlertBox("Exported sky as "+String(EXPORT_DIR)+skyFile+".fhs");
        }
        else
        {
            mGui->queueAlertBox("Failed to export sky "+skyFile+"!");
        }
    }
    void exportCycle(const String &cycleFile, const bool &load=false)
    {
        std::vector<std::pair<String,String> > filenames;
        std::vector<String> texNames;
        std::vector<String> materialNames;
        std::vector<String> particleNames;
        std::vector<String> particleScriptNames;
        std::vector<String> weatherNames;

        if(load && !loadCycle(cycleFile))return;

        const String cycleDirectory = SaveFile::findDirectoryOf(cycleFile+".cycle");
        if(cycleDirectory=="")
        {
            mGui->queueAlertBox("Cannot find "+cycleFile+".cycle!");
            return;
        }
        filenames.push_back(pair<String,String>(cycleDirectory, cycleFile+".cycle"));

        for(std::vector<WeatherMakerPeriod>::iterator it=mCycleList.begin(); it!=mCycleList.end(); it++)
        {
            WeatherMakerPeriod ref = *it;
            if(!ref.first)continue;

            //Weather
            Weather *weather = mWeatherMgr->getWeatherByName(ref.first->mType);
            if(!weather)continue;
            const String weatherName = ref.first->mType;
            bool exists = false;
            for(std::vector<String>::iterator jt=weatherNames.begin(); jt!=weatherNames.end(); jt++)
            {
                String cmpName = *jt;
                String cmpName2 = weatherName;
                StringUtil::toLowerCase(cmpName);
                StringUtil::toLowerCase(cmpName2);
                if(cmpName==cmpName2)
                {
                    exists = true;
                    break;
                }
            }
            if(exists)continue;
            weatherNames.push_back(weatherName+".weather");

            //Particle template
            const String particleName = weather->mParticle;
            ParticleSystem *pSys = ParticleSystemManager::getSingletonPtr()->getTemplate(particleName);
            if(!pSys)continue;
            exists = false;
            for(std::vector<String>::iterator jt=particleNames.begin(); jt!=particleNames.end(); jt++)
            {
                const String cmpName = *jt;
                if(cmpName==particleName)
                {
                    exists = true;
                    break;
                }
            }
            if(exists)continue;
            particleNames.push_back(particleName);

            //material/textures
            if(MaterialManager::getSingletonPtr()->resourceExists(pSys->getMaterialName()))
            {
                MaterialPtr mat = MaterialManager::getSingletonPtr()->getByName(pSys->getMaterialName());
                if(mat.isNull())continue;
                ZipManager::salvageMaterialAndTextures(mat,&materialNames,&texNames);
            }

            //script file
            const String scriptName = pSys->getOrigin();
            exists = false;
            for(std::vector<String>::iterator jt=particleScriptNames.begin(); jt!=particleScriptNames.end(); jt++)
            {
                String cmpName = *jt;
                String cmpName2 = scriptName;
                StringUtil::toLowerCase(cmpName);
                StringUtil::toLowerCase(cmpName2);
                if(cmpName==cmpName2)
                {
                    exists = true;
                    break;
                }
            }
            if(exists)continue;
            particleScriptNames.push_back(scriptName);
        }
        //Add to list
        for(std::vector<String>::iterator it=texNames.begin(); it!=texNames.end(); it++)
        {
            const String texName = *it;
            const String filename = SaveFile::findDirectoryOf(texName);
            if(filename!="")filenames.push_back(pair<String,String>(filename, texName));
            else
            {
                LoggerManager::getSingleton().logMessage("ERROR::SkyMaker:exportCycle: Cannot find texture "+texName+" for weather cycle "+cycleFile+"!");
                mGui->queueAlertBox("Cannot find texture "+texName+" for weather cycle "+cycleFile+"!");
            }
        }
        for(std::vector<String>::iterator it=materialNames.begin(); it!=materialNames.end(); it++)
        {
            const String materialName = *it;
            const String filename = SaveFile::findDirectoryOf(materialName);
            if(filename!="")filenames.push_back(pair<String,String>(filename, materialName));
            else
            {
                LoggerManager::getSingleton().logMessage("ERROR::SkyMaker:exportCycle: Cannot find material "+materialName+" for weather cycle "+cycleFile+"!");
                mGui->queueAlertBox("Cannot find material "+materialName+" for weather cycle "+cycleFile+"!");
            }
        }
        std::vector<pair<String,String> > particleScriptlets;
        for(std::vector<String>::iterator it=particleScriptNames.begin(); it!=particleScriptNames.end(); it++)
        {
            String particleScriptName = *it;
            String filename = SaveFile::findDirectoryOf(particleScriptName);
            if(filename=="")
            {
                LoggerManager::getSingleton().logMessage("ERROR::SkyMaker:exportCycle: Cannot find particleScript "+particleScriptName+" for weather cycle "+cycleFile+"!");
                mGui->queueAlertBox("Cannot find particleScript "+particleScriptName+" for weather cycle "+cycleFile+"!");
                continue;
            }
            ByteArray bArray;
            bArray.writeFromFile(filename);
            filename.erase((int)filename.length()-9,9);
            particleScriptName.erase((int)particleScriptName.length()-9,9);

            particleScriptlets = bArray.splitScript(filename,".particle","particle_system");
            unsigned short scriptCount = 0;
            for(std::vector<pair<String,String> >::const_iterator jt=particleScriptlets.begin(); jt!=particleScriptlets.end(); jt++)
            {
                const pair<String,String> scriptFilename = *jt;
                //Only include scriptlets that are required
                bool required = false;
                for(std::vector<String>::iterator kt=particleNames.begin(); kt!=particleNames.end(); kt++)
                {
                    const String particleName = *kt;
                    if(scriptFilename.first==particleName)
                    {
                        required = true;
                        particleNames.erase(kt);
                        break;
                    }
                }
                if(required)
                {
                    const String scriptShortname = particleScriptName+"_"+StringConverter::toString(scriptCount)+".particle";
                    filenames.push_back(pair<String,String>(scriptFilename.second, scriptShortname));
                }
                scriptCount++;
            }
        }
        for(std::vector<String>::iterator it=weatherNames.begin(); it!=weatherNames.end(); it++)
        {
            const String weatherName = *it;
            const String filename = SaveFile::findDirectoryOf(weatherName);
            if(filename!="")filenames.push_back(pair<String,String>(filename, weatherName));
            else
            {
                LoggerManager::getSingleton().logMessage("ERROR::SkyMaker:exportCycle: Cannot find weather "+weatherName+" for weather cycle "+cycleFile+"!");
                mGui->queueAlertBox("Cannot find weather "+weatherName+" for weather cycle "+cycleFile+"!");
            }
        }

        if(ZipManager::getSingletonPtr()->fhz(filenames, EXPORT_DIR+cycleFile+".fhw"))
        {
            mGui->queueAlertBox("Exported weather cycle as "+String(EXPORT_DIR)+cycleFile+".fhw");
        }
        else
        {
            mGui->queueAlertBox("Failed to export weather cycle "+cycleFile+"!");
        }

        while(!particleScriptlets.empty())
        {
            const pair<String,String> filename = particleScriptlets.back();
            particleScriptlets.pop_back();
            _unlink(filename.second.c_str());
        }
    }
};

#endif
