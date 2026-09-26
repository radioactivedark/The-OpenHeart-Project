// ##################################################################################################
// ##------------------------------------------FILE START------------------------------------------##
// ##################################################################################################


#ifndef _SERVERMANAGER_H_
#define _SERVERMANAGER_H_

#define MAX_LOGINFAIL_DURATION 300
#define MAX_LOGINFAIL_ATTEMPT 5
#define BAN_DURATION "'00:15:00.000000'"

#include <NetworkManager.h>
#include <DatabaseManager.h>
#include <InputManager.h>

#include <iostream>
//#include <process.h>
#include <signal.h>
#include <fstream>

using namespace std;

struct PlayerToken
{
	unsigned int mID;
	string mName;
	unsigned char mNumChars;
	string mMacAdd;
	PlayerToken()
	{
		mID = 0;
		mName = "";
		mNumChars = 0;
		mMacAdd = "";
	}
};

struct FailedLogin
{
	unsigned int mID;
	SystemAddress mIP;
	string mMac;
	unsigned char mAttempts;
	float mTimeElapsed;
	FailedLogin()
	{
		mID = 0;
		mIP = UNASSIGNED_SYSTEM_ADDRESS;
		mMac = "";
		mAttempts = 1;
		mTimeElapsed = 0;
	}
};

typedef map<SystemAddress, PlayerToken> TokenMap;

class ServerManager : public NetworkListener, public InputListener
{
private:
	NetworkManager *mNetMgr;
	DatabaseManager *mDBMgr;
	InputManager *mInputMgr;
	bool mRunning;
	TokenMap mToken;
	time_t mPrevTime;
	float mDBTimer;
	float mPollShutdownTimer;
	vector<FailedLogin> mFailedLoginList;
	unsigned long mChild_pID;
public:
	ServerManager()
	{
		mNetMgr = new NetworkManager(this);
		mDBMgr = new DatabaseManager();
		mInputMgr = new InputManager(this);
		mRunning = false;
		mChild_pID = 0;
	}
	~ServerManager()
	{
		delete mInputMgr;
		delete mDBMgr;
		delete mNetMgr;
	}
	const bool init()
	{
		cout << "FrontServer Initializing..." << endl;
		srand(time(NULL));
		time(&mPrevTime);
		mDBTimer = 0;
		mPollShutdownTimer = 0;
		return (mDBMgr->init() && mNetMgr->init());
	}
	void run()
	{
		executeProcess();
		cout << "FrontServer Running..." << endl;
		mRunning = true;
		while (mRunning)
		{
			mNetMgr->update();
			mInputMgr->update();
			const float timeElapsed = updateTimer();
			updateDBTimer(timeElapsed);
			updateFailedLogins(timeElapsed);
			updatePollShutdown(timeElapsed);
		}
	}
	const float updateTimer()
	{
		time_t currTime;
		time(&currTime);
		const float timeSinceLastUpdate = (float)difftime(currTime, mPrevTime);
		mPrevTime = currTime;

		return timeSinceLastUpdate;
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
	void updateFailedLogins(const float &timeElapsed)
	{
		vector<FailedLogin>::iterator it = mFailedLoginList.begin();
		while (it != mFailedLoginList.end())
		{
			FailedLogin *fl = &*it;
			fl->mTimeElapsed += timeElapsed;
			if (fl->mTimeElapsed>MAX_LOGINFAIL_DURATION)it = mFailedLoginList.erase(it);
			else it++;
		}
	}
	void updatePollShutdown(const float &timeElapsed)
	{
		mPollShutdownTimer += timeElapsed;
		//Refresh connection
		if (mPollShutdownTimer>60)
		{
			mPollShutdownTimer = 0;
			ifstream pollFile("poll");
			char signal = 0;
			if (pollFile.good())signal = pollFile.get();
			pollFile.close();
			if (signal == '0')mRunning = false;
		}
	}
	FailedLogin* getFailedLogin(const SystemAddress &add, const string &mac)
	{
		for (vector<FailedLogin>::iterator it = mFailedLoginList.begin(); it != mFailedLoginList.end(); it++)
		{
			FailedLogin *fl = &*it;
			if (fl->mIP.EqualsExcludingPort(add) || fl->mMac == mac)return fl;
		}
		return 0;
	}
	const bool executeProcess()
	{
		PROCESS_INFORMATION pi;
		STARTUPINFO si;

		// Set up the start up info struct.
		memset(&si, 0,  sizeof(STARTUPINFO));
		si.cb = sizeof(STARTUPINFO);

		// Launch the child process.
		const bool result = CreateProcess(
		"FHMainServer.exe",
		"",
		NULL, NULL,
		TRUE,
		CREATE_NEW_CONSOLE,
		NULL, NULL,
		&si,
		&pi);

		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );

		return result;

		/*mChild_pID = fork();
		if (mChild_pID != 0)return true;
		return execl("./FHMainServer", "FHMainServer", (char*)NULL);*/
	}
	void shutdown()
	{
		cout << "FrontServer Shutting down..." << endl;
		mNetMgr->shutdown();
		mDBMgr->shutdown();

		//const int killReturn = kill(mChild_pID, SIGUSR1);//SIGKILL);
		//if (killReturn == 3)cout << "pID does not exist!" << endl;
		//else if (killReturn == 1)cout << "No permission to send signal!" << endl;
		//cout << "Shutdown complete. Press enter to close." << endl;
		//cin.get();
	}
	void inputEvent(const string &input)
	{
		if (input == "quit")mRunning = false;
		else if (input == "stats")mNetMgr->printStats();
	}
	void allocateToken(const SystemAddress &add, const string &user, const unsigned int &id, const string &macAdd)
	{
		PlayerToken token;
		token.mName = user;
		token.mID = id;
		token.mMacAdd = macAdd;
		mToken[add] = token;
	}
	const string generateToken(const unsigned int &id)
	{
		string seed;
		const char x[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
			'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
			'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z' };
		for (int i = 0; i<10; i++)
		{
			const unsigned char c = rand() % 62;
			seed.push_back(x[c]);
		}
		return toString(id) + seed;
	}
	void deallocateToken(const SystemAddress &add)
	{
		TokenMap::iterator it = mToken.find(add);
		if (it == mToken.end())return;
		mToken.erase(it);
	}
	void disconnectEvent(const SystemAddress &add)
	{
		deallocateToken(add);
	}
	void saltRequest(const SystemAddress &add, const string &user, const string &macAdd)
	{
		string salt = "";
		unsigned int id = 0;
		const bool flag = mDBMgr->getSalt(user, salt, id);
		if (flag)
		{
			double banSecs = 0;
			const unsigned char isBanned = mDBMgr->isBanned(id, add.ToString(false), macAdd, banSecs);
			if (isBanned)
			{
				mNetMgr->sendLogin(add, false, true, int(banSecs) / 60);
				if (isBanned == DatabaseManager::BAN_USER)mDBMgr->spreadBan(id, add.ToString(false), macAdd);
			}
			else
			{
				allocateToken(add, user, id, macAdd);
				mNetMgr->sendSalt(add, salt);
			}
		}
		else
		{
			mNetMgr->sendLogin(add, false);
			mDBMgr->updateLoginAttempt(id, false, add.ToString(false), macAdd);
		}
	}
	void loginRequest(const SystemAddress &add, const string &hash)
	{
		TokenMap::iterator it = mToken.find(add);
		if (it == mToken.end())
		{
			mNetMgr->sendLogin(add, false);
			mDBMgr->updateLoginAttempt(0, false, add.ToString(false), "");
			return;
		}

		PlayerToken *token = &it->second;
		const bool flag = mDBMgr->matchPassword(token->mName, hash);
		mNetMgr->sendLogin(add, flag);
		mDBMgr->updateLoginAttempt(token->mID, flag, add.ToString(false), token->mMacAdd);
		//Add failed logins to list
		if (!flag)
		{
			FailedLogin *fl = getFailedLogin(add, token->mMacAdd);
			if (fl)
			{
				fl->mAttempts += 1;
				if (fl->mAttempts >= MAX_LOGINFAIL_ATTEMPT)
				{
					mDBMgr->updateBan(fl->mID, fl->mIP.ToString(false), fl->mMac);
					//Terminate failed login entry
					fl->mTimeElapsed = MAX_LOGINFAIL_DURATION;
				}
			}
			else
			{
				FailedLogin nFL;
				nFL.mID = token->mID;
				nFL.mIP = add;
				nFL.mMac = token->mMacAdd;
				mFailedLoginList.push_back(nFL);
			}
		}
	}
	void charSelect(const SystemAddress &add, const char &selection)
	{
		TokenMap::iterator it = mToken.find(add);
		if (it == mToken.end())return;
		CharacterData charData;
		unsigned char numChars = 0;
		if (mDBMgr->getCharacter(it->second.mID, selection, &charData, &numChars))
		{
			mNetMgr->sendCharSelect(add, selection, &charData, numChars);
			it->second.mNumChars = numChars;
		}
		else mNetMgr->sendCharSelect(add, selection);
	}
	void charCreate(const SystemAddress &add, CharacterData *data)
	{
		TokenMap::iterator it = mToken.find(add);
		if (it == mToken.end())return;
		bool success = (it->second.mNumChars<MAX_CHARACTERS);
		unsigned int availableID = 0;
		char selection = 0;
		if (success)
		{
			if (mDBMgr->getAvailableCharacterID(availableID))
			{
				data->mID = availableID;
				success = mDBMgr->updateCharacter(it->second.mID, data, true);
			}
			else success = mDBMgr->saveCharacter(it->second.mID, data);
			if (success)mDBMgr->getCharacterOrder(it->second.mID, data->mID, selection);
		}
		mNetMgr->sendCharCreate(add, success, selection);
	}
	void charUpdate(const SystemAddress &add, CharacterData *data)
	{
		TokenMap::iterator it = mToken.find(add);
		if (it == mToken.end())return;
		const bool success = mDBMgr->updateCharacter(it->second.mID, data);
		mNetMgr->sendCharCreate(add, success, 0);
	}
	void charDelete(const SystemAddress &add, const unsigned int &id)
	{
		TokenMap::iterator it = mToken.find(add);
		if (it == mToken.end())return;
		mDBMgr->deleteCharacter(id);
	}
	void connectMainEvent(const SystemAddress &add, const unsigned int &charID)
	{
		TokenMap::iterator it = mToken.find(add);
		if (it == mToken.end())return;
		PlayerToken *pt = &it->second;
		const string token = generateToken(pt->mID);
		mNetMgr->sendTokenToClient(add, token);
		//mNetMgr->sendTokenToMain(add,token,pt->mID,charID);
		mDBMgr->storeCharacterToken(charID, token);
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