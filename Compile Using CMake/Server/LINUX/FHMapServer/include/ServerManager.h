// ##################################################################################################
// ##------------------------------------------FILE START------------------------------------------##
// ##################################################################################################


#ifndef _SERVERMANAGER_H_
#define _SERVERMANAGER_H_

#define EARSHOT_SQUARED 4000000
#define UNIT_FAR_SQUARED 9000000

#include <NetworkManager.h>
#include <DatabaseManager.h>
#include <InputManager.h>

#include <iostream>

using namespace std;

struct TimedToken
{
	bool mUnconnected;
	unsigned int mUserID;
	unsigned int mCharID;
	float mTimer;
	TimedToken()
	{
		mUnconnected = true;
		mUserID = 0;
		mCharID = 0;
		mTimer = TOKEN_TIMER;
	}
};

struct PlayerToken
{
	string mToken;
	unsigned int mUserID;
	unsigned int mCharID;
	CharacterData mCharData;
	float mPosX;
	float mPosY;
	float mPosZ;
	unsigned char mEmote;
	bool mIsChangingMap;
	string mUsername;
	unsigned int mPartyID;
	unsigned int mGroupID;
	bool mGhostMode;
	unsigned char m_nSkill;
	PlayerToken()
	{
		mToken = "";
		mUserID = 0;
		mCharID = 0;
		mCharData = CharacterData();
		mPosX = mPosY = mPosZ = 0;
		mEmote = 0;
		mIsChangingMap = false;
		mUsername = "";
		mPartyID = 0;
		mGroupID = 0;
		mGhostMode = false;
		m_nSkill = 0;
	}
};

typedef map<SystemAddress, PlayerToken> TokenMap;
typedef map<string, TimedToken> TokenPool;

class ServerManager : public NetworkListener, public InputListener
{
private:
	NetworkManager *mNetMgr;
	DatabaseManager *mDBMgr;
	InputManager *mInputMgr;
	bool mRunning;
	TokenMap mToken;
	TokenPool mTokenPool;
	unsigned short mPort;
	string mMap;
	unsigned short mNumClients;
	time_t mPrevTime;
	bool mGivenFirstToken;
public:
	ServerManager(const unsigned short &port, const string &map)
	{
		mNetMgr = new NetworkManager(this);
		mDBMgr = new DatabaseManager();
		mInputMgr = new InputManager(this);
		mRunning = false;
		mPort = port;
		mMap = map;
		mNumClients = 0;
	}
	~ServerManager()
	{
		delete mInputMgr;
		delete mDBMgr;
		delete mNetMgr;
	}
	const bool init()
	{
		//cout << "MapServer Initializing..." << endl;
		cout << "Map: " << mMap << endl;
		//cout << "Port: " << toString(mPort) << endl;
		time(&mPrevTime);

		//Put dummy token in pool to let server wait before shutting down prior first token
		TimedToken dummyToken;
		dummyToken.mTimer = TOKEN_TIMER * 2;
		mTokenPool["dummy"] = dummyToken;

		return (mDBMgr->init() && mNetMgr->init(mPort));
	}
	void run()
	{
		//cout << "MapServer Running..." << endl;
		mRunning = true;
		while (mRunning)
		{
			mNetMgr->update();
			//mInputMgr->update();
			const float timeElapsed = updateTimers();
			updateTokens(timeElapsed);
		}
	}
	const float updateTimers()
	{
		time_t currTime;
		time(&currTime);
		const float timeSinceLastUpdate = (float)difftime(currTime, mPrevTime);
		mPrevTime = currTime;

		return timeSinceLastUpdate;
	}
	void updateTokens(const float &timeElapsed)
	{
		TokenPool::iterator it = mTokenPool.begin();

		//No tokens left, shutdown
		if (it == mTokenPool.end())mRunning = false;

		while (it != mTokenPool.end())
		{
			TimedToken *token = &it->second;
			if (token->mUnconnected)
			{
				token->mTimer -= timeElapsed;
				//Token expired, remove
				if (token->mTimer <= 0)
				{
					mTokenPool.erase(it);
					it = mTokenPool.begin();
					continue;
				}
			}
			it++;
		}
	}
	void shutdown()
	{
		cout << "MapServer Shutting down..." << endl;
		for (TokenMap::iterator it = mToken.begin(); it != mToken.end(); it++)
		{
			PlayerToken *token = &it->second;
			mDBMgr->saveCharacter(token->mCharID, token->mPosX, token->mPosY, token->mPosZ);
		}
		//mNetMgr->printStats();
		mNetMgr->shutdown();
		mDBMgr->shutdown();
		//cout << "Shutdown complete. Press enter to close." << endl;
		//cin.get();
	}
	void inputEvent(const string &input)
	{
		if (input == "quit")mRunning = false;
		else if (input == "stats")mNetMgr->printStats();
		else if (input == "debug")mNetMgr->toggleDebug();
		else if (input == "numclients")cout << "Num Clients: " << toString(mNumClients) << endl;
	}
	void allocateToken(const SystemAddress &add, const unsigned int &userID, const unsigned int &charID)
	{
		PlayerToken token;
		token.mUserID = userID;
		token.mCharID = charID;
		token.mUsername = mDBMgr->getUserName(userID);
		mToken[add] = token;
		mNumClients++;
	}
	void deallocateToken(const SystemAddress &add)
	{
		TokenMap::iterator it = mToken.find(add);
		if (it == mToken.end())return;
		mToken.erase(it);
		mNumClients--;
	}
	void tokenReceived(const string &token, const unsigned int &userID, const unsigned int &charID)
	{
		TimedToken t;
		t.mUserID = userID;
		t.mCharID = charID;
		mTokenPool[token] = t;
	}
	const pair<SystemAddress, PlayerToken*> getPlayerTokenByUserID(const unsigned int &userID)
	{
		for (TokenMap::iterator it = mToken.begin(); it != mToken.end(); it++)
		{
			PlayerToken *token = &it->second;
			if (token->mUserID == userID)
			{
				return pair<SystemAddress, PlayerToken*>(it->first, token);
			}
		}
		return pair<SystemAddress, PlayerToken*>(UNASSIGNED_SYSTEM_ADDRESS, 0);
	}
	const pair<SystemAddress, PlayerToken*> getPlayerTokenByCharID(const unsigned int &charID)
	{
		for (TokenMap::iterator it = mToken.begin(); it != mToken.end(); it++)
		{
			PlayerToken *token = &it->second;
			if (token->mCharID == charID)
			{
				return pair<SystemAddress, PlayerToken*>(it->first, token);
			}
		}
		return pair<SystemAddress, PlayerToken*>(UNASSIGNED_SYSTEM_ADDRESS, 0);
	}
	void connectEvent(const SystemAddress &add, const string &token, const bool &firstEntrance, const bool &ghostMode = false)
	{
		TokenPool::iterator pit = mTokenPool.find(token);
		if (pit == mTokenPool.end())
		{
			mNetMgr->sendTokenFail(add);
			return;
		}
		TimedToken *tt = &pit->second;
		tt->mUnconnected = false;
		const unsigned int charID = tt->mCharID;
		allocateToken(add, tt->mUserID, charID);

		PlayerToken *pt = &mToken[add];
		pt->mToken = token;
		int x, y, z = 0;
		unsigned int groupID = 0;
		unsigned int partyID = 0;
		mDBMgr->getCharacterInfo(charID, &pt->mCharData, x, y, z, groupID, partyID);
		pt->mPosX = x;
		pt->mPosY = y;
		pt->mPosZ = z;
		pt->mGroupID = groupID;
		pt->mPartyID = (firstEntrance ? 0 : partyID);
		pt->mEmote = pt->mCharData.mEmote;
		pt->mGhostMode = ghostMode;
		pt->m_nSkill = 0;
		mNetMgr->sendConnect(add, x, y, z);

		//Inform main server
		mNetMgr->sendMainCharInfo(token, firstEntrance);

		if (firstEntrance)
		{
			//Send group info
			unsigned int leaderID = 0;
			const string groupName = mDBMgr->getGroupInfo(groupID, leaderID);
			const bool isLeader = (leaderID == pt->mCharID);
			mNetMgr->sendMyGroupInfo(add, groupName, isLeader);
			if (groupID)
			{
				//Send group members info
				const vector<GroupMemberInfo> memberList = mDBMgr->getGroupMemberList(groupID);
				for (vector<GroupMemberInfo>::const_iterator it = memberList.begin(); it != memberList.end(); it++)
				{
					const GroupMemberInfo info = *it;
					mNetMgr->sendMyGroupMemberInfo(add, info);
				}
				//Send requests if is leader
				if (isLeader)
				{
					const vector<unsigned int> requestList = mDBMgr->getGroupRequestList(groupID);
					for (vector<unsigned int>::const_iterator it = requestList.begin(); it != requestList.end(); it++)
					{
						const unsigned int requesterID = *it;
						const string requesterName = mDBMgr->getCharacterName(requesterID);
						mNetMgr->sendGroupRequest(add, requesterID, requesterName, true);
					}
				}
			}
			//Send invites if no group
			else
			{
				const vector<unsigned int> inviteList = mDBMgr->getGroupInviteList(charID);
				for (vector<unsigned int>::const_iterator it = inviteList.begin(); it != inviteList.end(); it++)
				{
					const unsigned int inviterID = *it;
					const string inviterName = mDBMgr->getGroupName(inviterID);
					mNetMgr->sendGroupRequest(add, inviterID, inviterName, false);
				}
			}
			//Send block and blockedby info
			const vector<unsigned int> blockList = mDBMgr->getBlockList(pt->mUserID);
			for (vector<unsigned int>::const_iterator it = blockList.begin(); it != blockList.end(); it++)
			{
				const unsigned int blockID = *it;
				const string blockName = mDBMgr->getUserName(blockID);
				mNetMgr->sendBlockOrRequestInfo(add, false, blockID, blockName);
			}
			const vector<unsigned int> blockedByList = mDBMgr->getBlockedByList(pt->mUserID);
			for (vector<unsigned int>::const_iterator it = blockedByList.begin(); it != blockedByList.end(); it++)
			{
				const unsigned int blockerID = *it;
				mNetMgr->sendBlockedByInfo(add, blockerID, false);
			}
			//Send request info
			const vector<unsigned int> requestList = mDBMgr->getRequestList(pt->mUserID);
			for (vector<unsigned int>::const_iterator it = requestList.begin(); it != requestList.end(); it++)
			{
				const unsigned int requestID = *it;
				const string requestName = mDBMgr->getUserName(requestID);
				mNetMgr->sendBlockOrRequestInfo(add, true, requestID, requestName);
			}

			//Clear party if any
			if (partyID)mDBMgr->setCharacterParty(charID, 0);
		}
		//Send/update friend info
		const vector<unsigned int> friendList = mDBMgr->getFriendList(pt->mUserID);
		for (vector<unsigned int>::const_iterator it = friendList.begin(); it != friendList.end(); it++)
		{
			const unsigned int friendID = *it;
			const string friendName = (firstEntrance ? mDBMgr->getUserName(friendID) : "");
			const bool friendIsOnline = mDBMgr->isUserOnline(friendID);
			mNetMgr->sendFriendInfo(add, friendID, false, friendIsOnline, !firstEntrance, friendName);
		}


		//Send other players info to new arriver and
		//Broadcast arrival info to other players
		for (TokenMap::iterator it = mToken.begin(); it != mToken.end(); it++)
		{
			const SystemAddress target = it->first;
			PlayerToken *targetToken = &it->second;
			if (target != add)
			{
				if (!targetToken->mGhostMode)
				{
					mNetMgr->sendCharInfo(add, targetToken->mCharID, &targetToken->mCharData, targetToken->mPosX, targetToken->mPosY, targetToken->mPosZ, targetToken->mUserID, targetToken->mUsername);
					if (targetToken->m_nSkill)mNetMgr->sendCharSkill(add, targetToken->mCharID, targetToken->m_nSkill, true);
				}
				if (!ghostMode)mNetMgr->sendCharInfo(target, charID, &pt->mCharData, x, y, z, pt->mUserID, pt->mUsername);
			}
		}
	}
	void disconnectEvent(const SystemAddress &add, const bool &isProper)
	{
		TokenMap::iterator sender = mToken.find(add);
		if (sender == mToken.end())return;

		PlayerToken *pt = &sender->second;
		const string token = pt->mToken;


		TokenPool::iterator pit = mTokenPool.find(token);
		if (pit != mTokenPool.end())
		{
			//Begin token expiry
			TimedToken *tt = &pit->second;
			tt->mUnconnected = true;
			tt->mTimer = TOKEN_TIMER;
		}

		const unsigned int charID = pt->mCharID;

		//Inform main server
		mNetMgr->sendMainCharExit(token, pt->mIsChangingMap);

		if (!pt->mGhostMode)
		{
			//Broadcast exit info to other players
			for (TokenMap::iterator it = mToken.begin(); it != mToken.end(); it++)
			{
				const SystemAddress target = it->first;
				if (target != add)mNetMgr->sendCharExit(target, charID, isProper, pt->mIsChangingMap);
			}
			mDBMgr->saveCharacter(charID, pt->mPosX, pt->mPosY, pt->mPosZ);
		}
		deallocateToken(add);
	}
	void terminateEvent()
	{
		mRunning = false;
	}
	void charMapChange(const SystemAddress &add, const string &map, const float &x, const float &y, const float &z)
	{
		TokenMap::iterator sender = mToken.find(add);
		if (sender == mToken.end())return;

		PlayerToken *token = &sender->second;
		token->mIsChangingMap = true;
		token->mPosX = x;
		token->mPosY = y;
		token->mPosZ = z;
		mDBMgr->saveCharacterMap(token->mCharID, map);
	}
	void charGhostMapChange(const SystemAddress &add, const bool &unghost)
	{
		TokenMap::iterator sender = mToken.find(add);
		if (sender == mToken.end())return;

		PlayerToken *token = &sender->second;
		token->mIsChangingMap = true;
		token->mGhostMode = true;
		if (!unghost)mDBMgr->saveCharacter(token->mCharID, token->mPosX, token->mPosY, token->mPosZ);
	}
	void charPosition(const SystemAddress &add, const float &x, const float &y, const float &z)
	{
		TokenMap::iterator sender = mToken.find(add);
		if (sender == mToken.end())return;

		PlayerToken *token = &sender->second;
		//Do not update if client is in mapchange process
		if (token->mIsChangingMap)return;
		token->mPosX = x;
		token->mPosY = y;
		token->mPosZ = z;

		const unsigned int charID = token->mCharID;
		for (TokenMap::iterator it = mToken.begin(); it != mToken.end(); it++)
		{
			const SystemAddress target = it->first;
			if (target != add)mNetMgr->sendCharPosition(target, charID, x, y, z);
		}
	}
	void chatEvent(const SystemAddress &add, const string &caption, const unsigned char &channel, const string &whisperTarget)
	{
		TokenMap::iterator sender = mToken.find(add);
		if (sender == mToken.end())return;
		const unsigned int charID = sender->second.mCharID;
		const unsigned int senderID = sender->second.mUserID;
		switch (channel)
		{
		case CHAT_GROUP:
		{
			const unsigned int groupID = sender->second.mGroupID;
			if (groupID == 0)return;
			mNetMgr->relayChat(senderID, charID, caption, channel);
			for (TokenMap::iterator it = mToken.begin(); it != mToken.end(); it++)
			{
				const SystemAddress target = it->first;
				if (target != add && it->second.mGroupID == groupID)mNetMgr->sendChat(target, charID, caption, channel, senderID);
			}
		}
		break;
		case CHAT_PARTY:
		{
			const unsigned int partyID = sender->second.mPartyID;
			if (partyID == 0)return;
			mNetMgr->relayChat(senderID, charID, caption, channel);
			for (TokenMap::iterator it = mToken.begin(); it != mToken.end(); it++)
			{
				const SystemAddress target = it->first;
				if (target != add && it->second.mPartyID == partyID)mNetMgr->sendChat(target, charID, caption, channel, senderID);
			}
		}
		break;
		case CHAT_WHISPER:
		{
			const unsigned int userID = mDBMgr->getUserID(whisperTarget);
			if (userID == 0 || userID == senderID)mNetMgr->sendChat(add, 0, "", channel, 0, whisperTarget, true);
			else
			{
				if (!mDBMgr->isUserOnline(userID))
				{
					mNetMgr->sendChat(add, 0, "", channel, 0, whisperTarget, true, true);
					return;
				}
				const pair<SystemAddress, PlayerToken*> target = getPlayerTokenByUserID(userID);
				const bool ghostMode = sender->second.mGhostMode;
				//Player is in this map
				if (target.second)
				{
					mNetMgr->sendChat(target.first, ghostMode ? 0 : charID, caption, ghostMode ? CHAT_MODWHISPER : channel, ghostMode ? 0 : senderID, ghostMode ? ("MOD" + toString(senderID)) : sender->second.mUsername);
				}
				//Player is in another map
				else mNetMgr->relayChat(userID, charID, caption, ghostMode ? CHAT_MODWHISPER : channel);
			}
		}
		break;
		default:
		{
			const bool ghostMode = sender->second.mGhostMode;
			const float x = sender->second.mPosX;
			const float z = sender->second.mPosZ;
			const string finalCaption = (ghostMode ? ("<MOD" + toString(senderID) + "> " + caption) : caption);
			for (TokenMap::iterator it = mToken.begin(); it != mToken.end(); it++)
			{
				const SystemAddress target = it->first;
				if (ghostMode)mNetMgr->sendChat(target, 0, finalCaption, CHAT_MOD);
				else if (target != add && (channel == CHAT_GENERAL || it->second.mGhostMode || distanceSquared(x, z, it->second.mPosX, it->second.mPosZ) <= EARSHOT_SQUARED))mNetMgr->sendChat(target, charID, finalCaption, channel);
			}
		}
		break;
		}
	}
	void chatRelayEvent(const unsigned int &userID, const unsigned int &charID, const string &caption, const unsigned char &channel)
	{
		switch (channel)
		{
		case CHAT_GROUP:
		{
			const unsigned int groupID = mDBMgr->getCharacterGroup(charID);
			if (groupID == 0)return;
			const string sayerName = mDBMgr->getUserName(userID);
			for (TokenMap::iterator it = mToken.begin(); it != mToken.end(); it++)
			{
				const SystemAddress target = it->first;
				if (it->second.mGroupID == groupID)mNetMgr->sendChat(target, charID, caption, channel, userID, sayerName);
			}
		}
		break;
		case CHAT_PARTY:
		{
			const unsigned int partyID = mDBMgr->getCharacterParty(charID);
			if (partyID == 0)return;
			const string sayerName = mDBMgr->getCharacterName(charID);
			for (TokenMap::iterator it = mToken.begin(); it != mToken.end(); it++)
			{
				const SystemAddress target = it->first;
				if (it->second.mPartyID == partyID)mNetMgr->sendChat(target, charID, caption, channel, userID, sayerName);
			}
		}
		break;
		case CHAT_WHISPER:
		case CHAT_MODWHISPER:
			/*for(TokenMap::iterator it=mToken.begin(); it!=mToken.end(); it++)
			{
			const SystemAddress target = it->first;
			if(it->second.mUserID==userID)
			{
			mNetMgr->sendChat(target,charID,caption,channel,it->second.mUsername);
			break;
			}
			}*/
		{
			const pair<SystemAddress, PlayerToken*> target = getPlayerTokenByUserID(userID);
			if (target.second)
			{
				unsigned int sayerID = 0;
				const string whisperer = mDBMgr->getUserNameByCharID(charID, &sayerID);
				mNetMgr->sendChat(target.first, channel == CHAT_MODWHISPER ? 0 : charID, caption, channel, sayerID, channel == CHAT_MODWHISPER ? ("MOD" + toString(sayerID)) : whisperer);
			}
		}
		break;
		case CHAT_SYSTEM:
		case CHAT_MOD:
			for (TokenMap::iterator it = mToken.begin(); it != mToken.end(); it++)
			{
				const SystemAddress target = it->first;
				mNetMgr->sendChat(target, 0, caption, channel);
			}
			break;
		default: break;
		}
	}
	void charYaw(const SystemAddress &add, const unsigned char &yaw)
	{
		TokenMap::iterator sender = mToken.find(add);
		if (sender == mToken.end())return;

		const unsigned int charID = sender->second.mCharID;
		const float x = sender->second.mPosX;
		const float z = sender->second.mPosZ;
		for (TokenMap::iterator it = mToken.begin(); it != mToken.end(); it++)
		{
			const SystemAddress target = it->first;
			if (target != add && distanceSquared(x, z, it->second.mPosX, it->second.mPosZ) <= UNIT_FAR_SQUARED)mNetMgr->sendCharYaw(target, charID, yaw);
		}
	}
	void charHeadDir(const SystemAddress &add, const unsigned char &ew, const unsigned char &ns)
	{
		TokenMap::iterator sender = mToken.find(add);
		if (sender == mToken.end())return;

		const unsigned int charID = sender->second.mCharID;
		const float x = sender->second.mPosX;
		const float z = sender->second.mPosZ;
		for (TokenMap::iterator it = mToken.begin(); it != mToken.end(); it++)
		{
			const SystemAddress target = it->first;
			if (target != add && distanceSquared(x, z, it->second.mPosX, it->second.mPosZ) <= EARSHOT_SQUARED)mNetMgr->sendCharHeadDir(target, charID, ew, ns);
		}
	}
	void charAction(const SystemAddress &add, const unsigned char &action)
	{
		TokenMap::iterator sender = mToken.find(add);
		if (sender == mToken.end())return;

		const unsigned int charID = sender->second.mCharID;
		const float x = sender->second.mPosX;
		const float z = sender->second.mPosZ;
		for (TokenMap::iterator it = mToken.begin(); it != mToken.end(); it++)
		{
			const SystemAddress target = it->first;
			if (target != add && distanceSquared(x, z, it->second.mPosX, it->second.mPosZ) <= UNIT_FAR_SQUARED)mNetMgr->sendCharAction(target, charID, action);
		}
	}
	void charStatusFlags(const SystemAddress &add, const bool &ag, const bool &s)
	{
		TokenMap::iterator sender = mToken.find(add);
		if (sender == mToken.end())return;

		const unsigned int charID = sender->second.mCharID;
		const float x = sender->second.mPosX;
		const float z = sender->second.mPosZ;
		for (TokenMap::iterator it = mToken.begin(); it != mToken.end(); it++)
		{
			const SystemAddress target = it->first;
			if (target != add && distanceSquared(x, z, it->second.mPosX, it->second.mPosZ) <= UNIT_FAR_SQUARED)mNetMgr->sendCharStatusFlags(target, charID, ag, s);
		}
	}
	void charMoveModifier(const SystemAddress &add, const unsigned char &modifier)
	{
		TokenMap::iterator sender = mToken.find(add);
		if (sender == mToken.end())return;

		const unsigned int charID = sender->second.mCharID;
		const float x = sender->second.mPosX;
		const float z = sender->second.mPosZ;
		for (TokenMap::iterator it = mToken.begin(); it != mToken.end(); it++)
		{
			const SystemAddress target = it->first;
			if (target != add && distanceSquared(x, z, it->second.mPosX, it->second.mPosZ) <= UNIT_FAR_SQUARED)mNetMgr->sendCharMoveModifier(target, charID, modifier);
		}
	}
	void charEmote(const SystemAddress &add, const unsigned char &emote, const bool &bSetDefault)
	{
		TokenMap::iterator sender = mToken.find(add);
		if (sender == mToken.end())return;

		PlayerToken *token = &sender->second;
		const unsigned int charID = token->mCharID;

		if (bSetDefault)
		{
			mDBMgr->setCharacterEmote(charID, emote);
			return;
		}

		token->mEmote = emote;

		const float x = sender->second.mPosX;
		const float z = sender->second.mPosZ;
		for (TokenMap::iterator it = mToken.begin(); it != mToken.end(); it++)
		{
			const SystemAddress target = it->first;
			if (target != add && distanceSquared(x, z, it->second.mPosX, it->second.mPosZ) <= EARSHOT_SQUARED)mNetMgr->sendCharEmote(target, charID, emote);
		}
	}
	void charJump(const SystemAddress &add, const bool &jump)
	{
		TokenMap::iterator sender = mToken.find(add);
		if (sender == mToken.end())return;

		const unsigned int charID = sender->second.mCharID;
		const float x = sender->second.mPosX;
		const float z = sender->second.mPosZ;
		for (TokenMap::iterator it = mToken.begin(); it != mToken.end(); it++)
		{
			const SystemAddress target = it->first;
			if (target != add && distanceSquared(x, z, it->second.mPosX, it->second.mPosZ) <= UNIT_FAR_SQUARED)mNetMgr->sendCharJump(target, charID, jump);
		}
	}
	void movieEvent(const SystemAddress &add, const string &movie, const float &camYaw)
	{
		TokenMap::iterator sender = mToken.find(add);
		if (sender == mToken.end())return;

		const unsigned int charID = sender->second.mCharID;
		for (TokenMap::iterator it = mToken.begin(); it != mToken.end(); it++)
		{
			const SystemAddress target = it->first;
			if (target != add)mNetMgr->sendMovie(target, charID, movie, camYaw);
		}
	}
	void friendAddEvent(const SystemAddress &add, const bool &isBlock, const string &username)
	{
		TokenMap::iterator sender = mToken.find(add);
		if (sender == mToken.end())return;

		const unsigned int senderID = sender->second.mUserID;
		const unsigned int targetID = mDBMgr->getUserID(username);
		const bool success = (targetID != 0);

		if (senderID == targetID)return;
		mNetMgr->sendFriendAdd(add, isBlock, success, username);
		if (success)
		{
			//Block
			if (isBlock)
			{
				//Add block to DB, send block info if new
				if (mDBMgr->addBlock(senderID, targetID))
				{
					mNetMgr->sendBlockOrRequestInfo(add, false, targetID, username);
					//Broadcast block
					const pair<SystemAddress, PlayerToken*> target = getPlayerTokenByUserID(targetID);
					if (target.second)mNetMgr->sendBlockedByInfo(target.first, senderID, false);
					else mNetMgr->relayFriendInfo(false, true, senderID, targetID);
				}
			}
			//Friend request
			else
			{
				//Add request to DB
				if (!mDBMgr->isFriend(senderID, targetID) && mDBMgr->addFriendRequest(senderID, targetID))
				{
					//Broadcast request
					const pair<SystemAddress, PlayerToken*> target = getPlayerTokenByUserID(targetID);
					if (target.second)mNetMgr->sendBlockOrRequestInfo(target.first, true, senderID, sender->second.mUsername);
					else mNetMgr->relayFriendInfo(false, false, senderID, targetID, true);
				}
			}
		}
	}
	void friendRemoveEvent(const SystemAddress &add, const bool &isBlock, const unsigned int &targetID)
	{
		TokenMap::iterator sender = mToken.find(add);
		if (sender == mToken.end())return;

		const unsigned int senderID = sender->second.mUserID;

		//Block
		if (isBlock)
		{
			//Remove block from DB
			mDBMgr->deleteBlock(senderID, targetID);
			//Broadcast unblock
			const pair<SystemAddress, PlayerToken*> target = getPlayerTokenByUserID(targetID);
			if (target.second)mNetMgr->sendBlockedByInfo(target.first, senderID, true);
			else mNetMgr->relayFriendInfo(true, true, senderID, targetID);
		}
		//Friend
		else
		{
			//Remove friend from DB
			mDBMgr->deleteFriend(senderID, targetID);
			//Broadcast unfriend
			const pair<SystemAddress, PlayerToken*> target = getPlayerTokenByUserID(targetID);
			if (target.second)mNetMgr->sendFriendInfo(target.first, senderID, true);
			else mNetMgr->relayFriendInfo(true, false, senderID, targetID);
		}
	}
	void friendInfoRelayEvent(const bool &isRequest, const unsigned int &senderID, const unsigned int &targetID, const bool &isRemove)
	{
		const pair<SystemAddress, PlayerToken*> target = getPlayerTokenByUserID(targetID);
		if (target.second)
		{
			if (isRemove)
			{
				mNetMgr->sendFriendInfo(target.first, senderID, true);
			}
			else
			{
				const string username = mDBMgr->getUserName(senderID);
				if (isRequest)mNetMgr->sendBlockOrRequestInfo(target.first, true, senderID, username);
				else mNetMgr->sendFriendInfo(target.first, senderID, false, true, false, username);
			}
		}
	}
	void blockedByInfoRelayEvent(const unsigned int &senderID, const unsigned int &targetID, const bool &isRemove)
	{
		const pair<SystemAddress, PlayerToken*> target = getPlayerTokenByUserID(targetID);
		if (target.second)mNetMgr->sendBlockedByInfo(target.first, senderID, isRemove);
	}
	void friendAcceptEvent(const SystemAddress &add, const unsigned int &senderID, const bool &accept)
	{
		TokenMap::iterator accepter = mToken.find(add);
		if (accepter == mToken.end())return;

		const unsigned int accepterID = accepter->second.mUserID;
		//Check that senderID,accepterID exists in request DB
		if (mDBMgr->deleteFriendRequest(senderID, accepterID))
		{
			//Add friend to DB
			if (accept && !mDBMgr->isFriend(senderID, accepterID) && mDBMgr->addFriend(senderID, accepterID))
			{
				//Broadcast friend info
				const string senderName = mDBMgr->getUserName(senderID);
				const bool senderIsOnline = mDBMgr->isUserOnline(senderID);
				mNetMgr->sendFriendInfo(add, senderID, false, senderIsOnline, false, senderName);
				const pair<SystemAddress, PlayerToken*> target = getPlayerTokenByUserID(senderID);
				if (target.second)mNetMgr->sendFriendInfo(target.first, accepterID, false, true, false, accepter->second.mUsername);
				else mNetMgr->relayFriendInfo(false, false, accepterID, senderID, false);
			}
		}
	}
	void charOnlineStatus(const unsigned int &userID, const unsigned int &charID, const bool &isOnline)
	{
		mNetMgr->broadcastCharOnlineStatus(userID, charID, isOnline);
		if (!isOnline)mDBMgr->setCharacterOffline(charID);
	}
	void findCharEvent(const SystemAddress &add, const unsigned int &id, const bool &isUserID)
	{
		const string name = (isUserID ? mDBMgr->getUserName(id) : mDBMgr->getCharacterName(id));
		string mapName = mDBMgr->getCharacterMap(id, isUserID);
		if (mapName.length()>10)mapName.erase(mapName.length() - 10, 10);
		mNetMgr->sendFindChar(add, name, mapName);
	}
	void charBioUpdate(const SystemAddress &add, const string &bio)
	{
		TokenMap::iterator token = mToken.find(add);
		if (token == mToken.end())return;
		mDBMgr->saveCharBio(token->second.mCharID, bio);
	}
	void charBioRequestEvent(const SystemAddress &add, const unsigned int &charID, const bool &isBio, const bool &requestUserID)
	{
		if (!isBio)
		{
			unsigned int userID = 0;
			const string username = mDBMgr->getUserNameByCharID(charID, &userID);
			const string charname = mDBMgr->getCharacterName(charID);
			unsigned int groupID = 0;
			const string group = mDBMgr->getGroupNameByCharID(charID, &groupID);

			mNetMgr->sendCharBioInfo(add, username, charname, group, groupID, requestUserID ? userID : 0);
		}
		else
		{
			unsigned int finalCharID = charID;
			if (charID == 0)
			{
				TokenMap::iterator token = mToken.find(add);
				if (token == mToken.end())return;
				finalCharID = token->second.mCharID;
			}
			const string bio = mDBMgr->getCharBio(finalCharID);
			mNetMgr->sendCharBio(add, (charID == 0), bio);
		}
	}
	void modEvent(const SystemAddress &add, const string &command)
	{
		TokenMap::iterator token = mToken.find(add);
		if (token == mToken.end())return;

		string cmd = command;
		string content = command;
		const size_t cmdSpace = command.find_first_of(' ');
		if (cmdSpace != string::npos)
		{
			cmd.erase(cmdSpace);
			content.erase(0, cmdSpace + 1);
		}

		if (cmd == COMMAND_HELP)
		{
			string info = "/?\n";
			if (mDBMgr->getModPrivilege(token->second.mUserID, "ghost"))info += "/ghost (enter again to exit ghost mode)\n";
			if (mDBMgr->getModPrivilege(token->second.mUserID, "kick"))info += "/kick [username];[reason]\n";
			if (mDBMgr->getModPrivilege(token->second.mUserID, "ban"))info += "/ban [username] [value] [min/hour/day/week/month/year];[reason]\n";
			if (mDBMgr->getModPrivilege(token->second.mUserID, "unban"))info += "/unban [username]\n";
			if (mDBMgr->getModPrivilege(token->second.mUserID, "godspeak"))info += "/godspeak [message]\n";
			mNetMgr->sendMod(add, info);
		}
		else if (cmd == COMMAND_GHOST)
		{
			const int privilege = mDBMgr->getModPrivilege(token->second.mUserID, "ghost");
			if (privilege == 0)return;
			mNetMgr->sendMod(add, command);
		}
		else if (cmd == COMMAND_KICK)
		{
			const int privilege = mDBMgr->getModPrivilege(token->second.mUserID, "kick");
			if (privilege == 0)return;
			const vector<string> section = tokenize(content, ';', 1);
			if (section.size() <= 0)return;
			const unsigned int targetID = mDBMgr->getUserID(section[0]);
			if (targetID)
			{
				const pair<SystemAddress, PlayerToken*> target = getPlayerTokenByUserID(targetID);
				if (target.second)
				{
					mNetMgr->sendMod(target.first, command, false);
					mNetMgr->sendMod(add, command, true);
				}
				else mNetMgr->sendMod(add, COMMAND_NOSUCHUSER + section[0]);
			}
			else mNetMgr->sendMod(add, COMMAND_NOSUCHUSER + section[0]);
		}
		else if (cmd == COMMAND_BAN)
		{
			const int privilege = mDBMgr->getModPrivilege(token->second.mUserID, "ban");
			if (privilege == 0)return;
			const vector<string> section = tokenize(content, ';', 1);
			if (section.size() <= 0)return;
			const vector<string> part = tokenize(section[0]);
			if (part.size() >= 3)
			{
				//Spaced usernames
				string username = part[0];
				for (int i = 1; i<(int)part.size() - 2; i++)username += " " + part[i];
				const unsigned int targetID = mDBMgr->getUserID(username);
				if (targetID)
				{
					unsigned char type = 0;
					int value = atoi(part[part.size() - 2].c_str());
					if (part[part.size() - 1].find("hour") == 0){ value *= 60; type = 1; }
					else if (part[part.size() - 1].find("day") == 0){ value *= 1440; type = 2; }
					else if (part[part.size() - 1].find("week") == 0){ value *= 10080; type = 3; }
					else if (part[part.size() - 1].find("month") == 0){ value *= 43200; type = 4; }
					else if (part[part.size() - 1].find("year") == 0){ value *= 525600; type = 5; }

					if (value>privilege)value = privilege;

					if (value >= 525600){ value /= 525600; type = 5; }
					else if (value >= 43200){ value /= 43200; type = 4; }
					else if (value >= 10080){ value /= 10080; type = 3; }
					else if (value >= 1440){ value /= 1440; type = 2; }
					else if (value >= 60){ value /= 60; type = 1; }
					else type = 0;

					string typeStr = "minute";
					if (type == 1)typeStr = "hour";
					else if (type == 2)typeStr = "day";
					else if (type == 3)typeStr = "week";
					else if (type == 4)typeStr = "month";
					else if (type == 5)typeStr = "year";

					string command2 = cmd + " " + username + " " + toString(value) + " " + typeStr;
					if (value>1)command2 += "s";
					if (section.size()>1)command2 += ";" + section[1];

					mDBMgr->updateBan(targetID, mDBMgr->getIPByUserID(targetID), mDBMgr->getMACByUserID(targetID), value, typeStr);

					const pair<SystemAddress, PlayerToken*> target = getPlayerTokenByUserID(targetID);
					if (target.second)mNetMgr->sendMod(target.first, command2, false);
					mNetMgr->sendMod(add, command2, true);
				}
				else mNetMgr->sendMod(add, COMMAND_NOSUCHUSER + username);
			}
		}
		else if (cmd == COMMAND_UNBAN)
		{
			const int privilege = mDBMgr->getModPrivilege(token->second.mUserID, "unban");
			if (privilege == 0)return;
			const unsigned int targetID = mDBMgr->getUserID(content);
			if (targetID)
			{
				mDBMgr->unban(targetID);
				mNetMgr->sendMod(add, command);
			}
			else mNetMgr->sendMod(add, COMMAND_NOSUCHUSER + content);
		}
		else if (cmd == COMMAND_GODSPEAK)
		{
			const int privilege = mDBMgr->getModPrivilege(token->second.mUserID, "godspeak");
			if (privilege == 0)return;
			string caption = content;
			caption = "<MOD" + toString(token->second.mUserID) + "> " + caption;
			for (TokenMap::iterator it = mToken.begin(); it != mToken.end(); it++)
			{
				const SystemAddress target = it->first;
				mNetMgr->sendChat(target, 0, caption, CHAT_MOD);
			}
			//mNetMgr->relayChat(0,0,caption,CHAT_MOD);
		}
	}
	const vector<string> tokenize(string caption, const char &delim = ' ', const int &max = -1)
	{
		vector<string> result;
		int splits = 0;
		while (caption.length()>0)
		{
			const size_t i = caption.find_first_of(delim);
			if (i == string::npos || (splits >= max && max != -1))
			{
				result.push_back(caption);
				break;
			}
			string token = caption;
			token.erase(i);
			result.push_back(token);
			caption.erase(0, i + 1);
			splits++;
		}
		return result;
	}
	void myGroupEvent(const SystemAddress &add, const string &groupname)
	{
		TokenMap::iterator token = mToken.find(add);
		if (token == mToken.end())return;

		const unsigned int charID = token->second.mCharID;
		const unsigned int groupID = token->second.mGroupID;
		const bool isCreate = (groupID == 0);
		//Ignore update if renamer is not leader
		if (groupname == "" || (!isCreate && !mDBMgr->isGroupLeader(charID, groupID)))return;
		//Name exists
		if (mDBMgr->getGroupByName(groupname))
		{
			mNetMgr->sendMyGroupEvent(add, isCreate, false);
			return;
		}
		//Create
		if (isCreate)
		{
			token->second.mGroupID = mDBMgr->createGroup(groupname, charID);
			mDBMgr->setCharacterGroup(charID, token->second.mGroupID, 0);
			mNetMgr->sendMyGroupInfo(add, groupname, true);
			GroupMemberInfo info(charID, token->second.mCharData.mName);
			info.mRank = 0;
			info.mIsOnline = true;
			mNetMgr->sendMyGroupMemberInfo(add, info);
		}
		//Rename
		else
		{
			mDBMgr->renameGroup(groupID, groupname);
			//Update all groupmembers
			for (TokenMap::iterator it = mToken.begin(); it != mToken.end(); it++)
			{
				const SystemAddress target = it->first;
				if (it->second.mGroupID == groupID)mNetMgr->sendMyGroupInfo(target, groupname, (target == add));
			}
			mNetMgr->relayMyGroupInfo(groupID, groupname);
		}
		mNetMgr->sendMyGroupEvent(add, isCreate, true);
	}
	void myGroupRelayEvent(const unsigned int &groupID, const string &groupname)
	{
		//Update all groupmembers
		for (TokenMap::iterator it = mToken.begin(); it != mToken.end(); it++)
		{
			const SystemAddress target = it->first;
			if (it->second.mGroupID == groupID)mNetMgr->sendMyGroupInfo(target, groupname, false);
		}
	}
	void myGroupLeaveEvent(const SystemAddress &add, unsigned int charID, const bool &isKick)
	{
		TokenMap::iterator token = mToken.find(add);
		if (token == mToken.end())return;

		const unsigned int senderID = token->second.mCharID;
		const unsigned int groupID = token->second.mGroupID;
		if (charID == 0)charID = senderID;
		if (isKick)
		{
			//Only group leaders can kick
			if (!mDBMgr->isGroupLeader(senderID, groupID))return;
			//Cannot kick other groups
			unsigned int targetGroupID = mDBMgr->getCharacterGroup(charID);
			if (targetGroupID != groupID)return;
		}
		const bool isDisband = mDBMgr->isGroupLeader(charID, groupID);
		mDBMgr->setCharacterGroup(charID, 0);
		if (isDisband)
		{
			mDBMgr->deleteGroup(groupID);
		}
		//Update all groupmembers
		for (TokenMap::iterator it = mToken.begin(); it != mToken.end(); it++)
		{
			const SystemAddress target = it->first;
			if (it->second.mGroupID == groupID)
			{
				mNetMgr->sendMyGroupLeave(target, it->second.mCharID == charID ? 0 : charID, isKick, isDisband);
				if (isDisband)it->second.mGroupID = 0;
			}
		}
		mNetMgr->relayMyGroupLeave(groupID, charID, isKick, isDisband);

		if (!isDisband)
		{
			const pair<SystemAddress, PlayerToken*> pt = getPlayerTokenByCharID(charID);
			if (pt.second)pt.second->mGroupID = 0;
		}
	}
	void myGroupLeaveRelayEvent(const unsigned int &groupID, const unsigned int &charID, const bool &isKick, const bool &isDisband)
	{
		//Update all groupmembers
		for (TokenMap::iterator it = mToken.begin(); it != mToken.end(); it++)
		{
			const SystemAddress target = it->first;
			if (it->second.mGroupID == groupID)
			{
				mNetMgr->sendMyGroupLeave(target, it->second.mCharID == charID ? 0 : charID, isKick, isDisband);
				if (isDisband)it->second.mGroupID = 0;
			}
		}
		if (!isDisband)
		{
			const pair<SystemAddress, PlayerToken*> pt = getPlayerTokenByCharID(charID);
			if (pt.second)pt.second->mGroupID = 0;
		}
	}
	void viewGroupsEvent(const SystemAddress &add, const string &filter)
	{
		if (filter.length() < 3)return;

		const vector<pair<unsigned int, string> > groupList = mDBMgr->getGroupList(filter);
		for (vector<pair<unsigned int, string> >::const_iterator it = groupList.begin(); it != groupList.end(); it++)
		{
			const pair<unsigned int, string> group = *it;
			mNetMgr->sendViewGroupInfo(add, group.first, group.second);
		}
	}
	void viewGroupMembersEvent(const SystemAddress &add, const unsigned int &groupID)
	{
		const vector<GroupMemberInfo> memberList = mDBMgr->getGroupMemberList(groupID);
		for (vector<GroupMemberInfo>::const_iterator it = memberList.begin(); it != memberList.end(); it++)
		{
			const GroupMemberInfo info = *it;
			mNetMgr->sendViewGroupMemberInfo(add, info);
		}
	}
	void groupRequestEvent(const SystemAddress &add, const unsigned int &id, const bool &isRequest)
	{
		TokenMap::iterator token = mToken.find(add);
		if (token == mToken.end())return;

		const unsigned int charID = token->second.mCharID;
		const unsigned int groupID = token->second.mGroupID;
		//Ignore if already has goup
		if (isRequest)
		{
			if (groupID != 0)return;
		}
		//Ignore if inviter is not leader
		else if (groupID == 0 || !mDBMgr->isGroupLeader(charID, groupID))return;

		if (isRequest)
		{
			const unsigned int leaderID = mDBMgr->getGroupLeader(id);
			if (leaderID == 0)return;
			const unsigned int leaderUserID = mDBMgr->getUserIDByCharID(leaderID);
			if (mDBMgr->isBlocked(leaderUserID, token->second.mUserID))return;
			//Send group leader request if is new
			if (mDBMgr->addGroupRequest(charID, id))
			{
				const pair<SystemAddress, PlayerToken*> target = getPlayerTokenByCharID(leaderID);
				if (target.second)mNetMgr->sendGroupRequest(target.first, charID, token->second.mCharData.mName, true);
				else mNetMgr->relayGroupRequest(charID, id, false, true);
			}
		}
		else
		{
			const unsigned int targetUserID = mDBMgr->getUserIDByCharID(id);
			if (mDBMgr->isBlocked(targetUserID, token->second.mUserID))return;
			//Send target if invite is new
			if (mDBMgr->addGroupInvite(groupID, id))
			{
				const pair<SystemAddress, PlayerToken*> target = getPlayerTokenByCharID(id);
				if (target.second)
				{
					const string groupname = mDBMgr->getGroupName(groupID);
					mNetMgr->sendGroupRequest(target.first, groupID, groupname, false);
				}
				else mNetMgr->relayGroupRequest(groupID, id, false, false);
			}
		}
	}
	void groupRequestRelayEvent(const unsigned int &senderID, const unsigned int &targetID, const bool &isRequest)
	{
		if (isRequest)
		{
			const unsigned int leaderID = mDBMgr->getGroupLeader(targetID);
			if (leaderID == 0)return;
			const pair<SystemAddress, PlayerToken*> target = getPlayerTokenByCharID(leaderID);
			if (target.second)
			{
				const string senderName = mDBMgr->getCharacterName(senderID);
				mNetMgr->sendGroupRequest(target.first, senderID, senderName, true);
			}
		}
		else
		{
			const pair<SystemAddress, PlayerToken*> target = getPlayerTokenByCharID(targetID);
			if (target.second)
			{
				const string groupname = mDBMgr->getGroupName(senderID);
				mNetMgr->sendGroupRequest(target.first, senderID, groupname, false);
			}
		}
	}
	void groupAcceptEvent(const SystemAddress &add, const unsigned int &id, const bool &isRequest, const bool &isAccept)
	{
		TokenMap::iterator token = mToken.find(add);
		if (token == mToken.end())return;

		const unsigned int charID = token->second.mCharID;
		const unsigned int groupID = token->second.mGroupID;
		//Ignore if invitation accepter already has goup
		if (!isRequest)
		{
			if (groupID != 0)return;
		}
		//Ignore if request accepter is not leader, or if requester already has group
		else if (groupID == 0 || !mDBMgr->isGroupLeader(charID, groupID) || mDBMgr->getCharacterGroup(id) != 0)return;

		//Accept join request (as leader)
		if (isRequest)
		{
			if (mDBMgr->deleteGroupRequest(id, groupID))
			{
				//Take no action if denied
				if (!isAccept)return;
				mDBMgr->setCharacterGroup(id, groupID);
				//Nullify all requests made by requester
				mDBMgr->deleteAllGroupRequest(id, false);
				//Nullify all invitations to requester
				mDBMgr->deleteAllGroupInvite(id, true);
				const pair<SystemAddress, PlayerToken*> target = getPlayerTokenByCharID(id);
				if (target.second)
				{
					target.second->mGroupID = groupID;
					const SystemAddress targetAdd = target.first;
					const string groupName = mDBMgr->getGroupName(groupID);
					mNetMgr->sendMyGroupInfo(targetAdd, groupName, false);
					//Provide newcomer with groupmembers' info
					const vector<GroupMemberInfo> memberList = mDBMgr->getGroupMemberList(groupID);
					for (vector<GroupMemberInfo>::const_iterator it = memberList.begin(); it != memberList.end(); it++)
					{
						const GroupMemberInfo info = *it;
						if (info.mCharID != id)mNetMgr->sendMyGroupMemberInfo(targetAdd, info);
					}
				}
				mNetMgr->relayGroupRequest(id, groupID, true);

				//Provide groupmembers with newcomer's info
				const string charname = mDBMgr->getCharacterName(id);
				GroupMemberInfo info(id, charname);
				info.mIsOnline = mDBMgr->isCharacterOnline(id);
				for (TokenMap::iterator it = mToken.begin(); it != mToken.end(); it++)
				{
					if (it->second.mGroupID == groupID)mNetMgr->sendMyGroupMemberInfo(it->first, info, info.mIsOnline);
				}
			}
		}
		//Accept group invite (as groupless character)
		else
		{
			if (mDBMgr->deleteGroupInvite(id, charID))
			{
				//Take no action if denied
				if (!isAccept)return;
				mDBMgr->setCharacterGroup(charID, id);
				//Nullify all requests made by accepter
				mDBMgr->deleteAllGroupRequest(charID, false);
				//Nullify all invitations to accepter
				mDBMgr->deleteAllGroupInvite(charID, true);
				token->second.mGroupID = id;

				const string groupName = mDBMgr->getGroupName(id);
				mNetMgr->sendMyGroupInfo(add, groupName, false);
				//Provide newcomer with groupmembers' info
				const vector<GroupMemberInfo> memberList = mDBMgr->getGroupMemberList(id);
				for (vector<GroupMemberInfo>::const_iterator it = memberList.begin(); it != memberList.end(); it++)
				{
					const GroupMemberInfo info = *it;
					if (info.mCharID != charID)mNetMgr->sendMyGroupMemberInfo(add, info);
				}
				mNetMgr->relayGroupRequest(charID, id, true);

				//Provide groupmembers with newcomer's info
				GroupMemberInfo info(charID, token->second.mCharData.mName);
				info.mIsOnline = true;
				for (TokenMap::iterator it = mToken.begin(); it != mToken.end(); it++)
				{
					if (it->second.mGroupID == id)mNetMgr->sendMyGroupMemberInfo(it->first, info, true);
				}
			}
		}
	}
	void groupAcceptRelayEvent(const unsigned int &charID, const unsigned int &groupID)
	{
		const pair<SystemAddress, PlayerToken*> target = getPlayerTokenByCharID(charID);
		if (target.second)
		{
			target.second->mGroupID = groupID;
			const SystemAddress targetAdd = target.first;
			const string groupName = mDBMgr->getGroupName(groupID);
			mNetMgr->sendMyGroupInfo(targetAdd, groupName, false);

			//Provide newcomer with groupmembers' info
			const vector<GroupMemberInfo> memberList = mDBMgr->getGroupMemberList(groupID);
			for (vector<GroupMemberInfo>::const_iterator it = memberList.begin(); it != memberList.end(); it++)
			{
				const GroupMemberInfo info = *it;
				if (info.mCharID != charID)mNetMgr->sendMyGroupMemberInfo(targetAdd, info);
			}
		}
		//Provide groupmembers with newcomer's info
		const string charname = mDBMgr->getCharacterName(charID);
		GroupMemberInfo info(charID, charname);
		info.mIsOnline = mDBMgr->isCharacterOnline(charID);
		for (TokenMap::iterator it = mToken.begin(); it != mToken.end(); it++)
		{
			if (it->second.mGroupID == groupID)mNetMgr->sendMyGroupMemberInfo(it->first, info, info.mIsOnline);
		}
	}
	void groupBioRequest(const SystemAddress &add, const unsigned int &groupID)
	{
		TokenMap::iterator token = mToken.find(add);
		if (token == mToken.end())return;

		const string bio = mDBMgr->getGroupBio(groupID == 0 ? token->second.mGroupID : groupID);
		mNetMgr->sendGroupBio(add, bio);
	}
	void groupBioUpdate(const SystemAddress &add, const string &bio)
	{
		TokenMap::iterator token = mToken.find(add);
		if (token == mToken.end())return;

		const unsigned int charID = token->second.mCharID;
		const unsigned int groupID = token->second.mGroupID;
		if (mDBMgr->isGroupLeader(charID, groupID))mDBMgr->updateGroupBio(groupID, bio);
	}
	void duplicateLoginEvent(unsigned int charID)
	{
		const pair<SystemAddress, PlayerToken*> token = getPlayerTokenByCharID(charID);

		if (token.second)mNetMgr->sendDuplicateLoginEvent(token.first, token.second->mToken);
	}
	void reportEvent(const SystemAddress &add, const unsigned int &targetID, const string &message)
	{
		TokenMap::iterator token = mToken.find(add);
		if (token == mToken.end())return;

		mDBMgr->addReport(token->second.mUserID, targetID, message);
	}
	void homeEvent(const SystemAddress &add, const bool &isRequest)
	{
		TokenMap::iterator token = mToken.find(add);
		if (token == mToken.end())return;

		int x, y, z;
		string home = mDBMgr->getCharacterHome(token->second.mCharID, x, y, z);
		//Trim off seed to send to player
		if (home.length()>10)home.erase(home.length() - 10, 10);
		mNetMgr->sendHome(add, !isRequest, home, x, y, z);
	}
	void setHomeEvent(const SystemAddress &add, const bool &isReset, const float &x, const float &y, const float &z)
	{
		TokenMap::iterator token = mToken.find(add);
		if (token == mToken.end())return;

		mDBMgr->setCharacterHome(token->second.mCharID, isReset, mMap, x, y, z);
		//Trim off seed to send to player
		string home = isReset ? START_MAPNAME : mMap;
		if (isReset)
		{
			home.erase(0, 1);
			home.erase(home.length() - 1, 1);
		}
		if (home.length()>10)home.erase(home.length() - 10, 10);
		mNetMgr->sendHome(add, false, home, x, y, z);
	}
	const float distanceSquared(const float &startX, const float &startZ, const float &endX, const float &endZ)
	{
		const float dX = endX - startX;
		const float dZ = endZ - startZ;
		return (dX*dX + dZ*dZ);
	}
	void partyLeaveEvent(const SystemAddress &add)
	{
		TokenMap::iterator token = mToken.find(add);
		if (token == mToken.end())return;

		const unsigned int partyID = token->second.mPartyID;
		if (!partyID)return;
		const unsigned int senderCharID = token->second.mCharID;

		//Inform members in this map of leaver, including leaver themself
		for (TokenMap::iterator it = mToken.begin(); it != mToken.end(); it++)
		{
			if (it->second.mPartyID == partyID)mNetMgr->sendPartyLeave(it->first, senderCharID);
		}

		token->second.mPartyID = 0;
		mDBMgr->setCharacterParty(senderCharID, 0);

		//Inform other members in other maps
		mNetMgr->relayPartyLeave(senderCharID, partyID);
	}
	void partyLeaveRelayEvent(const unsigned int &charID, const unsigned int &partyID)
	{
		//Inform members in this map of leaver
		for (TokenMap::iterator it = mToken.begin(); it != mToken.end(); it++)
		{
			if (it->second.mPartyID == partyID)mNetMgr->sendPartyLeave(it->first, charID);
		}
	}
	void partyRequestEvent(const SystemAddress &add, const unsigned int &charID, const bool &isAccept)
	{
		TokenMap::iterator token = mToken.find(add);
		if (token == mToken.end())return;

		const unsigned int senderCharID = token->second.mCharID;
		const string senderCharName = token->second.mCharData.mName;
		const unsigned int senderPartyID = token->second.mPartyID;
		const unsigned int partyID = mDBMgr->getCharacterParty(charID);

		//Request
		if (!isAccept)
		{
			//I wanna join your party/I want you to join my party
			if (0 == senderPartyID || 0 == partyID)
			{
				const pair<SystemAddress, PlayerToken*> target = getPlayerTokenByCharID(charID);
				if (target.second)mNetMgr->sendPartyRequest(target.first, senderCharID, senderCharName, false);
				else mNetMgr->relayPartyRequest(senderCharID, charID, false);
			}
		}
		//Accept
		else
		{
			//I joined your party
			if (0 != partyID)
			{
				//Provide newcomer with members' info
				const vector<PartyMemberInfo> memberList = mDBMgr->getPartyMemberList(partyID);
				for (vector<PartyMemberInfo>::const_iterator it = memberList.begin(); it != memberList.end(); it++)
				{
					const PartyMemberInfo info = *it;
					if (info.mCharID != senderCharID)mNetMgr->sendPartyMemberInfo(add, info, false);
				}

				token->second.mPartyID = partyID;
				mDBMgr->setCharacterParty(senderCharID, partyID);

				//Provide members in this map with newcomer's info
				PartyMemberInfo info(senderCharID, senderCharName);
				for (TokenMap::iterator it = mToken.begin(); it != mToken.end(); it++)
				{
					if (it->second.mPartyID == partyID && it->second.mCharID != senderCharID)mNetMgr->sendPartyMemberInfo(it->first, info, true);
				}

				//Inform other members in other maps (swap target/sender)
				mNetMgr->relayPartyRequest(charID, senderCharID, true);
			}
			//You joined my party
			else if (0 != senderPartyID)
			{
				const pair<SystemAddress, PlayerToken*> target = getPlayerTokenByCharID(charID);
				string targetCharName = "";
				if (target.second)
				{
					//Provide newcomer with members' info
					const vector<PartyMemberInfo> memberList = mDBMgr->getPartyMemberList(senderPartyID);
					for (vector<PartyMemberInfo>::const_iterator it = memberList.begin(); it != memberList.end(); it++)
					{
						const PartyMemberInfo info = *it;
						if (info.mCharID != charID)mNetMgr->sendPartyMemberInfo(target.first, info, false);
					}

					target.second->mPartyID = senderPartyID;
					mDBMgr->setCharacterParty(charID, senderPartyID);

					targetCharName = target.second->mCharData.mName;
				}
				else targetCharName = mDBMgr->getCharacterName(charID);

				//Provide members in this map with newcomer's info
				PartyMemberInfo info(charID, targetCharName);
				for (TokenMap::iterator it = mToken.begin(); it != mToken.end(); it++)
				{
					if (it->second.mPartyID == senderPartyID && it->second.mCharID != charID)mNetMgr->sendPartyMemberInfo(it->first, info, true);
				}

				//Inform other members in other maps
				mNetMgr->relayPartyRequest(senderCharID, charID, true);
			}
			//Create new party
			else
			{
				mNetMgr->relayPartyCreate(senderCharID, charID);
			}
		}
	}
	void partyRequestRelayEvent(const unsigned int &senderID, const unsigned int &targetID, const bool &isAccept)
	{
		const pair<SystemAddress, PlayerToken*> target = getPlayerTokenByCharID(targetID);
		//Request only needs check if target is here
		if (!isAccept && !target.second)return;

		const unsigned int senderPartyID = mDBMgr->getCharacterParty(senderID);
		const unsigned int partyID = (target.second ? target.second->mPartyID : 0);

		//Request
		if (!isAccept)
		{
			//I wanna join your party/I want you to join my party
			if (0 == senderPartyID || 0 == partyID)
			{
				const string senderCharName = mDBMgr->getCharacterName(senderID);
				mNetMgr->sendPartyRequest(target.first, senderID, senderCharName, false);
			}
		}
		//Accept
		else
		{
			//You joined my party (or target is not here)
			if (0 == partyID && 0 != senderPartyID)
			{
				string targetCharName = "";
				//Target is here
				if (target.second)
				{
					//Provide newcomer with members' info
					const vector<PartyMemberInfo> memberList = mDBMgr->getPartyMemberList(senderPartyID);
					for (vector<PartyMemberInfo>::const_iterator it = memberList.begin(); it != memberList.end(); it++)
					{
						const PartyMemberInfo info = *it;
						if (info.mCharID != targetID)mNetMgr->sendPartyMemberInfo(target.first, info, false);
					}

					target.second->mPartyID = senderPartyID;
					mDBMgr->setCharacterParty(targetID, senderPartyID);

					targetCharName = target.second->mCharData.mName;
				}
				else targetCharName = mDBMgr->getCharacterName(targetID);

				//Provide members in this map with newcomer's info
				PartyMemberInfo info(targetID, targetCharName);
				for (TokenMap::iterator it = mToken.begin(); it != mToken.end(); it++)
				{
					if (it->second.mPartyID == senderPartyID && it->second.mCharID != targetID)mNetMgr->sendPartyMemberInfo(it->first, info, true);
				}
			}
		}
	}
	void partyCreateRelayEvent(const unsigned int &senderID, const unsigned int &targetID, const unsigned int &partyID)
	{
		const pair<SystemAddress, PlayerToken*> target = getPlayerTokenByCharID(targetID);
		//Target is here
		if (target.second)
		{
			//Provide newcomer with members' info
			const vector<PartyMemberInfo> memberList = mDBMgr->getPartyMemberList(partyID);
			for (vector<PartyMemberInfo>::const_iterator it = memberList.begin(); it != memberList.end(); it++)
			{
				const PartyMemberInfo info = *it;
				if (info.mCharID != targetID)mNetMgr->sendPartyMemberInfo(target.first, info, true);
			}

			target.second->mPartyID = partyID;
		}

		const pair<SystemAddress, PlayerToken*> sender = getPlayerTokenByCharID(senderID);
		//Sender is here
		if (sender.second)
		{
			//Provide newcomer with members' info
			const vector<PartyMemberInfo> memberList = mDBMgr->getPartyMemberList(partyID);
			for (vector<PartyMemberInfo>::const_iterator it = memberList.begin(); it != memberList.end(); it++)
			{
				const PartyMemberInfo info = *it;
				if (info.mCharID != senderID)mNetMgr->sendPartyMemberInfo(sender.first, info, true);
			}

			sender.second->mPartyID = partyID;
		}
	}
	void charSkill(const SystemAddress &add, const unsigned char &nSkill, const bool &bFlag, const bool &bPersistent)
	{
		TokenMap::iterator sender = mToken.find(add);
		if (sender == mToken.end())return;

		PlayerToken *token = &sender->second;
		const unsigned int nCharID = token->mCharID;

		if (bPersistent)token->m_nSkill = (bFlag ? nSkill : 0);

		for (TokenMap::iterator it = mToken.begin(); it != mToken.end(); it++)
		{
			const SystemAddress target = it->first;
			if (target != add)mNetMgr->sendCharSkill(target, nCharID, nSkill, bFlag);
		}
	}
};

#endif


// ##################################################################################################
// ##-------------------------------------------FILE END-------------------------------------------##
// ##################################################################################################
// ##-------------------------------------------MODIFIED-------------------------------------------##
// ## DATE: 9/25/2026                                                                              ##
// ## TIME: 6:33 PM (UTC-5:00)                                                                     ##
// ## BY: radioactivedark                                                                          ##
// ##################################################################################################