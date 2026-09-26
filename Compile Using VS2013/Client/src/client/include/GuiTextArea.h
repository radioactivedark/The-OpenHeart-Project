#ifndef _GUITEXTAREA_H_
#define _GUITEXTAREA_H_

#include <GuiTextField.h>
#include <GuiSliderList.h>

class GuiTextArea : public GuiTextField, public GuiSliderList
{
protected:
    String mString;
    unsigned short mStartLine;
    unsigned short mActiveLine;
    unsigned short mVisibleLines;
    unsigned short mNumLines;
public:
    bool mAllowNewLine;
    GuiTextArea(OverlayContainer *field, const String &name, const unsigned int *windowWidth, const unsigned int *windowHeight)
    : GuiTextField(field,name,windowWidth,windowHeight), GuiSliderList(field,name)
    {
        mString = "";
        mStartLine = 0;
        mActiveLine = 0;
        mNumLines = 0;
        mAllowNewLine = false;
        updateNumVisibleLines();
    }
    const bool isArea()
    {
        return true;
    }
    virtual void setCaption(const String &caption)
    {
        if(mBoundString)*mBoundString = caption;
        else mString = caption;
        if(mCaption)
        {
            String text = caption;
            wrapCaption(text);
            mCaption->setCaption(text);
            clipCaption();
        }
    }
    void update(const unsigned short &firstLine)
    {
        unsigned short activeLine = ((mNumLines>0&&mVisibleLines>0)? (firstLine+mVisibleLines-1) : 0);
        if(activeLine>0 && activeLine>=mNumLines)activeLine = mNumLines-1;
        if(mActiveLine==activeLine)return;

        String text = mString;
        if(mBoundString)text = *mBoundString;
        wrapCaption(text);
        mCaption->setCaption(text);
        mActiveLine = activeLine;
        clipCaption();
    }
    virtual const String getCaption()
    {
        if(mBoundString)return *mBoundString;
        return mString;
    }
    const String getVirtualCaption()
    {
        if(mCaption)return mCaption->getCaption();
        return "";
    }
    void setActiveLine(const unsigned short &line)
    {
        mActiveLine = line;
    }
    void clipCaption()
    {
        if(!mCaption)return;

        String caption = mCaption->getCaption();
        for(int i=0;i<(int)caption.length();i++)
            if(caption[i]=='\n')
            {
                caption.insert(i," ");
                i++;
            }
        if(caption.length()>0 && caption[caption.length()-1]=='\n')caption += " ";
        const StringVector line = StringUtil::split(caption,"\n");
        mNumLines = (unsigned short)line.size();
        mStartLine = 0;
        if(mNumLines>mVisibleLines && mActiveLine>=mVisibleLines)mStartLine = mActiveLine+1-mVisibleLines;

        String finalCaption = "";
        for(int i=mStartLine;i<mStartLine+mVisibleLines && i<(int)line.size();i++)
        {
            if(i!=mStartLine)finalCaption += "\n";
            finalCaption += line[i];
        }
        mCaption->setCaption(finalCaption);
        if(mNumLines>mVisibleLines)mSlider->show();
        else mSlider->hide();
    }
    const unsigned short getStartLine()
    {
        return mStartLine;
    }
    const unsigned short getSelectedLine()
    {
        return mActiveLine;
    }
    const unsigned short getVisibleLines()
    {
        return mVisibleLines;
    }
    const unsigned short getNumLines()
    {
        return mNumLines;
    }
    void updateNumVisibleLines()
    {
        mVisibleLines = (unsigned short)(mComponent->getHeight()/getCharHeight());
    }
    void show(const bool &flag)
    {
        if(flag)
        {
            mComponent->show();
            if(mNumLines>mVisibleLines)mSlider->show();
        }
        else
        {
            mComponent->hide();
            mSlider->hide();
        }
    }
};

#endif

