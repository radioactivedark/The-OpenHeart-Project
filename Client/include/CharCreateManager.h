#ifndef _CHARCREATEMANAGER_H_
#define _CHARCREATEMANAGER_H_

#define NUM_PRESETS 12
#define MAX_ITEMS 4

#include <CameraManager.h>
#include <UnitManager.h>
#include <MarkingManager.h>

class CharCreateManager : public Singleton<CharCreateManager>
{
private:
    Gui *mGui;
    Unit *mPlayer;
    UnitManager *mUnitMgr;
    CameraManager *mCameraMgr;
    bool mDoScroll;
    Real mCursorX;
    Real mScrollX;
    Real mZoom;
    PlayerData mPlayerData;
    unsigned char mColouringPart;
    OverlayElement *mColourBox;
    bool mPresetMode;
    bool mNeedsRender;
    bool mEditMode;
    bool mPresetMakerMode;
    String mUsername;
    std::vector<unsigned short> m_vnItems;
public:
    bool mIsExportedPreset;
    CharCreateManager()
    {
        reset();
    }
    ~CharCreateManager()
    {
        reset();
    }
    static CharCreateManager* getSingletonPtr();
    static CharCreateManager& getSingleton();
    void reset()
    {
        mGui = 0;
        mPlayer = 0;
        mUnitMgr = 0;
        mCameraMgr = 0;
        mCursorX = mScrollX = mZoom = 0;
        mDoScroll = false;
        mPlayerData = PlayerData();
        mColouringPart = 0;
        mColourBox = 0;
        mPresetMode = false;
        mNeedsRender = false;
        mEditMode = false;
        mPresetMakerMode = false;
        mIsExportedPreset = false;
        mUsername = "";
        m_vnItems.clear();
    }
    void init()
    {
        mGui = Gui::getSingletonPtr();
        GuiMultiPanel *panel = mGui->addMultiPanel("CharCreateScreen",true);

        GuiTextField *nameField = panel->getTextField("CharCreateScreen/Name");
        nameField->mMaxLength = 32;

        GuiHighlightList *speciesList = panel->getHighlightList("CharCreateScreen/Species");
        speciesList->pushLine("Feline");
        speciesList->pushLine("Canine");
        speciesList->update(0);
        speciesList->setSelection(0);

        GuiHighlightList *emoteList = panel->getHighlightList("CharCreateScreen/Emote");
        emoteList->pushLine("Normal");
        emoteList->pushLine("Smile");
        emoteList->pushLine("Frown");
        emoteList->pushLine("Subtle Frown");
        emoteList->pushLine("Smirk");
        emoteList->pushLine("Sleepy");
        emoteList->pushLine("Lazy Smile");
        emoteList->pushLine("Sad");
        emoteList->pushLine("Brow Raise Both");
        emoteList->pushLine("Cute Smile");
        emoteList->pushLine("Impressed Smile");
        emoteList->pushLine("Serious");
        emoteList->pushLine("Very Serious");
        emoteList->pushLine("Serious Smile");
        emoteList->pushLine("Cool");
        emoteList->pushLine("Cool Smile");
        emoteList->pushLine("Cool Grin");
        emoteList->update(0);
        emoteList->setSelection(0);

        GuiHighlightList *pEquipList = panel->getHighlightList("CharCreateScreen/Equip");
        pEquipList->pushLine("None");
        pEquipList->pushLine("Angel Wings");
        pEquipList->pushLine("Bat Wings");
        pEquipList->update(0);
        pEquipList->setSelection(0);

        GuiHighlightList *colourList = panel->getHighlightList("CharCreateScreen/Colour");
        colourList->pushLine("Pelt");
        colourList->pushLine("Underfur");
        colourList->pushLine("Eyes");
        colourList->pushLine("Above Eyes");
        colourList->pushLine("Below Eyes");
        colourList->pushLine("Nose");
        colourList->pushLine("Tail Tip");
        colourList->pushLine("Mane");
        colourList->pushLine("Markings");
        colourList->pushLine("Equip");
        colourList->update(0);
        colourList->setSelection(0);

        mColourBox = OverlayManager::getSingleton().getOverlayElement("CharCreateScreen/ColourBox");

        if(mEditMode)
        {
            nameField->setCaption(mPlayerData.mName);
            updateMeshList();
            updateMarkingList();
            OverlayManager::getSingleton().getOverlayElement("CharCreateScreen/SX2Label")->setCaption(mPlayerData.mSpecies=='f'?"Chin:":"Cheeks:");
            if(mPlayerData.mSpecies=='c')speciesList->setSelection(1);
            if(mPlayerData.mMesh[1]>0)panel->getHighlightList("CharCreateScreen/Tail")->setSelection(mPlayerData.mMesh[1]-1);
            if(mPlayerData.mMesh[2]>0)panel->getHighlightList("CharCreateScreen/Mane")->setSelection(mPlayerData.mMesh[2]);
            if(mPlayerData.mMesh[3]>0)panel->getHighlightList("CharCreateScreen/Ears")->setSelection(mPlayerData.mMesh[3]-1);
            if(mPlayerData.mMesh[4]>0)panel->getHighlightList("CharCreateScreen/Tuft")->setSelection(mPlayerData.mMesh[4]);
            if(mPlayerData.mMat[0]>0)panel->getHighlightList("CharCreateScreen/Eye")->setSelection(mPlayerData.mMat[0]);
            if(mPlayerData.mMat[1]>0)panel->getHighlightList("CharCreateScreen/MarkBody")->setSelection(mPlayerData.mMat[1]);
            if(mPlayerData.mMat[2]>0)panel->getHighlightList("CharCreateScreen/MarkHead")->setSelection(mPlayerData.mMat[2]);
            if(mPlayerData.mMat[3]>0)panel->getHighlightList("CharCreateScreen/MarkTail")->setSelection(mPlayerData.mMat[3]);
            if(mPlayerData.mEmote>0)
            {
                Emote *emote = EmoteManager::getSingleton().getEmote(mPlayerData.mEmote);
                const short selection = emote ? emoteList->findLine(emote->mName) : -1;
                if(selection>0)emoteList->setSelection(selection);
            }
            pEquipList->setSelection(mPlayerData.m_nEquip);
            if(mPlayerData.m_szItems!="")
            {
                const StringVector vszIndex = StringUtil::split(mPlayerData.m_szItems,";");
                for(int i=0; i<(int)vszIndex.size(); i++)
                {
                    const unsigned short nIndex = StringConverter::parseInt(vszIndex[i]);
                    m_vnItems.push_back(nIndex);
                }
            }

            updateItemsList();

            if(!mPresetMakerMode)panel->getComponentByName("CharCreateScreen/Delete")->show(true);
        }
        else
        {
            mPlayerData.mSpecies = 'f';
            mPlayerData.mEmote = EmoteManager::getSingleton().getEmoteIndex("Normal");
            resetPlayerData();
            setDefaultColours(&mPlayerData);

            panel->getComponentByName("CharCreateScreen/Delete")->show(false);
        }
        updateColourSliders();
        updateColourTextFields();
        for(int i=1;i<=3;i++)panel->getTextField("CharCreateScreen/CT"+StringConverter::toString(i))->mMaxLength = 3;
        updateColours();

        updateScaleSliders();

        mUnitMgr = UnitManager::getSingletonPtr();
        mUnitMgr->fixUsernameForPreset(mUsername);
        mPlayer = mUnitMgr->createUnit(mPlayerData);
        mPlayer->setPosition(Vector3(1510,102,1510));
        mPlayer->setGroundHeight(102);

        updatePresetList();
        GuiHighlightList *presetList = panel->getHighlightList("CharCreateScreen/PresetList");
        panel->getComponentByName("CharCreateScreen/Preset")->setLeft(mPresetMakerMode?0.05f:0.115f);

        if(mEditMode)
        {
            if(mPlayerData.mPreset)
            {
                if(mIsExportedPreset && mPlayerData.mPreset+NUM_PRESETS <= presetList->getNumLines())presetList->setSelection(mPlayerData.mPreset+NUM_PRESETS-1);
                else presetList->setSelection(mPlayerData.mPreset-1);
                setPresetMode(true);
            }
            else
            {
                presetList->clearSelection();
                setPresetMode(mPresetMakerMode);
            }
        }
        else
        {
            presetList->clearSelection();
            setPresetMode(mPresetMakerMode);
        }
        mNeedsRender = false;

        if(mPresetMakerMode)panel->selectTab(panel->getTab("CharCreateScreen/Page5"));

        mCameraMgr = CameraManager::getSingletonPtr();
        mCameraMgr->setPosition(Vector3(1540,112,1540));
        mCameraMgr->yaw(215);
        mCameraMgr->pitch(5);
    }
    void update(const Real &timeElapsed)
    {
        if(mDoScroll)updateScroll();
        mPlayer->yaw(mScrollX*300*(mDoScroll?1:50*timeElapsed),true);
        mPlayer->update(timeElapsed);
        mCameraMgr->setPosition(Vector3(1540,112,1540)+mZoom*(mPlayer->getHeadPosition()+mPlayer->getOrientation()*(mPlayer->getScale()*Vector3(0,1,4))-Vector3(1540,112,1540)));
    }
    void updateScroll()
    {
        mScrollX = mGui->getCursorX() - mCursorX;
        if(Math::Abs(mScrollX)>0.05f)mScrollX = 0.05f * (mScrollX<0?-1:1);
        mCursorX = mGui->getCursorX();
    }
    void doScroll(const bool &flag)
    {
        mDoScroll = flag;
        if(mDoScroll)
        {
            mScrollX = 0;
            mCursorX = mGui->getCursorX();
        }
    }
    void zoom(const Real &y)
    {
        if(!mDoScroll)return;
        mZoom += y;
        if(mZoom<0)mZoom = 0;
        else if(mZoom>0.75f)mZoom = 0.75f;
    }
    Unit* getPlayer()
    {
        return mPlayer;
    }
    void setUsername(const String &name)
    {
        mUsername = name;
    }
    void setPresetMakerMode(const bool &flag)
    {
        mPresetMakerMode = flag;
    }
    void buttonClicked(GuiComponent *button)
    {
        if(button->getName()=="CharCreateScreen/ItemAdd")
        {
            addRemoveItem();
        }
        else if(button->getName()=="CharCreateScreen/ItemRemoveAll")
        {
            removeAllItems();
        }
        else if(button->getName()=="CharCreateScreen/Preset")
        {
            setPresetMode(!mPresetMode);
        }
        else if(button->getName()=="CharCreateScreen/Export")
        {
            exportPreset();
        }
    }
    void changeEvent(GuiComponent *component)
    {
        if(component->isSlider())
        {
            GuiSlider *slider = static_cast<GuiSlider*>(component);
            unsigned short index = 0;
            if(slider->nameIsIndex("CharCreateScreen/SX",&index))
            {
                const unsigned char part = index-1;
                mPlayerData.mScale[part] = char((slider->getValue()-0.5f)*255);
                //General mods
                if(part<NUM_SCALABLES-3)mPlayer->setModScale(part,mPlayerData.mScale[part]);
                //Node scale mods
                else
                {
                    //Keep differences of node scales within 0.25
                    bool changes = false;
                    for(int i=1;i<=2;i++)
                    {
                        const unsigned char next = (part+i>=NUM_SCALABLES ? part-(3-i) : part+i);
                        if(Math::Abs(mPlayerData.mScale[part]-mPlayerData.mScale[next])>32)
                        {
                            mPlayerData.mScale[next] = mPlayerData.mScale[part] + 32*(mPlayerData.mScale[part]>mPlayerData.mScale[next]?-1:1);
                            changes = true;
                        }
                    }
                    if(changes)updateScaleSliders();
                    mPlayer->setScale(mPlayerData.mScale[NUM_SCALABLES-3],mPlayerData.mScale[NUM_SCALABLES-2],mPlayerData.mScale[NUM_SCALABLES-1]);
                }
            }
            else if(slider->nameIsIndex("CharCreateScreen/CX",&index))
            {
                const unsigned char rgb = index-1;
                mPlayerData.mColour[mColouringPart][rgb] = (unsigned char)(slider->getValue()*255);
                updateColourTextFields();
                updateColours();
            }
        }
        else if(component->isList())
        {
            GuiList *list = static_cast<GuiList*>(component);
            if(list->nameIs("CharCreateScreen/Species"))
            {
                //Change species
                mPlayerData.mSpecies = list->getSelectedLine()==0? 'f' : 'c';
                resetPlayerData();
                refreshPlayer();
            }
            else if(list->nameIs("CharCreateScreen/Emote"))
            {
                //Set default emote
                mPlayerData.mEmote = EmoteManager::getSingleton().getEmoteIndex(list->getSelection());
                mPlayer->setEmote(mPlayerData.mEmote,true);
            }
            else if(list->nameIs("CharCreateScreen/Tail"))
            {
                //Change tail
                mPlayerData.mMesh[1] = list->getSelectedLine() + 1;
                refreshPlayer();
            }
            else if(list->nameIs("CharCreateScreen/Mane"))
            {
                //Change mane
                mPlayerData.mMesh[2] = list->getSelectedLine();
                refreshPlayer();
            }
            else if(list->nameIs("CharCreateScreen/Ears"))
            {
                //Change mane
                mPlayerData.mMesh[3] = list->getSelectedLine() + 1;
                refreshPlayer();
            }
            else if(list->nameIs("CharCreateScreen/Tuft"))
            {
                //Change mane
                mPlayerData.mMesh[4] = list->getSelectedLine();
                refreshPlayer();
            }
            else if(list->nameIs("CharCreateScreen/Eye"))
            {
                //Change eye
                mPlayerData.mMat[0] = list->getSelectedLine();
                refreshPlayer();
            }
            else if(list->nameIs("CharCreateScreen/Colour"))
            {
                //Change colouring part
                mColouringPart = list->getSelectedLine();
                updateColourSliders();
                updateColourTextFields();
                updateColours();
            }
            else if(list->nameIs("CharCreateScreen/MarkBody"))
            {
                //Change body marking
                mPlayerData.mMat[1] = list->getSelectedLine();
                if(!mPresetMode)
                {
                    String speciesStr = "";
                    speciesStr.push_back(mPlayerData.mSpecies);
                    StringUtil::toUpperCase(speciesStr);
                    mPlayer->setMarkings(speciesStr,mPlayerData.mMat[1],0);
                    //mPlayer->setColour(8,mPlayerData.getColours(8),true,false,false);
                    for(int i=0;i<NUM_COLOURABLES;i++)mPlayer->setColour(i,mPlayerData.getColours(i),(i==8&&mPlayerData.mMat[1]),(i==8&&mPlayerData.mMat[2]),(i==8&&mPlayerData.mMat[3]));
                }
            }
            else if(list->nameIs("CharCreateScreen/MarkHead"))
            {
                //Change head marking
                mPlayerData.mMat[2] = list->getSelectedLine();
                if(!mPresetMode)
                {
                    String speciesStr = "";
                    speciesStr.push_back(mPlayerData.mSpecies);
                    StringUtil::toUpperCase(speciesStr);
                    mPlayer->setMarkings(speciesStr,mPlayerData.mMat[2],1);
                    //mPlayer->setColour(8,mPlayerData.getColours(8),false,true,false);
                    for(int i=0;i<NUM_COLOURABLES;i++)mPlayer->setColour(i,mPlayerData.getColours(i),(i==8&&mPlayerData.mMat[1]),(i==8&&mPlayerData.mMat[2]),(i==8&&mPlayerData.mMat[3]));
                }
            }
            else if(list->nameIs("CharCreateScreen/MarkTail"))
            {
                //Change tail marking
                mPlayerData.mMat[3] = list->getSelectedLine();
                if(!mPresetMode)
                {
                    String speciesStr = "";
                    speciesStr.push_back(mPlayerData.mSpecies);
                    StringUtil::toUpperCase(speciesStr);
                    mPlayer->setMarkings(speciesStr,mPlayerData.mMat[3],2);
                    //mPlayer->setColour(8,mPlayerData.getColours(8),false,false,true);
                    for(int i=0;i<NUM_COLOURABLES;i++)mPlayer->setColour(i,mPlayerData.getColours(i),(i==8&&mPlayerData.mMat[1]),(i==8&&mPlayerData.mMat[2]),(i==8&&mPlayerData.mMat[3]));
                }
            }
            else if(list->nameIs("CharCreateScreen/PresetList"))
            {
                //Change preset index
                String presetStr = list->getSelection();
                if(StringUtil::startsWith(presetStr,"preset"))
                {
                    GuiPanel *panel = mGui->getPanelByName("CharCreateScreen");
                    GuiButton *exportButton = panel->getButton("CharCreateScreen/Export");
                    exportButton->show(true);
                    presetStr.erase(0,7);
                    const unsigned char preset = (unsigned char)StringConverter::parseInt(presetStr);
                    if(mPlayer->setPreset(preset))mPlayerData.mPreset = preset;
                    else if(mPlayerData.mPreset)mPlayer->setPreset(mPlayerData.mPreset);
                    else setPresetMode(!mPresetMode);
                    mIsExportedPreset = false;
                }
                else if(StringUtil::startsWith(presetStr,mUsername,false))
                {
                    GuiPanel *panel = mGui->getPanelByName("CharCreateScreen");
                    GuiButton *exportButton = panel->getButton("CharCreateScreen/Export");
                    exportButton->show(false);
                    presetStr.erase(0,mUsername.length()+1);
                    const unsigned char preset = (unsigned char)StringConverter::parseInt(presetStr);
                    if(mPlayer->setPreset(preset,mUsername))mPlayerData.mPreset = preset;
                    else if(mPlayerData.mPreset)mPlayer->setPreset(mPlayerData.mPreset,mUsername);
                    else setPresetMode(!mPresetMode);
                    mIsExportedPreset = true;
                }
            }
            else if(list->nameIs("CharCreateScreen/Equip"))
            {
                //Change equip
                mPlayerData.m_nEquip = list->getSelectedLine();
                refreshPlayer();
            }
            else if(list->nameIs("CharCreateScreen/Item"))
            {
                updateItemSelection();
            }
        }
        else if(component->isTextComponent() && static_cast<GuiTextComponent*>(component)->isEditable())
        {
            GuiTextField *text = static_cast<GuiTextField*>(component);
            unsigned short index = 0;
            if(text->nameIs("CharCreateScreen/Name"))
            {
                mPlayerData.mName = text->getCaption();
            }
            else if(text->nameIsIndex("CharCreateScreen/CT",&index))
            {
                const unsigned char rgb = index-1;
                mPlayerData.mColour[mColouringPart][rgb] = StringConverter::parseInt(text->getCaption());
                text->setCaption(StringConverter::toString(mPlayerData.mColour[mColouringPart][rgb]));
                updateColourSliders();
                updateColours();
            }
        }
    }
    void resetPlayerData()
    {
        mPlayerData.mMesh.clear();
        mPlayerData.mMesh.push_back(1); //body
        mPlayerData.mMesh.push_back(1); //tail
        mPlayerData.mMesh.push_back(0); //mane
        mPlayerData.mMesh.push_back(1); //ears
        mPlayerData.mMesh.push_back(0); //tuft
        mPlayerData.mMat.clear();
        mPlayerData.mMat.push_back(0); //eyes
        mPlayerData.mMat.push_back(0); //body markings
        mPlayerData.mMat.push_back(0); //head markings
        mPlayerData.mMat.push_back(0); //tail markings
        updateMeshList();
        updateMarkingList();
        OverlayManager::getSingleton().getOverlayElement("CharCreateScreen/SX2Label")->setCaption(mPlayerData.mSpecies=='f'?"Chin:":"Cheeks:");

        mPlayerData.m_nEquip = 0;
        updateEquipList();
        mPlayerData.m_szItems = "";
        m_vnItems.clear();
        updateItemsList();
    }
    void updateScaleSliders()
    {
        GuiPanel *panel = mGui->getPanelByName("CharCreateScreen");
        for(int i=0;i<NUM_SCALABLES;i++)
            panel->getSlider("CharCreateScreen/SX"+StringConverter::toString(i+1))->setValue(float(mPlayerData.mScale[i])/255+0.5f);
    }
    void updateColourSliders()
    {
        GuiPanel *panel = mGui->getPanelByName("CharCreateScreen");
        for(int i=0;i<3;i++)
            panel->getSlider("CharCreateScreen/CX"+StringConverter::toString(i+1))->setValue(float(mPlayerData.mColour[mColouringPart][i])/255);
    }
    void updateColourTextFields()
    {
        GuiPanel *panel = mGui->getPanelByName("CharCreateScreen");
        for(int i=0;i<3;i++)
            panel->getComponentByName("CharCreateScreen/CT"+StringConverter::toString(i+1))->setCaption(StringConverter::toString(mPlayerData.mColour[mColouringPart][i]));
    }
    void updateColours()
    {
        if(mPlayer)mPlayer->setColour(mColouringPart,mPlayerData.getColours(mColouringPart),(mColouringPart==8&&mPlayerData.mMat[1]),(mColouringPart==8&&mPlayerData.mMat[2]),(mColouringPart==8&&mPlayerData.mMat[3]));
        mColourBox->setCustomParameter(1,mPlayerData.getColours(mColouringPart));
        CapabilitiesManager::getSingletonPtr()->assertCustomColour(mColourBox,1);
    }
    void updateMeshList()
    {
        GuiPanel *panel = mGui->getPanelByName("CharCreateScreen");
        GuiHighlightList *list = panel->getHighlightList("CharCreateScreen/Tail");
        list->clear();
        if(mPlayerData.mSpecies=='f')
        {
            list->pushLine("Lion Tail");
            list->pushLine("Thick Tail");
            list->pushLine("Rounded Tail");
            list->pushLine("Lynx Tail");
            list->pushLine("Tailess");
        }
        else
        {
            list->pushLine("Wolf Tail");
            list->pushLine("Thin Tail");
            list->pushLine("Cat Tail");
            list->pushLine("Sickle Tail");
            list->pushLine("Tailess");
        }
        list->update(0);
        list->setSelection(0);
        panel->getSlider("CharCreateScreen/TailLS")->updateSliderToListSelection();

        list = panel->getHighlightList("CharCreateScreen/Mane");
        list->clear();
        list->pushLine("Maneless");
        if(mPlayerData.mSpecies=='f')
        {
            list->pushLine("Fluffy");
            list->pushLine("Spiky");
            list->pushLine("Forward");
            list->pushLine("Side");
            list->pushLine("Standing");
            list->pushLine("Center Half");
            list->pushLine("Forward Half");
            list->pushLine("Side Half");
            list->pushLine("Center Full");
            list->pushLine("Side Full");
            list->pushLine("Semi Full");
            list->pushLine("Front Center Full");
        }
        else
        {
            list->pushLine("Side");
            list->pushLine("Forward");
            list->pushLine("Center");
            list->pushLine("Forward Mohawk");
            list->pushLine("Backward Mohawk");
            list->pushLine("Eyecover");
            list->pushLine("Flame");
            list->pushLine("Large Mohawk");
        }
        list->update(0);
        list->setSelection(0);
        panel->getSlider("CharCreateScreen/ManeLS")->updateSliderToListSelection();

        list = panel->getHighlightList("CharCreateScreen/Ears");
        list->clear();
        list->pushLine("Normal Ears");
        if(mPlayerData.mSpecies=='f')
        {
            list->pushLine("Long Ears");
            list->pushLine("Pointy Ears");
            list->pushLine("Large Ears");
            list->pushLine("Caracal Ears");
            list->pushLine("Cow Ears");
            list->pushLine("Dragon Ears");
            list->pushLine("Goblin Ears");
            list->pushLine("Leogui Ears");
            list->pushLine("Neko Ears");
            list->pushLine("Rounded Ears");
            list->pushLine("Ridged Ears");
            list->pushLine("Cat Ears");
            list->pushLine("Ghoul Ears");
            list->pushLine("Ripped Ears");
        }
        else
        {
            list->pushLine("Folded Ears");
            list->pushLine("Sharp Ears");
            list->pushLine("Pitbull Ears");
            list->pushLine("Back Ears");
            list->pushLine("Floppy Ears");
            list->pushLine("Hyena Ears");
            list->pushLine("Ripped Ears");
        }
        list->update(0);
        list->setSelection(0);
        panel->getSlider("CharCreateScreen/EarsLS")->updateSliderToListSelection();

        list = panel->getHighlightList("CharCreateScreen/Tuft");
        list->clear();
        list->pushLine("Tuftless");
        if(mPlayerData.mSpecies=='f')
        {
            list->pushLine("Elbow Tuft");
            list->pushLine("Elbow Mane");
            list->pushLine("Cheek Tuft");
            list->pushLine("Cheek Mane");
            list->pushLine("Elbow Rump");
            list->pushLine("Elbow Rump Mane");
            list->pushLine("Full Arm");
            list->pushLine("Full Arm Mane");
            list->pushLine("Full Tuft");
            list->pushLine("Full Mane");
            list->pushLine("Full Tuft Fade");
            list->pushLine("Back Mane");
            list->pushLine("Back, Cheek Mane");
            list->pushLine("Back, Full Mane");
        }
        else
        {
            list->pushLine("Butt Tuft");
            list->pushLine("Elbow Tuft");
            list->pushLine("Rump Tuft");
            list->pushLine("Elbow Rump");
            list->pushLine("Back Mane");
            list->pushLine("Back, Elbow Rump");
            list->pushLine("Full Tuft");
        }
        list->update(0);
        list->setSelection(0);
        panel->getSlider("CharCreateScreen/TuftLS")->updateSliderToListSelection();

        list = panel->getHighlightList("CharCreateScreen/Eye");
        list->clear();
        list->pushLine("Normal");
        list->pushLine("Slit");
        list->pushLine("Black");
        list->pushLine("Black Slit");
        list->pushLine("Pupiless");
        list->pushLine("Orb");
        list->pushLine("Whole");
        list->pushLine("Whole Slit");
        list->pushLine("Contracted");
        list->pushLine("Glow Slit");
        list->pushLine("More Glow");
        list->pushLine("Glow and Shine");
        list->pushLine("Glow Orb");
        list->pushLine("Full Orb");
        list->pushLine("Blind");
        list->pushLine("Blind Slit");
        list->pushLine("Contracted Slit");
        list->pushLine("Cross Eye");
        list->pushLine("Fire");
        list->pushLine("Oval");
        list->pushLine("Ring");
        list->pushLine("Swirl");
        list->update(0);
        list->setSelection(0);
        panel->getSlider("CharCreateScreen/EyeLS")->updateSliderToListSelection();
    }
    void updateMarkingList()
    {
        GuiPanel *panel = mGui->getPanelByName("CharCreateScreen");
        const String part[] = {"Body","Head","Tail"};
        for(int i=0;i<3;i++)
        {
            GuiHighlightList *markingList = panel->getHighlightList("CharCreateScreen/Mark"+part[i]);
            markingList->clear();
            markingList->pushLine("None");

            std::vector<String> *markNameList = MarkingManager::getSingleton().getMarkingList(mPlayerData.mSpecies,i);
            for(int j=0;j<(int)markNameList->size();j++)markingList->pushLine(markNameList->at(j));
            markingList->update(0);
            markingList->setSelection(0);
            panel->getSlider("CharCreateScreen/Mark"+part[i]+"LS")->updateSliderToListSelection();
        }
    }
    void updateEquipList()
    {
        GuiPanel *pPanel = mGui->getPanelByName("CharCreateScreen");
        GuiHighlightList *pEquipList = pPanel->getHighlightList("CharCreateScreen/Equip");
        pEquipList->update(0);
        pEquipList->setSelection(0);
    }
    void updateItemsList()
    {
        GuiPanel *pPanel = mGui->getPanelByName("CharCreateScreen");
        GuiHighlightDoubleList *pItemList = pPanel->getHighlightDoubleList("CharCreateScreen/Item");
        pItemList->clear();

        ItemsManager::getSingletonPtr()->fillList(pItemList);

        for(std::vector<unsigned short>::iterator it=m_vnItems.begin(); it!=m_vnItems.end(); it++)
        {
            const unsigned short nIndex = *it;

            pItemList->changeSecondLine(nIndex-1,"O");
        }

        pItemList->update(0);
        pItemList->setSelection(0);
        updateItemSelection();
    }
    PlayerData& getPlayerData()
    {
        return mPlayerData;
    }
    void refreshPlayer()
    {
        mUnitMgr->remakeUnit(mPlayer,mPlayerData);
        if(mPlayerData.mPreset)mPlayer->setPreset(mPlayerData.mPreset,mIsExportedPreset?mUsername:"");
        mNeedsRender = true;
    }
    void updatePresetList()
    {
        GuiPanel *panel = mGui->getPanelByName("CharCreateScreen");
        GuiHighlightList *presetList = panel->getHighlightList("CharCreateScreen/PresetList");
        presetList->clear();
        if(mPresetMakerMode)
        {
            for(int i=1;i<=NUM_PRESETS;i++)
            {
                if(mPlayer->testPreset(i))presetList->pushLine("Preset "+StringConverter::toString(i));
            }
        }
        if(mUsername!="")
        {
            for(int i=1;i<=NUM_PRESETS;i++)
            {
                bool hasPreset = false;
                if(!mPlayer->testPreset(i,mUsername))
                {
                    if(ZipManager::getSingletonPtr()->defhz(PRESET_DIR+mUsername+"_"+StringConverter::toString(i)+".fhp",true,true) && mPlayer->testPreset(i,mUsername))hasPreset = true;
                }
                else hasPreset = true;
                if(hasPreset)presetList->pushLine(mUsername+" "+StringConverter::toString(i));
            }
        }
        presetList->update();
    }
    void setPresetMode(const bool &flag)
    {
        const bool doRefresh = (mPresetMode!=flag);
        mPresetMode = flag;
        GuiPanel *panel = mGui->getPanelByName("CharCreateScreen");
        GuiList *presetList = panel->getList("CharCreateScreen/PresetList");
        presetList->show(mPresetMode);
        panel->getComponentByName("CharCreateScreen/Preset")->setCaption(mPresetMode?"Custom":"Use Preset");

        GuiButton *exportButton = panel->getButton("CharCreateScreen/Export");
        exportButton->show(mPresetMode && mPresetMakerMode);

        if(mPresetMode)
        {
            String presetStr = presetList->getSelection();
            if(StringUtil::startsWith(presetStr,"preset"))
            {
                if(presetStr.length()>0)presetStr.erase(0,7);
                const unsigned char preset = (unsigned char)StringConverter::parseInt(presetStr);
                mPlayerData.mPreset = preset>0?preset:1;
                if(!mPlayer->setPreset(preset))setPresetMode(false);
            }
            else if(StringUtil::startsWith(presetStr,mUsername,false))
            {
                exportButton->show(false);
                presetStr.erase(0,mUsername.length()+1);
                const unsigned char preset = (unsigned char)StringConverter::parseInt(presetStr);
                mPlayerData.mPreset = preset>0?preset:1;
                if(!mPlayer->setPreset(preset,mUsername))setPresetMode(false);
            }
        }
        else
        {
            mPlayerData.mPreset = 0;
            if(doRefresh)refreshPlayer();
        }
    }
    void exportPreset()
    {
        if(mUsername=="")
        {
            mGui->showAlertBox("Preset exporting requires entering a username in the login page.");
            return;
        }
        String presetNum = StringConverter::toString(mPlayerData.mPreset);

        //Read material textures
        std::vector<String> texList;
        const String matPart[10] = {"bodyMatL","bodyMatR","headMatL","headMatR","eyeMatL","eyeMatR","tailMat","maneMat","equipMatL","equipMatR"};
        for(int i=0;i<10;i++)
        {
            if(MaterialManager::getSingletonPtr()->resourceExists("preset_"+presetNum+"_"+matPart[i]))
            {
                MaterialPtr mat = MaterialManager::getSingletonPtr()->getByName("preset_"+presetNum+"_"+matPart[i]);
                Material::TechniqueIterator mit = mat->getTechniqueIterator();
                while(mit.hasMoreElements())
                {
                    Technique *tech = mit.getNext();
                    Technique::PassIterator pit = tech->getPassIterator();
                    while(pit.hasMoreElements())
                    {
                        Pass *pass = pit.getNext();
                        Pass::TextureUnitStateIterator tit = pass->getTextureUnitStateIterator();
                        while(tit.hasMoreElements())
                        {
                            TextureUnitState *tex = tit.getNext();
                            for(int i=0; i<(int)tex->getNumFrames(); i++)
                            {
                                const String texName = tex->getFrameTextureName(i);
                                if(!SaveFile::resourceExists(texName))continue;
                                bool exists = false;
                                for(std::vector<String>::iterator it=texList.begin(); it!=texList.end(); it++)
                                {
                                    const String cmpName = *it;
                                    if(texName==cmpName)
                                    {
                                        exists = true;
                                        break;
                                    }
                                }
                                if(!exists)texList.push_back(texName);
                            }
                        }
                    }
                }
            }
        }

        //Write new material file
        ByteArray bArr;
        if(!bArr.writeFromFile("my_presets/preset_"+presetNum+"/preset_"+presetNum+".material"))return;
        bArr.replace("preset",mUsername);
        const String tmpFilename = "my_presets/preset_"+presetNum+"/preset_"+presetNum+".tmp";
        if(!bArr.writeToFile(tmpFilename))return;

        std::vector<pair<String,String> > filenames;
        filenames.push_back(pair<String,String>("my_presets/preset_"+presetNum+"/preset_"+presetNum+".tmp",mUsername+"_"+presetNum+".material"));
        for(std::vector<String>::iterator it=texList.begin(); it!=texList.end(); it++)
        {
            const String tex = *it;
            ByteArray texArr;
            texArr.writeUTFBytes(tex);
            texArr.replace("preset",mUsername);
            filenames.push_back(pair<String,String>("my_presets/preset_"+presetNum+"/"+tex,texArr.toString()));
        }

        const String outname = "presets/"+mUsername+"_"+StringConverter::toString(mPlayerData.mPreset)+".fhp";
        if(ZipManager::getSingletonPtr()->fhz(filenames,outname))mGui->showAlertBox("Exported preset as "+outname);
        else mGui->showAlertBox("Failed to export preset.");
        _unlink(tmpFilename.c_str());
        updatePresetList();
    }
    const bool popNeedsRender()
    {
        const bool needsRender = mNeedsRender;
        mNeedsRender = false;
        return needsRender;
    }
    const bool validateName()
    {
        if(mPlayerData.mName=="")
        {
            mGui->showAlertBox("Please enter a name.");
            return false;
        }
        if(mPlayerData.mName.find_first_of("!@#$%^&*()_+-={}[]<>?,./\\;':\"`~|\t")!=string::npos)
        {
            mGui->showAlertBox("Name cannot have symbols!");
            return false;
        }
        for(int i=0; i<(int)mPlayerData.mName.length(); i++)
        {
            if(mPlayerData.mName[i] < 32 || mPlayerData.mName[i] > 126)
            {
                mGui->showAlertBox("Name cannot have symbols!");
                return false;
            }
        }
        if(StringUtil::startsWith(mPlayerData.mName," ") || StringUtil::endsWith(mPlayerData.mName," "))
        {
            mGui->showAlertBox("Name cannot start or end with spaces!");
            return false;
        }
        return true;
    }
    void setEditMode(const bool &flag, PlayerData *data=0)
    {
        mEditMode = flag;
        if(flag && data)mPlayerData = *data;
    }
    void setDefaultColours(PlayerData *playerData)
    {
        playerData->mColour[0][0]=184; playerData->mColour[0][1]=95; playerData->mColour[0][2]=29;
        playerData->mColour[1][0]=234; playerData->mColour[1][1]=179; playerData->mColour[1][2]=144;
        playerData->mColour[2][0]=255; playerData->mColour[2][1]=127; playerData->mColour[2][2]=45;
        playerData->mColour[3][0]=190; playerData->mColour[3][1]=110; playerData->mColour[3][2]=90;
        playerData->mColour[4][0]=250; playerData->mColour[4][1]=190; playerData->mColour[4][2]=170;
        playerData->mColour[5][0]=200; playerData->mColour[5][1]=80; playerData->mColour[5][2]=50;
        playerData->mColour[6][0]=100; playerData->mColour[6][1]=55; playerData->mColour[6][2]=10;
        playerData->mColour[7][0]=100; playerData->mColour[7][1]=55; playerData->mColour[7][2]=10;
        playerData->mColour[8][0]=255; playerData->mColour[8][1]=255; playerData->mColour[8][2]=0;
        playerData->mColour[9][0]=145; playerData->mColour[9][1]=135; playerData->mColour[9][2]=95;
    }
    void addRemoveItem()
    {
        GuiPanel *pPanel = mGui->getPanelByName("CharCreateScreen");
        GuiHighlightDoubleList *pItemList = pPanel->getHighlightDoubleList("CharCreateScreen/Item");

        const unsigned short nIndex = pItemList->getSelectedLine() + 1;
        bool bHasItem = false;

        for(std::vector<unsigned short>::iterator it=m_vnItems.begin(); it!=m_vnItems.end(); it++)
        {
            const unsigned short nComp = *it;
            //Found, remove item
            if(nComp==nIndex)
            {
                bHasItem = true;
                m_vnItems.erase(it);
                break;
            }
        }

        //Add item
        if(!bHasItem)
        {
            if(m_vnItems.size() >= MAX_ITEMS)
            {
                mGui->showAlertBox("Cannot have more than " + StringConverter::toString(MAX_ITEMS) + " items.");
                return;
            }
            m_vnItems.push_back(nIndex);
        }

        pItemList->changeSecondLine(nIndex-1,bHasItem ? "" : "O");
        pItemList->update();
        updateItemSelection();
        redefineItemString();
        refreshPlayer();
    }
    void removeAllItems()
    {
        mPlayer->removeAllItems();
        mPlayerData.m_szItems = "";
        m_vnItems.clear();

        updateItemsList();
    }
    void updateItemSelection()
    {
        GuiPanel *pPanel = mGui->getPanelByName("CharCreateScreen");
        GuiHighlightDoubleList *pItemList = pPanel->getHighlightDoubleList("CharCreateScreen/Item");

        const unsigned short nIndex = pItemList->getSelectedLine() + 1;
        bool bHasItem = false;

        for(std::vector<unsigned short>::iterator it=m_vnItems.begin(); it!=m_vnItems.end(); it++)
        {
            const unsigned short nComp = *it;
            //Found, remove item
            if(nComp==nIndex)
            {
                bHasItem = true;
                break;
            }
        }

        GuiButton *pButton = pPanel->getButton("CharCreateScreen/ItemAdd");
        pButton->setCaption(bHasItem ? "Remove" : "Add");
    }
    void redefineItemString()
    {
        mPlayerData.m_szItems = "";
        for(std::vector<unsigned short>::iterator it=m_vnItems.begin(); it!=m_vnItems.end(); it++)
        {
            const unsigned short nIndex = *it;

            mPlayerData.m_szItems += (mPlayerData.m_szItems==""?"":";") + StringConverter::toString(nIndex);
        }
    }
};

template<> CharCreateManager* Singleton<CharCreateManager>::ms_Singleton = 0;

CharCreateManager* CharCreateManager::getSingletonPtr()
{
	return ms_Singleton;
}

CharCreateManager& CharCreateManager::getSingleton()
{
	assert(ms_Singleton);
	return *ms_Singleton;
}

#endif
