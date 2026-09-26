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
#include <string>

using namespace std;
using namespace mysqlpp;


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
	enum BanType
	{
		BAN_NONE,
		BAN_USER,
		BAN_IP,
		BAN_MAC
	};
	const bool init(const bool &disconnect = false)
	{
		if (disconnect)mDB.disconnect();

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

		if (mDB.connect(database, ip, user, pass))
		{
			cout << "DB connected" << endl;
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
	const bool pingDB()
	{
		return mDB.ping();
	}
	void updateLoginAttempt(const unsigned int &uid, const bool &sf, const string &ip, const string &mac)
	{
		string queryBuffer = "INSERT INTO fh_log VALUES (NULL,";
		queryBuffer += "NOW(),";
		queryBuffer += toString(uid) + ",";
		queryBuffer += sf ? "1," : "0,";
		queryBuffer += "'" + ip + "',";
		queryBuffer += "'" + mac + "');";

		Query query = mDB.query(queryBuffer);
		SimpleResult res = query.execute();

		if (!res)cout << query.error() << endl;
	}
	void updateBan(const unsigned int &uid, const string &ip, const string &mac)
	{
		//User
		string queryBuffer = "INSERT INTO fh_users VALUES (";
		queryBuffer += toString(uid) + ",";
		queryBuffer += "DEFAULT,";
		queryBuffer += "ADDTIME(NOW(),"; queryBuffer += BAN_DURATION; queryBuffer += "), ";
		queryBuffer += "DEFAULT) ON DUPLICATE KEY UPDATE ";
		queryBuffer += "uid=" + toString(uid) + ",";
		queryBuffer += "ban_end="; queryBuffer += "ADDTIME(NOW(),"; queryBuffer += BAN_DURATION; queryBuffer += ");";

		Query query3 = mDB.query(queryBuffer);
		SimpleResult res3 = query3.execute();

		if (!res3)cout << query3.error() << endl;

		//IP
		queryBuffer = "INSERT INTO fh_BanIP VALUES (";
		queryBuffer += "'" + ip + "',";
		queryBuffer += toString(uid) + ",";
		queryBuffer += "ADDTIME(NOW(),"; queryBuffer += BAN_DURATION; queryBuffer += ") ";
		queryBuffer += ") ON DUPLICATE KEY UPDATE ";
		queryBuffer += "ref=" + toString(uid) + ",";
		queryBuffer += "ban_end="; queryBuffer += "ADDTIME(NOW(),"; queryBuffer += BAN_DURATION; queryBuffer += ");";

		Query query = mDB.query(queryBuffer);
		SimpleResult res = query.execute();

		if (!res)cout << query.error() << endl;

		//MAC
		queryBuffer = "INSERT INTO fh_BanMAC VALUES (";
		queryBuffer += "'" + mac + "',";
		queryBuffer += toString(uid) + ",";
		queryBuffer += "ADDTIME(NOW(),"; queryBuffer += BAN_DURATION; queryBuffer += ") ";
		queryBuffer += ") ON DUPLICATE KEY UPDATE ";
		queryBuffer += "id=" + toString(uid) + ",";
		queryBuffer += "ban_end="; queryBuffer += "ADDTIME(NOW(),"; queryBuffer += BAN_DURATION; queryBuffer += ");";

		Query query2 = mDB.query(queryBuffer);
		SimpleResult res2 = query2.execute();

		if (!res2)cout << query2.error() << endl;
	}
	const unsigned char isBanned(const unsigned int &uid, const string &ip, const string &mac, double &banSecs)
	{
		Query tquery = mDB.query("SELECT NOW()");
		StoreQueryResult tres = tquery.store();
		if (!tres || tres.empty() || tres.front().size() <= 0)return BAN_NONE;
		const time_t now = time_t(DateTime(tres.front()[0].c_str()));
		//UserID
		Query query = mDB.query("SELECT ban_end FROM fh_users WHERE uid=" + toString(uid));
		StoreQueryResult res = query.store();
		if (res && !res.empty())
		{
			Row row = res.front();
			if (row.size()>0)
			{
				banSecs = difftime(DateTime(row[0].c_str()), now);
				if (banSecs>0)return BAN_USER;
			}
		}
		//IP
		Query query2 = mDB.query("SELECT ban_end FROM fh_BanIP WHERE IP='" + ip + "'");
		StoreQueryResult res2 = query2.store();
		if (res2 && !res2.empty())
		{
			Row row = res2.front();
			if (row.size()>0)
			{
				banSecs = difftime(DateTime(row[0].c_str()), now);
				if (banSecs>0)return BAN_IP;
			}
		}
		//MacAdd
		Query query3 = mDB.query("SELECT ban_end FROM fh_BanMAC WHERE MAC='" + mac + "'");
		StoreQueryResult res3 = query3.store();
		if (res3 && !res3.empty())
		{
			Row row = res3.front();
			if (row.size()>0)
			{
				banSecs = difftime(DateTime(row[0].c_str()), now);
				if (banSecs>0)return BAN_MAC;
			}
		}
		return BAN_NONE;
	}
	void spreadBan(const unsigned int &uid, const string &ip, const string &mac)
	{
		Query query = mDB.query("SELECT ban_end FROM fh_users WHERE uid=" + toString(uid));
		StoreQueryResult res = query.store();
		if (!res || res.empty() || res.front().size() <= 0)return;
		const string banTime = res.front()[0].c_str();

		//IP
		string queryBuffer = "INSERT INTO fh_BanIP VALUES (";
		queryBuffer += "'" + ip + "',";
		queryBuffer += toString(uid) + ",";
		queryBuffer += "'" + banTime + "'";
		queryBuffer += ") ON DUPLICATE KEY UPDATE ";
		queryBuffer += "ref=" + toString(uid) + ",";
		queryBuffer += "ban_end='" + banTime + "';";

		Query query2 = mDB.query(queryBuffer);
		SimpleResult res2 = query2.execute();

		if (!res2)cout << query.error() << endl;

		//MAC
		queryBuffer = "INSERT INTO fh_BanMAC VALUES (";
		queryBuffer += "'" + mac + "',";
		queryBuffer += toString(uid) + ",";
		queryBuffer += "'" + banTime + "'";
		queryBuffer += ") ON DUPLICATE KEY UPDATE ";
		queryBuffer += "id=" + toString(uid) + ",";
		queryBuffer += "ban_end='" + banTime + "';";

		Query query3 = mDB.query(queryBuffer);
		SimpleResult res3 = query3.execute();

		if (!res3)cout << "FRONT:" << query3.error() << endl;
	}
	const bool getSalt(const string &user, string &salt, unsigned int &id)
	{
		Query query = mDB.query("SELECT password, id, block FROM jos_users WHERE username='" + user + "' limit 1");
		StoreQueryResult res = query.store();
		if (res && !res.empty())
		{
			Row row = res.front();
			if (row.size()<3)return false;
			const string buffer = row[0].c_str();
			id = parseInt(row[1]);
			if (parseInt(row[2]))return false;
			const size_t colon = buffer.find_first_of(':');
			if (colon >= buffer.length() || colon == string::npos)return false;
			salt = buffer.substr(colon + 1);
			return true;
		}
		return false;
	}
	const bool matchPassword(const string &user, const string &hash)
	{
		//cout << "User: " << user << endl;
		//cout << "Hash: " << hash << endl;
		Query query = mDB.query("SELECT password FROM jos_users WHERE username='" + user + "' limit 1");
		StoreQueryResult res = query.store();
		if (res && !res.empty())
		{
			Row row = res.front();
			if (row.size()<1)return false;
			const string buffer = row[0].c_str();
			const size_t colon = buffer.find_first_of(':');
			if (colon >= buffer.length() || colon == string::npos)return false;
			const string truePass = buffer.substr(0, colon);
			//cout << "Truehash: " << truePass << endl;
			return (truePass.compare(hash) == 0);
		}
		return false;
	}
	const bool getCharacter(const unsigned int &userID, const char &selection, CharacterData *data, unsigned char *numChars)
	{
		const string queryStr = "SELECT id, name FROM " + string(FH_CHARACTERS) + " WHERE user_id=" + toString(userID);

		Query query = mDB.query(queryStr);
		StoreQueryResult res = query.store();
		if (res && !res.empty())
		{
			char count = 0;
			*numChars = (unsigned char)res.size();
			for (StoreQueryResult::iterator it = res.begin(); it != res.end(); it++, count++)
			{
				if (count == selection)
				{
					//Set charID and name from first result
					Row row = *it;
					Row::iterator col = row.begin();
					data->mID = (unsigned int)parseLong(*col); col++;
					const String name = *col;
					data->mName = name.c_str();

					//Query appearance data
					string queryBuffer = "SELECT species, ";
					for (int i = 0; i<MAX_MESHES; i++)queryBuffer += "mesh_" + toString(i) + ", ";
					for (int i = 0; i<MAX_MATS; i++)queryBuffer += "mat_" + toString(i) + ", ";
					for (int i = 0; i<NUM_SCALABLES; i++)queryBuffer += "scale_" + toString(i) + ", ";
					for (int i = 0; i<NUM_COLOURABLES; i++)
						for (int j = 0; j<3; j++)queryBuffer += "colour_" + toString(i) + "_" + toString(j) + ", ";
					queryBuffer += "emote, preset, equip, items ";
					queryBuffer += "FROM " + string(FH_APPEARANCE) + " WHERE id=" + toString(data->mID);

					Query query2 = mDB.query(queryBuffer);
					StoreQueryResult res2 = query2.store();
					if (res2 && !res2.empty())
					{
						Row row2 = res2.front();
						if (row2.empty())return false;
						col = row2.begin();

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

						return true;
					}
					return false;
				}
			}
		}
		return false;
	}
	const bool getAvailableCharacterID(unsigned int &id)
	{
		Query query = mDB.query("SELECT id FROM " + string(FH_CHARACTERS) + " WHERE user_id=0");
		StoreQueryResult res = query.store();
		if (res && !res.empty())
		{
			Row row = res.front();
			if (row.empty())return false;
			id = parseLong(row[0]);
			return true;
		}
		return false;
	}
	const bool saveCharacter(const unsigned int &userID, CharacterData *data)
	{
		unsigned char numColumns = 0;
		Query countQuery = mDB.query("SHOW COLUMNS FROM " + string(FH_CHARACTERS));
		StoreQueryResult countRes = countQuery.store();
		if (countRes)numColumns = (int)countRes.size() - 7;

		//Insert character entry
		string queryBuffer = "INSERT INTO " + string(FH_CHARACTERS) + " VALUES (NULL,";
		queryBuffer += toString(userID) + ",";
		queryBuffer += "'" + data->mName + "',";
		queryBuffer += string(START_MAPNAME) + ",";
		queryBuffer += toString(START_POSX) + ",";
		queryBuffer += toString(START_POSY) + ",";
		queryBuffer += toString(START_POSZ);

		for (int i = 0; i<numColumns; i++)queryBuffer += ",DEFAULT";
		queryBuffer += ");";

		Query query = mDB.query(queryBuffer);
		SimpleResult res = query.execute();

		if (res)data->mID = res.insert_id();
		else cout << "FRONT:" << query.error() << endl;

		//Insert appearance entry
		if (data->mID)
		{
			queryBuffer = "INSERT INTO " + string(FH_APPEARANCE) + " VALUES (";
			queryBuffer += toString((int)data->mID) + ",";
			queryBuffer += "'"; queryBuffer.push_back(data->mSpecies); queryBuffer += "',";
			for (int i = 0; i<MAX_MESHES; i++)queryBuffer += toString((int)data->mMesh[i]) + ",";
			for (int i = 0; i<MAX_MATS; i++)queryBuffer += toString((int)data->mMat[i]) + ",";
			for (int i = 0; i<NUM_SCALABLES; i++)queryBuffer += toString((int)data->mScale[i]) + ",";
			for (int i = 0; i<NUM_COLOURABLES; i++)
				for (int j = 0; j<3; j++)queryBuffer += toString((int)data->mColour[i][j]) + ",";
			queryBuffer += toString((int)data->mEmote) + ",";
			queryBuffer += toString((int)data->mPreset) + ",";
			queryBuffer += toString((int)data->m_nEquip) + ",";
			queryBuffer += "'" + data->m_szItems + "');";

			Query query2 = mDB.query(queryBuffer);
			SimpleResult res2 = query2.execute();

			if (!res2)cout << "FRONT:" << query2.error() << endl;
			return res2;
		}
		return false;
	}
	const bool updateCharacter(const unsigned int &userID, CharacterData *data, const bool &resetData = false)
	{
		//Update characters entry
		string queryBuffer = "UPDATE " + string(FH_CHARACTERS) + " SET ";
		queryBuffer += "user_id=" + toString(userID) + ",";
		queryBuffer += "name='" + data->mName + "'";
		if (resetData)
		{
			queryBuffer += ",map=" + string(START_MAPNAME) + ",";
			queryBuffer += "pos_x=" + toString(START_POSX) + ",";
			queryBuffer += "pos_y=" + toString(START_POSY) + ",";
			queryBuffer += "pos_z=" + toString(START_POSZ) + ",";

			queryBuffer += "home_map=" + string(START_MAPNAME) + ",";
			queryBuffer += "home_x=" + toString(START_POSX) + ",";
			queryBuffer += "home_y=" + toString(START_POSY) + ",";
			queryBuffer += "home_z=" + toString(START_POSZ) + ",";

			queryBuffer += "group_id=0,";
			queryBuffer += "group_title=DEFAULT,";
			queryBuffer += "group_rank=DEFAULT,";
			queryBuffer += "party_id=0,";
			queryBuffer += "bio=DEFAULT";
		}

		queryBuffer += " WHERE id=" + toString(data->mID);

		Query query = mDB.query(queryBuffer);
		SimpleResult res = query.execute();

		if (!res)cout << "FRONT:" << query.error() << endl;
		else
		{
			//Update appearance entry
			queryBuffer = "UPDATE " + string(FH_APPEARANCE) + " SET ";
			queryBuffer += "species='"; queryBuffer.push_back(data->mSpecies); queryBuffer += "',";
			for (int i = 0; i<MAX_MESHES; i++)queryBuffer += "mesh_" + toString(i) + "=" + toString((int)data->mMesh[i]) + ",";
			for (int i = 0; i<MAX_MATS; i++)queryBuffer += "mat_" + toString(i) + "=" + toString((int)data->mMat[i]) + ",";
			for (int i = 0; i<NUM_SCALABLES; i++)queryBuffer += "scale_" + toString(i) + "=" + toString((int)data->mScale[i]) + ",";
			for (int i = 0; i<NUM_COLOURABLES; i++)
				for (int j = 0; j<3; j++)queryBuffer += "colour_" + toString(i) + "_" + toString(j) + "=" + toString((int)data->mColour[i][j]) + ",";
			queryBuffer += "emote=" + toString((int)data->mEmote) + ",";
			queryBuffer += "preset=" + toString((int)data->mPreset) + ",";
			queryBuffer += "equip=" + toString((int)data->m_nEquip) + ",";
			queryBuffer += "items='" + data->m_szItems + "'";

			queryBuffer += " WHERE id=" + toString(data->mID);

			Query query2 = mDB.query(queryBuffer);
			SimpleResult res2 = query2.execute();

			if (!res2)cout << "FRONT:" << query2.error() << endl;
			return res2;
		}
		return false;
	}
	const bool getCharacterOrder(const unsigned int &userID, const unsigned int &charID, char &selection)
	{
		Query query = mDB.query("SELECT id FROM " + string(FH_CHARACTERS) + " WHERE user_id=" + toString(userID));
		StoreQueryResult res = query.store();
		if (res && !res.empty())
		{
			unsigned char count = 0;
			for (StoreQueryResult::iterator it = res.begin(); it != res.end(); it++, count++)
			{
				Row row = *it;
				if (row.empty())return false;
				if (charID == (unsigned int)parseLong(row[0]))
				{
					selection = count;
					return true;
				}
			}
		}
		return false;
	}
	const bool deleteCharacter(const unsigned int &charID)
	{
		Query query = mDB.query("UPDATE " + string(FH_CHARACTERS) + " SET user_id=0 WHERE id=" + toString(charID));
		SimpleResult res = query.execute();

		if (!res)cout << "FRONT:" << query.error() << endl;
		//Delete all dependencies on this char
		else
		{
			//Group dependencies
			Query query2 = mDB.query("SELECT group_id FROM " + string(FH_CHARACTERS) + " WHERE id=" + toString(charID));
			StoreQueryResult res2 = query2.store();
			if (res2 && !res2.empty())
			{
				Row row = res2.front();
				if (!row.empty())
				{
					const unsigned int groupID = parseInt(row[0]);
					if (groupID)
					{
						//remove group if is leader
						Query query3 = mDB.query("SELECT id FROM fh_groups WHERE id=" + toString(groupID) + " AND leader_id=" + toString(charID));
						StoreQueryResult res3 = query3.store();
						if (res3 && !res3.empty())
						{
							Query query4 = mDB.query("UPDATE fh_groups SET leader_id=0 WHERE id=" + toString(groupID));
							query4.execute();
							Query query5 = mDB.query("DELETE FROM fh_group_request WHERE group_id=" + toString(groupID));
							query5.execute();
							Query query6 = mDB.query("DELETE FROM fh_group_invite WHERE group_id=" + toString(groupID));
							query6.execute();
							Query query7 = mDB.query("UPDATE " + string(FH_CHARACTERS) + " SET group_id=0 WHERE group_id=" + toString(groupID));
							query7.execute();
						}
					}
				}
			}
			//remove group requests/invites
			Query query3 = mDB.query("DELETE FROM fh_group_request WHERE char_id=" + toString(charID));
			query3.execute();
			Query query4 = mDB.query("DELETE FROM fh_group_invite WHERE char_id=" + toString(charID));
			query4.execute();
		}

		return res;
	}
	const int parseInt(const String &value)
	{
		return atoi(value.c_str());
	}
	const long parseLong(const String &value)
	{
		return atol(value.c_str());
	}
	/*void checkTables()
	{
	Query query = mDB.query("CHECK TABLE fh_BanIP,fh_BanMAC," + string(FH_CHARACTERS) + ",fh_log,fh_users");
	StoreQueryResult res = query.store();
	if(res && !res.empty())
	{
	for(StoreQueryResult::iterator it=res.begin(); it!=res.end(); it++)
	{
	Row row = *it;
	for(Row::iterator col=row.begin(); col!=row.end(); col++)
	{
	cout << *col << "\t";
	}
	cout << endl;
	}
	}
	}*/
	const bool storeCharacterToken(const unsigned int &charID, const string &token)
	{
		string queryBuffer = "UPDATE " + string(FH_CHARACTERS) + " SET ";
		queryBuffer += "token='" + token + "',";
		queryBuffer += "token_time=NOW()";
		queryBuffer += " WHERE id=" + toString(charID);

		Query query = mDB.query(queryBuffer);
		SimpleResult res = query.execute();

		if (!res)cout << "FRONT:" << query.error() << endl;

		return res;
	}
};

#endif


// ##################################################################################################
// ##-------------------------------------------FILE END-------------------------------------------##
// ##################################################################################################
// ##-------------------------------------------MODIFIED-------------------------------------------##
// ## DATE: 9/25/2026                                                                              ##
// ## TIME: 6:11 PM (UTC-5:00)                                                                     ##
// ## BY: radioactivedark                                                                          ##
// ##################################################################################################