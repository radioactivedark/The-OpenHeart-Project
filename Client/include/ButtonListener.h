#ifndef _BUTTONLISTENER_H_
#define _BUTTONLISTENER_H_

#include <GuiComponent.h>

class ButtonListener
{
public:
    ButtonListener(){}
    virtual ~ButtonListener(){}
    virtual bool buttonClicked(GuiComponent *button) = 0;
    virtual bool buttonHeld(GuiComponent *button) = 0;      //Only applies to holdable buttons
    virtual bool buttonReleased(GuiComponent *button) = 0;  //Only applies to holdable buttons
};

#endif
