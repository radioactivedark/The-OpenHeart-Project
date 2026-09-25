#ifndef _NETWORKMANAGER_H_
#define _NETWORKMANAGER_H_

#define DEBUG_LOUD false

#include "DataTypes.h"
#include "NetworkDefines.h"
#include "PacketReader.h"
#include "RakNetStatistics.h"
#include "RakSleep.h"
#include <iostream>
#include <stdio.h>

using namespace std;
using namespace RakNet;

class NetworkListener
{
public:
    NetworkListener(){}
    virtual ~NetworkListener(){}
    virtual void disconnectEvent(const SystemAddress &add) = 0;
    virtual void saltRequest(const SystemAddress &add, const string &user, const string &macAdd) = 0;
    virtual void loginRequest(const SystemAddress &add, const string &hash) = 0;
    virtual void charSelect(const SystemAddress &add, const char &selection) = 0;
    virtual void charCreate(const SystemAddress &add, CharacterData *data) = 0;
    virtual void charUpdate(const SystemAddress &add, CharacterData *data) = 0;
    virtual void charDelete(const SystemAddress &add, const unsigned int &id) = 0;
    virtual void connectMainEvent(const SystemAddress &add, const unsigned int &charID) = 0;
};

class NetworkManager
{
private:
    NetworkListener *mListener;
    RakPeerInterface *mServer;
    PacketReader mReader;
    PacketWriter mWriter;
    //SystemAddress mMainServerAdd;
public:
    NetworkManager(NetworkListener *listener)
    {
        mListener = listener;
        mServer = RakPeerInterface::GetInstance();
		mServer->SetIncomingPassword(SERVER_PASSWORD, (int)strlen(SERVER_PASSWORD));
		mWriter.setPeer(mServer);
		//mMainServerAdd = UNASSIGNED_SYSTEM_ADDRESS;
    }
    ~NetworkManager()
    {
        RakPeerInterface::DestroyInstance(mServer);
    }
    const bool init()
    {
        puts("Starting FrontServer");
        SocketDescriptor socketDesc(FRONT_SERVER_PORT,0);//[2];
        //socketDesc[0] = SocketDescriptor(FRONT_SERVER_PORT,0);
        //socketDesc[1] = SocketDescriptor(FRONT_SERVER_PORT2,0);

        const int flag = mServer->Startup(MAX_CLIENTS, &socketDesc, 1);
        mServer->SetMaximumIncomingConnections(MAX_CLIENTS);
        if(flag==0)puts("FrontServer started, waiting for connections.");
        else
        {
            puts("FrontServer failed to start.  Terminating.");
            return false;
        }
        mServer->SetTimeoutTime(60000,UNASSIGNED_SYSTEM_ADDRESS);
        mServer->SetUnreliableTimeout(10000);
        //mServer->SetOccasionalPing(true);
        printf("Max clients allowed: %i\n", MAX_CLIENTS);
        return true;
    }
    void shutdown()
    {
        mServer->Shutdown(300);
    }
    void printStats()
    {
        char temp[2048]="";
        RakNetStatistics *rss = mServer->GetStatistics(mServer->GetSystemAddressFromIndex(0));
        StatisticsToString(rss, temp, 2);
        printf("%s", temp);
        printf("Ping %i\n", mServer->GetAveragePing(mServer->GetSystemAddressFromIndex(0)));
    }
    void update()
    {
        RakSleep(30);
        for(Packet *p=mServer->Receive(); p; mServer->DeallocatePacket(p), p=mServer->Receive())
        {
            switch(p->data[0])
            {
                case ID_NEW_INCOMING_CONNECTION:
                    if(DEBUG_LOUD)printf("ID_NEW_INCOMING_CONNECTION from %s\n", p->systemAddress.ToString());
					break;

				case ID_DISCONNECTION_NOTIFICATION:
                    if(DEBUG_LOUD)printf("ID_DISCONNECTION_NOTIFICATION from %s\n", p->systemAddress.ToString());
                    mListener->disconnectEvent(p->systemAddress);
					break;

				case ID_CONNECTION_LOST:
                    if(DEBUG_LOUD)printf("ID_CONNECTION_LOST from %s\n", p->systemAddress.ToString());
                    mListener->disconnectEvent(p->systemAddress);
					break;

                case ID_LOGIN:
                    if(DEBUG_LOUD)printf("ID_LOGIN from %s\n", p->systemAddress.ToString());
                    readLogin(p);
					break;

                case ID_CHARSELECT:
                    if(DEBUG_LOUD)printf("ID_CHARSELECT from %s\n", p->systemAddress.ToString());
                    readCharSelect(p);
					break;

                case ID_CHARCREATE:
                    if(DEBUG_LOUD)printf("ID_CHARCREATE from %s\n", p->systemAddress.ToString());
                    readCharCreate(p);
					break;

                case ID_CHARDELETE:
                    if(DEBUG_LOUD)printf("ID_CHARDELETE from %s\n", p->systemAddress.ToString());
                    readCharDelete(p);
					break;

                case ID_CONNECT_MAIN:
                    if(DEBUG_LOUD)printf("ID_CONNECT_MAIN from %s\n", p->systemAddress.ToString());
                    readConnectMain(p);
					break;

                /*case ID_SERVER_CONNECT_MAIN:
                    if(DEBUG_LOUD)printf("ID_SERVER_CONNECT_MAIN from %s\n", p->systemAddress.ToString());
                    mMainServerAdd = p->systemAddress;
					break;*/

                default: break;
            }
        }
    }
    void readLogin(Packet *p)
    {
        mReader.load(p);
        if(!mReader.readBool())
        {
            const string version = mReader.readString(8);
            if(version!=VERSION_TOKEN)
            {
                mWriter.create(ID_LOGIN);
                mWriter.writeBool(false);
                mWriter.writeBool(true);
                mWriter.send(p->systemAddress);
                return;
            }
            const string user = mReader.readString(16);
            const string macAdd = mReader.readString(18);
            mListener->saltRequest(p->systemAddress,user,macAdd);
        }
        else mListener->loginRequest(p->systemAddress,mReader.readString(32));
    }
    void sendSalt(const SystemAddress &add, const string &salt)
    {
        mWriter.create(ID_LOGIN);
        mWriter.writeBool(false);
        mWriter.writeBool(false);
        mWriter.writeString(salt);
        mWriter.send(add);
    }
    void sendLogin(const SystemAddress &add, const bool &flag, const bool &banned=false, const unsigned int &banMins=0)
    {
        mWriter.create(ID_LOGIN);
        mWriter.writeBool(true);
        mWriter.writeBool(flag);
        if(banned)
        {
            mWriter.writeBool(true);
            mWriter.writeUInt(banMins);
        }
        mWriter.send(add);
    }
    void readCharSelect(Packet *p)
    {
        mReader.load(p);
        mListener->charSelect(p->systemAddress,mReader.readChar());
    }
    void sendCharSelect(const SystemAddress &add, const char &selection, CharacterData *data=0, const unsigned char &numChars=0)
    {
        mWriter.create(ID_CHARSELECT);
        mWriter.writeChar(selection);
        if(data)
        {
            mWriter.writeBool(true);

            mWriter.writeString(data->mName);
            mWriter.writeChar(data->mSpecies);
            for(int i=0;i<5;i++)mWriter.writeUChar(data->mMesh[i]);
            for(int i=0;i<4;i++)mWriter.writeUChar(data->mMat[i]);
            for(int i=0;i<10;i++)mWriter.writeChar(data->mScale[i]);
            for(int i=0;i<9;i++)
                for(int j=0;j<3;j++)mWriter.writeUChar(data->mColour[i][j]);
            mWriter.writeUChar(data->mEmote);
            mWriter.writeUChar(data->mPreset);

            mWriter.writeUChar(numChars);
            mWriter.writeUInt(data->mID);
        }
        else mWriter.writeBool(false);
        mWriter.send(add);
    }
    void readCharCreate(Packet *p)
    {
        mReader.load(p);
        CharacterData data;
        data.mName = mReader.readString(16);
        data.mSpecies = mReader.readChar();
        for(int i=0;i<5;i++)data.mMesh[i] = mReader.readUChar();
        for(int i=0;i<4;i++)data.mMat[i] = mReader.readUChar();
        for(int i=0;i<10;i++)data.mScale[i] = mReader.readChar();
        for(int i=0;i<9;i++)
            for(int j=0;j<3;j++)data.mColour[i][j] = mReader.readUChar();
        data.mEmote = mReader.readUChar();
        data.mPreset = mReader.readUChar();

        if(mReader.readBool())
        {
            data.mID = mReader.readUInt();
            mListener->charUpdate(p->systemAddress,&data);
        }
        else mListener->charCreate(p->systemAddress,&data);
    }
    void sendCharCreate(const SystemAddress &add, const bool &flag, const char &selection=0)
    {
        mWriter.create(ID_CHARCREATE);
        mWriter.writeBool(flag);
        if(flag)mWriter.writeChar(selection);
        mWriter.send(add);
    }
    void readCharDelete(Packet *p)
    {
        mReader.load(p);
        mListener->charDelete(p->systemAddress,mReader.readUInt());
    }
    void readConnectMain(Packet *p)
    {
        mReader.load(p);
        const unsigned int charID = mReader.readUInt();
        mListener->connectMainEvent(p->systemAddress,charID);
    }
    void sendTokenToClient(const SystemAddress &add, const string &token)
    {
        mWriter.create(ID_TOKEN);
        mWriter.writeString(token);

        mWriter.send(add);
    }
    /*void sendTokenToMain(const SystemAddress &add, const string &token, const unsigned int &userID, const unsigned int &charID)
    {
        mWriter.create(ID_TOKEN);
        mWriter.writeString(token);
        mWriter.writeUInt(userID);
        mWriter.writeUInt(charID);

        mWriter.send(mMainServerAdd);
    }*/

};

#endif
