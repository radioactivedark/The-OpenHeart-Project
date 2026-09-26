#ifndef _GUITEXTHANDLER_H_
#define _GUITEXTHANDLER_H_

using namespace std;

class GuiTextHandler : public Singleton<GuiTextHandler>
{
private:
    OverlayElement *mCaret;
    OverlayElement *mCaretText;
    GuiTextComponent *mActiveField;
    unsigned int mCaretPos;
    Real mCaretTimer;
    String mInput;
    unsigned int mInputChar;
    OIS::KeyCode mKey;
    Real mHoldTimer;
    Font *mCharFont;
    Real mCharHeight;
    unsigned short mMaxInputLength;
    bool mEditMode;
    Real mScreenRatio;
    bool mShiftDown;
public:
    Real m_fCaretOffset;
    GuiTextHandler()
    {
        mCaret = OverlayManager::getSingleton().getOverlayElement("Caret");
        mCaretText = OverlayManager::getSingleton().getOverlayElement("Caret/Text");
        OverlayManager::getSingleton().getByName("Caret")->show();
        mCaret->hide();
        mCaretTimer = 0;
        mCharFont = 0;
		mCharHeight = 0.03f;

        mActiveField = 0;
        mCaretPos = 0;
        mInput = "";
        mKey = OIS::KC_UNASSIGNED;
        mInputChar = 0;
        mHoldTimer = 0;
        mMaxInputLength = 0;
        mEditMode = false;
        mScreenRatio = 0.75f;
        mShiftDown = false;

        m_fCaretOffset = -0.004f;
    }
    ~GuiTextHandler()
    {
    }
    static GuiTextHandler* getSingletonPtr();
    static GuiTextHandler& getSingleton();
    void update(const Real &timeElapsed)
    {
        //Held key
        if(mKey!=OIS::KC_UNASSIGNED)
        {
            mHoldTimer -= timeElapsed;
            if(mHoldTimer<=0)
            {
                pushChar();
                mHoldTimer = 0.03f;
            }
        }
        //Blink caret
        if(mActiveField)
        {
            mCaretTimer += timeElapsed;
            if(mCaretTimer>=0.5f)
            {
                mCaretTimer = 0;
                if(mCaret->isVisible())mCaret->hide();
                else mCaret->show();
            }
        }
    }
    GuiTextComponent* getActiveField()
    {
        return mActiveField;
    }
    void setActiveField(GuiTextComponent *field, const Real &x, const Real &y, const bool &setCaret=true)
    {
        if(mActiveField==field)
        {
            if(setCaret)setCaretPosition(x,y,true);
            else updateField();
            return;
        }
        mActiveField = field;
        if(mActiveField)
        {
            mInput = mActiveField->getCaption();
            mCharFont = mActiveField->getCharFont();
            mCharHeight = mActiveField->getCharHeight();
            mEditMode = mActiveField->isEditable();
            if(mEditMode)mMaxInputLength = static_cast<GuiTextField*>(mActiveField)->mMaxLength;
            if(setCaret)setCaretPosition(x,y,true);
            else
            {
                mCaretPos = mInput.length();
                updateField();
            }
            updateCaretSize();
        }
        else
        {
            mInput = "";
            mCaretPos = 0;
            mMaxInputLength = 0;
            mEditMode = false;
        }
        keyReleased();
    }
    void releaseActiveField()
    {
        if(!mActiveField)return;
        if(mActiveField->isEditable())mActiveField->setCaption(mInput);
        setActiveField(0,0,0);
        mCaret->hide();
    }
    void keyPressed(const OIS::KeyCode &k, const unsigned int &c, const bool &shiftDown)
    {
        mKey = k;
        mInputChar = c;
        mHoldTimer = 0.5f;
        mShiftDown = shiftDown;
        pushChar();
    }
    void keyReleased()
    {
        mKey = OIS::KC_UNASSIGNED;
        mInputChar = 0;
        mHoldTimer = 0;
    }
    void pushChar()
    {
        using namespace OIS;
        if(!mActiveField)return;
        bool doPushChar = false;
        switch(mKey)
        {
            case KC_BACK:
                if(mCaretPos>0 && mEditMode)
                {
                    mInput.erase(mCaretPos-1,1);
                    mCaretPos--;
                }
                break;
            case KC_DELETE:
                if(mCaretPos<mInput.length() && mEditMode)mInput.erase(mCaretPos,1);
                break;
            case KC_END:
                mCaretPos = mInput.length();
                break;
            case KC_HOME:
                mCaretPos = 0;
                break;
            case KC_LEFT:
                if(mCaretPos>0)mCaretPos--;
                break;
            case KC_RIGHT:
                if(mCaretPos<mInput.length())mCaretPos++;
                break;
            case KC_UP:
            case KC_DOWN:
                if(mActiveField->isArea())
                {
                    String virtualCaption = mInput;
                    mActiveField->wrapCaption(virtualCaption);
                    const StringVector line = StringUtil::split(virtualCaption,"\n");
                    if(line.empty())break;

                    unsigned short activeLine = 0;
                    unsigned int virtualCaretPos = mCaretPos;
                    for(int i=0;i<(int)line.size();i++)
                    {
                        if(virtualCaretPos>(unsigned int)line[i].length())
                        {
                            virtualCaretPos -= (unsigned int)line[i].length();
                            activeLine++;
                        }
                        else break;
                    }
                    if(activeLine>=(int)line.size())activeLine = (int)line.size() - 1;
                    if(mKey==KC_UP)
                    {
                        if(activeLine==0)mCaretPos = 0;
                        else mCaretPos -= (unsigned int)line[activeLine-1].length();
                    }
                    else
                    {
                        mCaretPos += (unsigned int)line[activeLine].length();
                        if(mCaretPos>(unsigned int)mInput.length())mCaretPos = (unsigned int)mInput.length();
                    }
                }
                break;
            //Invalid keys
            case KC_UNASSIGNED:
            case KC_CAPITAL:
            case KC_TAB:
            case KC_LCONTROL:
            case KC_RCONTROL:
            case KC_LSHIFT:
            case KC_RSHIFT:
            case KC_LMENU:
            case KC_RMENU:
            case KC_LWIN:
            case KC_RWIN:
            case KC_PGUP:
            case KC_PGDOWN:
            //case KC_RETURN:
            //case KC_NUMPADENTER:
            case KC_ESCAPE:
            case KC_NUMLOCK:
            case KC_SCROLL:
            case KC_SYSRQ:
            case KC_PAUSE:
            case KC_INSERT:
            case KC_MUTE:
            case KC_VOLUMEUP:
            case KC_VOLUMEDOWN:
            case KC_F1: case KC_F2: case KC_F3: case KC_F4: case KC_F5:
            case KC_F6: case KC_F7: case KC_F8: case KC_F9: case KC_F10:
            case KC_F11: case KC_F12: case KC_F13: case KC_F14: case KC_F15:
                return;
            case KC_COLON:
                mInputChar = (mShiftDown ? '"' : '\'');
                doPushChar = true;
                break;
            case KC_PREVTRACK:
                mInputChar = (mShiftDown ? '+' : '=');
                doPushChar = true;
                break;
                break;
            default:
                doPushChar = true;
                break;
        }
        if(doPushChar && !((mMaxInputLength && mInput.length()>=mMaxInputLength) || !mEditMode))
        {
            if(mInputChar > 126)mInputChar = 126;
            //Push input char
            if(mCaretPos>=mInput.length())mInput.push_back(mInputChar);
            else mInput.insert(mCaretPos,1,mInputChar);
            mCaretPos++;
        }
        //Update component caption
        updateField();
    }
    void updateField()
    {
        //Place caret
        const Vector2 pos = mActiveField->getPosition();
        Real width = 0;

        String input = mInput;
        if(mActiveField->isArea())
        {
            for(int i=0;i<(int)input.length();i++)
                if(input[i]=='\n')
                {
                    input.insert(i," ");
                    i++;
                }
            mActiveField->wrapCaption(input);
        }
        StringVector line = StringUtil::split(input,"\n");
        //Include final newline, if any
        if(mInput.length()>0 && mInput[mInput.length()-1]=='\n')line.push_back("");

        unsigned short activeLine = 0;
        if(!line.empty())
        {
            unsigned int virtualCaretPos = mCaretPos;
            for(int i=0;i<(int)line.size();i++)
            {
                if(virtualCaretPos>(unsigned int)line[i].length())
                {
                    virtualCaretPos -= (int)line[i].length();
                    activeLine++;
                }
                else break;
            }
            if(activeLine>=(int)line.size())activeLine = (int)line.size() - 1;
            const String caption = line[activeLine];

            if(mCharFont)
            {
                if(mActiveField->isMasked())width = mCharFont->getGlyphAspectRatio('*') * mCaretPos;
                else
                {
                    if(virtualCaretPos==(unsigned int)caption.length() && activeLine<(int)line.size()-1)activeLine++;
                    else
                        for(unsigned int i=0;i<virtualCaretPos && i<(unsigned int)caption.length();i++)
                        {
                            if(caption[i] == ' ')width += mCharFont->getGlyphAspectRatio(0x0030);
                            else width += mCharFont->getGlyphAspectRatio(caption[i]);
                        }
                }
            }
        }

        if(mActiveField->isArea())static_cast<GuiTextArea*>(mActiveField)->setActiveLine(activeLine);
        if(mEditMode || mActiveField->isArea())mActiveField->setCaption(mInput);
        if(mActiveField->isArea())
        {
            GuiTextArea *activeArea = static_cast<GuiTextArea*>(mActiveField);
            activeLine -= activeArea->getStartLine();
            static_cast<GuiSlider*>(activeArea->getSliderRef())->updateSliderToListSelection();
        }

        mCaret->setPosition(pos.x + width*mCharHeight*mScreenRatio + m_fCaretOffset,pos.y + activeLine*mCharHeight -0.007f);
        mCaret->show();
        mCaretTimer = 0;
    }
    void setCaretPosition(const Real &x, const Real &y, const bool &showCaret)
    {
        if(!mCharFont || !mActiveField)return;
        if(showCaret)
        {
            mCaret->show();
            mCaretTimer = 0;
        }
        const Vector2 pos = mActiveField->getPosition();
        Real width = pos.x+0.01f;

        String virtualCaption = mInput;
        for(int i=0;i<(int)virtualCaption.length();i++)
            if(virtualCaption[i]=='\n')
            {
                virtualCaption.insert(i," ");
                i++;
            }
        mActiveField->wrapCaption(virtualCaption);
        StringVector line = StringUtil::split(virtualCaption,"\n");
        //Include final newline, if any
        if(mInput.length()>0 && mInput[mInput.length()-1]=='\n')line.push_back("");

        unsigned short selectedLine = 0;
        if(line.empty())mCaretPos = 0;
        else
        {
            selectedLine = (unsigned short)((y - (pos.y+0.007f))/mCharHeight);
            if(mActiveField->isArea())selectedLine += static_cast<GuiTextArea*>(mActiveField)->getStartLine();
            if(selectedLine>=(int)line.size())selectedLine = (int)line.size()-1;

            const String caption = line[selectedLine];
            bool found = false;
            for(unsigned int i=0;i<(unsigned int)caption.length();i++)
            {
                Real charWidth = 0;
                if(mActiveField->isMasked())charWidth = mCharFont->getGlyphAspectRatio('*');
                else if(caption[i] == ' ')
                {
                    //Final space has no width
                    if(i+1<(unsigned int)caption.length())charWidth = mCharFont->getGlyphAspectRatio(0x0030);
                }
                else charWidth = mCharFont->getGlyphAspectRatio(caption[i]);

                const Real halfWidth = charWidth*mCharHeight*mScreenRatio*0.5f;
                width += halfWidth;
                if(x<=width)
                {
                    mCaretPos = i;
                    width -= halfWidth;
                    found = true;
                    break;
                }
                width += halfWidth;
            }
            if(!found)
            {
                mCaretPos = caption.length();
                if(caption.length()>0&&caption[caption.length()-1]==' ')mCaretPos--;
            }
            for(int j=0;j<selectedLine;j++)mCaretPos += (int)line[j].length();
            if(mActiveField->isArea())selectedLine -= static_cast<GuiTextArea*>(mActiveField)->getStartLine();
        }
        mCaret->setPosition(width-0.01f+m_fCaretOffset,pos.y+selectedLine*mCharHeight-0.007f);
    }
    void windowResized(const unsigned int &windowWidth, const unsigned int &windowHeight)
    {
        mScreenRatio = Real(windowHeight)/windowWidth;
        updateCaretSize();
    }
    void updateCaretSize()
    {
        if(!mActiveField)return;
        mCaretText->setParameter("char_height",StringConverter::toString(mActiveField->getCharHeight(true)*1.33f));
        mCaretText->setTop(2+mActiveField->getCaptionTopOffset());
    }
    const bool isCaretAtEnd()
    {
        return (mCaretPos == (unsigned int)mInput.length());
    }
};

template<> GuiTextHandler* Singleton<GuiTextHandler>::ms_Singleton = 0;

GuiTextHandler* GuiTextHandler::getSingletonPtr()
{
	return ms_Singleton;
}

GuiTextHandler& GuiTextHandler::getSingleton()
{
	assert(ms_Singleton);
	return *ms_Singleton;
}

#endif
