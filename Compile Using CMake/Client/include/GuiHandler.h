#ifndef _GUIHANDLER_H_
#define _GUIHANDLER_H_

#include <SoundManager.h>
#include <KeyboardManager.h>

class GuiHandler : public OIS::KeyListener, public OIS::MouseListener, public WindowEventListener
{
private:
    Gui *mGui;
    GuiTextHandler *mTextHandler;
    RenderWindow *mWindow;
	OIS::InputManager *inputManager;
	OIS::Keyboard *keyboard;
	OIS::Mouse *mouse;
	KeyboardListener *mKeyListener;
	OIS::MouseListener *mMouseListener;
	ButtonListener *mButtonListener;
	ChangeListener *mChangeListener;

    GuiPanel *mActivePanel;
	GuiComponent *mHoverButton;
	//bool mShowCursor;
	//OverlayElement *mCursor;
	//Real mCursorX, mCursorY;
	GuiCursor mCursor;
	bool mLeftDown, mRightDown;
	unsigned int mWindowWidth, mWindowHeight;
	int mWindowLeft, mWindowTop;

	bool mFullScreen;
	Real *mMouseSensitivity;
	SoundManager *mSoundMgr;
	KeyboardManager *mKeyboardMgr;
public:
    GuiHandler(RenderWindow *win) : mCursor(OverlayManager::getSingleton().getOverlayElement("Cursor"))
    {
        mWindowWidth = mWindowHeight = 0;
        mTextHandler = new GuiTextHandler();
        mGui = new Gui(&mCursor,&mActivePanel,&mWindowWidth,&mWindowHeight);
        mWindow = win;
        mKeyListener = 0;
        mMouseListener = 0;
        mButtonListener = 0;

        mActivePanel = 0;
        mHoverButton = 0;
        OverlayManager::getSingleton().getByName("Cursor")->show();
        mLeftDown = mRightDown = false;

        mSoundMgr = SoundManager::getSingletonPtr();
        mKeyboardMgr = KeyboardManager::getSingletonPtr();
    }
    ~GuiHandler()
    {
        WindowEventUtilities::removeWindowEventListener(mWindow, this);
        //Shut down OIS
        inputManager->destroyInputObject(keyboard);
        inputManager->destroyInputObject(mouse);
        OIS::InputManager::destroyInputSystem(inputManager);

        delete mTextHandler;
        delete mGui;
    }
    void init(const bool &fullscreen, Real *mouseSensitivity)
    {
        mFullScreen = fullscreen;
        mMouseSensitivity = mouseSensitivity;

        //Initialize OIS
        OIS::ParamList pl;
        size_t windowHnd = 0;
        std::ostringstream windowHndStr;
        mWindow->getCustomAttribute("WINDOW", &windowHnd);
        windowHndStr << windowHnd;
        pl.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));
        #if defined OIS_WIN32_PLATFORM
        if(!mFullScreen)
        {
            pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_FOREGROUND" )));
            pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE")));
            if(ShowCursor(false)<-1)ShowCursor(true);
        }
        //pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_FOREGROUND")));
        //pl.insert(std::make_pair(std::string("w32_keyboard"), std::string("DISCL_NONEXCLUSIVE")));
        #elif defined OIS_LINUX_PLATFORM
        //pl.insert(std::make_pair(std::string("x11_mouse_grab"), std::string("false")));
        //pl.insert(std::make_pair(std::string("x11_mouse_hide"), std::string("false")));
        //pl.insert(std::make_pair(std::string("x11_keyboard_grab"), std::string("false")));
        //pl.insert(std::make_pair(std::string("XAutoRepeatOn"), std::string("true")));
        #endif
        inputManager = OIS::InputManager::createInputSystem(pl);

        keyboard = (OIS::Keyboard*)inputManager->createInputObject(OIS::OISKeyboard, true);
        mouse = (OIS::Mouse*)inputManager->createInputObject(OIS::OISMouse, true);

        keyboard->setEventCallback(this);
        mouse->setEventCallback(this);
        keyboard->setTextTranslation(OIS::Keyboard::Unicode);

        WindowEventUtilities::addWindowEventListener(mWindow, this);
        windowResized(mWindow);

        mCursor.mCursor->setPosition(0.5*mWindowWidth,0.5*mWindowHeight);
        mCursor.mX = mCursor.mY = 0.5;
    }
    void enterProcedure()
    {
        mActivePanel = (mGui->mPanels.empty()?0:mGui->mPanels.front());
        mHoverButton = 0;

        /*mShowCursor = mGui->showCursor;
        if(mShowCursor)mCursor->show();
        else mCursor->hide();*/
    }
    void exitProcedure()
    {
        if(mKeyboardMgr->isSettingKey())mKeyboardMgr->doSetKey(false);
        if(mHoverButton)mHoverButton->unhover();
        mTextHandler->releaseActiveField();
        setListeners(0,0,0,0);
        mActivePanel = 0;
        mHoverButton = 0;
    }
    void update(const Real &timeElapsed)
    {
        keyboard->capture();
        mouse->capture();

        updateButtons();
        mTextHandler->update(timeElapsed);
    }
    void updateButtons()
    {
        if(mHoverButton)
        {
            if(mHoverButton->isInArea(mCursor.mX,mCursor.mY))
            {
                if(mLeftDown && !mHoverButton->isPressed())mHoverButton->pressed();
                return;
            }
            else
            {
                mHoverButton->unhover();
                if(!mLeftDown)mHoverButton = 0;
            }
        }
        if(mLeftDown || (mGui->isComboBoxOpen() && mGui->getComboBox()->isInArea(mCursor.mX,mCursor.mY)))return;
        GuiPanel *panel = mGui->getPanelAt(mCursor.mX,mCursor.mY);
        if(panel)
        {
            for(std::vector<GuiComponent*>::iterator jt = panel->mComponents.begin(); jt!=panel->mComponents.end(); jt++)
            {
                GuiComponent *component = *jt;
                if(component->isInArea(mCursor.mX,mCursor.mY))
                {
                    if(component->isTextComponent() && !static_cast<GuiTextComponent*>(component)->mClickable)continue;
                    component->hover();
                    mHoverButton = component;
                    return;
                }
            }
        }
    }
    void setListeners(KeyboardListener *kl, OIS::MouseListener *ml, ButtonListener *bl, ChangeListener *cl)
    {
        mKeyListener = kl;
        mMouseListener = ml;
        mButtonListener = bl;
        mChangeListener = cl;
    }
    bool keyPressed(const OIS::KeyEvent &arg)
    {
        using namespace OIS;

        //Changing keyboard controls
        if(mKeyboardMgr->isSettingKey())
        {
            mKeyboardMgr->setCurrentKey(arg.key);
            mKeyboardMgr->doSetKey(false);
            return true;
        }

        const char keyIndex = mKeyboardMgr->getIndex(arg.key);
        if(arg.key==KC_RETURN||arg.key==KC_NUMPADENTER)
        {
            GuiTextComponent *textfield = mTextHandler->getActiveField();
            //Newline
            if(textfield && keyboard->isModifierDown(Keyboard::Shift) && textfield->isArea() && static_cast<GuiTextArea*>(textfield)->mAllowNewLine)
            {
                mTextHandler->keyPressed(arg.key,'\n',true);
            }
            else
            {
                mTextHandler->releaseActiveField();
                if(textfield && textfield->isEditable())
                {
                    mChangeListener->changeEvent(textfield,ChangeListener::EVENT_KEYPRESSED);
                    return true;
                }
                mKeyListener->keyPressed(KeyboardIndex::KB_RETURN);
                return true;
            }
        }
        else if(keyIndex==KeyboardIndex::KB_PRINTSCREEN)
        {
            mWindow->writeContentsToTimestampedFile("screenshots/screenshot_",".png");
        }
        else mTextHandler->keyPressed(arg.key,arg.text,keyboard->isModifierDown(Keyboard::Shift));

        if(mKeyListener && !mTextHandler->getActiveField())
        {
            mKeyListener->keyPressed(keyIndex);
            mKeyListener->keyPressed(arg);
        }

        return true;
    }
    bool keyReleased(const OIS::KeyEvent &arg)
    {
        mTextHandler->keyReleased();

        if(mKeyListener && !mTextHandler->getActiveField())
        {
            mKeyListener->keyReleased(mKeyboardMgr->getIndex(arg.key));
            mKeyListener->keyReleased(arg);
        }

        return true;
    }
    bool mouseMoved(const OIS::MouseEvent &arg)
    {
        //Update cursor
        if(mCursor.mShowCursor)
        {
            if(mFullScreen)
            {
                const int x = int(mCursor.mCursor->getLeft()+(*mMouseSensitivity)*arg.state.X.rel);
                const int y = int(mCursor.mCursor->getTop()+(*mMouseSensitivity)*arg.state.Y.rel);
                mCursor.mCursor->setPosition(x<0?0:((unsigned)x>mWindowWidth?mWindowWidth:x),y<0?0:((unsigned)y>mWindowHeight?mWindowHeight:y));
            }
            else
            {
                #if defined OIS_WIN32_PLATFORM
                tagPOINT lpPoint;
                GetCursorPos(&lpPoint);
                const int x = lpPoint.x-mWindowLeft-4;
                const int y = lpPoint.y-mWindowTop-30;
                if(y<0)
                {
                    if(ShowCursor(true)>0)ShowCursor(false);
                }
                else if(ShowCursor(false)<-1)ShowCursor(true);
                #else
                const int x = (int)mCursor.mCursor->getLeft()+arg.state.X.rel;
                const int y = (int)mCursor.mCursor->getTop()+arg.state.Y.rel;
                #endif
                mCursor.mCursor->setPosition(x<0?0:((unsigned)x>mWindowWidth?mWindowWidth:x),y<0?0:((unsigned)y>mWindowHeight?mWindowHeight:y));
            }
            mCursor.mX = mCursor.mCursor->getLeft()/mWindowWidth;
            mCursor.mY = mCursor.mCursor->getTop()/mWindowHeight;
        }
        else
        {
            #if defined OIS_WIN32_PLATFORM
            if(!mFullScreen)SetCursorPos(int(mCursor.mX*mWindowWidth)+mWindowLeft+4,int(mCursor.mY*mWindowHeight)+mWindowTop+30);
            #endif
        }

        //Update scrollers
        if(mLeftDown && mHoverButton && mHoverButton->isSlider())
        {
            GuiSlider *slider = static_cast<GuiSlider*>(mHoverButton);
            if(slider->isHeld() && slider->updateByCursor(mCursor.mX,mCursor.mY,false))mChangeListener->changeEvent(mHoverButton,ChangeListener::EVENT_MOUSEMOVED);
        }

        //Update sizers
        if(mLeftDown && mHoverButton && mHoverButton->isSizer())
        {
            if(static_cast<GuiSizer*>(mHoverButton)->updateByCursor(mCursor.mX,mCursor.mY))mChangeListener->changeEvent(mHoverButton,ChangeListener::EVENT_MOUSEMOVED);
        }

        //Update movers
        if(mLeftDown && mHoverButton && mHoverButton->isMover())
        {
            if(static_cast<GuiMover*>(mHoverButton)->updateByCursor(mCursor.mX,mCursor.mY))mChangeListener->changeEvent(mHoverButton,ChangeListener::EVENT_MOUSEMOVED);
        }

        //Update combo box
        if(mGui->isComboBoxOpen() && mGui->getComboBox()->isInArea(mCursor.mX,mCursor.mY))
        {
            mGui->getComboBoxList()->doSelect(mCursor.mY);
        }

        //Update held button
        if(mLeftDown && mHoverButton && mHoverButton->isButton() && static_cast<GuiButton*>(mHoverButton)->isHoldable())
        {
            GuiButton *button = static_cast<GuiButton*>(mHoverButton);
            //Held and unhovered
            if(button->isHeld())
            {
                if(!mHoverButton->isInArea(mCursor.mX,mCursor.mY))
                {
                    button->setIsHeld(false);
                    mButtonListener->buttonReleased(button);
                }
            }
            //Rehovered and held
            else
            {
                if(mHoverButton->isInArea(mCursor.mX,mCursor.mY))
                {
                    button->setIsHeld(true);
                    mButtonListener->buttonHeld(button);
                }
            }
        }

        //Update maps
        if(mLeftDown && mHoverButton && mHoverButton->isMap())
        {
            if(static_cast<GuiMap*>(mHoverButton)->updateByCursor(mCursor.mX,mCursor.mY))mChangeListener->changeEvent(mHoverButton,ChangeListener::EVENT_MOUSEMOVED);
        }

        if(mMouseListener)mMouseListener->mouseMoved(arg);

        return true;
    }
    bool mousePressed(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
    {
        if(mKeyboardMgr->isSettingKey())
        {
            mKeyboardMgr->doSetKey(false);
            return true;
        }

        bool isInActivePanel = (mActivePanel && mActivePanel->isInArea(mCursor.mX,mCursor.mY));
        const bool comboBoxWasOpen = mGui->isComboBoxOpen();
        const String comboBoxPrevName = mGui->getComboBoxName();
        if(id==OIS::MB_Left)
        {
            mLeftDown = true;
            //Select combo box
            if(mGui->isComboBoxOpen() && mGui->getComboBox()->isInArea(mCursor.mX,mCursor.mY))
            {
                mGui->hideComboBox();
                mChangeListener->comboBoxEvent(mGui->getComboBoxName());
                mSoundMgr->play2DSound(SOUND_CLICK);
                return true;
            }
            //Select alert box
            if(mGui->getAlertBox()->isInArea(mCursor.mX,mCursor.mY))
            {
                isInActivePanel = true;
            }
            //Select topmost panel
            else if(mActivePanel && !isInActivePanel)
            {
                int pos = 0;
                GuiPanel *panel = mGui->getPanelAt(mCursor.mX,mCursor.mY,&pos);
                if(panel)
                {
                    mGui->setActivePanel(panel,pos);
                    isInActivePanel = true;
                }
            }
            //Unselect active text field
            if(mTextHandler->getActiveField() && mHoverButton!=mTextHandler->getActiveField())
            {
                GuiTextComponent *textfield = mTextHandler->getActiveField();
                mTextHandler->releaseActiveField();
                if(textfield->isEditable())mChangeListener->changeEvent(textfield,ChangeListener::EVENT_MOUSEPRESSED);
            }
            //Click hovered button
            if(mHoverButton)
            {
                mHoverButton->pressed();
                //Select active text field
                if(mHoverButton->isTextComponent())
                {
                    GuiTextComponent *textfield = static_cast<GuiTextComponent*>(mHoverButton);
                    if(textfield->isEditable())mTextHandler->setActiveField(textfield,mCursor.mX,mCursor.mY);
                    else /*if(!mTextHandler->isCaretAtEnd())*/mChangeListener->changeEvent(textfield,ChangeListener::EVENT_MOUSEPRESSED);
                }
                //Update scrollers
                else if(mHoverButton->isSlider())
                {
                    if(static_cast<GuiSlider*>(mHoverButton)->updateByCursor(mCursor.mX,mCursor.mY,true))mChangeListener->changeEvent(mHoverButton,ChangeListener::EVENT_MOUSEPRESSED);
                }
                //Update list
                else if(mHoverButton->isList())
                {
                    if(static_cast<GuiList*>(mHoverButton)->doSelect(mCursor.mY))mChangeListener->changeEvent(mHoverButton,ChangeListener::EVENT_MOUSEPRESSED);
                }
                //Update multipanel tabs
                else if(mHoverButton->isTab())
                {
                    if(mActivePanel->isMultiPanel())
                    {
                        static_cast<GuiMultiPanel*>(mActivePanel)->selectTab(static_cast<GuiTab*>(mHoverButton));
                        mChangeListener->changeEvent(mHoverButton,ChangeListener::EVENT_MOUSEPRESSED);
                    }
                }
                //Update sizers
                else if(mHoverButton->isSizer())
                {
                    static_cast<GuiSizer*>(mHoverButton)->setStartPos(mCursor.mX,mCursor.mY);
                }
                //Update movers
                else if(mHoverButton->isMover())
                {
                    static_cast<GuiMover*>(mHoverButton)->setStartPos(mCursor.mX,mCursor.mY);
                }
                //Update held buttons
                else if(mHoverButton->isButton() && static_cast<GuiButton*>(mHoverButton)->isHoldable())
                {
                    GuiButton *button = static_cast<GuiButton*>(mHoverButton);
                    button->setIsHeld(true);
                    mButtonListener->buttonHeld(button);
                }
                //Update maps
                else if(mHoverButton->isMap())
                {
                    if(static_cast<GuiMap*>(mHoverButton)->updateByCursor(mCursor.mX,mCursor.mY))mChangeListener->changeEvent(mHoverButton,ChangeListener::EVENT_MOUSEPRESSED);
                }

                //Play sound
                if(!mHoverButton->isTextComponent())mSoundMgr->play2DSound(SOUND_CLICK);
            }
        }
        else if(id==OIS::MB_Right)
        {
            mRightDown = true;
            //Unselect active text field
            if(mTextHandler->getActiveField() && mHoverButton!=mTextHandler->getActiveField())
            {
                GuiTextComponent *textfield = mTextHandler->getActiveField();
                mTextHandler->releaseActiveField();
                if(textfield->isEditable())mChangeListener->changeEvent(textfield,ChangeListener::EVENT_MOUSEPRESSED);
            }
            if(mCursor.mHideOnRightClick)mCursor.show(false);
        }
        else if(id==OIS::MB_Middle)
        {
            if(mCursor.mHideOnRightClick)mCursor.show(false);
        }

        //Hide combo box if it was open before clicking (i.e. not spawned by a click event)
        if(comboBoxWasOpen && comboBoxPrevName==mGui->getComboBoxName())mGui->hideComboBox();

        if(!isInActivePanel && mMouseListener)mMouseListener->mousePressed(arg,id);
        return true;
    }
    bool mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
    {
        if(id==OIS::MB_Left)
        {
            mLeftDown = false;
            //Click hovered button
            if(mHoverButton && mHoverButton->isInArea(mCursor.mX,mCursor.mY))
            {
                if(mHoverButton->isButton())
                {
                    GuiButton *button = static_cast<GuiButton*>(mHoverButton);
                    bool buttonEvent = true;
                    //AlertBox OK
                    if(mHoverButton->nameIs("AlertBox/OK"))
                    {
                        mGui->hideAlertBox();
                        mGui->dequeueAlertBox();
                        mChangeListener->alertBoxEvent(mGui->getAlertBoxName(),true);
                        buttonEvent = false;
                    }
                    //AlertBox Cancel
                    else if(mHoverButton->nameIs("AlertBox/Cancel"))
                    {
                        mGui->hideAlertBox();
                        mGui->dequeueAlertBox();
                        mChangeListener->alertBoxEvent(mGui->getAlertBoxName(),false);
                        buttonEvent = false;
                    }
                    //CheckBox
                    else if(button->isCheckBox())static_cast<GuiCheckBox*>(mHoverButton)->toggleChecked();
                    //Holdable button
                    else if(button->isHoldable() && button->isHeld())
                    {
                        button->setIsHeld(false);
                        mButtonListener->buttonReleased(button);
                    }
                    //Close button
                    else if(mHoverButton->getName()=="Close")
                    {
                        if(mActivePanel->getOverlayContainer()==mHoverButton->getParent())mActivePanel->show(false);
                        buttonEvent = false;
                    }

                    if(buttonEvent)mButtonListener->buttonClicked(mHoverButton);
                }
                else if(mHoverButton->isSlider())static_cast<GuiSlider*>(mHoverButton)->setIsHeld(false);

                mHoverButton->unhover();
                mHoverButton = 0;
            }
        }
        else if(id==OIS::MB_Right)
        {
            mRightDown = false;
            if(mCursor.mHideOnRightClick)mCursor.show(true);
        }
        else if(id==OIS::MB_Middle)
        {
            if(mCursor.mHideOnRightClick)mCursor.show(true);
        }

        if(mMouseListener)mMouseListener->mouseReleased(arg,id);
        return true;
    }
    void windowResized(RenderWindow *rw)
    {
		unsigned int depth;
		rw->getMetrics(mWindowWidth, mWindowHeight, depth, mWindowLeft, mWindowTop);

		const OIS::MouseState &ms = mouse->getMouseState();
		ms.width = mWindowWidth;
		ms.height = mWindowHeight;

		mGui->windowResized();
		mTextHandler->windowResized(mWindowWidth,mWindowHeight);
    }
    void windowMoved(RenderWindow *rw)
    {
        unsigned int depth;
		rw->getMetrics(mWindowWidth, mWindowHeight, depth, mWindowLeft, mWindowTop);
    }
    void updateStats()
    {
        const RenderTarget::FrameStats& stats = mWindow->getStatistics();
        mGui->mAvgFPS = "Average FPS: " + StringConverter::toString(stats.avgFPS);
        mGui->mCurFPS = "Current FPS: " + StringConverter::toString(stats.lastFPS);
        mGui->mBestFPS = "Best FPS: " + StringConverter::toString(stats.bestFPS)
            +" Time:"+StringConverter::toString(stats.bestFrameTime)+" ms";
        mGui->mWorstFPS = "Wrost FPS: " + StringConverter::toString(stats.worstFPS)
            +" Time:"+StringConverter::toString(stats.worstFrameTime)+" ms";
        mGui->mTriCount = "Triangle Count: " + StringConverter::toString(stats.triangleCount);
        mGui->mBatchCount = "Batch Count: " + StringConverter::toString(stats.batchCount);
        mGui->updateStats();
    }
};

#endif
