#ifndef _KEYBOARDMANAGER_H_
#define _KEYBOARDMANAGER_H_

#define NUM_CONTROLS 45
#define KB_MAPMAKER_MOVE KB_FRIENDS
#define KB_MAPMAKER_CLONE KB_CHAT

#include <SaveFile.h>
#include <GuiList.h>

namespace KeyboardIndex
{
    enum
    {
        KB_OPTIONS,
        KB_FRONT,
        KB_BACK,
        KB_LEFT,
        KB_RIGHT,
        KB_UP,
        KB_DOWN,
        KB_JUMP,
        KB_SIT,
        KB_STAND,
        KB_WALK,
        KB_CROUCH,
        KB_AUTORUN,
        KB_EQUIP,
        KB_HEADTRACK,
        KB_RUNMODE,
        KB_CAM_FIRSTPERSON,
        KB_CAM_FIXED,
        KB_CAM_FREE,
        KB_CAM_CHASE,
        KB_CAM_UP,
        KB_CAM_DOWN,
        KB_CAM_LEFT,
        KB_CAM_RIGHT,
        KB_CAM_FASTER,
        KB_CAM_SLOWER,
        KB_PRINTSCREEN,
        KB_TOGGLEUI,
        KB_EMOTE0,
        KB_EMOTE1,
        KB_EMOTE2,
        KB_EMOTE3,
        KB_EMOTE4,
        KB_EMOTE5,
        KB_EMOTE6,
        KB_EMOTE7,
        KB_EMOTE8,
        KB_EMOTE9,
        KB_CHAT,
        KB_MINIMAP,
        KB_INFO,
        KB_ACTIONS,
        KB_FRIENDS,
        KB_PARTY,
        KB_GROUPS,
        KB_MOVIEMAKER,
        KB_RETURN   //cannot be changed
    };
}

class KeyboardListener
{
public:
    virtual const bool keyPressed(const char &index) = 0;
    virtual const bool keyReleased(const char &index) = 0;
    virtual const bool keyPressed(const OIS::KeyEvent &arg) = 0;
    virtual const bool keyReleased(const OIS::KeyEvent &arg) = 0;
};

class KeyboardManager : public Singleton<KeyboardManager>
{
private:
    bool mIsSettingKey;
    unsigned short mCurrentIndex;
    std::map<int,unsigned char> mKeyMap;
    std::map<int,unsigned char> mKeyMap2;
    GuiHighlightDoubleList *mList;
public:
    KeyboardManager()
    {
        mIsSettingKey = false;
        mCurrentIndex = 0;
        mList = 0;

        load();
    }
    ~KeyboardManager()
    {
    }
    static KeyboardManager* getSingletonPtr();
    static KeyboardManager& getSingleton();
    void setList(GuiHighlightDoubleList *list)
    {
        mList = list;
        updateList();
    }
    void setDefaults()
    {
        mKeyMap.clear();
        using namespace OIS;
        unsigned char i = 0;
        mKeyMap[(int)KC_ESCAPE] = i++;
        mKeyMap[(int)KC_W] = i++;
        mKeyMap[(int)KC_S] = i++;
        mKeyMap[(int)KC_A] = i++;
        mKeyMap[(int)KC_D] = i++;
        mKeyMap[(int)KC_R] = i++;
        mKeyMap[(int)KC_F] = i++;
        mKeyMap[(int)KC_SPACE] = i++;
        mKeyMap[(int)KC_Z] = i++;
        mKeyMap[(int)KC_X] = i++;
        mKeyMap[(int)KC_LSHIFT] = i++;
        mKeyMap[(int)KC_LCONTROL] = i++;
        mKeyMap[(int)KC_Q] = i++;
        mKeyMap[(int)KC_E] = i++;
        mKeyMap[(int)KC_LMENU] = i++;
        mKeyMap[(int)KC_RCONTROL] = i++;
        mKeyMap[(int)KC_F1] = i++;
        mKeyMap[(int)KC_F2] = i++;
        mKeyMap[(int)KC_F3] = i++;
        mKeyMap[(int)KC_F4] = i++;
        mKeyMap[(int)KC_UP] = i++;
        mKeyMap[(int)KC_DOWN] = i++;
        mKeyMap[(int)KC_LEFT] = i++;
        mKeyMap[(int)KC_RIGHT] = i++;
        mKeyMap[(int)KC_EQUALS] = i++;
        mKeyMap[(int)KC_MINUS] = i++;
        mKeyMap[(int)KC_SYSRQ] = i++;
        mKeyMap[(int)KC_TAB] = i++;
        mKeyMap[(int)KC_0] = i++;
        mKeyMap[(int)KC_1] = i++;
        mKeyMap[(int)KC_2] = i++;
        mKeyMap[(int)KC_3] = i++;
        mKeyMap[(int)KC_4] = i++;
        mKeyMap[(int)KC_5] = i++;
        mKeyMap[(int)KC_6] = i++;
        mKeyMap[(int)KC_7] = i++;
        mKeyMap[(int)KC_8] = i++;
        mKeyMap[(int)KC_9] = i++;

        mKeyMap[(int)KC_C] = i++;
        mKeyMap[(int)KC_V] = i++;
        mKeyMap[(int)KC_B] = i++;
        mKeyMap[(int)KC_N] = i++;
        mKeyMap[(int)KC_M] = i++;
        mKeyMap[(int)KC_COMMA] = i++;
        mKeyMap[(int)KC_PERIOD] = i++;
        mKeyMap[(int)KC_SLASH] = i++;
    }
    void load()
    {
        setDefaults();

        SaveFile sf;
        if(!sf.load("keyboard.cfg"))
        {
            save();
            return;
        }
        bool resave = false;
        for(int i=0;i<NUM_CONTROLS;i++)
        {
            String buffer = "";
            if(sf.getSetting(getControlName(i),buffer))mKeyMap[StringConverter::parseInt(buffer)] = i;
            else
            {
                resave = true;
                break;
            }
        }
        if(resave)
        {
            setDefaults();
            save();
        }
    }
    void save()
    {
        SaveFile sf("keyboard.cfg");
        sf.clear();
        for(int i=0;i<NUM_CONTROLS;i++)
        {
            const OIS::KeyCode key = getKey(i);
            if(key != OIS::KC_UNASSIGNED)sf.setSetting(getControlName(i),StringConverter::toString((int)key));
        }
        sf.save();
    }
    const OIS::KeyCode getKey(const unsigned char &index)
    {
        for(std::map<int,unsigned char>::iterator it=mKeyMap.begin(); it!=mKeyMap.end(); it++)
        {
            if(it->second==index)return (OIS::KeyCode)it->first;
        }
        return OIS::KC_UNASSIGNED;
    }
    const char getIndex(const OIS::KeyCode &key)
    {
        std::map<int,unsigned char>::iterator it = mKeyMap.find(key);
        if(it != mKeyMap.end())return it->second;
        return -1;
    }
    void setCurrentKey(const OIS::KeyCode &key)
    {
        //Swap existing key
        char prevIndex = -1;
        std::map<int,unsigned char>::iterator prevKey = mKeyMap.find(key);
        if(prevKey != mKeyMap.end())prevIndex = prevKey->second;
        for(std::map<int,unsigned char>::iterator it=mKeyMap.begin(); it!=mKeyMap.end(); it++)
        {
            if(it->second==mCurrentIndex)
            {
                if(prevIndex==-1)mKeyMap.erase(it);
                else
                {
                    it->second = prevIndex;
                    if(mList)mList->changeSecondLine(prevIndex,getKeyName((OIS::KeyCode)it->first));
                }
                break;
            }
        }
        mKeyMap[key] = mCurrentIndex;
        if(mList)
        {
            mList->changeSecondLine(mCurrentIndex,getKeyName(key));
            mList->update();
        }
        save();
    }
    const unsigned char getCurrentKey()
    {
        return mCurrentIndex;
    }
    const String getControlName(const unsigned char &index)
    {
        if(index>=NUM_CONTROLS)return "";
        const String control[] = {
                    "Options",
                    "Front",
                    "Back",
                    "Left",
                    "Right",
                    "Rise",
                    "Fall",
                    "Jump",
                    "Sit",
                    "Stand",
                    "Walk",
                    "Crouch",
                    "Auto Run",
                    "Use Equip",
                    "Head Track",
                    "Run Mode",
                    "Cam FirstPerson Mode",
                    "Cam Fixed Mode",
                    "Cam Free Mode",
                    "Cam Chase Mode",
                    "Cam Up",
                    "Cam Down",
                    "Cam Left",
                    "Cam Right",
                    "Cam Faster",
                    "Cam Slower",
                    "Print Screen",
                    "Show/Hide GUI",
                    "Emote 0",
                    "Emote 1",
                    "Emote 2",
                    "Emote 3",
                    "Emote 4",
                    "Emote 5",
                    "Emote 6",
                    "Emote 7",
                    "Emote 8",
                    "Emote 9",
                    "Show Chat",
                    "Toggle MiniMap",
                    "Show Info",
                    "Show Actions",
                    "Show Friends",
                    "Show Party",
                    "Show Group",
                    "Show MovieMaker"
                    };
        return control[index];
    }
    void doSetKey(const bool &flag, const unsigned char &index=0)
    {
        mIsSettingKey = flag;
        if(flag && index<NUM_CONTROLS)mCurrentIndex = index;
    }
    const bool isSettingKey()
    {
        return mIsSettingKey;
    }
    void updateList()
    {
        if(!mList)return;
        mList->clear();

        std::map<unsigned char,int> keyMap;
        for(std::map<int,unsigned char>::iterator it=mKeyMap.begin(); it!=mKeyMap.end(); it++)
        {
            keyMap[it->second] = it->first;
        }
        for(std::map<unsigned char,int>::iterator it=keyMap.begin(); it!=keyMap.end(); it++)
        {
            mList->pushLine(getControlName(it->first),getKeyName((OIS::KeyCode)it->second));
        }

        mList->update();
    }
    const String getKeyName(const OIS::KeyCode &key)
    {
        using namespace OIS;
        switch(key)
        {
            default: break;
            case KC_ESCAPE: return "Esc";
            case KC_1: return "1";
            case KC_2: return "2";
            case KC_3: return "3";
            case KC_4: return "4";
            case KC_5: return "5";
            case KC_6: return "6";
            case KC_7: return "7";
            case KC_8: return "8";
            case KC_9: return "9";
            case KC_0: return "0";
            case KC_MINUS: return "-";
            case KC_EQUALS: return "=";
            case KC_BACK: return "Backspace";
            case KC_TAB: return "Tab";
            case KC_Q: return "Q";
            case KC_W: return "W";
            case KC_E: return "E";
            case KC_R: return "R";
            case KC_T: return "T";
            case KC_Y: return "Y";
            case KC_U: return "U";
            case KC_I: return "I";
            case KC_O: return "O";
            case KC_P: return "P";
            case KC_LBRACKET: return "[";
            case KC_RBRACKET: return "]";
            case KC_RETURN: return "Return";
            case KC_LCONTROL: return "Left Ctrl";
            case KC_A: return "A";
            case KC_S: return "S";
            case KC_D: return "D";
            case KC_F: return "F";
            case KC_G: return "G";
            case KC_H: return "H";
            case KC_J: return "J";
            case KC_K: return "K";
            case KC_L: return "L";
            case KC_SEMICOLON: return ";";
            case KC_APOSTROPHE: return "'";
            case KC_GRAVE: return "`";
            case KC_LSHIFT: return "Left Shift";
            case KC_BACKSLASH: return "\\";
            case KC_Z: return "Z";
            case KC_X: return "X";
            case KC_C: return "C";
            case KC_V: return "V";
            case KC_B: return "B";
            case KC_N: return "N";
            case KC_M: return "M";
            case KC_COMMA: return ",";
            case KC_PERIOD: return ".";
            case KC_SLASH: return "/";
            case KC_RSHIFT: return "Right Shift";
            case KC_MULTIPLY: return "Numpad *";
            case KC_LMENU: return "Left Alt";
            case KC_SPACE: return "Spacebar";
            case KC_CAPITAL: return "Caps Lock";
            case KC_F1: return "F1";
            case KC_F2: return "F2";
            case KC_F3: return "F3";
            case KC_F4: return "F4";
            case KC_F5: return "F5";
            case KC_F6: return "F6";
            case KC_F7: return "F7";
            case KC_F8: return "F8";
            case KC_F9: return "F9";
            case KC_F10: return "F10";
            case KC_NUMLOCK: return "Num Lock";
            case KC_SCROLL: return "Scroll Lock";
            case KC_NUMPAD7: return "Numpad 7";
            case KC_NUMPAD8: return "Numpad 8";
            case KC_NUMPAD9: return "Numpad 9";
            case KC_SUBTRACT: return "Numpad -";
            case KC_NUMPAD4: return "Numpad 4";
            case KC_NUMPAD5: return "Numpad 5";
            case KC_NUMPAD6: return "Numpad 6";
            case KC_ADD: return "Numpad +";
            case KC_NUMPAD1: return "Numpad 1";
            case KC_NUMPAD2: return "Numpad 2";
            case KC_NUMPAD3: return "Numpad 3";
            case KC_NUMPAD0: return "Numpad 0";
            case KC_DECIMAL: return "Numpad .";
            case KC_OEM_102: return "OEM 102";
            case KC_F11: return "F11";
            case KC_F12: return "F12";
            case KC_F13: return "F13";
            case KC_F14: return "F14";
            case KC_F15: return "F15";
            case KC_KANA: return "Kana";
            case KC_ABNT_C1: return "ABNT C1";
            case KC_CONVERT: return "Convert";
            case KC_NOCONVERT: return "No Convert";
            case KC_YEN: return "Yen";
            case KC_ABNT_C2: return "ABNT C2";
            case KC_NUMPADEQUALS: return "Numpad =";
            case KC_PREVTRACK: return "Prev Track";
            case KC_AT: return "At";
            case KC_COLON: return "Colon";
            case KC_UNDERLINE: return "Underline";
            case KC_KANJI: return "Kanji";
            case KC_STOP: return "Stop";
            case KC_AX: return "Ax";
            case KC_UNLABELED: return "Unlabeled";
            case KC_NEXTTRACK: return "Next Track";
            case KC_NUMPADENTER: return "Numpad Enter";
            case KC_RCONTROL: return "Right Ctrl";
            case KC_MUTE: return "Mute";
            case KC_CALCULATOR: return "Calculator";
            case KC_PLAYPAUSE: return "Play/Pause";
            case KC_MEDIASTOP: return "Media Stop";
            case KC_VOLUMEDOWN: return "Volume Down";
            case KC_VOLUMEUP: return "Volume Up";
            case KC_WEBHOME: return "Web Home";
            case KC_NUMPADCOMMA: return "Numpad ,";
            case KC_DIVIDE: return "Divide";
            case KC_SYSRQ: return "Prt Sc";
            case KC_RMENU: return "Right Alt";
            case KC_PAUSE: return "Pause";
            case KC_HOME: return "Home";
            case KC_UP: return "Arrow Up";
            case KC_PGUP: return "Page Up";
            case KC_LEFT: return "Arrow Left";
            case KC_RIGHT: return "Arrow Right";
            case KC_END: return "End";
            case KC_DOWN: return "Arrow Down";
            case KC_PGDOWN: return "Page Down";
            case KC_INSERT: return "Insert";
            case KC_DELETE: return "Delete";
            case KC_LWIN: return "Left Windows";
            case KC_RWIN: return "Right Windows";
        }
        return "Undefined:"+StringConverter::toString((int)key);
    }
};

template<> KeyboardManager* Singleton<KeyboardManager>::ms_Singleton = 0;

KeyboardManager* KeyboardManager::getSingletonPtr()
{
	return ms_Singleton;
}

KeyboardManager& KeyboardManager::getSingleton()
{
	assert(ms_Singleton);
	return *ms_Singleton;
}

#endif
