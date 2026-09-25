#ifndef _GUILIST_H_
#define _GUILIST_H_

#include <GuiSliderList.h>

class GuiList : public GuiComponent, public GuiSliderList
{
protected:
    OverlayElement *mCaption;
    std::vector<String> mLine;
    unsigned short mVisibleLines;
    short mSelectedLine;
    unsigned short mFirstLine;
public:
    GuiList(OverlayContainer *list, const String &name) : GuiSliderList(list,name)
    {
        mComponent = list;
        mName = name;
        mCaption = OverlayManager::getSingleton().createOverlayElementFromTemplate("GuiTemplate/BasicText","TextArea",mName+"Text");
        list->addChild(mCaption);
        const Real charHeight = StringConverter::parseReal(mCaption->getParameter("char_height"));

        mVisibleLines = (unsigned short)(list->getHeight()/charHeight);
        mFirstLine = 0;
        mSelectedLine = -1;
    }
    virtual ~GuiList()
    {
        const String childName = mCaption->getName();
        mComponent->removeChild(childName);
        OverlayManager::getSingleton().destroyOverlayElement(childName);
    }
    virtual void pushLine(const String &caption)
    {
        mLine.push_back(caption);
        if((int)mLine.size()>mVisibleLines)mSlider->show();
    }
    virtual void clear()
    {
        mLine.clear();
        mSlider->hide();
        mSelectedLine = -1;
    }
    void clearSelection()
    {
        mSelectedLine = -1;
    }
    virtual void update(const unsigned short &firstLine)
    {
        updateCaption(firstLine);
    }
    virtual void update()
    {
        update(mFirstLine);
    }
    virtual void updateCaption(const unsigned short &firstLine)
    {
        if(firstLine>=(int)mLine.size())
        {
            mCaption->setCaption("");
            return;
        }
        mFirstLine = firstLine;
        String caption = "";
        unsigned short numLines = 0;
        unsigned short i = 0;
        for(std::vector<String>::iterator it=mLine.begin(); it!=mLine.end(); it++)
        {
            if(i<firstLine)
            {
                i++;
                continue;
            }
            const String line = *it;
            caption += (i!=firstLine?"\n":"") + line;
            numLines++;
            if(numLines>=mVisibleLines)break;
            i++;
        }
        mCaption->setCaption(caption);
    }
    virtual const bool doSelect(const Real &cursorY)
    {
        if(mLine.empty())
        {
            mSelectedLine = -1;
            return true;
        }
        const Real charHeight = StringConverter::parseReal(mCaption->getParameter("char_height"));
        unsigned short selection = (unsigned short)((cursorY-mComponent->_getDerivedTop())/charHeight)+mFirstLine;

        if(selection>=mLine.size())selection = mLine.size()-1;
        mSelectedLine = selection;
        return true;
    }
    const String getSelection()
    {
        if(mSelectedLine>=(int)mLine.size() || mSelectedLine<0)return "";
        return mLine[mSelectedLine];
    }
    const unsigned short getSelectedLine()
    {
        if(mSelectedLine<0)return 0;
        return mSelectedLine;
    }
    const String getLine(const unsigned short &line)
    {
        if(line>=(int)mLine.size())return "";
        return mLine[line];
    }
    void changeLine(const unsigned short &line, const String &caption, const bool &append=false)
    {
        if(line>=(int)mLine.size())return;
        if(append)mLine[line] = mLine[line] + caption;
        else mLine[line] = caption;
    }
    const short findLine(const String &caption)
    {
        short i = 0;
        for(std::vector<String>::iterator it=mLine.begin(); it!=mLine.end(); it++)
        {
            const String line = *it;
            if(line==caption)return i;
            i++;
        }
        return -1;
    }
    virtual const bool eraseLine(const String &caption)
    {
        short i = 0;
        for(std::vector<String>::iterator it=mLine.begin(); it!=mLine.end(); it++)
        {
            const String line = *it;
            if(line==caption)
            {
                mLine.erase(it);
                if(i==mSelectedLine)mSelectedLine = -1;
                else if(i>mSelectedLine)mSelectedLine -= 1;
                return true;
            }
            i++;
        }
        return false;
    }
    const bool isList()
    {
        return true;
    }
    virtual const bool isHighlightList()
    {
        return false;
    }
    const unsigned short getNumLines()
    {
        return mLine.size();
    }
    const unsigned short getVisibleLines()
    {
        return mVisibleLines;
    }
    void setVisibleLines(const unsigned short &lines)
    {
        const Real charHeight = StringConverter::parseReal(mCaption->getParameter("char_height"));
        mComponent->setHeight(charHeight*lines);
        mSlider->setHeight(mComponent->getHeight());
        mVisibleLines = lines;
    }
    const Real getHeight()
    {
        return mComponent->getHeight();
    }
    void show(const bool &flag)
    {
        if(flag)
        {
            mComponent->show();
            if((int)mLine.size()>mVisibleLines)mSlider->show();
        }
        else
        {
            mComponent->hide();
            mSlider->hide();
        }
    }
};

class GuiHighlightList : public GuiList
{
private:
    OverlayElement *mHighlight;
    bool mAllowReclick;
public:
    GuiHighlightList(OverlayContainer *list, const String &name) : GuiList(list,name)
    {
        const Real charHeight = StringConverter::parseReal(mCaption->getParameter("char_height"));

        mHighlight = OverlayManager::getSingleton().createOverlayElementFromTemplate("GuiTemplate/Highlight","Panel",mName+"Highlight");
        list->addChild(mHighlight);
        mHighlight->setWidth(list->getWidth());
        mHighlight->setHeight(charHeight);
        mHighlight->hide();

        mAllowReclick = false;
    }
    virtual ~GuiHighlightList()
    {
        const String childName = mHighlight->getName();
        mComponent->removeChild(childName);
        OverlayManager::getSingleton().destroyOverlayElement(childName);
    }
    void update(const unsigned short &firstLine)
    {
        updateCaption(firstLine);

        if(mSelectedLine>=firstLine && mSelectedLine<firstLine+mVisibleLines)
        {
            mHighlight->setTop((mSelectedLine-firstLine)*StringConverter::parseReal(mCaption->getParameter("char_height")));
            mHighlight->show();
        }
        else mHighlight->hide();
    }
    void update()
    {
        update(mFirstLine);
    }
    const bool doSelect(const Real &cursorY)
    {
        if(mLine.empty())
        {
            mSelectedLine = -1;
            mHighlight->hide();
            return false;
        }
        const Real charHeight = StringConverter::parseReal(mCaption->getParameter("char_height"));
        unsigned short selection = (unsigned short)((cursorY-mComponent->_getDerivedTop())/charHeight)+mFirstLine;

        if(selection>=mLine.size())selection = mLine.size()-1;
        if(selection>=mFirstLine && selection<mFirstLine+mVisibleLines)
        {
            mHighlight->setTop((selection-mFirstLine)*charHeight);
            mHighlight->show();
        }
        else mHighlight->hide();
        if(mAllowReclick || selection!=mSelectedLine)
        {
            mSelectedLine = selection;
            return true;
        }
        return false;
    }
    void setSelection(const unsigned short &line, const bool &clampSlider=false)
    {
        mSelectedLine = (line>=(int)mLine.size() ? (int)mLine.size()-1 : line);
        if(clampSlider)
        {
            update( (mSelectedLine>=mVisibleLines?mSelectedLine-(mVisibleLines-1):0) );
            if(mSliderRef)
            {
                const unsigned short numLines = getNumLines() - mVisibleLines;
                if(mSelectedLine<mVisibleLines || numLines==0)mSliderRef->setValue(0);
                else mSliderRef->setValue(Real(mSelectedLine-mVisibleLines+1)/numLines);
            }
        }
        else update(mFirstLine);
    }
    void setAllowReclick(const bool &flag)
    {
        mAllowReclick = flag;
    }
    void setWidth(const Real &width)
    {
        mComponent->setWidth(width);
        mHighlight->setWidth(width);
    }
    const bool isHighlightList()
    {
        return true;
    }
    virtual const bool isDoubleList()
    {
        return false;
    }
};

class GuiHighlightDoubleList : public GuiHighlightList
{
private:
    OverlayElement *mSecondCaption;
    std::vector<String> mSecondLine;
public:
    GuiHighlightDoubleList(OverlayContainer *list, const String &name) : GuiHighlightList(list,name)
    {
        mSecondCaption = OverlayManager::getSingleton().createOverlayElementFromTemplate("GuiTemplate/BasicText2","TextArea",mName+"Text2");
        list->addChild(mSecondCaption);
    }
    virtual ~GuiHighlightDoubleList()
    {
        const String childName = mSecondCaption->getName();
        mComponent->removeChild(childName);
        OverlayManager::getSingleton().destroyOverlayElement(childName);
    }
    void pushLine(const String &caption)
    {
        mLine.push_back(caption);
        mSecondLine.push_back("");
        if((int)mLine.size()>mVisibleLines)mSlider->show();
    }
    void pushLine(const String &caption, const String &secondCaption)
    {
        mLine.push_back(caption);
        mSecondLine.push_back(secondCaption);
        if((int)mLine.size()>mVisibleLines)mSlider->show();
    }
    void clear()
    {
        mLine.clear();
        mSecondLine.clear();
        mSlider->hide();
        mSelectedLine = -1;
    }
    void updateCaption(const unsigned short &firstLine)
    {
        if(firstLine>=(int)mLine.size())
        {
            mCaption->setCaption("");
            mSecondCaption->setCaption("");
            return;
        }
        mFirstLine = firstLine;
        String caption = "";
        unsigned short numLines = 0;
        unsigned short i = 0;
        for(std::vector<String>::iterator it=mLine.begin(); it!=mLine.end(); it++)
        {
            if(i<firstLine)
            {
                i++;
                continue;
            }
            const String line = *it;
            caption += (i!=firstLine?"\n":"") + line;
            numLines++;
            if(numLines>=mVisibleLines)break;
            i++;
        }
        mCaption->setCaption(caption);

        caption = "";
        numLines = 0;
        i = 0;
        for(std::vector<String>::iterator it=mSecondLine.begin(); it!=mSecondLine.end(); it++)
        {
            if(i<firstLine)
            {
                i++;
                continue;
            }
            const String line = *it;
            caption += (i!=firstLine?"\n":"") + line;
            numLines++;
            if(numLines>=mVisibleLines)break;
            i++;
        }
        mSecondCaption->setCaption(caption);
    }
    const bool eraseLine(const String &caption)
    {
        short i = 0;
        bool found = false;
        for(std::vector<String>::iterator it=mLine.begin(); it!=mLine.end(); it++)
        {
            const String line = *it;
            if(line==caption)
            {
                mLine.erase(it);
                if(i==mSelectedLine)mSelectedLine = -1;
                else if(i>mSelectedLine)mSelectedLine -= 1;
                found = true;
                break;
            }
            i++;
        }
        for(std::vector<String>::iterator it=mSecondLine.begin(); it!=mSecondLine.end(); it++)
        {
            const String line = *it;
            if(line==caption)
            {
                mSecondLine.erase(it);
                if(i==mSelectedLine)mSelectedLine = -1;
                else if(i>mSelectedLine)mSelectedLine -= 1;
                return true;
            }
            i++;
        }
        return found;
    }
    void changeSecondLine(const unsigned short &line, const String &caption, const bool &append=false)
    {
        if(line>=(int)mSecondLine.size())return;
        if(append)mSecondLine[line] = mSecondLine[line] + caption;
        else mSecondLine[line] = caption;
    }
    const bool isDoubleList()
    {
        return true;
    }
};

#endif

