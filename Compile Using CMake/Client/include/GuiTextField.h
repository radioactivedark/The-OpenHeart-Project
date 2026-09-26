#ifndef _GUITEXTFIELD_H_
#define _GUITEXTFIELD_H_

#include <GuiComponent.h>
#include <OgreFontManager.h>

class GuiTextComponent : public GuiComponent
{
protected:
    OverlayElement *mCaption;
    const unsigned int *mWindowWidth;
    const unsigned int *mWindowHeight;
    Real mCharHeightRatio;
    bool mHasOwnCharHeight;
public:
    bool mClickable;
    enum
    {
        ALIGN_LEFT,
        ALIGN_RIGHT,
        ALIGN_CENTER
    };
    GuiTextComponent(OverlayContainer *field, const String &name, const unsigned int *windowWidth, const unsigned int *windowHeight)
    : mWindowWidth(windowWidth), mWindowHeight(windowHeight)
    {
        mClickable = true;
        mComponent = field;
        mName = name;
        mCaption = OverlayManager::getSingleton().createOverlayElementFromTemplate("GuiTemplate/TextFieldText","TextArea",mName+"Text");
        field->addChild(mCaption);
        mHasOwnCharHeight = false;
        if(*windowHeight!=0)windowResized();
    }
    virtual ~GuiTextComponent()
    {
        const String childName = mCaption->getName();
        mComponent->removeChild(childName);
        OverlayManager::getSingleton().destroyOverlayElement(childName);
    }
    virtual void setCaption(const String &caption)
    {
        if(!mCaption)return;
        mCaption->setCaption(caption);
        if(mCaption->getParameter("alignment")=="center")
        {
            unsigned short numLines = (caption.length()>0 ? 1 : 0);
            for(int i=0;i<(int)caption.length();i++)
                if(caption[i]=='\n')numLines++;
            mCaption->setTop((mComponent->getHeight()-getCharHeight()*numLines)*0.5f*(*mWindowHeight));
        }
    }
    virtual const String getCaption()
    {
        if(mCaption)return mCaption->getCaption();
        return "";
    }
    const bool isTextComponent()
    {
        return true;
    }
    virtual const bool isEditable()
    {
        return false;
    }
    const Vector2 getPosition()
    {
        return Vector2(mComponent->_getDerivedLeft(),mComponent->_getDerivedTop());
    }
    const Real getCharHeight(const bool &pixels=false)
    {
        if(!mCaption)return 0.03f;
        return (pixels ? StringConverter::parseReal(mCaption->getParameter("char_height")) : mCharHeightRatio);
    }
    Font* getCharFont()
    {
        if(!mCaption)return 0;
        return dynamic_cast<Font*>(FontManager::getSingleton().getByName(mCaption->getParameter("font_name")).getPointer());
    }
    virtual const bool isMasked()
    {
        return false;
    }
    virtual const bool isArea()
    {
        return false;
    }
    void setColours(const ColourValue &top, const ColourValue &bottom)
    {
        if(!mCaption)return;
        mCaption->setParameter("colour_top",StringConverter::toString(top));
        mCaption->setParameter("colour_bottom",StringConverter::toString(bottom));
    }
    static void wrapCaption(String &caption, Font *font, const Real &charHeight, const Real &boxWidth)
    {
        Real textWidth = 0;
        Real widthFromSpace = 0;
        int lastSpace = -1;
        for(int i=0;i<(int)caption.length();i++)
        {
            if(caption[i]==' ')
            {
                lastSpace = i;
                const Real charWidth = font->getGlyphAspectRatio(0x0030);
                textWidth += charWidth;
                widthFromSpace = charWidth;
            }
            else if(caption[i]=='\n')
            {
                lastSpace = -1;
                textWidth = 0;
                widthFromSpace = 0;
            }
            else
            {
                const Real charWidth = font->getGlyphAspectRatio(caption[i]);
                textWidth += charWidth;
                if(lastSpace!=-1)widthFromSpace += charWidth;
            }

            if(textWidth*charHeight>=boxWidth*0.9f)
            {
                if(lastSpace==-1)
                {
                    if(i+1<(int)caption.length())// && caption[i+1]!='\n')
                    {
                        caption.insert(i+1,"\n");
                        textWidth = 0;
                    }
                }
                else
                {
                    if(lastSpace+1<(int)caption.length())// && caption[lastSpace+1]!='\n')
                    {
                        caption.insert(lastSpace+1,"\n");
                        textWidth = widthFromSpace;
                        lastSpace = -1;
                    }
                }
            }
        }
    }
    void wrapCaption(String &caption)
    {
        Font *font = getCharFont();
        const Real charHeight = getCharHeight(true);
        const Real boxWidth = mComponent->getWidth()*(*mWindowWidth);
        wrapCaption(caption,font,charHeight,boxWidth);
    }
    const Real getCaptionWidth(const String &caption)
    {
        Real textWidth = 0;
        Font *font = getCharFont();
        const Real charHeight = getCharHeight(false);

        for(int i=0;i<(int)caption.length();i++)
        {
            if(caption[i]==' ')
            {
                const Real charWidth = font->getGlyphAspectRatio(0x0030);
                textWidth += charWidth;
            }
            else
            {
                const Real charWidth = font->getGlyphAspectRatio(caption[i]);
                textWidth += charWidth;
            }
        }

        return (textWidth*charHeight);
    }
    void setAlignment(const unsigned char &type)
    {
        if(!mCaption)return;
        switch(type)
        {
            case ALIGN_LEFT:
                mCaption->setParameter("alignment","left");
                mCaption->setLeft(0);
                break;
            case ALIGN_RIGHT:
                mCaption->setParameter("alignment","right");
                mCaption->setLeft(mComponent->getWidth()*(*mWindowWidth)+2);
                break;
            case ALIGN_CENTER:
                mCaption->setParameter("alignment","center");
                mCaption->setParameter("horz_alignment","center");
                mCaption->setLeft(mComponent->getParent()->getWidth()*0.5f*(*mWindowWidth)+2);
                mCaption->setTop(mComponent->getHeight()*0.5f);
                break;
            default: break;
        }
    }
    void windowResized()
    {
        if(mHasOwnCharHeight)
        {
            mCharHeightRatio = StringConverter::parseReal(mCaption->getParameter("char_height"))/(*mWindowHeight);
        }
        else
        {
            const unsigned short charHeight = (unsigned short)((*mWindowHeight)*0.03f);
            mCaption->setParameter("char_height",StringConverter::toString(charHeight));
            mCharHeightRatio = Real(charHeight)/(*mWindowHeight);
        }
        const String alignment = mCaption->getParameter("alignment");
        if(alignment=="right")setAlignment(ALIGN_RIGHT);
        else if(alignment=="center")setAlignment(ALIGN_CENTER);

        if(isArea())updateNumVisibleLines();
    }
    void setCharHeight(const unsigned short &height)
    {
        mCaption->setParameter("char_height",StringConverter::toString(height));
        mCharHeightRatio = (Real)height/(*mWindowHeight);
        mHasOwnCharHeight = true;

        if(isArea())updateNumVisibleLines();
    }
    virtual void updateNumVisibleLines()
    {
    }
    void setCaptionTopOffset(const Real &top)
    {
        mCaption->setTop(2+top);
    }
    const Real getCaptionTopOffset()
    {
        return mCaption->getTop()-2;
    }
};

class GuiTextField : public GuiTextComponent
{
protected:
    String *mBoundString;
public:
    unsigned short mMaxLength;
    GuiTextField(OverlayContainer *field, const String &name, const unsigned int *windowWidth, const unsigned int *windowHeight)
    : GuiTextComponent(field,name,windowWidth,windowHeight)
    {
        mBoundString = 0;
        mMaxLength = 0;
    }
    virtual void setCaption(const String &caption)
    {
        if(mCaption)mCaption->setCaption(caption);
        if(mBoundString)*mBoundString = caption;
    }
    const bool isEditable()
    {
        return true;
    }
    virtual void bindString(String *s)
    {
        mBoundString = 0;
        setCaption(*s);
        mBoundString = s;
    }
};

#endif
