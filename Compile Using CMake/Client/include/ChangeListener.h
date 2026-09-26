#ifndef _CHANGELISTENER_H_
#define _CHANGELISTENER_H_

#include <GuiComponent.h>

class ChangeListener
{
public:
    enum
    {
        EVENT_MOUSEMOVED,
        EVENT_MOUSEPRESSED,
        EVENT_KEYPRESSED,
        EVENT_AUTO
    };
    ChangeListener(){}
    virtual ~ChangeListener(){}
    virtual bool changeEvent(GuiComponent *component,const unsigned char &event) = 0;
    virtual bool comboBoxEvent(const String &name) = 0;
    virtual bool alertBoxEvent(const String &name, const bool &flag) = 0;
};

#endif

