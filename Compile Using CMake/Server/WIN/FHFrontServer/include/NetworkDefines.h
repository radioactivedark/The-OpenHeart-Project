#ifndef _NETWORKDEFINES_H_
#define _NETWORKDEFINES_H_

// Server password, much like the crypt key, can be whatever you want. However, the server password doesn't have a char length req.
// Given that everything here needs to be the same across all servers and your client, I'd recommend copy & pasting this file
// to all other versions of this file when making changes.
#define SERVER_PASSWORD "CHANGE_ME_TO_YOUR_OWN_PASSWORD"
#define VERSION_TOKEN "INSERT_YOUR_VER_TOKEN_HERE"
#define FRONT_SERVER_IP "INSERT_YOUR_SERVER_IP_HERE"
#define FRONT_SERVER_PORT 30000
#define FRONT_SERVER_PORT2 30001
#define MAIN_SERVER_IP "INSERT_YOUR_SERVER_IP_HERE"
#define MAIN_SERVER_PORT 30002
#define MAP_SERVER_IP "INSERT_YOUR_SERVER_IP_HERE"
#define MAP_SERVER_PORT 30003
#define MAX_CLIENTS 255
#define MAX_CHARACTERS 100
#define TOKEN_TIMER 15

enum
{
    ID_TOKEN = 113,
    ID_LOGIN = 114,
    ID_CHARSELECT = 115,
    ID_CHARCREATE = 116,
    ID_CHARDELETE = 117,
    ID_CONNECT_MAIN = 118,
    ID_SERVER_CONNECT_MAIN = 119,
    ID_CONNECT_MAP = 120,
    ID_TOKEN_FAIL = 121,
    ID_CHAR_INFO = 122,
    ID_CHAR_EXIT = 123,
    ID_CHAR_MAPCHANGE = 124,
    ID_CHAR_POSITION = 125,
    ID_CHAT = 126,
    ID_CHAR_YAW = 127,
    ID_CHAR_HEADDIR = 128,
    ID_CHAR_ACTION = 129,
    ID_CHAR_MOVEMODIFIER = 130,
    ID_CHAR_EMOTE = 131,
    ID_CHAR_JUMP = 132,
    ID_CHAR_STATUSFLAGS = 133,
    ID_MOVIE = 134,
    ID_FRIENDADD = 135,
    ID_FRIEND = 136,
    ID_CHAR_ONLINESTATUS = 137,
    ID_FINDCHAR = 138,
    ID_CHAR_BIO = 139,
    ID_MOD = 140,
    ID_MYGROUP = 141,
    ID_VIEWGROUP = 142,
    ID_GROUPREQUEST = 143,
    ID_GROUPBIO = 144,
    ID_MISC = 145,
    ID_HOME = 146,
    ID_PARTY = 147,
    ID_PARTYREQUEST = 148,
    ID_CHAR_SKILL = 149
};

#endif
