#ifndef _INFOCALLER_H_
#define _INFOCALLER_H_

class InfoCaller
{
public:
    InfoCaller(){}
    virtual ~InfoCaller(){}
    virtual void viewInfo(const unsigned int &userID, const unsigned int &charID, const String &username="", const String &charname="") = 0;
    virtual void setOwnGroup(const String &groupname) = 0;
    virtual const unsigned int getOwnCharID() = 0;
};

#endif
