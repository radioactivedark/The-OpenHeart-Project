#ifndef _CHAT_H_
#define _CHAT_H_

enum
{
    CHAT_LOCAL,
    CHAT_GENERAL,
    CHAT_GROUP,
    CHAT_PARTY,
    CHAT_WHISPER,
    CHAT_ACTION,
    CHAT_MOVIE,
    CHAT_SYSTEM,
    CHAT_MOD,
    CHAT_MODWHISPER
};

#define COMMAND_NOSUCHUSER "/nouser "
#define COMMAND_HELP "/?"
#define COMMAND_GHOST "/ghost"
#define COMMAND_KICK "/kick"
#define COMMAND_BAN "/ban"
#define COMMAND_UNBAN "/unban"
#define COMMAND_GODSPEAK "/godspeak"

#endif
