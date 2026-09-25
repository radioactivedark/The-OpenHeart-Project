#ifndef _GUIMASKEDTEXTFIELD_H_
#define _GUIMASKEDTEXTFIELD_H_

#include <GuiTextField.h>

class GuiMaskedTextField : public GuiTextField
{
public:
    GuiMaskedTextField(OverlayContainer *field, const String &name, const unsigned int *windowWidth, const unsigned int *windowHeight)
    : GuiTextField(field,name,windowWidth,windowHeight)
    {
    }
    void setCaption(const String &caption)
    {
        if(mCaption)mCaption->setCaption(mask(caption));
        if(mBoundString)*mBoundString = caption;
    }
    const String getCaption()
    {
        if(mBoundString)return *mBoundString;
        return "";
    }
    void bindString(String *s)
    {
        mBoundString = s;
        if(mCaption)mCaption->setCaption(mask(*mBoundString));
    }
    const String mask(const String &caption)
    {
        String maskedText = "";
        for(int i=0;i<(int)caption.length();i++)maskedText.push_back('*');
        return maskedText;
    }
    const bool isMasked()
    {
        return true;
    }
};

#endif
