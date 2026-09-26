// ##################################################################################################
// ##------------------------------------------FILE START------------------------------------------##
// ##################################################################################################


#ifndef _NETWORKMANAGER_H_
#define _NETWORKMANAGER_H_

//#define DEBUG_LOUD true

#include "DataTypes.h"
#include "NetworkDefines.h"
#include "PacketReader.h"
#include "RakNetStatistics.h"
#include "RakSleep.h"
#include <iostream>
#include "Chat.h"
#include <stdlib.h>

using namespace std;
using namespace RakNet;

class NetworkListener
{
public:
	NetworkListener(){}
	virtual ~NetworkListener(){}
	virtual void tokenReceived(const string &token, const unsigned int &userID, const unsigned int &charID) = 0;
	virtual void connectEvent(const SystemAddress &add, const string &token, const bool &firstEntrance, const bool &ghostMode) = 0;
	virtual void disconnectEvent(const SystemAddress &add, const bool &isProper) = 0;
	virtual void terminateEvent() = 0;
	virtual void charMapChange(const SystemAddress &add, const string &map, const float &x, const float &y, const float &z) = 0;
	virtual void charGhostMapChange(const SystemAddress &add, const bool &unghost) = 0;
	virtual void charPosition(const SystemAddress &add, const float &x, const float &y, const float &z) = 0;
	virtual void chatEvent(const SystemAddress &add, const string &caption, const unsigned char &channel, const string &whisperTarget) = 0;
	virtual void chatRelayEvent(const unsigned int &userID, const unsigned int &charID, const string &caption, const unsigned char &channel) = 0;
	virtual void charYaw(const SystemAddress &add, const unsigned char &yaw) = 0;
	virtual void charHeadDir(const SystemAddress &add, const unsigned char &ew, const unsigned char &ns) = 0;
	virtual void charAction(const SystemAddress &add, const unsigned char &action) = 0;
	virtual void charStatusFlags(const SystemAddress &add, const bool &ag, const bool &s) = 0;
	virtual void charMoveModifier(const SystemAddress &add, const unsigned char &modifier) = 0;
	virtual void charEmote(const SystemAddress &add, const unsigned char &emote, const bool &bSetDefault) = 0;
	virtual void charJump(const SystemAddress &add, const bool &jump) = 0;
	virtual void movieEvent(const SystemAddress &add, const string &movie, const float &camYaw) = 0;
	virtual void friendAddEvent(const SystemAddress &add, const bool &isBlock, const string &username) = 0;
	virtual void friendRemoveEvent(const SystemAddress &add, const bool &isBlock, const unsigned int &userID) = 0;
	virtual void friendAcceptEvent(const SystemAddress &add, const unsigned int &userID, const bool &accept) = 0;
	virtual void friendInfoRelayEvent(const bool &isRequest, const unsigned int &senderID, const unsigned int &targetID, const bool &isRemove) = 0;
	virtual void blockedByInfoRelayEvent(const unsigned int &senderID, const unsigned int &targetID, const bool &isRemove) = 0;
	virtual void charOnlineStatus(const unsigned int &userID, const unsigned int &charID, const bool &isOnline) = 0;
	virtual void findCharEvent(const SystemAddress &add, const unsigned int &id, const bool &isUserID) = 0;
	virtual void charBioUpdate(const SystemAddress &add, const string &bio) = 0;
	virtual void charBioRequestEvent(const SystemAddress &add, const unsigned int &charID, const bool &isBio, const bool &requestUserID) = 0;
	virtual void modEvent(const SystemAddress &add, const string &command) = 0;
	virtual void myGroupEvent(const SystemAddress &add, const string &groupname) = 0;
	virtual void myGroupRelayEvent(const unsigned int &groupID, const string &groupname) = 0;
	virtual void myGroupLeaveEvent(const SystemAddress &add, unsigned int charID, const bool &isKick) = 0;
	virtual void myGroupLeaveRelayEvent(const unsigned int &groupID, const unsigned int &charID, const bool &isKick, const bool &isDisband) = 0;
	virtual void viewGroupsEvent(const SystemAddress &add, const string &filter) = 0;
	virtual void viewGroupMembersEvent(const SystemAddress &add, const unsigned int &groupID) = 0;
	virtual void groupRequestEvent(const SystemAddress &add, const unsigned int &id, const bool &isRequest) = 0;
	virtual void groupRequestRelayEvent(const unsigned int &senderID, const unsigned int &targetID, const bool &isRequest) = 0;
	virtual void groupAcceptEvent(const SystemAddress &add, const unsigned int &id, const bool &isRequest, const bool &isAccept) = 0;
	virtual void groupAcceptRelayEvent(const unsigned int &charID, const unsigned int &groupID) = 0;
	virtual void groupBioRequest(const SystemAddress &add, const unsigned int &groupID) = 0;
	virtual void groupBioUpdate(const SystemAddress &add, const string &bio) = 0;
	virtual void reportEvent(const SystemAddress &add, const unsigned int &targetID, const string &message) = 0;
	virtual void duplicateLoginEvent(unsigned int charID) = 0;
	virtual void homeEvent(const SystemAddress &add, const bool &isRequest) = 0;
	virtual void setHomeEvent(const SystemAddress &add, const bool &isReset, const float &x, const float &y, const float &z) = 0;
	virtual void partyLeaveEvent(const SystemAddress &add) = 0;
	virtual void partyLeaveRelayEvent(const unsigned int &charID, const unsigned int &partyID) = 0;
	virtual void partyRequestEvent(const SystemAddress &add, const unsigned int &charID, const bool &isAccept) = 0;
	virtual void partyRequestRelayEvent(const unsigned int &senderID, const unsigned int &targetID, const bool &isAccept) = 0;
	virtual void partyCreateRelayEvent(const unsigned int &senderID, const unsigned int &targetID, const unsigned int &partyID) = 0;
	virtual void charSkill(const SystemAddress &add, const unsigned char &nSkill, const bool &bFlag, const bool &bPersistent) = 0;
};

class NetworkManager
{
private:
	NetworkListener *mListener;
	RakPeerInterface *mServer;
	PacketReader mReader;
	PacketWriter mWriter;
	SystemAddress mMainServerAdd;
	bool DEBUG_LOUD;
public:
	NetworkManager(NetworkListener *listener)
	{
		mListener = listener;
		mServer = RakPeerInterface::GetInstance();
		mServer->SetIncomingPassword(SERVER_PASSWORD, (int)strlen(SERVER_PASSWORD));
		mWriter.setPeer(mServer);
		mMainServerAdd = UNASSIGNED_SYSTEM_ADDRESS;
		DEBUG_LOUD = true;
	}
	~NetworkManager()
	{
		RakPeerInterface::DestroyInstance(mServer);
	}
	const bool init(const unsigned short &port)
	{
		//puts("Starting MapServer");
		SocketDescriptor socketDesc(port, 0);
		const int flag = mServer->Startup(MAX_CLIENTS, &socketDesc, 1);
		mServer->SetMaximumIncomingConnections(MAX_CLIENTS);
		if (flag == 0)
		{
			//puts("MapServer started, connecting to main server.");
			//mServer->Connect(MAIN_SERVER_IP, MAIN_SERVER_PORT, SERVER_PASSWORD, (int)strlen(SERVER_PASSWORD));
			mServer->Connect("127.0.0.1", MAIN_SERVER_PORT, SERVER_PASSWORD, (int)strlen(SERVER_PASSWORD));
		}
		else
		{
			puts("MapServer failed to start.  Terminating.");
			return false;
		}
		mServer->SetTimeoutTime(60000, UNASSIGNED_SYSTEM_ADDRESS);
		mServer->SetUnreliableTimeout(10000);
		//mServer->SetOccasionalPing(true);
		//printf("Max clients allowed: %i\n", MAX_CLIENTS);
		return true;
	}
	void shutdown()
	{
		mServer->Shutdown(300);
	}
	void toggleDebug()
	{
		DEBUG_LOUD = !DEBUG_LOUD;
	}
	void printStats()
	{
		char temp[2048] = "";
		RakNetStatistics *rss = mServer->GetStatistics(mServer->GetSystemAddressFromIndex(0));
		StatisticsToString(rss, temp, 2);
		printf("%s", temp);
		printf("Ping %i\n", mServer->GetAveragePing(mServer->GetSystemAddressFromIndex(0)));
	}
	void update()
	{
		RakSleep(1);
		for (Packet *p = mServer->Receive(); p; mServer->DeallocatePacket(p), p = mServer->Receive())
		{
			switch (p->data[0])
			{
				//Main server replies
			case ID_CONNECTION_REQUEST_ACCEPTED:
				if (DEBUG_LOUD)printf("ID_CONNECTION_REQUEST_ACCEPTED from %s\n", p->systemAddress.ToString());
				mMainServerAdd = p->systemAddress;
				mWriter.create(ID_SERVER_CONNECT_MAIN);
				mWriter.send(mMainServerAdd);
				break;
			case ID_CONNECTION_ATTEMPT_FAILED:
			case ID_ALREADY_CONNECTED:
			case ID_CONNECTION_BANNED:
			case ID_NO_FREE_INCOMING_CONNECTIONS:
			case ID_INVALID_PASSWORD:
			case ID_INCOMPATIBLE_PROTOCOL_VERSION:
				if (DEBUG_LOUD)printf("ID_CONNECTION_ATTEMPT_FAILED from %s\n", p->systemAddress.ToString());
				mListener->terminateEvent();
				break;
			case ID_NEW_INCOMING_CONNECTION:
				if (DEBUG_LOUD)printf("ID_NEW_INCOMING_CONNECTION from %s\n", p->systemAddress.ToString());
				break;

			case ID_DISCONNECTION_NOTIFICATION:
				if (DEBUG_LOUD)printf("ID_DISCONNECTION_NOTIFICATION from %s\n", p->systemAddress.ToString());
				if (p->systemAddress == mMainServerAdd)mListener->terminateEvent();
				else mListener->disconnectEvent(p->systemAddress, true);
				break;

			case ID_CONNECTION_LOST:
				if (DEBUG_LOUD)printf("ID_CONNECTION_LOST from %s\n", p->systemAddress.ToString());
				if (p->systemAddress == mMainServerAdd)mListener->terminateEvent();
				else mListener->disconnectEvent(p->systemAddress, false);
				break;

			case ID_TOKEN:
				if (DEBUG_LOUD)printf("ID_TOKEN from %s\n", p->systemAddress.ToString());
				readToken(p);
				break;

			case ID_CONNECT_MAP:
				if (DEBUG_LOUD)printf("ID_CONNECT_MAP from %s\n", p->systemAddress.ToString());
				readConnect(p);
				break;

			case ID_CHAR_MAPCHANGE:
				if (DEBUG_LOUD)printf("ID_CHAR_MAPCHANGE from %s\n", p->systemAddress.ToString());
				readMapChange(p);
				break;

			case ID_CHAR_POSITION:
				if (DEBUG_LOUD)printf("ID_CHAR_POSITION from %s\n", p->systemAddress.ToString());
				readCharPosition(p);
				break;

			case ID_CHAT:
				if (false && DEBUG_LOUD)printf("ID_CHAT from %s\n", p->systemAddress.ToString());
				if (p->systemAddress == mMainServerAdd)readChatRelay(p);
				else readChat(p);
				break;

			case ID_CHAR_YAW:
				if (DEBUG_LOUD)printf("ID_CHAR_YAW from %s\n", p->systemAddress.ToString());
				readCharYaw(p);
				break;

			case ID_CHAR_HEADDIR:
				if (DEBUG_LOUD)printf("ID_CHAR_HEADDIR from %s\n", p->systemAddress.ToString());
				readCharHeadDir(p);
				break;

			case ID_CHAR_ACTION:
				if (DEBUG_LOUD)printf("ID_CHAR_ACTION from %s\n", p->systemAddress.ToString());
				readCharAction(p);
				break;

			case ID_CHAR_STATUSFLAGS:
				if (DEBUG_LOUD)printf("ID_CHAR_STATUSFLAGS from %s\n", p->systemAddress.ToString());
				readCharStatusFlags(p);
				break;

			case ID_CHAR_MOVEMODIFIER:
				if (DEBUG_LOUD)printf("ID_CHAR_MOVEMODIFIER from %s\n", p->systemAddress.ToString());
				readCharMoveModifier(p);
				break;

			case ID_CHAR_EMOTE:
				if (DEBUG_LOUD)printf("ID_CHAR_EMOTE from %s\n", p->systemAddress.ToString());
				readCharEmote(p);
				break;

			case ID_CHAR_JUMP:
				if (DEBUG_LOUD)printf("ID_CHAR_JUMP from %s\n", p->systemAddress.ToString());
				readCharJump(p);
				break;

			case ID_MOVIE:
				if (DEBUG_LOUD)printf("ID_MOVIE from %s\n", p->systemAddress.ToString());
				readMovie(p);
				break;

			case ID_FRIENDADD:
				if (DEBUG_LOUD)printf("ID_FRIENDADD from %s\n", p->systemAddress.ToString());
				readFriendAdd(p);
				break;

			case ID_FRIEND:
				if (DEBUG_LOUD)printf("ID_FRIEND from %s\n", p->systemAddress.ToString());
				if (p->systemAddress == mMainServerAdd)readFriendInfoRelay(p);
				else readFriendAccept(p);
				break;

			case ID_CHAR_ONLINESTATUS:
				if (DEBUG_LOUD)printf("ID_CHAR_ONLINESTATUS from %s\n", p->systemAddress.ToString());
				readCharOnlineStatus(p);
				break;

			case ID_FINDCHAR:
				if (DEBUG_LOUD)printf("ID_FINDCHAR from %s\n", p->systemAddress.ToString());
				readFindChar(p);
				break;

			case ID_CHAR_BIO:
				if (DEBUG_LOUD)printf("ID_CHAR_BIO from %s\n", p->systemAddress.ToString());
				readCharBio(p);
				break;

			case ID_MOD:
				if (DEBUG_LOUD)printf("ID_MOD from %s\n", p->systemAddress.ToString());
				readMod(p);
				break;

			case ID_MYGROUP:
				if (DEBUG_LOUD)printf("ID_MYGROUP from %s\n", p->systemAddress.ToString());
				if (p->systemAddress == mMainServerAdd)readMyGroupRelay(p);
				else readMyGroup(p);
				break;

			case ID_VIEWGROUP:
				if (DEBUG_LOUD)printf("ID_VIEWGROUP from %s\n", p->systemAddress.ToString());
				readViewGroup(p);
				break;

			case ID_GROUPREQUEST:
				if (DEBUG_LOUD)printf("ID_GROUPREQUEST from %s\n", p->systemAddress.ToString());
				if (p->systemAddress == mMainServerAdd)readGroupRequestRelay(p);
				else readGroupRequest(p);
				break;

			case ID_GROUPBIO:
				if (DEBUG_LOUD)printf("ID_GROUPBIO from %s\n", p->systemAddress.ToString());
				readGroupBio(p);
				break;

			case ID_MISC:
				if (DEBUG_LOUD)printf("ID_MISC from %s\n", p->systemAddress.ToString());
				readMisc(p);
				break;

			case ID_HOME:
				if (DEBUG_LOUD)printf("ID_HOME from %s\n", p->systemAddress.ToString());
				readHome(p);
				break;

			case ID_PARTY:
				if (DEBUG_LOUD)printf("ID_PARTY from %s\n", p->systemAddress.ToString());
				if (p->systemAddress == mMainServerAdd)readPartyRelay(p);
				else readParty(p);
				break;

			case ID_PARTYREQUEST:
				if (DEBUG_LOUD)printf("ID_PARTYREQUEST from %s\n", p->systemAddress.ToString());
				if (p->systemAddress == mMainServerAdd)readPartyRequestRelay(p);
				else readPartyRequest(p);
				break;

			case ID_CHAR_SKILL:
				if (DEBUG_LOUD)printf("ID_CHAR_SKILL from %s\n", p->systemAddress.ToString());
				readCharSkill(p);
				break;

			default: cout << "Packet received" << endl; break;
			}
		}
	}
	void readToken(Packet *p)
	{
		mReader.load(p);
		const string token = mReader.readString(16);
		const unsigned int userID = mReader.readUInt();
		const unsigned int charID = mReader.readUInt();
		mListener->tokenReceived(token, userID, charID);
	}
	void sendTokenFail(const SystemAddress &add)
	{
		mWriter.create(ID_TOKEN_FAIL);
		mWriter.send(add);
	}
	void readConnect(Packet *p)
	{
		mReader.load(p);
		const string token = mReader.readString(16);
		const bool firstEntrance = mReader.readBool();
		const bool ghostMode = mReader.readBool();
		mListener->connectEvent(p->systemAddress, token, firstEntrance, ghostMode);
	}
	void sendConnect(const SystemAddress &add, const int &x, const int &y, const int &z)
	{
		mWriter.create(ID_CONNECT_MAP);
		mWriter.writeInt(x);
		mWriter.writeInt(y);
		mWriter.writeInt(z);

		mWriter.send(add);
	}
	void readMapChange(Packet *p)
	{
		mReader.load(p);
		const bool ghostMode = mReader.readBool();
		if (ghostMode)
		{
			const bool unghost = mReader.readBool();
			mListener->charGhostMapChange(p->systemAddress, unghost);
			return;
		}
		const string mapName = mReader.readString(32);
		const float x = mReader.readFloat();
		const float y = mReader.readFloat();
		const float z = mReader.readFloat();

		mListener->charMapChange(p->systemAddress, mapName, x, y, z);
	}
	void sendCharInfo(const SystemAddress &add, const unsigned int &charID, CharacterData *data, const float &x, const float &y, const float &z, const unsigned int &userID, const string &username)
	{
		mWriter.create(ID_CHAR_INFO);
		mWriter.writeUInt(charID);

		mWriter.writeString(data->mName);
		mWriter.writeChar(data->mSpecies);
		for (int i = 0; i<MAX_MESHES; i++)mWriter.writeUChar(data->mMesh[i]);
		for (int i = 0; i<MAX_MATS; i++)mWriter.writeUChar(data->mMat[i]);
		for (int i = 0; i<NUM_SCALABLES; i++)mWriter.writeChar(data->mScale[i]);
		for (int i = 0; i<NUM_COLOURABLES; i++)
			for (int j = 0; j<3; j++)mWriter.writeUChar(data->mColour[i][j]);
		mWriter.writeUChar(data->mEmote);
		mWriter.writeUChar(data->mPreset);
		mWriter.writeUChar(data->m_nEquip);
		mWriter.writeString(data->m_szItems);

		mWriter.writeFloat(x);
		mWriter.writeFloat(y);
		mWriter.writeFloat(z);
		mWriter.writeUInt(userID);
		mWriter.writeString(username);

		mWriter.send(add);
	}
	void sendMainCharInfo(const string &token, const bool &isFirstEntrance)
	{
		mWriter.create(ID_CHAR_INFO);
		mWriter.writeString(token);
		mWriter.writeBool(isFirstEntrance);

		mWriter.send(mMainServerAdd);
	}
	void sendCharExit(const SystemAddress &add, const unsigned int &charID, const bool &isProper, const bool &isMapChange)
	{
		mWriter.create(ID_CHAR_EXIT);
		mWriter.writeUInt(charID);
		mWriter.writeBool(isProper);
		mWriter.writeBool(isMapChange);

		mWriter.send(add);
	}
	void sendMainCharExit(const string &token, const bool &isChangingMap)
	{
		mWriter.create(ID_CHAR_EXIT);
		mWriter.writeString(token);
		mWriter.writeBool(isChangingMap);

		mWriter.send(mMainServerAdd);
	}
	void readCharPosition(Packet *p)
	{
		mReader.load(p);
		const float x = mReader.readFloat();
		const float y = mReader.readFloat();
		const float z = mReader.readFloat();
		mListener->charPosition(p->systemAddress, x, y, z);
	}
	void sendCharPosition(const SystemAddress &add, const unsigned int &charID, const float &x, const float &y, const float &z)
	{
		mWriter.create(ID_CHAR_POSITION);
		mWriter.writeUInt(charID);
		mWriter.writeFloat(x);
		mWriter.writeFloat(y);
		mWriter.writeFloat(z);

		mWriter.sendUnreliable(add);
	}
	void readChat(Packet *p)
	{
		mReader.load(p);
		const string caption = mReader.readString();
		const unsigned char channel = mReader.readUChar();
		const string whisperTarget = (channel == CHAT_WHISPER ? mReader.readString() : "");
		mListener->chatEvent(p->systemAddress, caption, channel, whisperTarget);
	}
	void sendChat(const SystemAddress &add, const unsigned int &charID, const string &caption, const unsigned char &channel, const unsigned int &userID = 0, const string &whisperer = "", const bool &fail = false, const bool &isOffline = false)
	{
		mWriter.create(ID_CHAT);
		mWriter.writeUInt(charID);
		mWriter.writeString(caption);
		mWriter.writeUChar(channel);
		if (userID != 0 || whisperer != "")mWriter.writeUInt(userID);
		if (whisperer != "")
		{
			mWriter.writeString(whisperer);
			if (fail)
			{
				mWriter.writeBool(true);
				mWriter.writeBool(isOffline);
			}
		}

		mWriter.send(add);
	}
	void relayChat(const unsigned int &userID, const unsigned int &charID, const string &caption, const unsigned char &channel)
	{
		mWriter.create(ID_CHAT);
		mWriter.writeUInt(userID);
		mWriter.writeUInt(charID);
		mWriter.writeString(caption);
		mWriter.writeUChar(channel);

		mWriter.send(mMainServerAdd);
	}
	void readChatRelay(Packet *p)
	{
		mReader.load(p);
		const unsigned int userID = mReader.readUInt();
		const unsigned int charID = mReader.readUInt();
		const string caption = mReader.readString();
		const unsigned char channel = mReader.readUChar();
		mListener->chatRelayEvent(userID, charID, caption, channel);
	}
	void readCharYaw(Packet *p)
	{
		mReader.load(p);
		mListener->charYaw(p->systemAddress, mReader.readUChar());
	}
	void sendCharYaw(const SystemAddress &add, const unsigned int &charID, const unsigned char &yaw)
	{
		mWriter.create(ID_CHAR_YAW);
		mWriter.writeUInt(charID);
		mWriter.writeUChar(yaw);

		mWriter.sendUnreliable(add);
	}
	void readCharHeadDir(Packet *p)
	{
		mReader.load(p);
		const unsigned char ew = mReader.readUChar();
		const unsigned char ns = mReader.readUChar();
		mListener->charHeadDir(p->systemAddress, ew, ns);
	}
	void sendCharHeadDir(const SystemAddress &add, const unsigned int &charID, const unsigned char &ew, const unsigned char &ns)
	{
		mWriter.create(ID_CHAR_HEADDIR);
		mWriter.writeUInt(charID);
		mWriter.writeUChar(ew);
		mWriter.writeUChar(ns);

		mWriter.sendUnreliable(add);
	}
	void readCharAction(Packet *p)
	{
		mReader.load(p);
		mListener->charAction(p->systemAddress, mReader.readUChar());
	}
	void sendCharAction(const SystemAddress &add, const unsigned int &charID, const unsigned char &action)
	{
		mWriter.create(ID_CHAR_ACTION);
		mWriter.writeUInt(charID);
		mWriter.writeUChar(action);

		mWriter.sendUnreliable(add);
	}
	void readCharStatusFlags(Packet *p)
	{
		mReader.load(p);
		const bool ag = mReader.readBool();
		const bool s = mReader.readBool();
		mListener->charStatusFlags(p->systemAddress, ag, s);
	}
	void sendCharStatusFlags(const SystemAddress &add, const unsigned int &charID, const bool &ag, const bool &s)
	{
		mWriter.create(ID_CHAR_STATUSFLAGS);
		mWriter.writeUInt(charID);
		mWriter.writeBool(ag);
		mWriter.writeBool(s);

		mWriter.sendUnreliable(add);
	}
	void readCharMoveModifier(Packet *p)
	{
		mReader.load(p);
		const unsigned char modifier = mReader.readUChar();
		mListener->charMoveModifier(p->systemAddress, modifier);
	}
	void sendCharMoveModifier(const SystemAddress &add, const unsigned int &charID, const unsigned char &modifier)
	{
		mWriter.create(ID_CHAR_MOVEMODIFIER);
		mWriter.writeUInt(charID);
		mWriter.writeUChar(modifier);

		mWriter.send(add);
	}
	void readCharEmote(Packet *p)
	{
		mReader.load(p);
		const unsigned char nEmote = mReader.readUChar();
		const bool bSetDefault = mReader.readBool();
		mListener->charEmote(p->systemAddress, nEmote, bSetDefault);
	}
	void sendCharEmote(const SystemAddress &add, const unsigned int &charID, const unsigned char &emote)
	{
		mWriter.create(ID_CHAR_EMOTE);
		mWriter.writeUInt(charID);
		mWriter.writeUChar(emote);

		mWriter.send(add);
	}
	void readCharJump(Packet *p)
	{
		mReader.load(p);
		mListener->charJump(p->systemAddress, mReader.readBool());
	}
	void sendCharJump(const SystemAddress &add, const unsigned int &charID, const bool &jump)
	{
		mWriter.create(ID_CHAR_JUMP);
		mWriter.writeUInt(charID);
		mWriter.writeBool(jump);

		mWriter.send(add);
	}
	void readMovie(Packet *p)
	{
		mReader.load(p);
		const string movie = mReader.readString();
		const float camYaw = mReader.readFloat();
		mListener->movieEvent(p->systemAddress, movie, camYaw);
	}
	void sendMovie(const SystemAddress &add, const unsigned int &charID, const string &movie, const float camYaw)
	{
		mWriter.create(ID_MOVIE);
		mWriter.writeUInt(charID);
		mWriter.writeString(movie);
		mWriter.writeFloat(camYaw);

		mWriter.send(add);
	}
	void readFriendAdd(Packet *p)
	{
		mReader.load(p);
		const bool isBlock = mReader.readBool();
		const bool add = mReader.readBool();
		if (add)
		{
			const string username = mReader.readString();
			mListener->friendAddEvent(p->systemAddress, isBlock, username);
		}
		else
		{
			const unsigned int userID = mReader.readUInt();
			mListener->friendRemoveEvent(p->systemAddress, isBlock, userID);
		}
	}
	void sendFriendAdd(const SystemAddress &add, const bool &isBlock, const bool &success, const string &username)
	{
		mWriter.create(ID_FRIENDADD);
		mWriter.writeBool(isBlock);
		mWriter.writeBool(success);
		mWriter.writeString(username);

		mWriter.send(add);
	}
	void readFriendAccept(Packet *p)
	{
		mReader.load(p);
		const unsigned int userID = mReader.readUInt();
		const bool accept = mReader.readBool();
		mListener->friendAcceptEvent(p->systemAddress, userID, accept);
	}
	void sendBlockOrRequestInfo(const SystemAddress &add, const bool &isRequest, const unsigned int &userID, const string &username)
	{
		mWriter.create(ID_FRIEND);
		mWriter.writeBool(false);
		mWriter.writeBool(true);
		mWriter.writeUInt(userID);
		mWriter.writeBool(isRequest);
		if (!isRequest)mWriter.writeBool(false);
		mWriter.writeString(username);

		mWriter.send(add);
	}
	void sendBlockedByInfo(const SystemAddress &add, const unsigned int &userID, const bool &isRemove)
	{
		mWriter.create(ID_FRIEND);
		mWriter.writeBool(isRemove);
		mWriter.writeBool(true);
		mWriter.writeUInt(userID);
		if (!isRemove)
		{
			mWriter.writeBool(false);
			mWriter.writeBool(true);
		}

		mWriter.send(add);
	}
	void readFriendInfoRelay(Packet *p)
	{
		mReader.load(p);
		const bool isRemove = mReader.readBool();
		const bool isBlockedBy = mReader.readBool();
		const bool isRequest = ((isBlockedBy || isRemove) ? false : mReader.readBool());
		const unsigned int senderID = mReader.readUInt();
		const unsigned int targetID = mReader.readUInt();
		if (isBlockedBy)mListener->blockedByInfoRelayEvent(senderID, targetID, isRemove);
		else mListener->friendInfoRelayEvent(isRequest, senderID, targetID, isRemove);
	}
	void sendFriendInfo(const SystemAddress &add, const unsigned int &userID, const bool isRemove, const bool &isOnline = false, const bool &isUpdate = false, const string &username = "")
	{
		mWriter.create(ID_FRIEND);
		mWriter.writeBool(isRemove);
		mWriter.writeBool(false);
		mWriter.writeUInt(userID);
		if (!isRemove)
		{
			mWriter.writeBool(isOnline);
			mWriter.writeBool(isUpdate);
			if (!isUpdate)mWriter.writeString(username);
		}

		mWriter.send(add);
	}
	void relayFriendInfo(const bool &isRemove, const bool &isBlockedBy, const unsigned int &senderID, const unsigned int &targetID, const bool &isRequest = false)
	{
		mWriter.create(ID_FRIEND);
		mWriter.writeBool(isRemove);
		mWriter.writeBool(isBlockedBy);
		if (!isBlockedBy && !isRemove)mWriter.writeBool(isRequest);
		mWriter.writeUInt(senderID);
		mWriter.writeUInt(targetID);

		mWriter.send(mMainServerAdd);
	}
	void readCharOnlineStatus(Packet *p)
	{
		mReader.load(p);
		const unsigned int userID = mReader.readUInt();
		const unsigned int charID = mReader.readUInt();
		const bool isOnline = mReader.readBool();
		mListener->charOnlineStatus(userID, charID, isOnline);
	}
	void broadcastCharOnlineStatus(const unsigned int &userID, const unsigned int &charID, const bool &isOnline)
	{
		mWriter.create(ID_CHAR_ONLINESTATUS);
		mWriter.writeUInt(userID);
		mWriter.writeUInt(charID);
		mWriter.writeBool(isOnline);
		mWriter.sendAllBut(mMainServerAdd);
	}
	void readFindChar(Packet *p)
	{
		mReader.load(p);
		const unsigned int id = mReader.readUInt();
		const bool isUserID = mReader.readBool();
		mListener->findCharEvent(p->systemAddress, id, isUserID);
	}
	void sendFindChar(const SystemAddress &add, const string &name, const string &mapName)
	{
		mWriter.create(ID_FINDCHAR);
		mWriter.writeString(name);
		mWriter.writeString(mapName);

		mWriter.send(add);
	}
	void readCharBio(Packet *p)
	{
		mReader.load(p);

		const bool isRequest = mReader.readBool();
		if (!isRequest)
		{
			const string bio = mReader.readString(1024);
			mListener->charBioUpdate(p->systemAddress, bio);
		}
		else
		{
			const unsigned int charID = mReader.readUInt();
			const bool isBio = mReader.readBool();
			const bool requestUserID = (!isBio ? mReader.readBool() : false);
			mListener->charBioRequestEvent(p->systemAddress, charID, isBio, requestUserID);
		}
	}
	void sendCharBioInfo(const SystemAddress &add, const string &username, const string &charname, const string &group, const unsigned int &groupID, const unsigned int &userID)
	{
		mWriter.create(ID_CHAR_BIO);
		mWriter.writeBool(false);
		mWriter.writeString(username);
		mWriter.writeString(charname);
		mWriter.writeString(group);
		mWriter.writeUInt(groupID);
		if (userID)mWriter.writeUInt(userID);

		mWriter.send(add);
	}
	void sendCharBio(const SystemAddress &add, const bool &isOwn, const string &bio)
	{
		mWriter.create(ID_CHAR_BIO);
		mWriter.writeBool(true);
		mWriter.writeBool(isOwn);
		mWriter.writeString(bio);

		mWriter.send(add);
	}
	void readMod(Packet *p)
	{
		mReader.load(p);

		const string command = mReader.readString(256);
		mListener->modEvent(p->systemAddress, command);
	}
	void sendMod(const SystemAddress &add, const string &command, const bool &bIsSender = true)
	{
		mWriter.create(ID_MOD);
		mWriter.writeBool(bIsSender);
		mWriter.writeString(command);

		mWriter.send(add);
	}
	void readMyGroup(Packet *p)
	{
		mReader.load(p);
		const bool isCreate = mReader.readBool();
		if (isCreate)
		{
			const string groupname = mReader.readString(32);
			mListener->myGroupEvent(p->systemAddress, groupname);
		}
		else
		{
			const unsigned int charID = mReader.readUInt();
			const bool isKick = mReader.readBool();
			mListener->myGroupLeaveEvent(p->systemAddress, charID, isKick);
		}
	}
	void sendMyGroupInfo(const SystemAddress &add, const string &groupName, const bool &isLeader = false)
	{
		mWriter.create(ID_MYGROUP);
		mWriter.writeBool(true);        //is info
		mWriter.writeBool(false);       //is not member info
		const bool hasGroup = (groupName != "");
		mWriter.writeBool(hasGroup);
		if (hasGroup)
		{
			mWriter.writeString(groupName);
			mWriter.writeBool(isLeader);
		}

		mWriter.send(add);
	}
	void sendMyGroupMemberInfo(const SystemAddress &add, const GroupMemberInfo &info, const bool &announce = false)
	{
		mWriter.create(ID_MYGROUP);
		mWriter.writeBool(true);        //is info
		mWriter.writeBool(true);        //is member info
		mWriter.writeUInt(info.mCharID);
		mWriter.writeString(info.mCharName);
		mWriter.writeUChar(info.mRank);
		mWriter.writeString(info.mTitle);
		mWriter.writeBool(info.mIsOnline);
		if (announce)mWriter.writeBool(true);

		mWriter.send(add);
	}
	void relayMyGroupInfo(const unsigned int &groupID, const string &groupname)
	{
		mWriter.create(ID_MYGROUP);
		mWriter.writeUInt(groupID);
		mWriter.writeBool(false);   //is not leave event
		mWriter.writeString(groupname);

		mWriter.send(mMainServerAdd);
	}
	void readMyGroupRelay(Packet *p)
	{
		mReader.load(p);
		const unsigned int groupID = mReader.readUInt();
		const bool isLeave = mReader.readBool();
		if (!isLeave)
		{
			const string groupname = mReader.readString(32);
			mListener->myGroupRelayEvent(groupID, groupname);
		}
		else
		{
			const unsigned int charID = mReader.readUInt();
			const bool isKick = mReader.readBool();
			const bool isDisband = mReader.readBool();
			mListener->myGroupLeaveRelayEvent(groupID, charID, isKick, isDisband);
		}
	}
	void sendMyGroupLeave(const SystemAddress &add, const unsigned int &charID, const bool &isKick, const bool &isDisband)
	{
		mWriter.create(ID_MYGROUP);
		mWriter.writeBool(false);    //is event
		mWriter.writeBool(true);    //is leave event
		mWriter.writeUInt(charID);
		mWriter.writeBool(isKick);
		if (isDisband)mWriter.writeBool(true);

		mWriter.send(add);
	}
	void relayMyGroupLeave(const unsigned int &groupID, const unsigned int &charID, const bool &isKick, const bool &isDisband)
	{
		mWriter.create(ID_MYGROUP);
		mWriter.writeUInt(groupID);
		mWriter.writeBool(true);    //is leave event
		mWriter.writeUInt(charID);
		mWriter.writeBool(isKick);
		if (isDisband)mWriter.writeBool(true);

		mWriter.send(mMainServerAdd);
	}
	void sendMyGroupEvent(const SystemAddress &add, const bool &isCreate, const bool &success)
	{
		mWriter.create(ID_MYGROUP);
		mWriter.writeBool(false);    //is event
		mWriter.writeBool(false);    //is not leave event
		mWriter.writeBool(isCreate);
		mWriter.writeBool(success);

		mWriter.send(add);
	}
	void readViewGroup(Packet *p)
	{
		mReader.load(p);
		const bool isViewMembers = mReader.readBool();
		if (!isViewMembers)
		{
			const string filter = mReader.readString(32);
			mListener->viewGroupsEvent(p->systemAddress, filter);
		}
		else
		{
			const unsigned int groupID = mReader.readUInt();
			mListener->viewGroupMembersEvent(p->systemAddress, groupID);
		}
	}
	void sendViewGroupInfo(const SystemAddress &add, const unsigned int &groupID, const string &groupname)
	{
		mWriter.create(ID_VIEWGROUP);
		mWriter.writeBool(false);    //is not member info
		mWriter.writeUInt(groupID);
		mWriter.writeString(groupname);

		mWriter.send(add);
	}
	void sendViewGroupMemberInfo(const SystemAddress &add, const GroupMemberInfo &info)
	{
		mWriter.create(ID_VIEWGROUP);
		mWriter.writeBool(true);        //is member info
		mWriter.writeUInt(info.mCharID);
		mWriter.writeString(info.mCharName);
		mWriter.writeUChar(info.mRank);
		mWriter.writeString(info.mTitle);
		mWriter.writeBool(info.mIsOnline);

		mWriter.send(add);
	}
	void readGroupRequest(Packet *p)
	{
		mReader.load(p);
		const bool isAcceptDeny = mReader.readBool();
		const unsigned int id = mReader.readUInt();
		const bool isRequest = mReader.readBool();
		if (!isAcceptDeny)mListener->groupRequestEvent(p->systemAddress, id, isRequest);
		else
		{
			const bool isAccept = mReader.readBool();
			mListener->groupAcceptEvent(p->systemAddress, id, isRequest, isAccept);
		}
	}
	void sendGroupRequest(const SystemAddress &add, const unsigned int &id, const string &name, const bool &isRequest)
	{
		mWriter.create(ID_GROUPREQUEST);
		mWriter.writeBool(isRequest);
		mWriter.writeUInt(id);
		mWriter.writeString(name);

		mWriter.send(add);
	}
	void relayGroupRequest(const unsigned int &senderID, const unsigned int &targetID, const bool &isAccept, const bool &isRequest = true)
	{
		mWriter.create(ID_GROUPREQUEST);
		mWriter.writeBool(isAccept);
		mWriter.writeUInt(senderID);
		mWriter.writeUInt(targetID);
		mWriter.writeBool(isRequest);

		mWriter.send(mMainServerAdd);
	}
	void readGroupRequestRelay(Packet *p)
	{
		mReader.load(p);
		const bool isAccept = mReader.readBool();
		const unsigned int senderID = mReader.readUInt();
		const unsigned int targetID = mReader.readUInt();
		if (isAccept)mListener->groupAcceptRelayEvent(senderID, targetID);
		else
		{
			const bool isRequest = mReader.readBool();
			mListener->groupRequestRelayEvent(senderID, targetID, isRequest);
		}
	}
	void readGroupBio(Packet *p)
	{
		mReader.load(p);
		const bool isRequest = mReader.readBool();
		if (isRequest)mListener->groupBioRequest(p->systemAddress, mReader.readUInt());
		else mListener->groupBioUpdate(p->systemAddress, mReader.readString(1024));
	}
	void sendGroupBio(const SystemAddress &add, const string &bio)
	{
		mWriter.create(ID_GROUPBIO);
		mWriter.writeString(bio);

		mWriter.send(add);
	}
	void readMisc(Packet *p)
	{
		mReader.load(p);
		const unsigned char type = mReader.readUChar();
		if (type == 0 && p->systemAddress == mMainServerAdd)
		{
			const unsigned int charID = mReader.readUInt();
			mListener->duplicateLoginEvent(charID);
		}
		else if (type == 1)
		{
			const unsigned int targetID = mReader.readUInt();
			const string message = mReader.readString(256);
			mListener->reportEvent(p->systemAddress, targetID, message);
		}
	}
	void sendDuplicateLoginEvent(const SystemAddress &add, const string &token)
	{
		mWriter.create(ID_MISC);
		mWriter.writeUChar(0);
		mWriter.writeString(token);

		mWriter.send(add);
	}
	void readHome(Packet *p)
	{
		mReader.load(p);
		const bool isRequest = mReader.readBool();
		const bool isGoHome = mReader.readBool();
		if (isRequest || isGoHome)mListener->homeEvent(p->systemAddress, isRequest);
		else
		{
			const bool resetHome = mReader.readBool();
			const float x = resetHome ? 0 : mReader.readFloat();
			const float y = resetHome ? 0 : mReader.readFloat();
			const float z = resetHome ? 0 : mReader.readFloat();
			mListener->setHomeEvent(p->systemAddress, resetHome, x, y, z);
		}
	}
	void sendHome(const SystemAddress &add, const bool &isGoHome, const string &home, const int &x, const int &y, const int &z)
	{
		mWriter.create(ID_HOME);
		mWriter.writeBool(isGoHome);
		mWriter.writeString(home);
		if (isGoHome)
		{
			mWriter.writeInt(x);
			mWriter.writeInt(y);
			mWriter.writeInt(z);
		}

		mWriter.send(add);
	}
	void readParty(Packet *p)
	{
		mReader.load(p);
		const bool isLeave = mReader.readBool();

		if (isLeave)mListener->partyLeaveEvent(p->systemAddress);
	}
	void sendPartyLeave(const SystemAddress &add, const unsigned int &charID)
	{
		mWriter.create(ID_PARTY);
		mWriter.writeBool(false);        //is leave
		mWriter.writeUInt(charID);

		mWriter.send(add);
	}
	void relayPartyLeave(const unsigned int &senderID, const unsigned int &partyID)
	{
		mWriter.create(ID_PARTY);
		mWriter.writeBool(false);        //is leave
		mWriter.writeUInt(senderID);
		mWriter.writeUInt(partyID);

		mWriter.send(mMainServerAdd);
	}
	void readPartyRelay(Packet *p)
	{
		mReader.load(p);
		const bool isCreate = mReader.readBool();
		if (isCreate)
		{
			const unsigned int senderID = mReader.readUInt();
			const unsigned int targetID = mReader.readUInt();
			const unsigned int partyID = mReader.readUInt();

			mListener->partyCreateRelayEvent(senderID, targetID, partyID);
		}
		//Is leave
		else
		{
			const unsigned int senderID = mReader.readUInt();
			const unsigned int partyID = mReader.readUInt();

			mListener->partyLeaveRelayEvent(senderID, partyID);
		}
	}
	void sendPartyMemberInfo(const SystemAddress &add, const PartyMemberInfo &info, const bool &announce = false)
	{
		mWriter.create(ID_PARTY);
		mWriter.writeBool(true);        //is info
		mWriter.writeUInt(info.mCharID);
		mWriter.writeString(info.mCharName);
		if (announce)mWriter.writeBool(true);

		mWriter.send(add);
	}
	void relayPartyCreate(const unsigned int &senderID, const unsigned int &targetID)
	{
		mWriter.create(ID_PARTY);
		mWriter.writeBool(true);        //is create
		mWriter.writeUInt(senderID);
		mWriter.writeUInt(targetID);

		mWriter.send(mMainServerAdd);
	}
	void readPartyRequest(Packet *p)
	{
		mReader.load(p);
		const bool isAccept = mReader.readBool();       //false = is request
		const unsigned int id = mReader.readUInt();

		mListener->partyRequestEvent(p->systemAddress, id, isAccept);
	}
	void sendPartyRequest(const SystemAddress &add, const unsigned int &id, const string &name, const bool &isFail)
	{
		mWriter.create(ID_PARTYREQUEST);
		mWriter.writeBool(isFail);
		mWriter.writeUInt(id);
		mWriter.writeString(name);

		mWriter.send(add);
	}
	void relayPartyRequest(const unsigned int &senderID, const unsigned int &targetID, const bool &isAccept)
	{
		mWriter.create(ID_PARTYREQUEST);
		mWriter.writeBool(isAccept);
		mWriter.writeUInt(senderID);
		mWriter.writeUInt(targetID);

		mWriter.send(mMainServerAdd);
	}
	void readPartyRequestRelay(Packet *p)
	{
		mReader.load(p);
		const bool isAccept = mReader.readBool();       //false = is request
		const unsigned int senderID = mReader.readUInt();
		const unsigned int targetID = mReader.readUInt();

		mListener->partyRequestRelayEvent(senderID, targetID, isAccept);
	}
	void readCharSkill(Packet *p)
	{
		mReader.load(p);
		const unsigned char nSkill = mReader.readUChar();
		const bool bFlag = mReader.readBool();
		const bool bPersistent = mReader.readBool();
		mListener->charSkill(p->systemAddress, nSkill, bFlag, bPersistent);
	}
	void sendCharSkill(const SystemAddress &cAdd, const unsigned int &nCharID, const unsigned char &nSkill, const bool &bFlag)
	{
		mWriter.create(ID_CHAR_SKILL);
		mWriter.writeUInt(nCharID);
		mWriter.writeUChar(nSkill);
		mWriter.writeBool(bFlag);

		mWriter.send(cAdd);
	}
};

#endif


// ##################################################################################################
// ##-------------------------------------------FILE END-------------------------------------------##
// ##################################################################################################
// ##-------------------------------------------MODIFIED-------------------------------------------##
// ## DATE: 9/25/2026                                                                              ##
// ## TIME: 6:30 PM (UTC-5:00)                                                                     ##
// ## BY: radioactivedark                                                                          ##
// ##################################################################################################