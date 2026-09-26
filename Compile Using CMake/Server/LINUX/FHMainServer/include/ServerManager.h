// ##################################################################################################
// ##------------------------------------------FILE START------------------------------------------##
// ##################################################################################################


#ifndef _SERVERMANAGER_H_
#define _SERVERMANAGER_H_

#include <NetworkManager.h>
#include <DatabaseManager.h>
#include <InputManager.h>
#include <Chat.h>

#include <iostream>
#include <string.h>
#include <sys/timeb.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#define TIME_SPEED 0.334f   //5mins real time = 1h game time

using namespace std;

volatile sig_atomic_t usr_interrupt = 0;

void synch_signal(int sig)
{
	usr_interrupt = 1;
}




struct PlayerTokenMini
{
	string mToken;
	unsigned int mUserID;
	unsigned int mCharID;
	PlayerTokenMini(const string &token, const unsigned int &userID, const unsigned int &charID)
	{
		mToken = token;
		mUserID = userID;
		mCharID = charID;
	}
};

struct MapServerToken
{
	unsigned short mPort;
	string mMap;
	SystemAddress mAdd;
	vector<PlayerTokenMini> mTokenQueue;
	MapServerToken()
	{
		mPort = 0;
		mMap = "";
		mAdd = UNASSIGNED_SYSTEM_ADDRESS;
	}
};

typedef map<string, PlayerToken> TokenMap;

class ServerManager : public NetworkListener, public InputListener
{
private:
	NetworkManager *mNetMgr;
	DatabaseManager *mDBMgr;
	InputManager *mInputMgr;
	bool mRunning;
	TokenMap mToken;
	vector<MapServerToken> mMapServer;
	//time_t mPrevTime;
	struct timeb mPrevTime;
	float mDayTime;
	float mWeatherTime;
	float mDBTimer;
	vector<bool> mPartyIDPool;
public:
	ServerManager()
	{
		mNetMgr = new NetworkManager(this);
		mDBMgr = new DatabaseManager();
		mInputMgr = new InputManager(this);
		mRunning = false;

		//PartyID 0 cannot be used
		mPartyIDPool.push_back(true);
	}
	~ServerManager()
	{
		delete mInputMgr;
		delete mDBMgr;
		delete mNetMgr;
	}
	const bool init()
	{
		cout << "MainServer Initializing..." << endl;

		ftime(&mPrevTime);
		mDayTime = 1000;
		mWeatherTime = 0;
		mDBTimer = 0;

		return (mDBMgr->init() && mNetMgr->init());
	}
	void run()
	{
		struct sigaction usr_action;
		sigset_t block_mask;

		sigfillset(&block_mask);
		usr_action.sa_handler = synch_signal;
		usr_action.sa_mask = block_mask;
		usr_action.sa_flags = 0;
		sigaction(SIGUSR1, &usr_action, NULL);

		signal(SIGCHLD, SIG_IGN);

		mDBMgr->clearAllOnlineStatus();
		cout << "MainServer Running..." << endl;
		mRunning = true;
		while (mRunning)
		{
			mNetMgr->update();
			//mInputMgr->update();
			const float timeElapsed = updateTimers();
			updateTokens(timeElapsed);
			updateDBTimer(timeElapsed);

			if (usr_interrupt == 1)break;
		}
	}
	const float updateTimers()
	{
		struct timeb currTime;
		ftime(&currTime);
		const float timeSinceLastUpdate = (currTime.millitm - mPrevTime.millitm)*0.001f + difftime(currTime.time, mPrevTime.time);
		mPrevTime = currTime;

		const float skyTimeElapsed = timeSinceLastUpdate * TIME_SPEED;
		mDayTime += skyTimeElapsed;
		if (mDayTime>2400)mDayTime -= 2400;
		mWeatherTime += skyTimeElapsed;
		if (mWeatherTime>7000)mWeatherTime -= 7000;

		return timeSinceLastUpdate;
	}
	void updateTokens(const float &timeElapsed)
	{
		TokenMap::iterator it = mToken.begin();
		while (it != mToken.end())
		{
			PlayerToken *token = &it->second;
			if (token->mUnconnected)
			{
				token->mTimer -= timeElapsed;
				//Token expired, remove
				if (token->mTimer <= 0)
				{
					if (token->mMapAdd != UNASSIGNED_SYSTEM_ADDRESS)
					{
						//Update player online status and clear party
						const unsigned int partyID = mDBMgr->getCharacterParty(token->mCharID);
						if (partyID>0 && mDBMgr->getNumPartyMembers(partyID) <= 0)unassignPartyID(partyID);
						mDBMgr->setCharacterOnlineStatus(token->mCharID, false, true);
						//Send to each map server to broadcast to all players
						for (vector<MapServerToken>::iterator mit = mMapServer.begin(); mit != mMapServer.end(); mit++)
						{
							MapServerToken mapToken = *mit;
							mNetMgr->sendCharOnlineStatus(mapToken.mAdd, token->mUserID, token->mCharID, false);
						}
					}

					mToken.erase(it);
					it = mToken.begin();
					continue;
				}
			}
			it++;
		}
	}
	void updateDBTimer(const float &timeElapsed)
	{
		mDBTimer += timeElapsed;
		//Refresh connection
		if (mDBTimer>1800)
		{
			mDBTimer = 0;
			if (!mDBMgr->pingDB())mRunning = mDBMgr->init(true);
		}
	}
	void shutdown()
	{
		cout << "MainServer Shutting down..." << endl;
		mNetMgr->shutdown();
		mDBMgr->shutdown();
		//cout << "Shutdown complete. Press enter to close." << endl;
		//cin.get();
	}
	void inputEvent(const string &input)
	{
		if (input == "quit")mRunning = false;
		else if (input == "stats")mNetMgr->printStats();
		else if (input == "time")cout << "Daytime: " << toString(mDayTime) << " Weathertime: " << toString(mWeatherTime) << endl;
	}
	PlayerToken* tokenReceived(const string &token, const unsigned int &userID, const unsigned int &charID)
	{
		PlayerToken pt;
		pt.mUserID = userID;
		pt.mCharID = charID;
		pt.mUsername = mDBMgr->getUserName(pt.mUserID);
		mToken[token] = pt;
		return &mToken[token];
	}
	void connectEvent(const SystemAddress &add, const string &token, const unsigned int &charID = 0, const string &ghostMap = "")
	{
		PlayerToken *pt = 0;
		bool newConnection = false;
		//New connection
		if (charID != 0)
		{
			const unsigned int userID = mDBMgr->getUserID(charID);
			//Boot duplicate login
			if (mDBMgr->clearUserOnlineStatus(userID))
			{
				PlayerToken *dupToken = getPlayerTokenByUserID(userID);
				if (dupToken)
				{
					mNetMgr->sendDuplicateLoginEvent(dupToken->mMapAdd, dupToken->mCharID);
					erasePlayerToken(dupToken);
				}
			}
			if (mDBMgr->matchToken(charID, token))pt = tokenReceived(token, userID, charID);
			else
			{
				mNetMgr->sendTokenFail(add);
				return;
			}
			newConnection = true;
		}
		//Map change
		else
		{
			TokenMap::iterator it = mToken.find(token);
			if (it == mToken.end())
			{
				mNetMgr->sendTokenFail(add);
				return;
			}
			pt = &it->second;
		}
		if (!pt)return;
		pt->mTimer = TOKEN_TIMER;

		string map = "";
		if (ghostMap == "")mDBMgr->getCharacterInfo(pt->mCharID, map);
		else map = ghostMap;

		const unsigned short mapPort = getMapPort(map, PlayerTokenMini(token, pt->mUserID, pt->mCharID));

		//Trim off seed to send to player
		if (map.length()>10)map.erase(map.length() - 10, 10);
		mNetMgr->sendConnect(add, mapPort, map, mDayTime, mWeatherTime);
	}
	void serverConnectEvent(const SystemAddress &add)
	{
		for (vector<MapServerToken>::iterator it = mMapServer.begin(); it != mMapServer.end(); it++)
		{
			MapServerToken *token = &*it;
			if (token->mPort == add.GetPort())
			{
				token->mAdd = add;
				while (!token->mTokenQueue.empty())
				{
					PlayerTokenMini pt = token->mTokenQueue.back();
					token->mTokenQueue.pop_back();
					mNetMgr->sendTokenToMap(add, pt.mToken, pt.mUserID, pt.mCharID);
				}
				return;
			}
		}
	}
	void disconnectEvent(const SystemAddress &add, const bool &isConnectionLost)
	{
		//Check if disconnectee is server
		bool isServer = false;
		for (vector<MapServerToken>::iterator it = mMapServer.begin(); it != mMapServer.end(); it++)
		{
			MapServerToken token = *it;
			if (token.mAdd == add)
			{
				isServer = true;
				mMapServer.erase(it);
				break;
			}
		}

		//Initiate loose clients' expiry like a map change event
		if (isServer && isConnectionLost)
		{
			for (TokenMap::iterator it = mToken.begin(); it != mToken.end(); it++)
			{
				PlayerToken *pt = &it->second;
				if (pt->mMapAdd == add)
				{
					pt->mUnconnected = true;
					pt->mTimer = TOKEN_TIMER;
				}
			}
		}
	}
	const unsigned short getMapPort(const string &map, const PlayerTokenMini &pToken)
	{
		for (vector<MapServerToken>::iterator it = mMapServer.begin(); it != mMapServer.end(); it++)
		{
			MapServerToken *token = &*it;
			if (token->mMap == map)
			{
				if (token->mAdd != UNASSIGNED_SYSTEM_ADDRESS)mNetMgr->sendTokenToMap(token->mAdd, pToken.mToken, pToken.mUserID, pToken.mCharID);
				else token->mTokenQueue.push_back(pToken);
				return token->mPort;
			}
		}
		return spawnMapServer(map, pToken);
	}
	const unsigned short spawnMapServer(const string &map, const PlayerTokenMini &pToken)
	{
		unsigned short port = MAP_SERVER_PORT;
		vector<MapServerToken> copyVect = mMapServer;
		bool found = false;
		while (!found)
		{
			found = true;
			for (vector<MapServerToken>::iterator it = copyVect.begin(); it != copyVect.end(); it++)
			{
				MapServerToken token = *it;
				if (token.mPort == port)
				{
					found = false;
					copyVect.erase(it);
					break;
				}
			}
			if (!found)port++;
		}
		executeProcess(toString(port), map);

		MapServerToken token;
		token.mMap = map;
		token.mPort = port;
		token.mTokenQueue.push_back(pToken);
		mMapServer.push_back(token);

		return port;
	}
	const bool executeProcess(const string &param1, const string &param2)
	{
		/*STARTUPINFO si = { sizeof(si) };
		PROCESS_INFORMATION pi;

		char commandLine[64];
		strcpy(commandLine, " ");
		strcat(commandLine, param1.c_str());
		strcat(commandLine, " ");
		strcat(commandLine, param2.c_str());

		// Launch the child process.
		const bool result = CreateProcess(
			"FHMapServer.exe",
			commandLine,
			NULL, NULL,
			TRUE,
			CREATE_NEW_CONSOLE,
			NULL, NULL,
			&si,
			&pi);

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

		return result;*/
		pid_t child_pid = fork();
		if(child_pid!=0)return true;
		return execl("./FHMapServer","FHMapServer",param1.c_str(),param2.c_str(),(char*)NULL);
	}
	void terminateEvent(const bool &isProper)
	{
		if (isProper)mRunning = false;
		else mMapServer.clear();
	}
	void mapConnectEvent(const SystemAddress &add, const string &token, const bool &isFirstEntrance)
	{
		TokenMap::iterator it = mToken.find(token);
		if (it == mToken.end())return;

		PlayerToken *pt = &it->second;
		pt->mUnconnected = false;
		pt->mMapAdd = add;

		if (isFirstEntrance)
		{
			//Update player online status
			mDBMgr->setCharacterOnlineStatus(pt->mCharID, true);
			//Send to each map server to broadcast to all players
			for (vector<MapServerToken>::iterator it = mMapServer.begin(); it != mMapServer.end(); it++)
			{
				MapServerToken token = *it;
				mNetMgr->sendCharOnlineStatus(token.mAdd, pt->mUserID, pt->mCharID, true);
			}
		}
	}
	void mapDisconnectEvent(const string &token, const bool &isMapChange)
	{
		TokenMap::iterator it = mToken.find(token);
		if (it == mToken.end())return;

		PlayerToken *pt = &it->second;
		//Player disconnected
		if (!isMapChange)
		{
			//Update player online status
			mDBMgr->setCharacterOnlineStatus(pt->mCharID, false, true);
			//Send to each map server to broadcast to all players
			for (vector<MapServerToken>::iterator mit = mMapServer.begin(); mit != mMapServer.end(); mit++)
			{
				MapServerToken token = *mit;
				mNetMgr->sendCharOnlineStatus(token.mAdd, pt->mUserID, pt->mCharID, false);
			}
			mToken.erase(it);
		}
		//Player changing map
		else
		{
			pt->mUnconnected = true;
			pt->mTimer = TOKEN_TIMER;
		}
	}
	PlayerToken* getPlayerTokenByUserID(const unsigned int &userID)
	{
		for (TokenMap::iterator it = mToken.begin(); it != mToken.end(); it++)
		{
			PlayerToken *pt = &it->second;
			if (pt->mUserID == userID)return pt;
		}
		return 0;
	}
	void erasePlayerToken(PlayerToken *pt)
	{
		for (TokenMap::iterator it = mToken.begin(); it != mToken.end(); it++)
		{
			if (&it->second == pt)
			{
				mToken.erase(it);
				return;
			}
		}
	}
	void chatRelayEvent(const SystemAddress &sendingMap, const unsigned int &userID, const unsigned int &charID, const string &caption, const unsigned char &channel)
	{
		switch (channel)
		{
		case CHAT_WHISPER:
		{
			PlayerToken *pt = getPlayerTokenByUserID(userID);
			if (pt)mNetMgr->sendChatRelay(pt->mMapAdd, userID, charID, caption, channel);
		}
		break;
		case CHAT_GROUP:
		case CHAT_PARTY:
		case CHAT_SYSTEM:
		case CHAT_MOD:
			for (vector<MapServerToken>::iterator it = mMapServer.begin(); it != mMapServer.end(); it++)
			{
				MapServerToken mit = *it;
				const SystemAddress add = mit.mAdd;
				if (add != sendingMap)mNetMgr->sendChatRelay(add, userID, charID, caption, channel);
			}
			break;
		default: break;
		}
	}
	void friendRelayEvent(const SystemAddress &sendingMap, const bool &isRemove, const bool &isBlockedBy, const unsigned int &senderID, const unsigned int &targetID, const bool &isRequest)
	{
		PlayerToken *pt = getPlayerTokenByUserID(targetID);
		if (pt)mNetMgr->sendFriendRelay(pt->mMapAdd, isRemove, isBlockedBy, senderID, targetID, isRequest);
	}
	void myGroupRelayEvent(const SystemAddress &sendingMap, const unsigned int &groupID, const string &groupname)
	{
		for (vector<MapServerToken>::iterator it = mMapServer.begin(); it != mMapServer.end(); it++)
		{
			MapServerToken mit = *it;
			const SystemAddress add = mit.mAdd;
			if (add != sendingMap)mNetMgr->sendMyGroupRelay(add, groupID, groupname);
		}
	}
	void myGroupLeaveRelayEvent(const SystemAddress &sendingMap, const unsigned int &groupID, const unsigned int &charID, const bool &isKick, const bool &isDisband)
	{
		for (vector<MapServerToken>::iterator it = mMapServer.begin(); it != mMapServer.end(); it++)
		{
			MapServerToken mit = *it;
			const SystemAddress add = mit.mAdd;
			if (add != sendingMap)mNetMgr->sendMyGroupLeaveRelay(add, groupID, charID, isKick, isDisband);
		}
	}
	void groupRequestRelayEvent(const SystemAddress &sendingMap, const bool &isAccept, const unsigned int &senderID, const unsigned int &targetID, const bool &isRequest)
	{
		for (vector<MapServerToken>::iterator it = mMapServer.begin(); it != mMapServer.end(); it++)
		{
			MapServerToken mit = *it;
			const SystemAddress add = mit.mAdd;
			if (add != sendingMap)mNetMgr->sendGroupRequestRelay(add, isAccept, senderID, targetID, isRequest);
		}
	}
	const unsigned int getAvailablePartyID()
	{
		unsigned int partyID = 1;
		for (vector<bool>::iterator it = mPartyIDPool.begin(); it != mPartyIDPool.end(); it++)
		{
			if (it == mPartyIDPool.begin())continue;
			const bool taken = *it;
			if (!taken)
			{
				mPartyIDPool[partyID] = true;
				return partyID;
			}
			partyID++;
		}
		mPartyIDPool.push_back(true);

		return partyID;
	}
	void unassignPartyID(const unsigned int &id)
	{
		if (id>0 && id<mPartyIDPool.size())
		{
			mPartyIDPool[id] = false;
		}
	}
	void partyCreateRelayEvent(const SystemAddress &sendingMap, const unsigned int &senderID, const unsigned int &targetID)
	{
		const int partyID = getAvailablePartyID();

		mDBMgr->setCharacterParty(senderID, partyID);
		mDBMgr->setCharacterParty(targetID, partyID);

		for (vector<MapServerToken>::iterator it = mMapServer.begin(); it != mMapServer.end(); it++)
		{
			MapServerToken mit = *it;
			const SystemAddress add = mit.mAdd;
			mNetMgr->sendPartyCreateRelay(add, senderID, targetID, partyID);
		}
	}
	void partyLeaveRelayEvent(const SystemAddress &sendingMap, const unsigned int &senderID, const unsigned int &partyID)
	{
		for (vector<MapServerToken>::iterator it = mMapServer.begin(); it != mMapServer.end(); it++)
		{
			MapServerToken mit = *it;
			const SystemAddress add = mit.mAdd;
			if (add != sendingMap)mNetMgr->sendPartyLeaveRelay(add, senderID, partyID);
		}

		if (mDBMgr->getNumPartyMembers(partyID) <= 0)unassignPartyID(partyID);
	}
	void partyRequestRelayEvent(const SystemAddress &sendingMap, const bool &isAccept, const unsigned int &senderID, const unsigned int &targetID)
	{
		for (vector<MapServerToken>::iterator it = mMapServer.begin(); it != mMapServer.end(); it++)
		{
			MapServerToken mit = *it;
			const SystemAddress add = mit.mAdd;
			if (add != sendingMap)mNetMgr->sendPartyRequestRelay(add, isAccept, senderID, targetID);
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