#ifndef _NETWORKMANAGER_H_
#define _NETWORKMANAGER_H_

#include "NetworkDefines.h"
#include "PacketReader.h"
#include "UnitManager.h"
#include "Chat.h"
#include "RakNetStatistics.h"

using namespace RakNet;

class NetworkListener
{
public:
    NetworkListener(){}
    virtual ~NetworkListener(){}
    //Front Server Events
    virtual void frontServerReply(const unsigned char &reply, const unsigned int &banMins=0){}
    virtual void frontServerSaltReply(const String &salt){}
    virtual void charSelectReply(const char &selection, PlayerData *data, const unsigned char &numCharacters, const unsigned int &charID){}
    virtual void charCreateReply(const bool &success, const char &selection){}
    virtual void frontServerToken(const String &token){}
    //Main Server Events
    virtual void mainServerReply(const unsigned char &reply){}
    virtual void mainServerMapReply(const String &map, const Real &dayTime, const Real &weatherTime){}
    //Map Server Events
    virtual void mapServerReply(const unsigned char &reply){}
    virtual void mapServerInfo(const Vector3 &position){}
    virtual void charInfo(const unsigned int &charID, PlayerData &data, const Vector3 &position, const unsigned int &userID, const String &username){}
    virtual void charExit(const unsigned int &charID, const bool &isProper, const bool &isMapChange){}
    virtual void charPositionUpdate(const unsigned int &charID, const Vector3 &position){}
    virtual void chatEvent(const unsigned int &charID, const String &caption, const unsigned char &channel, const unsigned int &userID, const String &whisperer="", const bool &whisperFail=false, const bool &isOffline=false, const bool &doLipSync=true){}
    virtual void charYawUpdate(const unsigned int &charID, const unsigned char &yaw){}
    virtual void charHeadDirUpdate(const unsigned int &charID, const unsigned char &ew, const unsigned char &ns){}
    virtual void charActionUpdate(const unsigned int &charID, const unsigned char &action){}
    virtual void charStatusFlagsUpdate(const unsigned int &charID, const bool &antiGravity, const bool &swimming){}
    virtual void charMoveModifierEvent(const unsigned int &charID, const unsigned char &moveModifier){}
    virtual void charEmoteEvent(const unsigned int &charID, const unsigned char &emote){}
    virtual void charJumpEvent(const unsigned int &charID, const bool &flag){}
    virtual void movieEvent(const unsigned int &charID, const String &movie, const Real &camYaw){}
    virtual void addFriendEvent(const String &username, const bool &isBlock, const bool &success){}
    virtual void friendListInfo(const unsigned int &userID, const String &username, const bool &isOnline){}
    virtual void blockListInfo(const unsigned int &userID, const String &username, const bool &isBlockedBy){}
    virtual void requestListInfo(const unsigned int &userID, const String &username){}
    virtual void friendListUpdate(const unsigned int &userID, const bool &isOnline){}
    virtual void friendRemove(const unsigned int &userID){}
    virtual void blockedByRemove(const unsigned int &userID){}
    virtual void charOnlineStatusEvent(const unsigned int &userID, const unsigned int &charID, const bool &isOnline){}
    virtual void findCharEvent(const String &name, const String &mapName){}
    virtual void charBioInfoEvent(const String &username, const String &charname, const String &group, const unsigned int &groupID, const unsigned int &userID){}
    virtual void charBioEvent(const bool &isOwn, const String &bio){}
    virtual void modEvent(const bool &bIsSender, const String &command){}
    virtual void myGroupInfo(const String &groupname, const bool &isLeader){}
    virtual void myGroupMemberInfo(const unsigned int &charID, const String &charname, const unsigned char &rank, const String &title, const bool &isOnline, const bool &announce){}
    virtual void myGroupEvent(const bool &isCreate, const bool &success){}
    virtual void myGroupLeaveEvent(const unsigned int &charID, const bool &isKick, const bool &isDisband){}
    virtual void viewGroupInfo(const unsigned int &groupID, const String &groupname){}
    virtual void viewGroupMembersInfo(const unsigned int &charID, const String &charname, const unsigned char &rank, const String &title){}
    virtual void groupRequestInfo(const unsigned int &charID, const String &charname, const bool &isRequest){}
    virtual void groupBio(const String &bio){}
    virtual void duplicateLoginEvent(const String &token){}
    virtual void homeInfo(const String &home){}
    virtual void goHomeEvent(const String &home, const Vector3 &position){}
    virtual void partyMemberInfo(const unsigned int &charID, const String &charname, const bool &announce){}
    virtual void partyLeaveEvent(const unsigned int &charID){}
    virtual void partyRequestEvent(const unsigned int &charID, const String &charname, const bool &isFail){}
    virtual void charSkillEvent(const unsigned int &nCharID, const unsigned char &nSkill, const bool &bFlag){}
};

class NetworkManager : public Singleton<NetworkManager>
{
private:
    RakPeerInterface *mClient;
    std::vector<NetworkListener*> mListeners;
    bool mStarted;
    PacketReader mReader;
    PacketWriter mWriter;
    SystemAddress mServerAdd;
    bool mIsFrontServer;
    bool mIsMainServer;
    bool mConnected;
    unsigned short mMapPort;
    bool mDoMapTransition;
    bool mDoShutdown;
    String mMacAddress;
public:
    NetworkManager(const String &macAddress="")
    {
        mClient = RakPeerInterface::GetInstance();
        mWriter.setPeer(mClient);
        mServerAdd = UNASSIGNED_SYSTEM_ADDRESS;
        mIsFrontServer = mIsMainServer = false;
        mStarted = false;
        mConnected = false;
        mMapPort = 0;
        mDoMapTransition = false;
        mDoShutdown = false;
        mMacAddress = macAddress;
    }
    ~NetworkManager()
    {
        trueShutdown();
        RakPeerInterface::DestroyInstance(mClient);
    }
    static NetworkManager* getSingletonPtr();
    static NetworkManager& getSingleton();
    enum
    {
        SERVER_CONNECT_SUCCESS,
        SERVER_CONNECT_FAIL,
        SERVER_CONNECT_FULL,
        SERVER_CONNECT_MISMATCH,
        SERVER_CONNECT_LOST,
        SERVER_LOGIN_SUCCESS,
        SERVER_LOGIN_FAIL,
        SERVER_LOGIN_BANNED,
        SERVER_DISCONNECTED,
        SERVER_TOKEN_FAIL
    };
    void addListener(NetworkListener *listener)
    {
        mListeners.push_back(listener);
    }
    void clearListeners()
    {
        mListeners.clear();
    }
    void update(const Real &timeElapsed)
    {
        if(mDoShutdown)trueShutdown();
        if(!mStarted)return;

        if(mDoMapTransition)
        {
            shutdown();
            connectMapServer();
            mDoMapTransition = false;
            return;
        }

        mWriter.updateQueue(timeElapsed,mServerAdd);

        for(Packet *p=mClient->Receive(); p; mClient->DeallocatePacket(p), p=mClient->Receive())
        {
            switch(p->data[0])
			{
			    //Connection
                case ID_CONNECTION_REQUEST_ACCEPTED:
                    mServerAdd = p->systemAddress;
                    mConnected = true;
                    broadcastServerReply(SERVER_CONNECT_SUCCESS);
                    break;
                case ID_CONNECTION_ATTEMPT_FAILED:
                case ID_ALREADY_CONNECTED:
                case ID_CONNECTION_BANNED:
                    broadcastServerReply(SERVER_CONNECT_FAIL);
                    break;
                case ID_NO_FREE_INCOMING_CONNECTIONS:
                    broadcastServerReply(SERVER_CONNECT_FULL);
                    break;
                case ID_INVALID_PASSWORD:
                case ID_INCOMPATIBLE_PROTOCOL_VERSION:
                    broadcastServerReply(SERVER_CONNECT_MISMATCH);
                    break;
                case ID_DISCONNECTION_NOTIFICATION:
                case ID_CONNECTION_LOST:
                    if(mServerAdd!=UNASSIGNED_SYSTEM_ADDRESS)
                    {
                        mServerAdd = UNASSIGNED_SYSTEM_ADDRESS;
                        broadcastServerReply(p->data[0]==ID_CONNECTION_LOST?SERVER_CONNECT_LOST:SERVER_DISCONNECTED);
                        mIsFrontServer = false;
                        mIsMainServer = false;
                        mConnected = false;
                    }
                    break;
                //Front server
                case ID_LOGIN:
                    readLogin(p);
                    break;
                case ID_CHARSELECT:
                    readCharSelect(p);
                    break;
                case ID_CHARCREATE:
                    readCharCreate(p);
                    break;
                case ID_TOKEN:
                    readToken(p);
                    break;
                //Main server
                case ID_CONNECT_MAIN:
                    readConnectMain(p);
                    break;
                case ID_TOKEN_FAIL:
                    broadcastServerReply(SERVER_TOKEN_FAIL);
                    break;
                //Map server
                case ID_CONNECT_MAP:
                    readConnectMap(p);
                    break;
                case ID_CHAR_INFO:
                    readCharInfo(p);
                    break;
                case ID_CHAR_EXIT:
                    readCharExit(p);
                    break;
                case ID_CHAR_POSITION:
                    readCharPosition(p);
                    break;
                case ID_CHAT:
                    readChat(p);
                    break;
                case ID_CHAR_YAW:
                    readCharYaw(p);
                    break;
                case ID_CHAR_HEADDIR:
                    readCharHeadDir(p);
                    break;
                case ID_CHAR_ACTION:
                    readCharAction(p);
                    break;
                case ID_CHAR_STATUSFLAGS:
                    readCharStatusFlags(p);
                    break;
                case ID_CHAR_MOVEMODIFIER:
                    readCharMoveModifier(p);
                    break;
                case ID_CHAR_EMOTE:
                    readCharEmote(p);
                    break;
                case ID_CHAR_JUMP:
                    readCharJump(p);
                    break;
                case ID_MOVIE:
                    readMovie(p);
                    break;
                case ID_FRIENDADD:
                    readAddFriend(p);
                    break;
                case ID_FRIEND:
                    readFriendInfo(p);
                    break;
                case ID_CHAR_ONLINESTATUS:
                    readCharOnlineStatus(p);
                    break;
                case ID_FINDCHAR:
                    readFindChar(p);
                    break;
                case ID_CHAR_BIO:
                    readCharBio(p);
                    break;
                case ID_MOD:
                    readMod(p);
                    break;
                case ID_MYGROUP:
                    readMyGroupInfo(p);
                    break;
                case ID_VIEWGROUP:
                    readViewGroupInfo(p);
                    break;
                case ID_GROUPREQUEST:
                    readGroupRequest(p);
                    break;
                case ID_GROUPBIO:
                    readGroupBio(p);
                    break;
                case ID_MISC:
                    readMisc(p);
                    break;
                case ID_HOME:
                    readHome(p);
                    break;
                case ID_PARTY:
                    readPartyInfo(p);
                    break;
                case ID_PARTYREQUEST:
                    readPartyRequest(p);
                    break;
                case ID_CHAR_SKILL:
                    readCharSkill(p);
                    break;

                default: break;
			}
		}
    }
    void connectFrontServer()
    {
        if(mDoShutdown)trueShutdown();
        mIsFrontServer = true;
        SocketDescriptor socketDesc;
        mStarted = (mClient->Startup(1, &socketDesc, 1)==0);
        if(!mStarted)LoggerManager::getSingleton().logMessage("Network:: Startup Failed!");
        mClient->Connect(FRONT_SERVER_IP, FRONT_SERVER_PORT, SERVER_PASSWORD, (int)strlen(SERVER_PASSWORD));
    }
    void connectMainServer()
    {
        if(mDoShutdown)trueShutdown();
        mIsMainServer = true;
        SocketDescriptor socketDesc;
        mStarted = (mClient->Startup(1, &socketDesc, 1)==0);
        if(!mStarted)LoggerManager::getSingleton().logMessage("Network:: Startup Failed!");
        mClient->Connect(MAIN_SERVER_IP, MAIN_SERVER_PORT, SERVER_PASSWORD, (int)strlen(SERVER_PASSWORD));
    }
    void connectMapServer()
    {
        if(mDoShutdown)trueShutdown();
        SocketDescriptor socketDesc;
        mStarted = (mClient->Startup(1, &socketDesc, 1)==0);
        if(!mStarted)LoggerManager::getSingleton().logMessage("Network:: Startup Failed!");
        mClient->Connect(MAP_SERVER_IP, mMapPort, SERVER_PASSWORD, (int)strlen(SERVER_PASSWORD));
    }
    void doMapTransition()
    {
        mDoMapTransition = true;
    }
    void disconnect()
    {
        //printStats();
        mIsFrontServer = false;
        mIsMainServer = false;
        mConnected = false;
        if(mServerAdd!=UNASSIGNED_SYSTEM_ADDRESS)
        {
            mClient->CloseConnection(mServerAdd,true);
            mServerAdd = UNASSIGNED_SYSTEM_ADDRESS;
        }
        mWriter.clearQueue();
    }
    void printStats()
    {
        char temp[2048]="";
        RakNetStatistics *rss = mClient->GetStatistics(mClient->GetSystemAddressFromIndex(0));
        StatisticsToString(rss, temp, 2);

        ofstream outfile("raknetStats.txt",ios::app);
        outfile.write(temp,strlen(temp));
        strcpy(temp,"");
        itoa(mClient->GetAveragePing(mClient->GetSystemAddressFromIndex(0)),temp,10);
        if(mIsFrontServer)strcat(temp," @@END FRONT\n");
        else if(mIsMainServer)strcat(temp," @@END MAIN\n");
        else strcat(temp," @@END MAP\n");
        outfile.write(temp,strlen(temp));
        outfile.close();
    }
    void shutdown()
    {
        mDoShutdown = true;
    }
    void trueShutdown()
    {
        disconnect();
        if(mStarted)
        {
            mClient->Shutdown(300);
            mStarted = false;
        }
        mDoShutdown = false;
    }
    void broadcastServerReply(const unsigned char &reply)
    {
        if(mIsFrontServer)
        {
            if(!mListeners.empty())mListeners.front()->frontServerReply(reply);
        }
        else if(mIsMainServer)
        {
            if(!mListeners.empty())mListeners.front()->mainServerReply(reply);
        }
        else for(std::vector<NetworkListener*>::iterator it=mListeners.begin(); it!=mListeners.end(); it++)
        {
            NetworkListener *listener = *it;
            listener->mapServerReply(reply);
        }
    }
    void sendLogin(const String &username)
    {
        mWriter.create(ID_LOGIN);
        mWriter.writeBool(false);
        mWriter.writeString(VERSION_TOKEN);
        mWriter.writeString(username);
        mWriter.writeString(mMacAddress);

        mWriter.send(mServerAdd);
    }
    void sendHashedPassword(const String &hash)
    {
        mWriter.create(ID_LOGIN);
        mWriter.writeBool(true);
        mWriter.writeString(hash);

        mWriter.send(mServerAdd);
    }
    void readLogin(Packet *p)
    {
        mReader.load(p);
        if(!mReader.readBool())
        {
            const bool versionOutdated = mReader.readBool();
            if(!mListeners.empty())
            {
                if(versionOutdated)mListeners.front()->frontServerReply(SERVER_CONNECT_MISMATCH);
                else mListeners.front()->frontServerSaltReply(mReader.readString(32));
            }
        }
        else
        {
            const bool success = mReader.readBool();
            if(success)
            {
                if(!mListeners.empty())mListeners.front()->frontServerReply(SERVER_LOGIN_SUCCESS);
            }
            else
            {
                if(!mListeners.empty())
                {
                    const bool isBan = mReader.readBool();
                    mListeners.front()->frontServerReply(isBan?SERVER_LOGIN_BANNED:SERVER_LOGIN_FAIL, isBan?mReader.readUInt():0);
                }
            }
        }
    }
    void sendCharSelect(const char &selection)
    {
        mWriter.create(ID_CHARSELECT);
        mWriter.writeChar(selection);

        mWriter.send(mServerAdd);
    }
    void readCharSelect(Packet *p)
    {
        mReader.load(p);
        const char selection = mReader.readChar();
        if(mReader.readBool())
        {
            PlayerData playerData;
            readPlayerData(playerData);

            const unsigned char numChars = mReader.readUChar();
            const unsigned int charID = mReader.readUInt();

            if(!mListeners.empty())mListeners.front()->charSelectReply(selection,&playerData,numChars,charID);
        }
        else if(!mListeners.empty())mListeners.front()->charSelectReply(selection,0,0,0);
    }
    void readPlayerData(PlayerData &playerData)
    {
        playerData.mName = mReader.readString(32);
        playerData.mSpecies = mReader.readChar();
        for(int i=0;i<MAX_MESHES;i++)playerData.mMesh.push_back(mReader.readUChar());
        for(int i=0;i<MAX_MATS;i++)playerData.mMat.push_back(mReader.readUChar());
        for(int i=0;i<NUM_SCALABLES;i++)playerData.mScale[i] = mReader.readChar();
        for(int i=0;i<NUM_COLOURABLES;i++)
            for(int j=0;j<3;j++)playerData.mColour[i][j] = mReader.readUChar();
        playerData.mEmote = mReader.readUChar();
        playerData.mPreset = mReader.readUChar();
        playerData.m_nEquip = mReader.readUChar();
        playerData.m_szItems = mReader.readString(32);
    }
    void sendCharCreate(PlayerData &data, const bool &isEdit, const unsigned int &id)
    {
        mWriter.create(ID_CHARCREATE);
        mWriter.writeString(data.mName);
        mWriter.writeChar(data.mSpecies);
        for(int i=0;i<MAX_MESHES;i++)mWriter.writeUChar((int)data.mMesh.size()>i ? data.mMesh[i] : 0);
        for(int i=0;i<MAX_MATS;i++)mWriter.writeUChar((int)data.mMat.size()>i ? data.mMat[i] : 0);
        for(int i=0;i<NUM_SCALABLES;i++)mWriter.writeChar(data.mScale[i]);
        for(int i=0;i<NUM_COLOURABLES;i++)
            for(int j=0;j<3;j++)mWriter.writeUChar(data.mColour[i][j]);
        mWriter.writeUChar(data.mEmote);
        mWriter.writeUChar(data.mPreset);
        mWriter.writeUChar(data.m_nEquip);
        mWriter.writeString(data.m_szItems);

        mWriter.writeBool(isEdit);
        if(isEdit)mWriter.writeUInt(id);

        mWriter.send(mServerAdd);
    }
    void readCharCreate(Packet *p)
    {
        mReader.load(p);
        const bool success = mReader.readBool();
        if(!mListeners.empty())mListeners.front()->charCreateReply(success,success?mReader.readChar():0);
    }
    void sendCharDelete(const unsigned int &id)
    {
        mWriter.create(ID_CHARDELETE);
        mWriter.writeUInt(id);

        mWriter.send(mServerAdd);
    }
    void sendCharStart(const unsigned int &charID)
    {
        mWriter.create(ID_CONNECT_MAIN);
        mWriter.writeUInt(charID);

        mWriter.send(mServerAdd);
    }
    void readToken(Packet *p)
    {
        mReader.load(p);
        if(!mListeners.empty())mListeners.front()->frontServerToken(mReader.readString(16));
    }
    void sendConnectMain(const String &token, const unsigned int &charID, const String &ghostMap="")
    {
        mWriter.create(ID_CONNECT_MAIN);
        mWriter.writeString(token);
        if(charID!=0 || ghostMap!="")mWriter.writeUInt(charID);
        if(ghostMap!="")
        {
            mWriter.writeBool(true);
            mWriter.writeString(ghostMap);
        }

        mWriter.send(mServerAdd);
    }
    void readConnectMain(Packet *p)
    {
        mReader.load(p);
        mMapPort = mReader.readUShort();
        const String map = mReader.readString(32);
        const Real dayTime = mReader.readFloat();
        const Real weatherTime = mReader.readFloat();
        if(!mListeners.empty())mListeners.front()->mainServerMapReply(map,dayTime,weatherTime);
    }
    void sendConnectMap(const String &token, const bool &firstEntrance, const bool &ghostMode=false)
    {
        mWriter.create(ID_CONNECT_MAP);
        mWriter.writeString(token);
        mWriter.writeBool(firstEntrance);
        if(ghostMode)mWriter.writeBool(true);

        mWriter.send(mServerAdd);
    }
	void readConnectMap(Packet *p)
	{
		mReader.load(p);
		const int posX = mReader.readInt();
		const int posY = mReader.readInt();
		const int posZ = mReader.readInt();

		// snapshots the listener list so callbacks can safely add/remove listeners
		std::vector<NetworkListener*> listenersCopy = mListeners;

		for (std::vector<NetworkListener*>::iterator it = listenersCopy.begin(); it != listenersCopy.end(); it++)
		{
			NetworkListener *listener = *it;
			listener->mapServerInfo(Vector3(posX, posY, posZ));
		}
	}
    void sendMapChange(const String &map, const Vector3 &position)
    {
        mWriter.create(ID_CHAR_MAPCHANGE);
        mWriter.writeBool(false);
        mWriter.writeString(map);
        mWriter.writeFloat(position.x);
        mWriter.writeFloat(position.y);
        mWriter.writeFloat(position.z);

        mWriter.send(mServerAdd);
    }
    void sendGhostMapChange(const bool &unghost)
    {
        mWriter.create(ID_CHAR_MAPCHANGE);
        mWriter.writeBool(true);
        mWriter.writeBool(unghost);

        mWriter.send(mServerAdd);
    }
    void readCharInfo(Packet *p)
    {
        mReader.load(p);
        const unsigned int charID = mReader.readUInt();
        PlayerData playerData;
        readPlayerData(playerData);
        const float posX = mReader.readFloat();
        const float posY = mReader.readFloat();
        const float posZ = mReader.readFloat();
        const unsigned int userID = mReader.readUInt();
        const String username = mReader.readString(32);

        for(std::vector<NetworkListener*>::iterator it=mListeners.begin(); it!=mListeners.end(); it++)
        {
            NetworkListener *listener = *it;
            listener->charInfo(charID,playerData,Vector3(posX,posY,posZ),userID,username);
        }
    }
    void readCharExit(Packet *p)
    {
        mReader.load(p);
        const unsigned int charID = mReader.readUInt();
        const bool isProper = mReader.readBool();
        const bool isMapChange = mReader.readBool();
        for(std::vector<NetworkListener*>::iterator it=mListeners.begin(); it!=mListeners.end(); it++)
        {
            NetworkListener *listener = *it;
            listener->charExit(charID,isProper,isMapChange);
        }
    }
    void sendCharPosition(const Vector3 &position)
    {
        if(!mConnected)return;
        mWriter.create(ID_CHAR_POSITION);
        mWriter.writeFloat(position.x);
        mWriter.writeFloat(position.y);
        mWriter.writeFloat(position.z);

        mWriter.sendUnreliable(mServerAdd);
    }
    void readCharPosition(Packet *p)
    {
        mReader.load(p);
        const unsigned int charID = mReader.readUInt();
        const float posX = mReader.readFloat();
        const float posY = mReader.readFloat();
        const float posZ = mReader.readFloat();
        for(std::vector<NetworkListener*>::iterator it=mListeners.begin(); it!=mListeners.end(); it++)
        {
            NetworkListener *listener = *it;
            listener->charPositionUpdate(charID,Vector3(posX,posY,posZ));
        }
    }
    void sendChat(const String &caption, const unsigned char &channel, const String &whisperTarget="", const bool &doLipSync=false)
    {
        if(!mConnected)return;
        mWriter.create(ID_CHAT);
        mWriter.writeString(caption);
        mWriter.writeUChar(channel);
        if(whisperTarget!="")mWriter.writeString(whisperTarget);
        //mWriter.writeBool(doLipSync);

        mWriter.send(mServerAdd);
    }
    void readChat(Packet *p)
    {
        mReader.load(p);
        const unsigned int charID = mReader.readUInt();
        const String caption = mReader.readString(256);
        const unsigned char channel = mReader.readUChar();
        const unsigned int userID = mReader.readUInt();
        const String whisperer = ((channel==CHAT_WHISPER||channel==CHAT_MODWHISPER) ? mReader.readString(32) : "");
        const bool whisperFail = ((channel==CHAT_WHISPER||channel==CHAT_MODWHISPER) ? mReader.readBool() : false);
        const bool isOffline = ((channel==CHAT_WHISPER||channel==CHAT_MODWHISPER) && whisperFail ? mReader.readBool() : false);
        const bool doLipSync = true;//mReader.readBool();
        for(std::vector<NetworkListener*>::iterator it=mListeners.begin(); it!=mListeners.end(); it++)
        {
            NetworkListener *listener = *it;
            listener->chatEvent(charID,caption,channel,userID,whisperer,whisperFail,isOffline,doLipSync);
        }
    }
    void sendCharYaw(const unsigned char &yaw)
    {
        if(!mConnected)return;
        mWriter.create(ID_CHAR_YAW);
        mWriter.writeUChar(yaw);

        mWriter.sendUnreliable(mServerAdd);
    }
    void readCharYaw(Packet *p)
    {
        mReader.load(p);
        const unsigned int charID = mReader.readUInt();
        const unsigned char yaw = mReader.readUChar();
        for(std::vector<NetworkListener*>::iterator it=mListeners.begin(); it!=mListeners.end(); it++)
        {
            NetworkListener *listener = *it;
            listener->charYawUpdate(charID,yaw);
        }
    }
    void sendCharHeadDir(const unsigned char &ew, const unsigned char &ns)
    {
        if(!mConnected)return;
        mWriter.create(ID_CHAR_HEADDIR);
        mWriter.writeUChar(ew);
        mWriter.writeUChar(ns);

        mWriter.sendUnreliable(mServerAdd);
    }
    void readCharHeadDir(Packet *p)
    {
        mReader.load(p);
        const unsigned int charID = mReader.readUInt();
        const unsigned char ew = mReader.readUChar();
        const unsigned char ns = mReader.readUChar();
        for(std::vector<NetworkListener*>::iterator it=mListeners.begin(); it!=mListeners.end(); it++)
        {
            NetworkListener *listener = *it;
            listener->charHeadDirUpdate(charID,ew,ns);
        }
    }
    void sendCharAction(const unsigned char &action)
    {
        if(!mConnected)return;
        mWriter.create(ID_CHAR_ACTION);
        mWriter.writeUChar(action);

        mWriter.sendUnreliable(mServerAdd);
    }
    void readCharAction(Packet *p)
    {
        mReader.load(p);
        const unsigned int charID = mReader.readUInt();
        const unsigned char action = mReader.readUChar();
        for(std::vector<NetworkListener*>::iterator it=mListeners.begin(); it!=mListeners.end(); it++)
        {
            NetworkListener *listener = *it;
            listener->charActionUpdate(charID,action);
        }
    }
    void sendCharStatusFlags(const bool &antiGravity, const bool &swimming)
    {
        if(!mConnected)return;
        mWriter.create(ID_CHAR_STATUSFLAGS);
        mWriter.writeBool(antiGravity);
        mWriter.writeBool(swimming);

        mWriter.sendUnreliable(mServerAdd);
    }
    void readCharStatusFlags(Packet *p)
    {
        mReader.load(p);
        const unsigned int charID = mReader.readUInt();
        const bool antiGravity = mReader.readBool();
        const bool swimming = mReader.readBool();
        for(std::vector<NetworkListener*>::iterator it=mListeners.begin(); it!=mListeners.end(); it++)
        {
            NetworkListener *listener = *it;
            listener->charStatusFlagsUpdate(charID,antiGravity,swimming);
        }
    }
    void sendCharMoveModifier(const unsigned char &moveModifier)
    {
        if(!mConnected)return;
        mWriter.create(ID_CHAR_MOVEMODIFIER);
        mWriter.writeUChar(moveModifier);

        mWriter.send(mServerAdd,true);
    }
    void readCharMoveModifier(Packet *p)
    {
        mReader.load(p);
        const unsigned int charID = mReader.readUInt();
        const unsigned char moveModifier = mReader.readUChar();
        for(std::vector<NetworkListener*>::iterator it=mListeners.begin(); it!=mListeners.end(); it++)
        {
            NetworkListener *listener = *it;
            listener->charMoveModifierEvent(charID,moveModifier);
        }
    }
    void sendCharEmote(const unsigned char &emote, const bool &bSetDefault=false)
    {
        if(!mConnected)return;
        mWriter.create(ID_CHAR_EMOTE);
        mWriter.writeUChar(emote);
        if(bSetDefault)mWriter.writeBool(true);

        mWriter.send(mServerAdd,true);
    }
    void readCharEmote(Packet *p)
    {
        mReader.load(p);
        const unsigned int charID = mReader.readUInt();
        const unsigned char emote = mReader.readUChar();
        for(std::vector<NetworkListener*>::iterator it=mListeners.begin(); it!=mListeners.end(); it++)
        {
            NetworkListener *listener = *it;
            listener->charEmoteEvent(charID,emote);
        }
    }
    void sendCharJump(const bool &flag)
    {
        if(!mConnected)return;
        mWriter.create(ID_CHAR_JUMP);
        mWriter.writeBool(flag);

        mWriter.send(mServerAdd,true);
    }
    void readCharJump(Packet *p)
    {
        mReader.load(p);
        const unsigned int charID = mReader.readUInt();
        const bool isJumping = mReader.readBool();
        for(std::vector<NetworkListener*>::iterator it=mListeners.begin(); it!=mListeners.end(); it++)
        {
            NetworkListener *listener = *it;
            listener->charJumpEvent(charID,isJumping);
        }
    }
    void sendMovie(const String &movie, const Real &camYaw)
    {
        if(!mConnected)return;
        mWriter.create(ID_MOVIE);
        mWriter.writeString(movie);
        mWriter.writeFloat(camYaw);

        mWriter.send(mServerAdd);
    }
    void readMovie(Packet *p)
    {
        mReader.load(p);
        const unsigned int charID = mReader.readUInt();
        const String movie = mReader.readString(256);
        const Real camYaw = mReader.readFloat();
        for(std::vector<NetworkListener*>::iterator it=mListeners.begin(); it!=mListeners.end(); it++)
        {
            NetworkListener *listener = *it;
            listener->movieEvent(charID,movie,camYaw);
        }
    }
    void sendAddFriend(const String &username, const bool &isBlock)
    {
        if(!mConnected)return;
        mWriter.create(ID_FRIENDADD);
        mWriter.writeBool(isBlock);     //Friend/Block
        mWriter.writeBool(true);         //true = add, false = remove
        mWriter.writeString(username);

        mWriter.send(mServerAdd);
    }
    void sendRemoveFriend(const unsigned int &userID, const bool &isBlock)
    {
        if(!mConnected)return;
        mWriter.create(ID_FRIENDADD);
        mWriter.writeBool(isBlock);     //Friend/Block
        mWriter.writeBool(false);         //true = add, false = remove
        mWriter.writeUInt(userID);

        mWriter.send(mServerAdd);
    }
    void sendAcceptFriend(const unsigned int &userID, const bool &accept)
    {
        if(!mConnected)return;
        mWriter.create(ID_FRIEND);
        mWriter.writeUInt(userID);
        mWriter.writeBool(accept);

        mWriter.send(mServerAdd);
    }
    void readAddFriend(Packet *p)
    {
        mReader.load(p);
        const bool isBlock = mReader.readBool();
        const bool success = mReader.readBool();
        const String username = mReader.readString(32);
        for(std::vector<NetworkListener*>::iterator it=mListeners.begin(); it!=mListeners.end(); it++)
        {
            NetworkListener *listener = *it;
            listener->addFriendEvent(username,isBlock,success);
        }
    }
    void readFriendInfo(Packet *p)
    {
        mReader.load(p);
        const bool isRemove = mReader.readBool();
        const bool isBlockOrRequest = mReader.readBool();   //for remove, false = friend, true = block
        const unsigned int userID = mReader.readUInt();
        if(isRemove)
        {
            for(std::vector<NetworkListener*>::iterator it=mListeners.begin(); it!=mListeners.end(); it++)
            {
                NetworkListener *listener = *it;
                if(isBlockOrRequest)listener->blockedByRemove(userID);
                else listener->friendRemove(userID);
            }
            return;
        }
        if(isBlockOrRequest)
        {
            const bool isRequest = mReader.readBool();
            const bool isBlockedBy = (isRequest ? false : mReader.readBool());
            const String username = (isBlockedBy ? "" : mReader.readString(32));
            for(std::vector<NetworkListener*>::iterator it=mListeners.begin(); it!=mListeners.end(); it++)
            {
                NetworkListener *listener = *it;
                if(isRequest)listener->requestListInfo(userID,username);
                else listener->blockListInfo(userID,username,isBlockedBy);
            }
            return;
        }
        const bool isOnline = mReader.readBool();
        const bool isUpdate = mReader.readBool();
        if(isUpdate)
        {
            for(std::vector<NetworkListener*>::iterator it=mListeners.begin(); it!=mListeners.end(); it++)
            {
                NetworkListener *listener = *it;
                listener->friendListUpdate(userID,isOnline);
            }
            return;
        }
        const String username = mReader.readString(32);
        for(std::vector<NetworkListener*>::iterator it=mListeners.begin(); it!=mListeners.end(); it++)
        {
            NetworkListener *listener = *it;
            listener->friendListInfo(userID,username,isOnline);
        }
    }
    void readCharOnlineStatus(Packet *p)
    {
        mReader.load(p);
        const unsigned int userID = mReader.readUInt();
        const unsigned int charID = mReader.readUInt();
        const bool isOnline = mReader.readBool();
        for(std::vector<NetworkListener*>::iterator it=mListeners.begin(); it!=mListeners.end(); it++)
        {
            NetworkListener *listener = *it;
            listener->charOnlineStatusEvent(userID,charID,isOnline);
        }
    }
    void sendFindChar(const unsigned int &id, const bool &isUserID)
    {
        if(!mConnected)return;
        mWriter.create(ID_FINDCHAR);
        mWriter.writeUInt(id);
        mWriter.writeBool(isUserID);

        mWriter.send(mServerAdd,true);
    }
    void readFindChar(Packet *p)
    {
        mReader.load(p);
        const String name = mReader.readString(32);
        const String mapName = mReader.readString(32);
        for(std::vector<NetworkListener*>::iterator it=mListeners.begin(); it!=mListeners.end(); it++)
        {
            NetworkListener *listener = *it;
            listener->findCharEvent(name,mapName);
        }
    }
    void sendCharBio(const String &bio)
    {
        if(!mConnected)return;
        mWriter.create(ID_CHAR_BIO);
        mWriter.writeBool(false);       //is not request
        mWriter.writeString(bio);

        mWriter.send(mServerAdd,true);
    }
    void sendCharBioRequest(const unsigned int &charID, const bool &isBio, const bool &requestUserID=false)
    {
        if(!mConnected)return;
        mWriter.create(ID_CHAR_BIO);
        mWriter.writeBool(true);       //is request
        mWriter.writeUInt(charID);
        mWriter.writeBool(isBio);
        mWriter.writeBool(requestUserID);

        mWriter.send(mServerAdd,true);
    }
    void readCharBio(Packet *p)
    {
        mReader.load(p);
        const bool isBio = mReader.readBool();
        if(!isBio)
        {
            const String username = mReader.readString(32);
            const String charname = mReader.readString(32);
            const String group = mReader.readString(32);
            const unsigned int groupID = mReader.readUInt();
            const unsigned int userID = mReader.readUInt(); //0 if not requested
            for(std::vector<NetworkListener*>::iterator it=mListeners.begin(); it!=mListeners.end(); it++)
            {
                NetworkListener *listener = *it;
                listener->charBioInfoEvent(username,charname,group,groupID,userID);
            }
        }
        else
        {
            const bool isOwn = mReader.readBool();
            const String bio = mReader.readString(1024);
            for(std::vector<NetworkListener*>::iterator it=mListeners.begin(); it!=mListeners.end(); it++)
            {
                NetworkListener *listener = *it;
                listener->charBioEvent(isOwn,bio);
            }

        }
    }
    void sendMod(const String &command)
    {
        mWriter.create(ID_MOD);
        mWriter.writeString(command);

        mWriter.send(mServerAdd);
    }
    void readMod(Packet *p)
    {
        mReader.load(p);
        const bool bIsSender = mReader.readBool();
        const String command = mReader.readString(256);
        for(std::vector<NetworkListener*>::iterator it=mListeners.begin(); it!=mListeners.end(); it++)
        {
            NetworkListener *listener = *it;
            listener->modEvent(bIsSender,command);
        }
    }
    void sendCreateGroup(const String &groupname)
    {
        mWriter.create(ID_MYGROUP);
        mWriter.writeBool(true);  //is create
        mWriter.writeString(groupname);

        mWriter.send(mServerAdd);
    }
    void sendLeaveGroup(const unsigned int &charID, const bool &isKick)
    {
        mWriter.create(ID_MYGROUP);
        mWriter.writeBool(false);  //is modification
        mWriter.writeUInt(charID);  //0 = self
        mWriter.writeBool(isKick);

        mWriter.send(mServerAdd);
    }
    void readMyGroupInfo(Packet *p)
    {
        mReader.load(p);
        const bool isInfo = mReader.readBool();
        if(isInfo)
        {
            const bool isMember = mReader.readBool();
            if(isMember)
            {
                const unsigned int charID = mReader.readUInt();
                const String charname = mReader.readString(32);
                const unsigned char rank = mReader.readUChar();
                const String title = mReader.readString(32);
                const bool isOnline = mReader.readBool();
                const bool announce = mReader.readBool();
                for(std::vector<NetworkListener*>::iterator it=mListeners.begin(); it!=mListeners.end(); it++)
                {
                    NetworkListener *listener = *it;
                    listener->myGroupMemberInfo(charID,charname,rank,title,isOnline,announce);
                }
            }
            else
            {
                const bool hasGroup = mReader.readBool();
                const String groupName = hasGroup?mReader.readString(32):"";
                const bool isLeader = hasGroup?mReader.readBool():false;
                for(std::vector<NetworkListener*>::iterator it=mListeners.begin(); it!=mListeners.end(); it++)
                {
                    NetworkListener *listener = *it;
                    listener->myGroupInfo(groupName,isLeader);
                }
            }
        }
        //Is event
        else
        {
            const bool isLeave = mReader.readBool();
            if(isLeave)
            {
                const unsigned int charID = mReader.readUInt();
                const bool isKick = mReader.readBool();
                const bool isDisband = mReader.readBool();
                for(std::vector<NetworkListener*>::iterator it=mListeners.begin(); it!=mListeners.end(); it++)
                {
                    NetworkListener *listener = *it;
                    listener->myGroupLeaveEvent(charID,isKick,isDisband);
                }
            }
            else
            {
                const bool isCreate = mReader.readBool();   //false = rename group
                const bool success = mReader.readBool();
                for(std::vector<NetworkListener*>::iterator it=mListeners.begin(); it!=mListeners.end(); it++)
                {
                    NetworkListener *listener = *it;
                    listener->myGroupEvent(isCreate,success);
                }
            }
        }
    }
    void sendViewGroupRequest(const bool &viewMembers, const unsigned int &groupID=0, const String &searchFilter="")
    {
        mWriter.create(ID_VIEWGROUP);
        mWriter.writeBool(viewMembers);
        if(viewMembers)mWriter.writeUInt(groupID);
        else mWriter.writeString(searchFilter);

        mWriter.send(mServerAdd,true);
    }
    void readViewGroupInfo(Packet *p)
    {
        mReader.load(p);
        const bool isViewMembers = mReader.readBool();
        const unsigned int id = mReader.readUInt();
        const String name = mReader.readString(32);
        const unsigned char rank = isViewMembers?mReader.readUChar():0;
        const String title = isViewMembers?mReader.readString(32):"";
        //const bool isOnline = isViewMembers?mReader.readBool():false;
        for(std::vector<NetworkListener*>::iterator it=mListeners.begin(); it!=mListeners.end(); it++)
        {
            NetworkListener *listener = *it;
            if(isViewMembers)listener->viewGroupMembersInfo(id,name,rank,title);
            else listener->viewGroupInfo(id,name);
        }
    }
    void sendGroupRequest(const unsigned int &groupID, const bool &isRequest)
    {
        mWriter.create(ID_GROUPREQUEST);
        mWriter.writeBool(false);       //is not accept/deny
        mWriter.writeUInt(groupID);     //charID for invite
        mWriter.writeBool(isRequest);

        mWriter.send(mServerAdd);
    }
    void sendAcceptGroupRequest(const unsigned int &groupID, const bool &isAccept, const bool &isRequest)
    {
        mWriter.create(ID_GROUPREQUEST);
        mWriter.writeBool(true);        //is accept/deny
        mWriter.writeUInt(groupID);     //charID for invite
        mWriter.writeBool(isRequest);
        mWriter.writeBool(isAccept);    //false = deny

        mWriter.send(mServerAdd);
    }
    void readGroupRequest(Packet *p)
    {
        mReader.load(p);
        const bool isRequest = mReader.readBool();
        const unsigned int id = mReader.readUInt();
        const String name = mReader.readString(32);
        for(std::vector<NetworkListener*>::iterator it=mListeners.begin(); it!=mListeners.end(); it++)
        {
            NetworkListener *listener = *it;
            listener->groupRequestInfo(id,name,isRequest);
        }
    }
    void sendGroupBioRequest(const unsigned int &groupID)
    {
        mWriter.create(ID_GROUPBIO);
        mWriter.writeBool(true);    //is request
        mWriter.writeUInt(groupID); //0 = my group

        mWriter.send(mServerAdd,true);
    }
    void sendGroupBioUpdate(const String &bio)
    {
        mWriter.create(ID_GROUPBIO);
        mWriter.writeBool(false);    //is update
        mWriter.writeString(bio);

        mWriter.send(mServerAdd);
    }
    void readGroupBio(Packet *p)
    {
        mReader.load(p);
        const String bio = mReader.readString(256);
        for(std::vector<NetworkListener*>::iterator it=mListeners.begin(); it!=mListeners.end(); it++)
        {
            NetworkListener *listener = *it;
            listener->groupBio(bio);
        }
    }
    void readMisc(Packet *p)
    {
        mReader.load(p);
        const unsigned char type = mReader.readUChar();
        if(type==0)
        {
            for(std::vector<NetworkListener*>::iterator it=mListeners.begin(); it!=mListeners.end(); it++)
            {
                NetworkListener *listener = *it;
                listener->duplicateLoginEvent(mReader.readString(16));
            }
        }
    }
    void sendReport(const unsigned int &targetID, const String &message)
    {
        mWriter.create(ID_MISC);
        mWriter.writeUChar(1);
        mWriter.writeUInt(targetID);
        mWriter.writeString(message);

        mWriter.send(mServerAdd,true);
    }
    void sendHome(const bool &requestHome, const bool &goHome, const bool &resetHome=false, const Vector3 &position=Vector3::ZERO)
    {
        mWriter.create(ID_HOME);
        mWriter.writeBool(requestHome);
        if(!requestHome)
        {
            mWriter.writeBool(goHome);  //false = set home
            if(!goHome)
            {
                mWriter.writeBool(resetHome);
                if(!resetHome)
                {
                    mWriter.writeFloat(position.x);
                    mWriter.writeFloat(position.y);
                    mWriter.writeFloat(position.z);
                }
            }
        }

        mWriter.send(mServerAdd,true);
    }
    void readHome(Packet *p)
    {
        mReader.load(p);
        const bool goHome = mReader.readBool();
        const String home = mReader.readString(32);
        for(std::vector<NetworkListener*>::iterator it=mListeners.begin(); it!=mListeners.end(); it++)
        {
            NetworkListener *listener = *it;
            if(!goHome)listener->homeInfo(home);
            else
            {
                Vector3 position;
                position.x = mReader.readInt();
                position.y = mReader.readInt();
                position.z = mReader.readInt();
                listener->goHomeEvent(home,position);
            }
        }
    }
    void sendLeaveParty()
    {
        mWriter.create(ID_PARTY);
        mWriter.writeBool(true);       //is leave = true

        mWriter.send(mServerAdd,true);
    }
    void readPartyInfo(Packet *p)
    {
        mReader.load(p);
        const bool isInfo = mReader.readBool();
        if(isInfo)
        {
            const unsigned int charID = mReader.readUInt();
            const String charname = mReader.readString(32);
            const bool announce = mReader.readBool();
            for(std::vector<NetworkListener*>::iterator it=mListeners.begin(); it!=mListeners.end(); it++)
            {
                NetworkListener *listener = *it;
                listener->partyMemberInfo(charID,charname,announce);
            }
        }
        //Is leave event
        else
        {
            const unsigned int charID = mReader.readUInt();
            for(std::vector<NetworkListener*>::iterator it=mListeners.begin(); it!=mListeners.end(); it++)
            {
                NetworkListener *listener = *it;
                listener->partyLeaveEvent(charID);
            }
        }
    }
    void sendPartyRequest(const unsigned int &charID, const bool &isAccept)
    {
        mWriter.create(ID_PARTYREQUEST);
        mWriter.writeBool(isAccept);
        mWriter.writeUInt(charID);

        mWriter.send(mServerAdd,true);
    }
    void readPartyRequest(Packet *p)
    {
        mReader.load(p);
        const bool isFail = mReader.readBool();
        const unsigned int id = mReader.readUInt();
        const String name = mReader.readString(32);
        for(std::vector<NetworkListener*>::iterator it=mListeners.begin(); it!=mListeners.end(); it++)
        {
            NetworkListener *listener = *it;
            listener->partyRequestEvent(id,name,isFail);
        }
    }
    void sendCharSkill(const unsigned char &nSkill, const bool &bFlag=false, const bool &bPersistent=false)
    {
        if(!mConnected)return;
        mWriter.create(ID_CHAR_SKILL);
        mWriter.writeUChar(nSkill);
        mWriter.writeBool(bFlag);
        if(bPersistent)mWriter.writeBool(true); //persistent skills are saved on server (to update arriving players)

        mWriter.send(mServerAdd,true);
    }
    void readCharSkill(Packet *p)
    {
        mReader.load(p);
        const unsigned int nCharID = mReader.readUInt();
        const unsigned char nSkill = mReader.readUChar();
        const bool bFlag = mReader.readBool();
        for(std::vector<NetworkListener*>::iterator it=mListeners.begin(); it!=mListeners.end(); it++)
        {
            NetworkListener *listener = *it;
            listener->charSkillEvent(nCharID,nSkill,bFlag);
        }
    }
};

template<> NetworkManager* Singleton<NetworkManager>::ms_Singleton = 0;

NetworkManager* NetworkManager::getSingletonPtr()
{
	return ms_Singleton;
}

NetworkManager& NetworkManager::getSingleton()
{
	assert(ms_Singleton);
	return *ms_Singleton;
}

#endif
