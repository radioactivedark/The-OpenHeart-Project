#ifndef _GAMEPANELLISTENER_H_
#define _GAMEPANELLISTENER_H_

class GamePanelListener
{
public:
    GamePanelListener(){}
    virtual ~GamePanelListener(){}
    enum
    {
        GAMEPANEL_FRIENDS,
        GAMEPANEL_PARTY,
        GAMEPANEL_GROUP,
        GAMEPANEL_CHAT
    };
    virtual void highlightButton(const unsigned char &panel, const bool &flag, const bool &incrementReminder=false) = 0;
};

#endif
