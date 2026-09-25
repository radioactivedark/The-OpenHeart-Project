#ifndef _DATABASEMANAGER_H_
#define _DATABASEMANAGER_H_

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
    const bool init(const bool &disconnect=false)
    {
        if(disconnect)mDB.disconnect();

        char database[256] = "";
        char ip[256] = "";
        char user[256] = "";
        char pass[256] = "";
        ifstream inFile("db",ios::in);
        if(inFile.good())
        {
            inFile.getline(database,256);
            inFile.getline(ip,256);
            inFile.getline(user,256);
            inFile.getline(pass,256);
        }
        inFile.close();

        if(mDB.connect(database, ip, user, pass))
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
        if(mConnected)mDB.disconnect();
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
    const int parseInt(const String &value)
    {
        return atoi(value.c_str());
    }
    const long parseLong(const String &value)
    {
        return atol(value.c_str());
    }
    const unsigned int getUserID(const unsigned int &id)
    {
        Query query = mDB.query("SELECT user_id FROM fh_characters WHERE id="+toString(id));
        StoreQueryResult res = query.store();
        if(res && !res.empty())
        {
            Row row = res.front();
            if(!row.empty())return parseInt(row[0]);
        }
        return 0;
    }
    const string getUserName(const unsigned int &id)
    {
        Query query = mDB.query("SELECT username FROM jos_users WHERE id="+toString(id));
        StoreQueryResult res = query.store();
        if(res && !res.empty())
        {
            Row row = res.front();
            if(!row.empty())return row[0].c_str();
        }
        return "";
    }
    const bool getCharacterInfo(const unsigned int &id, string &map)
    {
        Query query = mDB.query("SELECT map FROM fh_characters WHERE id="+toString(id));
        StoreQueryResult res = query.store();
        if(res && !res.empty())
        {
            Row row = res.front();
            Row::iterator col = row.begin();
            const String mapName = *col;
            map = mapName.c_str();
            if(map=="NULL")map = "";
        }
        return res;
    }
    /*void checkUsersTable()
    {
        time_t start,end;
        time(&start);
        Query query = mDB.query("CHECK TABLE fh_BanIP,fh_BanMAC,fh_characters,fh_log,fh_users");
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
        time(&end);
        cout << "Check took " << toString(difftime(end,start)) << " seconds." << endl;
    }*/
    const bool matchToken(const unsigned int &charID, const string &token)
    {
        Query query = mDB.query("SELECT token, token_time FROM fh_characters WHERE id="+toString(charID));
        StoreQueryResult res = query.store();
        Query query2 = mDB.query("SELECT NOW()");
        StoreQueryResult res2 = query2.store();
        if(res && res2 && !res.empty() && !res2.empty())
        {
            Row row = res.front();
            if(row.size()!=2)return false;
            Row::iterator col = row.begin();
            const String tokenCmp = *col; col++;
            if(token!=tokenCmp.c_str())return false;
            const String timeCmp = *col;
            Row row2 = res2.front();
            if(row2.size()<1)return false;
            const String timeNow = row2[0];
            if(difftime(time_t(DateTime(timeNow.c_str())),time_t(DateTime(timeCmp.c_str())))>10.0f)return false;
            return true;
        }
        return false;
    }
    void setCharacterOnlineStatus(const unsigned int &id, const bool &isOnline, const bool &clearParty=false)
    {
        string queryBuffer = "UPDATE fh_characters SET is_online=";
        queryBuffer += (isOnline?"1":"0");
        if(clearParty)queryBuffer += ",party_id=0";
        queryBuffer += " WHERE id="+toString(id);
        Query query = mDB.query(queryBuffer);
        query.execute();
    }
    const bool clearUserOnlineStatus(const unsigned int &userID, const bool &clearParty=true)
    {
        Query query = mDB.query("SELECT id FROM fh_characters WHERE user_id="+toString(userID)+" AND is_online=1");
        StoreQueryResult res = query.store();
        if(res && !res.empty())
        {
            string queryStr = "UPDATE fh_characters SET is_online=0";
            if(clearParty)queryStr += ",party_id=0";
            queryStr += " WHERE user_id="+toString(userID);
            Query query2 = mDB.query(queryStr);
            query2.execute();
            return true;
        }
        return false;
    }
    void clearAllOnlineStatus(const bool &clearParty=true)
    {
        string queryStr = "UPDATE fh_characters SET is_online=0";
        if(clearParty)queryStr += ",party_id=0";
        Query query2 = mDB.query(queryStr);
        query2.execute();
    }
    const int getNumPartyMembers(const unsigned int &partyID)
    {
        int memberCount = 0;
        Query query = mDB.query("SELECT id FROM fh_characters WHERE is_online=1 AND party_id="+toString(partyID));
        StoreQueryResult res = query.store();
        if(res)
        {
            for(StoreQueryResult::iterator it=res.begin(); it!=res.end(); it++)
            {
                Row row = *it;
                Row::iterator col = row.begin();
                const int charID = parseInt(*col);

                if(charID)memberCount++;
            }
        }
        return memberCount;
    }
    const unsigned int getCharacterParty(const unsigned int &id)
    {
        Query query = mDB.query("SELECT party_id FROM fh_characters WHERE is_online=1 AND id="+toString(id));
        StoreQueryResult res = query.store();
        if(res && !res.empty())
        {
            Row row = res.front();
            if(!row.empty())return parseInt(row[0]);
        }
        return 0;
    }
    const bool setCharacterParty(const unsigned int &charID, const unsigned int &partyID)
    {
        Query query = mDB.query("UPDATE fh_characters SET party_id="+toString(partyID)+" WHERE id="+toString(charID));
        SimpleResult res = query.execute();

        return res;
    }
};

#endif
