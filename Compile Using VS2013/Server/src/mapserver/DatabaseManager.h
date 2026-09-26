// ##################################################################################################
// ##------------------------------------------FILE START------------------------------------------##
// ##################################################################################################


#ifndef _DATABASEMANAGER_H_
#define _DATABASEMANAGER_H_

#define START_MAPNAME "'fhLonelyCavec4k3UMIs54'"//"'fhBonfireIsland0lK6441G3N'"
#define START_POSX 2500//7600
#define START_POSY 78//160
#define START_POSZ 2500//7600

#define FH_CHARACTERS "fh_characters"
#define FH_APPEARANCE "fh_appearance"
#define JOS_USERS "jos_users"

#include "DataTypes.h"
#include <mysql++.h>
#include <iostream>
#include <sstream>
#include <fstream>

using namespace mysqlpp;
using namespace std;

template <class T>
inline const string toString(const T& t)
{
	stringstream ss;
	ss << t;
	return ss.str();
}

class DatabaseManager
{
private:
	Connection mDB;
	bool mConnected;
public:
	DatabaseManager()
	{
		mDB = Connection(false);
		mConnected = false;
	}
	~DatabaseManager()
	{
	}
	const bool init()
	{
		char database[256] = "";
		char ip[256] = "";
		char user[256] = "";
		char pass[256] = "";
		ifstream inFile("db", ios::in);
		if (inFile.good())
		{
			inFile.getline(database, 256);
			inFile.getline(ip, 256);
			inFile.getline(user, 256);
			inFile.getline(pass, 256);
		}
		inFile.close();

		if ((mDB.connect(database, ip, user, pass)))
		{
			//cout << "DB connected" << endl;
			mConnected = true;
		}
		else
		{
			cout << "DB connection failed: " << mDB.error() << endl;
			mConnected = false;
		}
		return mConnected;
	}
	void shutdown()
	{
		if (mConnected)mDB.disconnect();
		mConnected = false;
	}
	const bool isConnected()
	{
		return mConnected;
	}
	const int parseInt(const String &value)
	{
		return atoi(value.c_str());
	}
	const long parseLong(const String &value)
	{
		return atol(value.c_str());
	}
	const bool getCharacterInfo(const unsigned int &id, CharacterData *data, int &x, int &y, int &z, unsigned int &groupID, unsigned int &partyID)
	{
		string queryBuffer = "SELECT name, pos_x, pos_y, pos_z, group_id, party_id FROM " + string(FH_CHARACTERS) + " WHERE id=" + toString(id);

		Query query = mDB.query(queryBuffer);
		StoreQueryResult res = query.store();
		if (!res || res.empty())return false;
		else
		{
			Row row = res.front();
			Row::iterator col = row.begin();
			data->mID = id;
			const String name = *col; col++;
			data->mName = name.c_str();
			x = parseInt(*col); col++;
			y = parseInt(*col); col++;
			z = parseInt(*col); col++;
			groupID = parseInt(*col); col++;
			partyID = parseInt(*col);
		}

		//Select appearance
		queryBuffer = "SELECT species, ";
		for (int i = 0; i<MAX_MESHES; i++)queryBuffer += "mesh_" + toString(i) + ", ";
		for (int i = 0; i<MAX_MATS; i++)queryBuffer += "mat_" + toString(i) + ", ";
		for (int i = 0; i<NUM_SCALABLES; i++)queryBuffer += "scale_" + toString(i) + ", ";
		for (int i = 0; i<NUM_COLOURABLES; i++)
			for (int j = 0; j<3; j++)queryBuffer += "colour_" + toString(i) + "_" + toString(j) + ", ";
		queryBuffer += "emote, preset, equip, items ";
		queryBuffer += "FROM " + string(FH_APPEARANCE) + " WHERE id=" + toString(id);

		Query query2 = mDB.query(queryBuffer);
		StoreQueryResult res2 = query2.store();
		if (!res2 || res2.empty())return false;
		else
		{
			Row row = res2.front();
			Row::iterator col = row.begin();
			const String species = *col; col++;
			if (species.length()>0)data->mSpecies = species.c_str()[0];
			else data->mSpecies = 'f';
			for (int i = 0; i<MAX_MESHES; i++){ data->mMesh[i] = parseInt(*col); col++; }
			for (int i = 0; i<MAX_MATS; i++){ data->mMat[i] = parseInt(*col); col++; }
			for (int i = 0; i<NUM_SCALABLES; i++){ data->mScale[i] = parseInt(*col); col++; }
			for (int i = 0; i<NUM_COLOURABLES; i++)
				for (int j = 0; j<3; j++){ data->mColour[i][j] = parseInt(*col); col++; }
			data->mEmote = parseInt(*col); col++;
			data->mPreset = parseInt(*col); col++;
			data->m_nEquip = parseInt(*col); col++;
			const String szItems = *col;
			data->m_szItems = szItems.c_str();
		}
		return true;
	}
	const bool saveCharacter(const unsigned int &charID, const float &posX, const float &posY, const float &posZ)
	{
		string queryBuffer = "UPDATE " + string(FH_CHARACTERS) + " SET ";
		queryBuffer += "pos_x=" + toString((int)posX) + ",";
		queryBuffer += "pos_y=" + toString((int)posY) + ",";
		queryBuffer += "pos_z=" + toString((int)posZ);
		queryBuffer += " WHERE id=" + toString(charID);

		Query query = mDB.query(queryBuffer);
		SimpleResult res = query.execute();

		if (!res)cout << query.error() << endl;

		return res;
	}
	const bool saveCharacterMap(const unsigned int &charID, const string &map)
	{
		string queryBuffer = "UPDATE " + string(FH_CHARACTERS) + " SET ";
		queryBuffer += "map='" + map + "'";
		queryBuffer += " WHERE id=" + toString(charID);

		Query query = mDB.query(queryBuffer);
		SimpleResult res = query.execute();

		if (!res)cout << query.error() << endl;

		return res;
	}
	const bool setCharacterEmote(const unsigned int &charID, const unsigned char &nEmote)
	{
		string queryBuffer = "UPDATE " + string(FH_APPEARANCE) + " SET ";
		queryBuffer += "emote=" + toString((int)nEmote);
		queryBuffer += " WHERE id=" + toString(charID);

		Query query = mDB.query(queryBuffer);
		SimpleResult res = query.execute();

		if (!res)cout << query.error() << endl;

		return res;
	}
	const unsigned int getUserID(const string &username)
	{
		string queryBuffer = "SELECT id FROM " + string(JOS_USERS) + " WHERE username='" + username + "' limit 1";

		Query query = mDB.query(queryBuffer);
		StoreQueryResult res = query.store();
		if (res && !res.empty())
		{
			Row row = res.front();
			if (row.size()>0)return parseInt(row.front());
		}
		return 0;
	}
	const string getUserName(const unsigned int &id)
	{
		Query query = mDB.query("SELECT username FROM " + string(JOS_USERS) + " WHERE id=" + toString(id) + " LIMIT 1");
		StoreQueryResult res = query.store();
		if (res && !res.empty())
		{
			Row row = res.front();
			if (!row.empty())return row[0].c_str();
		}
		return "";
	}
	const string getCharacterName(const unsigned int &id)
	{
		Query query = mDB.query("SELECT name FROM " + string(FH_CHARACTERS) + " WHERE id=" + toString(id));
		StoreQueryResult res = query.store();
		if (res && !res.empty())
		{
			Row row = res.front();
			if (!row.empty())return row[0].c_str();
		}
		return "";
	}
	const unsigned int getUserIDByCharID(const unsigned int &id)
	{
		Query query = mDB.query("SELECT user_id FROM " + string(FH_CHARACTERS) + " WHERE id=" + toString(id));
		StoreQueryResult res = query.store();
		if (res && !res.empty())
		{
			Row row = res.front();
			if (!row.empty())return parseInt(row[0]);
		}
		return 0;
	}
	const string getUserNameByCharID(const unsigned int &id, unsigned int *userID = 0)
	{
		const unsigned int nUserID = getUserIDByCharID(id);
		if (!nUserID)
		{
			if (userID)*userID = 0;
			return "";
		}

		if (userID)*userID = nUserID;

		Query query = mDB.query("SELECT username FROM " + string(JOS_USERS) + " WHERE id=" + toString(nUserID) + " LIMIT 1");
		StoreQueryResult res = query.store();
		if (res && !res.empty())
		{
			Row row = res.front();
			if (!row.empty())
			{
				return row[0].c_str();
			}
		}
		return "";
	}
	const bool addFriend(const unsigned int &senderID, const unsigned int &targetID)
	{
		Query query = mDB.query("INSERT INTO fh_friend (`sender_id`, `target_id`) VALUES(" + toString(senderID) + "," + toString(targetID) + ")");
		SimpleResult res = query.execute();
		if (!res)return false;
		return true;
	}
	const bool deleteFriend(const unsigned int &senderID, const unsigned int &targetID)
	{
		Query query = mDB.query("DELETE FROM fh_friend WHERE ((sender_id=" + toString(senderID) + " AND target_id=" + toString(targetID)
			+ ") OR " + "(target_id=" + toString(senderID) + " AND sender_id=" + toString(targetID) + "))");
		SimpleResult res = query.execute();
		if (!res)return false;
		return true;
	}
	const bool addBlock(const unsigned int &senderID, const unsigned int &targetID)
	{
		Query query = mDB.query("INSERT INTO fh_block (`sender_id`, `target_id`) VALUES(" + toString(senderID) + "," + toString(targetID) + ")");
		SimpleResult res = query.execute();
		if (!res)return false;
		return true;
	}
	const bool deleteBlock(const unsigned int &senderID, const unsigned int &targetID)
	{
		Query query = mDB.query("DELETE FROM fh_block WHERE sender_id=" + toString(senderID) + " AND target_id=" + toString(targetID));
		SimpleResult res = query.execute();
		if (!res)return false;
		return true;
	}
	const bool isBlocked(const unsigned int &senderID, const unsigned int &targetID)
	{
		Query query = mDB.query("SELECT sender_id FROM fh_block WHERE (sender_id=" + toString(senderID) + " AND target_id=" + toString(targetID)
			+ ") OR (sender_id=" + toString(targetID) + " AND target_id=" + toString(senderID) + ")");
		StoreQueryResult res = query.store();
		if (res && !res.empty())return true;
		return false;
	}
	const bool addFriendRequest(const unsigned int &senderID, const unsigned int &targetID)
	{
		Query query = mDB.query("INSERT INTO fh_friend_request (`sender_id`, `target_id`) VALUES(" + toString(senderID) + "," + toString(targetID) + ")");
		SimpleResult res = query.execute();
		if (!res)return false;
		return true;
	}
	const bool deleteFriendRequest(const unsigned int &senderID, const unsigned int &targetID)
	{
		Query query = mDB.query("DELETE FROM fh_friend_request WHERE sender_id=" + toString(senderID) + " AND target_id=" + toString(targetID));
		SimpleResult res = query.execute();
		if (!res)return false;
		return true;
	}
	const bool isCharacterOnline(const unsigned int &charID)
	{
		Query query = mDB.query("SELECT is_online FROM " + string(FH_CHARACTERS) + " WHERE id=" + toString(charID));
		StoreQueryResult res = query.store();
		if (res && !res.empty())
		{
			Row row = res.front();
			if (!row.empty())return (bool)parseInt(row[0]);
		}
		return false;
	}
	const bool isUserOnline(const unsigned int &userID)
	{
		Query query = mDB.query("SELECT is_online FROM " + string(FH_CHARACTERS) + " WHERE user_id=" + toString(userID) + " AND is_online=1 LIMIT 1");
		StoreQueryResult res = query.store();
		if (res)
		{
			for (StoreQueryResult::iterator it = res.begin(); it != res.end(); it++)
			{
				Row row = *it;
				if (!row.empty() && (bool)parseInt(row[0]))return true;
			}
		}
		return false;
	}
	const vector<unsigned int> getBlockList(const unsigned int &userID)
	{
		vector<unsigned int> blockList;
		Query query = mDB.query("SELECT target_id FROM fh_block WHERE sender_id=" + toString(userID));
		StoreQueryResult res = query.store();
		if (res)
		{
			for (StoreQueryResult::iterator it = res.begin(); it != res.end(); it++)
			{
				Row row = *it;
				if (!row.empty())blockList.push_back(parseInt(row[0]));
			}
		}
		return blockList;
	}
	const vector<unsigned int> getBlockedByList(const unsigned int &userID)
	{
		vector<unsigned int> blockedByList;
		Query query = mDB.query("SELECT sender_id FROM fh_block WHERE target_id=" + toString(userID));
		StoreQueryResult res = query.store();
		if (res)
		{
			for (StoreQueryResult::iterator it = res.begin(); it != res.end(); it++)
			{
				Row row = *it;
				if (!row.empty())blockedByList.push_back(parseInt(row[0]));
			}
		}
		return blockedByList;
	}
	const vector<unsigned int> getRequestList(const unsigned int &userID)
	{
		vector<unsigned int> requestList;
		Query query = mDB.query("SELECT sender_id FROM fh_friend_request WHERE target_id=" + toString(userID));
		StoreQueryResult res = query.store();
		if (res)
		{
			for (StoreQueryResult::iterator it = res.begin(); it != res.end(); it++)
			{
				Row row = *it;
				if (!row.empty())requestList.push_back(parseInt(row[0]));
			}
		}
		return requestList;
	}
	const vector<unsigned int> getFriendList(const unsigned int &userID)
	{
		vector<unsigned int> friendList;
		set<unsigned int> cSet;

		Query query = mDB.query("SELECT target_id FROM fh_friend WHERE sender_id=" + toString(userID));
		StoreQueryResult res = query.store();
		if (res)
		{
			for (StoreQueryResult::iterator it = res.begin(); it != res.end(); it++)
			{
				Row row = *it;
				if (!row.empty())cSet.insert(parseInt(row[0]));
			}
		}

		Query query2 = mDB.query("SELECT sender_id FROM fh_friend WHERE target_id=" + toString(userID));
		StoreQueryResult res2 = query2.store();
		if (res2)
		{
			for (StoreQueryResult::iterator it = res2.begin(); it != res2.end(); it++)
			{
				Row row = *it;
				if (!row.empty())cSet.insert(parseInt(row[0]));
			}
		}

		friendList.assign(cSet.begin(), cSet.end());

		return friendList;
	}
	const bool isFriend(const unsigned int &senderID, const unsigned int &targetID)
	{
		Query query = mDB.query("SELECT target_id FROM fh_friend WHERE ((sender_id=" + toString(senderID) + " AND target_id=" + toString(targetID)
			+ ") OR (target_id=" + toString(senderID) + " AND sender_id=" + toString(targetID) + "))");
		StoreQueryResult res = query.store();
		if (res && !res.empty())return true;
		return false;
	}
	const string getCharacterMap(const unsigned int &charID, const bool &useUserID, const bool &onlyOnline = true)
	{
		string queryBuffer = "SELECT map FROM " + string(FH_CHARACTERS) + " WHERE ";
		queryBuffer += (useUserID ? "user_id=" : "id=");
		queryBuffer += toString(charID);
		if (useUserID || onlyOnline)queryBuffer += " AND is_online=1";
		Query query = mDB.query(queryBuffer);
		StoreQueryResult res = query.store();
		if (res && !res.empty())
		{
			Row row = res.front();
			if (!row.empty())return row[0].c_str();
		}
		return "";
	}
	const bool saveCharBio(const unsigned int &charID, const string &bio)
	{
		string queryBuffer = "UPDATE " + string(FH_CHARACTERS) + " SET ";
		queryBuffer += "bio='" + fixApostrophes(bio) + "'";
		queryBuffer += " WHERE id=" + toString(charID);

		Query query = mDB.query(queryBuffer);
		SimpleResult res = query.execute();

		if (!res)cout << "MAP:" << query.error() << endl;

		return res;
	}
	const string getCharBio(const unsigned int &charID)
	{
		Query query = mDB.query("SELECT bio FROM " + string(FH_CHARACTERS) + " WHERE id=" + toString(charID));
		StoreQueryResult res = query.store();
		if (res && !res.empty())
		{
			Row row = res.front();
			if (!row.empty())return (row[0].is_null() ? "" : row[0].c_str());
		}
		return "";
	}
	const int getModPrivilege(const unsigned int &userID, const string &type)
	{
		Query query = mDB.query("SELECT " + type + " FROM fh_modgroup WHERE id IN (SELECT modgroup FROM fh_users WHERE uid=" + toString(userID) + ")");
		StoreQueryResult res = query.store();
		if (res && !res.empty())
		{
			Row row = res.front();
			if (!row.empty())return parseInt(row[0]);
		}
		return 0;
	}
	void updateBan(const unsigned int &uid, const string &ip, const string &mac, const int &value, const string &intervalType)
	{
		const string banDuration = " + INTERVAL " + toString(value) + " " + intervalType;
		//User
		string queryBuffer;
		if (uid)
		{
			queryBuffer = "INSERT INTO fh_users VALUES (";
			queryBuffer += toString(uid) + ",";
			queryBuffer += "NULL,";
			queryBuffer += "(NOW()" + banDuration + ") ";
			queryBuffer += ") ON DUPLICATE KEY UPDATE ";
			queryBuffer += "uid=" + toString(uid) + ",";
			queryBuffer += "ban_end="; queryBuffer += "(NOW()" + banDuration + ") ";

			Query query3 = mDB.query(queryBuffer);
			query3.execute();
		}
		//IP
		if (ip != "")
		{
			queryBuffer = "INSERT INTO fh_BanIP VALUES (";
			queryBuffer += "'" + ip + "',";
			queryBuffer += toString(uid) + ",";
			queryBuffer += "(NOW()" + banDuration + ") ";
			queryBuffer += ") ON DUPLICATE KEY UPDATE ";
			queryBuffer += "ref=" + toString(uid) + ",";
			queryBuffer += "ban_end="; queryBuffer += "(NOW()" + banDuration + ") ";

			Query query = mDB.query(queryBuffer);
			query.execute();
		}
		//MAC
		if (mac != "")
		{
			queryBuffer = "INSERT INTO fh_BanMAC VALUES (";
			queryBuffer += "'" + mac + "',";
			queryBuffer += toString(uid) + ",";
			queryBuffer += "(NOW()" + banDuration + ") ";
			queryBuffer += ") ON DUPLICATE KEY UPDATE ";
			queryBuffer += "id=" + toString(uid) + ",";
			queryBuffer += "ban_end="; queryBuffer += "(NOW()" + banDuration + ") ";

			Query query2 = mDB.query(queryBuffer);
			query2.execute();
		}
	}
	const string getMACByUserID(const unsigned int &id)
	{
		Query query = mDB.query("SELECT MAC, DateTime FROM fh_log WHERE uid=" + toString(id) + " ORDER BY DateTime DESC LIMIT 1");
		StoreQueryResult res = query.store();
		if (res && !res.empty())
		{
			Row row = res.front();
			if (!row.empty())return row[0].c_str();
		}
		return "";
	}
	const string getIPByUserID(const unsigned int &id)
	{
		Query query = mDB.query("SELECT IP, DateTime FROM fh_log WHERE uid=" + toString(id) + " ORDER BY DateTime DESC LIMIT 1");
		StoreQueryResult res = query.store();
		if (res && !res.empty())
		{
			Row row = res.front();
			if (!row.empty())return row[0].c_str();
		}
		return "";
	}
	void unban(const unsigned int &userid)
	{
		Query query = mDB.query("DELETE FROM fh_users WHERE uid=" + toString(userid));
		query.execute();
		Query query2 = mDB.query("DELETE FROM fh_BanIP WHERE ref=" + toString(userid));
		query2.execute();
		Query query3 = mDB.query("DELETE FROM fh_BanMAC WHERE id=" + toString(userid));
		query3.execute();
	}
	const string getGroupNameByCharID(const unsigned int &id, unsigned int *groupID = 0)
	{
		const unsigned int nGroupID = getCharacterGroup(id);
		if (!nGroupID)
		{
			if (groupID)*groupID = 0;
			return "";
		}
		if (groupID)*groupID = nGroupID;

		Query query = mDB.query("SELECT name FROM fh_groups WHERE id=" + toString(nGroupID));
		StoreQueryResult res = query.store();
		if (res && !res.empty())
		{
			Row row = res.front();
			if (!row.empty())
			{
				return row[0].c_str();
			}
		}
		return "";
	}
	const unsigned int getCharacterGroup(const unsigned int &id)
	{
		Query query = mDB.query("SELECT group_id FROM " + string(FH_CHARACTERS) + " WHERE id=" + toString(id));
		StoreQueryResult res = query.store();
		if (res && !res.empty())
		{
			Row row = res.front();
			if (!row.empty())return parseInt(row[0]);
		}
		return 0;
	}
	const string getGroupInfo(const unsigned int &groupID, unsigned int &leaderID)
	{
		Query query = mDB.query("SELECT name,leader_id FROM fh_groups WHERE id=" + toString(groupID) + " AND leader_id>0");
		StoreQueryResult res = query.store();
		if (res && !res.empty())
		{
			Row row = res.front();
			Row::iterator col = row.begin();
			const String name = *col; col++;
			leaderID = parseInt(*col);
			return name.c_str();
		}
		return "";
	}
	const bool isGroupLeader(const unsigned int &charID, const unsigned int &groupID)
	{
		Query query = mDB.query("SELECT leader_id FROM fh_groups WHERE id=" + toString(groupID));
		StoreQueryResult res = query.store();
		if (res && !res.empty())
		{
			Row row = res.front();
			if (!row.empty())return ((unsigned)parseInt(row[0]) == charID);
		}
		return false;
	}
	const unsigned int getGroupByName(const string &groupname)
	{
		Query query = mDB.query("SELECT id FROM fh_groups WHERE name='" + fixApostrophes(groupname) + "' AND leader_id>0");
		StoreQueryResult res = query.store();
		if (res && !res.empty())
		{
			Row row = res.front();
			if (!row.empty())return parseInt(row[0]);
		}
		return 0;
	}
	const unsigned int createGroup(const string &groupname, const unsigned int &charID)
	{
		//Use disbanded groups, if any
		Query query = mDB.query("SELECT id FROM fh_groups WHERE leader_id=0 LIMIT 1");
		StoreQueryResult res = query.store();
		if (res && !res.empty())
		{
			Row row = res.front();
			if (!row.empty())
			{
				const unsigned int groupID = parseInt(row[0]);
				Query query2 = mDB.query("UPDATE fh_groups SET leader_id=" + toString(charID) + ",name='" + fixApostrophes(groupname) + "',bio=DEFAULT,private=DEFAULT WHERE id=" + toString(groupID));
				query2.execute();
				return groupID;
			}
		}

		Query query2 = mDB.query("INSERT INTO fh_groups VALUES(NULL,'" + fixApostrophes(groupname) + "'," + toString(charID) + ",DEFAULT,DEFAULT)");
		SimpleResult res2 = query2.execute();
		if (res)return res2.insert_id();
		return 0;
	}
	const bool setCharacterGroup(const unsigned int &charID, const unsigned int &groupID, const unsigned char &rank = 255)
	{
		Query query = mDB.query("UPDATE " + string(FH_CHARACTERS) + " SET group_id=" + toString(groupID) + ",group_title=DEFAULT,group_rank=" + toString(int(rank)) + " WHERE id=" + toString(charID));
		SimpleResult res = query.execute();

		if (!res)cout << "MAP:" << query.error() << endl;

		return res;
	}
	const bool renameGroup(const unsigned int &groupID, const string &groupname)
	{
		Query query = mDB.query("UPDATE fh_groups SET name='" + fixApostrophes(groupname) + "' WHERE id=" + toString(groupID));
		SimpleResult res = query.execute();

		if (!res)cout << "MAP:" << query.error() << endl;

		return res;
	}
	const string getGroupBio(const unsigned int &groupID)
	{
		Query query = mDB.query("SELECT bio FROM fh_groups WHERE id=" + toString(groupID));
		StoreQueryResult res = query.store();
		if (res && !res.empty())
		{
			Row row = res.front();
			if (!row.empty())return (row[0].is_null() ? "" : row[0].c_str());
		}
		return "";
	}
	const bool updateGroupBio(const unsigned int &groupID, const string &bio)
	{
		Query query = mDB.query("UPDATE fh_groups SET bio='" + fixApostrophes(bio) + "' WHERE id=" + toString(groupID));
		SimpleResult res = query.execute();

		if (!res)cout << "MAP:" << query.error() << endl;

		return res;
	}
	void deleteGroup(const unsigned int &groupID)
	{
		Query query = mDB.query("UPDATE " + string(FH_CHARACTERS) + " SET group_id=0 WHERE group_id=" + toString(groupID));
		query.execute();
		Query query2 = mDB.query("UPDATE fh_groups SET leader_id=0 WHERE id=" + toString(groupID));
		query2.execute();
		//remove group requests/invites
		Query query3 = mDB.query("DELETE FROM fh_group_request WHERE group_id=" + toString(groupID));
		query3.execute();
		Query query4 = mDB.query("DELETE FROM fh_group_invite WHERE group_id=" + toString(groupID));
		query4.execute();
	}
	const string fixApostrophes(string caption)
	{
		size_t found = caption.find_first_of("'");
		while (found != string::npos)
		{
			caption.insert(found, "'");
			found = caption.find_first_of("'", found + 2);
		}

		return caption;
	}
	const string fixWildcards(string caption)
	{
		size_t found = caption.find_first_of("%");
		while (found != string::npos)
		{
			caption.insert(found, "\\");
			found = caption.find_first_of("%", found + 2);
		}

		return caption;
	}
	const vector<GroupMemberInfo> getGroupMemberList(const unsigned int &groupID)
	{
		vector<GroupMemberInfo> memberList;
		Query query = mDB.query("SELECT id,name,group_rank,group_title,is_online FROM " + string(FH_CHARACTERS) + " WHERE user_id>0 AND group_id=" + toString(groupID));
		StoreQueryResult res = query.store();
		if (res)
		{
			for (StoreQueryResult::iterator it = res.begin(); it != res.end(); it++)
			{
				GroupMemberInfo info(0, "");

				Row row = *it;
				Row::iterator col = row.begin();
				info.mCharID = parseInt(*col); col++;
				const String name = *col; col++;
				info.mCharName = name.c_str();
				info.mRank = parseInt(*col); col++;
				const String title = *col; col++;
				info.mTitle = (title.is_null() ? "" : title.c_str());
				info.mIsOnline = (bool)parseInt(*col);

				if (info.mCharID)memberList.push_back(info);
			}
		}
		return memberList;
	}
	const vector<pair<unsigned int, string> > getGroupList(string filter)
	{
		fixWildcards(filter);
		fixApostrophes(filter);

		vector<pair<unsigned int, string> > groupList;
		Query query = mDB.query("SELECT id,name FROM fh_groups WHERE leader_id>0 AND name LIKE '%" + filter + "%'");
		StoreQueryResult res = query.store();
		if (res)
		{
			for (StoreQueryResult::iterator it = res.begin(); it != res.end(); it++)
			{
				Row row = *it;
				Row::iterator col = row.begin();
				const unsigned int groupID = parseInt(*col); col++;
				const String groupname = *col;
				string groupname2 = string(groupname.data(), groupname.length());

				groupList.push_back(pair<unsigned int, string>(groupID, groupname2));
			}
		}
		return groupList;
	}
	const unsigned int getGroupLeader(const unsigned int &groupID)
	{
		Query query = mDB.query("SELECT leader_id FROM fh_groups WHERE id=" + toString(groupID));
		StoreQueryResult res = query.store();
		if (res && !res.empty())
		{
			Row row = res.front();
			if (!row.empty())return parseInt(row[0]);
		}
		return 0;
	}
	const string getGroupName(const unsigned int &groupID)
	{
		Query query = mDB.query("SELECT name FROM fh_groups WHERE id=" + toString(groupID));
		StoreQueryResult res = query.store();
		if (res && !res.empty())
		{
			Row row = res.front();
			if (!row.empty())return row[0].c_str();
		}
		return "";
	}
	const bool addGroupRequest(const unsigned int &charID, const unsigned int &groupID)
	{
		Query query = mDB.query("INSERT INTO fh_group_request (`char_id`, `group_id`) VALUES(" + toString(charID) + "," + toString(groupID) + ")");
		SimpleResult res = query.execute();
		if (!res)return false;
		return true;
	}
	const bool deleteGroupRequest(const unsigned int &charID, const unsigned int &groupID)
	{
		Query query = mDB.query("DELETE FROM fh_group_request WHERE char_id=" + toString(charID) + " AND group_id=" + toString(groupID));
		SimpleResult res = query.execute();
		if (!res)return false;
		return true;
	}
	const bool deleteAllGroupRequest(const unsigned int &id, const bool &isGroupID)
	{
		string queryBuffer = "DELETE FROM fh_group_request WHERE ";
		queryBuffer += isGroupID ? "group_id=" : "char_id=";
		queryBuffer += toString(id);
		Query query = mDB.query(queryBuffer);
		SimpleResult res = query.execute();
		if (!res)return false;
		return true;
	}
	const bool addGroupInvite(const unsigned int &groupID, const unsigned int &charID)
	{
		Query query = mDB.query("INSERT INTO fh_group_invite (`group_id`, `char_id`) VALUES(" + toString(groupID) + "," + toString(charID) + ")");
		SimpleResult res = query.execute();
		if (!res)return false;
		return true;
	}
	const bool deleteGroupInvite(const unsigned int &groupID, const unsigned int &charID)
	{
		Query query = mDB.query("DELETE FROM fh_group_invite WHERE group_id=" + toString(groupID) + " AND char_id=" + toString(charID));
		SimpleResult res = query.execute();
		if (!res)return false;
		return true;
	}
	const bool deleteAllGroupInvite(const unsigned int &id, const bool &isCharID)
	{
		string queryBuffer = "DELETE FROM fh_group_invite WHERE ";
		queryBuffer += isCharID ? "char_id=" : "group_id=";
		queryBuffer += toString(id);
		Query query = mDB.query(queryBuffer);
		SimpleResult res = query.execute();
		if (!res)return false;
		return true;
	}
	const vector<unsigned int> getGroupRequestList(const unsigned int &groupID)
	{
		vector<unsigned int> requestList;
		Query query = mDB.query("SELECT char_id FROM fh_group_request WHERE group_id=" + toString(groupID));
		StoreQueryResult res = query.store();
		if (res)
		{
			for (StoreQueryResult::iterator it = res.begin(); it != res.end(); it++)
			{
				Row row = *it;
				if (!row.empty())requestList.push_back(parseInt(row[0]));
			}
		}
		return requestList;
	}
	const vector<unsigned int> getGroupInviteList(const unsigned int &charID)
	{
		vector<unsigned int> inviteList;
		Query query = mDB.query("SELECT group_id FROM fh_group_invite WHERE char_id=" + toString(charID));
		StoreQueryResult res = query.store();
		if (res)
		{
			for (StoreQueryResult::iterator it = res.begin(); it != res.end(); it++)
			{
				Row row = *it;
				if (!row.empty())inviteList.push_back(parseInt(row[0]));
			}
		}
		return inviteList;
	}
	const bool addReport(const unsigned int &reporterID, const unsigned int &targetID, const string &message)
	{
		Query query = mDB.query("INSERT INTO fh_reports VALUES(NULL," + toString(reporterID) + "," + toString(targetID) + ",'" + fixApostrophes(message) + "',NOW(),DEFAULT)");
		SimpleResult res = query.execute();
		if (!res)
		{
			cout << "MAP:" << query.error() << endl;
			return false;
		}
		return true;
	}
	const string getCharacterHome(const unsigned int &charID, int &x, int &y, int &z)
	{
		Query query = mDB.query("SELECT home_map,home_x,home_y,home_z FROM " + string(FH_CHARACTERS) + " WHERE id=" + toString(charID));
		StoreQueryResult res = query.store();
		if (res && !res.empty())
		{
			Row row = res.front();
			Row::iterator col = row.begin();
			const String home = *col; col++;
			x = parseInt(*col); col++;
			y = parseInt(*col); col++;
			z = parseInt(*col);
			return home.c_str();
		}
		return "";
	}
	void setCharacterHome(const unsigned int &charID, const bool &isReset, const string &map, const int &x, const int &y, const int &z)
	{
		string queryBuffer = "UPDATE " + string(FH_CHARACTERS) + " SET ";
		if (isReset)
		{
			queryBuffer += "home_map="; queryBuffer += START_MAPNAME;
			queryBuffer += ",home_x=" + toString(START_POSX);
			queryBuffer += ",home_y=" + toString(START_POSY);
			queryBuffer += ",home_z=" + toString(START_POSZ);
		}
		else
		{
			queryBuffer += "home_map='" + map;
			queryBuffer += "',home_x=" + toString(x);
			queryBuffer += ",home_y=" + toString(y);
			queryBuffer += ",home_z=" + toString(z);
		}
		queryBuffer += " WHERE id=" + toString(charID);
		Query query = mDB.query(queryBuffer);
		query.execute();
	}
	const vector<PartyMemberInfo> getPartyMemberList(const unsigned int &partyID)
	{
		vector<PartyMemberInfo> memberList;
		Query query = mDB.query("SELECT id,name FROM " + string(FH_CHARACTERS) + " WHERE is_online=1 AND party_id=" + toString(partyID));
		StoreQueryResult res = query.store();
		if (res)
		{
			for (StoreQueryResult::iterator it = res.begin(); it != res.end(); it++)
			{
				PartyMemberInfo info(0, "");

				Row row = *it;
				Row::iterator col = row.begin();
				info.mCharID = parseInt(*col); col++;
				const String name = *col;
				info.mCharName = name.c_str();

				if (info.mCharID)memberList.push_back(info);
			}
		}
		return memberList;
	}
	const unsigned int getCharacterParty(const unsigned int &id)
	{
		Query query = mDB.query("SELECT party_id FROM " + string(FH_CHARACTERS) + " WHERE is_online=1 AND id=" + toString(id));
		StoreQueryResult res = query.store();
		if (res && !res.empty())
		{
			Row row = res.front();
			if (!row.empty())return parseInt(row[0]);
		}
		return 0;
	}
	const bool setCharacterParty(const unsigned int &charID, const unsigned int &partyID)
	{
		Query query = mDB.query("UPDATE " + string(FH_CHARACTERS) + " SET party_id=" + toString(partyID) + " WHERE id=" + toString(charID));
		SimpleResult res = query.execute();

		if (!res)cout << "MAP:" << query.error() << endl;

		return res;
	}
	const bool setCharacterOffline(const unsigned int &charID)
	{
		Query query = mDB.query("UPDATE " + string(FH_CHARACTERS) + " SET is_online = 0, party_id = 0 WHERE id = " + toString(charID));
		SimpleResult res = query.execute();

		if (!res)cout << "MAP:" << query.error() << endl;

		return res;
	}
};

#endif


// ##################################################################################################
// ##-------------------------------------------FILE END-------------------------------------------##
// ##################################################################################################
// ##-------------------------------------------MODIFIED-------------------------------------------##
// ## DATE: 9/25/2026                                                                              ##
// ## TIME: 6:13 PM (UTC-5:00)                                                                     ##
// ## BY: radioactivedark                                                                          ##
// ##################################################################################################