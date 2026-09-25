#ifndef _NETWORKMANAGER_H_
#define _NETWORKMANAGER_H_

#define DEBUG_LOUD false

#include "DataTypes.h"
#include "NetworkDefines.h"
#include "PacketReader.h"
#include "RakNetStatistics.h"
#include "RakSleep.h"
#include <iostream>

using namespace std;
using namespace RakNet;

class NetworkListener
{
public:
    NetworkListener(){}
    virtual ~NetworkListener(){}
    //virtual void terminateEvent(const bool &isProper) = 0;
    //virtual void tokenReceived(const string &token, const unsigned int &userID, const unsigned int &charID) = 0;
    virtual void connectEvent(const SystemAddress &add, const string &token, const unsigned int &charID, const string &ghostMap) = 0;
    virtual void disconnectEvent(const SystemAddress &add, const bool &isConnectionLost) = 0;
    virtual void serverConnectEvent(const SystemAddress &add) = 0;
    virtual void mapConnectEvent(const SystemAddress &add, const string &token, const bool &isFirstEntrance) = 0;
    virtual void mapDisconnectEvent(const string &token, const bool &isMapChange) = 0;
    virtual void chatRelayEvent(const SystemAddress &sendingMap, const unsigned int &userID, const unsigned int &charID, const string &caption, const unsigned char &channel) = 0;
    virtual void friendRelayEvent(const SystemAddress &sendingMap, const bool &isRemove, const bool &isBlockedBy, const unsigned int &senderID, const unsigned int &targetID, const bool &isRequest) = 0;
    virtual void myGroupRelayEvent(const SystemAddress &sendingMap, const unsigned int &groupID, const string &groupname) = 0;
    virtual void myGroupLeaveRelayEvent(const SystemAddress &sendingMap, const unsigned int &groupID, const unsigned int &charID, const bool &isKick, const bool &isDisband) = 0;
    virtual void groupRequestRelayEvent(const SystemAddress &sendingMap, const bool &isAccept, const unsigned int &senderID, const unsigned int &targetID, const bool &isRequest) = 0;
    virtual void partyCreateRelayEvent(const SystemAddress &sendingMap, const unsigned int &senderID, const unsigned int &targetID) = 0;
    virtual void partyLeaveRelayEvent(const SystemAddress &sendingMap, const unsigned int &senderID, const unsigned int &partyID) = 0;
    virtual void partyRequestRelayEvent(const SystemAddress &sendingMap, const bool &isAccept, const unsigned int &senderID, const unsigned int &targetID) = 0;
};

class NetworkManager
{
private:
    NetworkListener *mListener;
    RakPeerInterface *mServer;
    PacketReader mReader;
    PacketWriter mWriter;
    //SystemAddress mFrontServerAdd;
    //bool mAttemptReconnect;
    //bool mIsReconnecting;
public:
    NetworkManager(NetworkListener *listener)
    {
        mListener = listener;
        mServer = RakPeerInterface::GetInstance();
		mServer->SetIncomingPassword(SERVER_PASSWORD, (int)strlen(SERVER_PASSWORD));
		mWriter.setPeer(mServer);
		//mFrontServerAdd = UNASSIGNED_SYSTEM_ADDRESS;
		//mAttemptReconnect = false;
		//mIsReconnecting = true;
    }
    ~NetworkManager()
    {
        RakPeerInterface::DestroyInstance(mServer);
    }
    const bool init()
    {
        puts("Starting MainServer");
        SocketDescriptor socketDesc(MAIN_SERVER_PORT,0);
        const int flag = mServer->Startup(MAX_CLIENTS, &socketDesc, 1);
        mServer->SetMaximumIncomingConnections(MAX_CLIENTS);
        if(flag==0)
        {
            puts("MainServer started, waiting for connections.");
            //mServer->Connect(FRONT_SERVER_IP, FRONT_SERVER_PORT, SERVER_PASSWORD, (int)strlen(SERVER_PASSWORD));
            //mServer->Connect("127.0.0.1", FRONT_SERVER_PORT2, SERVER_PASSWORD, (int)strlen(SERVER_PASSWORD));
        }
        else
        {
            puts("MainServer failed to start.  Terminating.");
            return false;
        }
        mServer->SetTimeoutTime(60000,UNASSIGNED_SYSTEM_ADDRESS);
        mServer->SetUnreliableTimeout(10000);
        mServer->SetOccasionalPing(true);
        printf("Max clients allowed: %i\n", MAX_CLIENTS);
        return true;
    }
    void printStats()
    {
        char temp[2048]="";
        RakNetStatistics *rss = mServer->GetStatistics(mServer->GetSystemAddressFromIndex(0));
        StatisticsToString(rss, temp, 2);
        printf("%s", temp);
        printf("Ping %i\n", mServer->GetAveragePing(mServer->GetSystemAddressFromIndex(0)));
    }
    void shutdown()
    {
        mServer->Shutdown(300);
    }
    void update()
    {
        /*if(mAttemptReconnect)
        {
            mAttemptReconnect = false;
            mIsReconnecting = true;
            shutdown();
            RakSleep(3000);
            init();
        }*/
        RakSleep(30);
        for(Packet *p=mServer->Receive(); p; mServer->DeallocatePacket(p), p=mServer->Receive())
        {
            switch(p->data[0])
            {
                /*//Front server replies
                case ID_CONNECTION_REQUEST_ACCEPTED:
                    if(DEBUG_LOUD)printf("ID_CONNECTION_REQUEST_ACCEPTED from %s\n", p->systemAddress.ToString());
                    mIsReconnecting = false;
                    mFrontServerAdd = p->systemAddress;
                    mWriter.create(ID_SERVER_CONNECT_MAIN);
                    mWriter.send(mFrontServerAdd);
                    break;
                case ID_CONNECTION_ATTEMPT_FAILED:
                case ID_ALREADY_CONNECTED:
                case ID_CONNECTION_BANNED:
                case ID_NO_FREE_INCOMING_CONNECTIONS:
                case ID_INVALID_PASSWORD:
                case ID_INCOMPATIBLE_PROTOCOL_VERSION:
                    if(DEBUG_LOUD)printf("ID_CONNECTION_ATTEMPT_FAILED from %s\n", p->systemAddress.ToString());
                    if(mIsReconnecting)mAttemptReconnect = true;
                    else mListener->terminateEvent(true);
                    break;*/

                case ID_NEW_INCOMING_CONNECTION:
                    if(DEBUG_LOUD)printf("ID_NEW_INCOMING_CONNECTION from %s\n", p->systemAddress.ToString());
					break;

				case ID_DISCONNECTION_NOTIFICATION:
                    if(DEBUG_LOUD)printf("ID_DISCONNECTION_NOTIFICATION from %s\n", p->systemAddress.ToString());
                    /*if(p->systemAddress==mFrontServerAdd)mListener->terminateEvent(true);
                    else */mListener->disconnectEvent(p->systemAddress,false);
					break;

				case ID_CONNECTION_LOST:
                    if(DEBUG_LOUD)printf("ID_CONNECTION_LOST from %s\n", p->systemAddress.ToString());
                    /*if(p->systemAddress==mFrontServerAdd)
                    {
                        mListener->terminateEvent(false);
                        mAttemptReconnect = true;
                    }
                    else */mListener->disconnectEvent(p->systemAddress,true);
					break;

                /*case ID_TOKEN:
                    if(DEBUG_LOUD)printf("ID_TOKEN from %s\n", p->systemAddress.ToString());
                    if(p->systemAddress==mFrontServerAdd)readToken(p);
					break;*/

                case ID_CONNECT_MAIN:
                    if(DEBUG_LOUD)printf("ID_CONNECT_MAIN from %s\n", p->systemAddress.ToString());
                    readConnect(p);
					break;

                case ID_SERVER_CONNECT_MAIN:
                    if(DEBUG_LOUD)printf("ID_SERVER_CONNECT_MAIN from %s\n", p->systemAddress.ToString());
                    mListener->serverConnectEvent(p->systemAddress);
					break;

                case ID_CHAR_INFO:
                    if(DEBUG_LOUD)printf("ID_CHAR_INFO from %s\n", p->systemAddress.ToString());
                    readMapConnect(p);
					break;

                case ID_CHAR_EXIT:
                    if(DEBUG_LOUD)printf("ID_CHAR_EXIT from %s\n", p->systemAddress.ToString());
                    readMapDisconnect(p);
					break;

                case ID_CHAT:
                    if(DEBUG_LOUD)printf("ID_CHAT from %s\n", p->systemAddress.ToString());
                    readChatRelay(p);
					break;

                case ID_FRIEND:
                    if(DEBUG_LOUD)printf("ID_FRIEND from %s\n", p->systemAddress.ToString());
                    readFriendRelay(p);
					break;

                case ID_MYGROUP:
                    if(DEBUG_LOUD)printf("ID_MYGROUP from %s\n", p->systemAddress.ToString());
                    readMyGroupRelay(p);
					break;

                case ID_GROUPREQUEST:
                    if(DEBUG_LOUD)printf("ID_GROUPREQUEST from %s\n", p->systemAddress.ToString());
                    readGroupRequestRelay(p);
					break;

                case ID_PARTY:
                    if(DEBUG_LOUD)printf("ID_PARTY from %s\n", p->systemAddress.ToString());
                    readPartyRelay(p);
					break;

                case ID_PARTYREQUEST:
                    if(DEBUG_LOUD)printf("ID_PARTYREQUEST from %s\n", p->systemAddress.ToString());
                    readPartyRequestRelay(p);
					break;

                default: break;
            }
        }
    }
    /*void readToken(Packet *p)
    {
        mReader.load(p);
        const string token = mReader.readString(16);
        const unsigned int userID = mReader.readUInt();
        const unsigned int charID = mReader.readUInt();
        mListener->tokenReceived(token,userID,charID);
    }*/
    void sendTokenFail(const SystemAddress &add)
    {
        mWriter.create(ID_TOKEN_FAIL);
        mWriter.send(add);
    }
    void readConnect(Packet *p)
    {
        mReader.load(p);
        const string token = mReader.readString(16);
        const unsigned int charID = mReader.readUInt(); //only sent on first connect
        const bool ghostMode = mReader.readBool();
        const string ghostMap = ghostMode?mReader.readString(32):"";
        mListener->connectEvent(p->systemAddress,token,charID,ghostMap);
    }
    void sendConnect(const SystemAddress &add, const unsigned short &mapPort, const string &map, const float &dayTime, const float &weatherTime)
    {
        mWriter.create(ID_CONNECT_MAIN);
        mWriter.writeUShort(mapPort);
        mWriter.writeString(map);
        mWriter.writeFloat(dayTime);
        mWriter.writeFloat(weatherTime);

        mWriter.send(add);
    }
    void readMapConnect(Packet *p)
    {
        mReader.load(p);
        const string token = mReader.readString(16);
        const bool isFirstEntrance = mReader.readBool();
        mListener->mapConnectEvent(p->systemAddress,token,isFirstEntrance);
    }
    void readMapDisconnect(Packet *p)
    {
        mReader.load(p);
        const string token = mReader.readString(16);
        const bool isMapChange = mReader.readBool();
        mListener->mapDisconnectEvent(token,isMapChange);
    }
    void sendTokenToMap(const SystemAddress &add, const string &token, const unsigned int &userID, const unsigned int &charID)
    {
        mWriter.create(ID_TOKEN);
        mWriter.writeString(token);
        mWriter.writeUInt(userID);
        mWriter.writeUInt(charID);

        mWriter.send(add);
    }
    void readChatRelay(Packet *p)
    {
        mReader.load(p);
        const unsigned int userID = mReader.readUInt(); //userID for whisper, group/party ID for group/party
        const unsigned int charID = mReader.readUInt(); //sender charID
        const string caption = mReader.readString();
        const unsigned char channel = mReader.readUChar();
        mListener->chatRelayEvent(p->systemAddress,userID,charID,caption,channel);
    }
    void sendChatRelay(const SystemAddress &add, const unsigned int &userID, const unsigned int &charID, const string &caption, const unsigned char &channel)
    {
        mWriter.create(ID_CHAT);
        mWriter.writeUInt(userID);
        mWriter.writeUInt(charID);
        mWriter.writeString(caption);
        mWriter.writeUChar(channel);

        mWriter.send(add);
    }
    void readFriendRelay(Packet *p)
    {
        mReader.load(p);
        const bool isRemove = mReader.readBool();
        const bool isBlockedBy = mReader.readBool();
        const bool isRequest = ((isBlockedBy||isRemove) ? false : mReader.readBool());
        const unsigned int senderID = mReader.readUInt();
        const unsigned int targetID = mReader.readUInt();
        mListener->friendRelayEvent(p->systemAddress,isRemove,isBlockedBy,senderID,targetID,isRequest);
    }
    void sendFriendRelay(const SystemAddress &add, const bool &isRemove, const bool &isBlockedBy, const unsigned int &senderID, const unsigned int &targetID, const bool &isRequest)
    {
        mWriter.create(ID_FRIEND);
        mWriter.writeBool(isRemove);
        mWriter.writeBool(isBlockedBy);
        if(!isBlockedBy && !isRemove)mWriter.writeBool(isRequest);
        mWriter.writeUInt(senderID);
        mWriter.writeUInt(targetID);

        mWriter.send(add);
    }
    void sendCharOnlineStatus(const SystemAddress &add, const unsigned int &userID, const unsigned int &charID, const bool &isOnline)
    {
        mWriter.create(ID_CHAR_ONLINESTATUS);
        mWriter.writeUInt(userID);
        mWriter.writeUInt(charID);
        mWriter.writeBool(isOnline);

        mWriter.send(add);
    }
    void readMyGroupRelay(Packet *p)
    {
        mReader.load(p);
        const unsigned int groupID = mReader.readUInt();
        const bool isLeave = mReader.readBool();
        if(!isLeave)
        {
            const string groupname = mReader.readString(32);
            mListener->myGroupRelayEvent(p->systemAddress,groupID,groupname);
        }
        else
        {
            const unsigned int charID = mReader.readUInt();
            const bool isKick = mReader.readBool();
            const bool isDisband = mReader.readBool();
            mListener->myGroupLeaveRelayEvent(p->systemAddress,groupID,charID,isKick,isDisband);
        }
    }
    void sendMyGroupRelay(const SystemAddress &add, const unsigned int &groupID, const string &groupname)
    {
        mWriter.create(ID_MYGROUP);
        mWriter.writeUInt(groupID);
        mWriter.writeBool(false);
        mWriter.writeString(groupname);

        mWriter.send(add);
    }
    void sendMyGroupLeaveRelay(const SystemAddress &add, const unsigned int &groupID, const unsigned int &charID, const bool &isKicked, const bool &isDisband)
    {
        mWriter.create(ID_MYGROUP);
        mWriter.writeUInt(groupID);
        mWriter.writeBool(true);
        mWriter.writeUInt(charID);
        mWriter.writeBool(isKicked);
        if(isDisband)mWriter.writeBool(true);

        mWriter.send(add);
    }
    void readGroupRequestRelay(Packet *p)
    {
        mReader.load(p);
        const bool isAccept = mReader.readBool();
        const unsigned int senderID = mReader.readUInt();
        const unsigned int targetID = mReader.readUInt();
        const bool isRequest = mReader.readBool();
        mListener->groupRequestRelayEvent(p->systemAddress,isAccept,senderID,targetID,isRequest);
    }
    void sendGroupRequestRelay(const SystemAddress &add, const bool &isAccept, const unsigned int &senderID, const unsigned int &targetID, const bool &isRequest)
    {
        mWriter.create(ID_GROUPREQUEST);
        mWriter.writeBool(isAccept);
        mWriter.writeUInt(senderID);
        mWriter.writeUInt(targetID);
        mWriter.writeBool(isRequest);

        mWriter.send(add);
    }
    void sendDuplicateLoginEvent(const SystemAddress &add, const unsigned int &charID)
    {
        mWriter.create(ID_MISC);
        mWriter.writeUChar(0);
        mWriter.writeUInt(charID);

        mWriter.send(add);
    }
    void readPartyRelay(Packet *p)
    {
        mReader.load(p);
        const bool isCreate = mReader.readBool();
        if(isCreate)
        {
            const unsigned int senderID = mReader.readUInt();
            const unsigned int targetID = mReader.readUInt();
            mListener->partyCreateRelayEvent(p->systemAddress,senderID,targetID);
        }
        //Is leave
        else
        {
            const unsigned int senderID = mReader.readUInt();
            const unsigned int partyID = mReader.readUInt();
            mListener->partyLeaveRelayEvent(p->systemAddress,senderID,partyID);
        }
    }
    void sendPartyCreateRelay(const SystemAddress &add, const unsigned int &senderID, const unsigned int &targetID, const unsigned int &partyID)
    {
        mWriter.create(ID_PARTY);
        mWriter.writeBool(true);    //is create
        mWriter.writeUInt(senderID);
        mWriter.writeUInt(targetID);
        mWriter.writeUInt(partyID);

        mWriter.send(add);
    }
    void sendPartyLeaveRelay(const SystemAddress &add, const unsigned int &senderID, const unsigned int &partyID)
    {
        mWriter.create(ID_PARTY);
        mWriter.writeBool(false);    //is leave
        mWriter.writeUInt(senderID);
        mWriter.writeUInt(partyID);

        mWriter.send(add);
    }
    void readPartyRequestRelay(Packet *p)
    {
        mReader.load(p);
        const bool isAccept = mReader.readBool();
        const unsigned int senderID = mReader.readUInt();
        const unsigned int targetID = mReader.readUInt();
        mListener->partyRequestRelayEvent(p->systemAddress,isAccept,senderID,targetID);
    }
    void sendPartyRequestRelay(const SystemAddress &add, const bool &isAccept, const unsigned int &senderID, const unsigned int &targetID)
    {
        mWriter.create(ID_PARTYREQUEST);
        mWriter.writeBool(isAccept);
        mWriter.writeUInt(senderID);
        mWriter.writeUInt(targetID);

        mWriter.send(add);
    }
};

#endif
