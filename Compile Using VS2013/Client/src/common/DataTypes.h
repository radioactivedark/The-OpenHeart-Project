#ifndef _DATATYPES_H_
#define _DATATYPES_H_

#define MAX_MESHES 5
#define MAX_MATS 4
#define NUM_SCALABLES 10
#define NUM_COLOURABLES 10

#include <string>

struct CharacterData
{
    unsigned int mID;
    std::string mName;
    char mSpecies;
    unsigned char mMesh[5];
    unsigned char mMat[4];
    char mScale[10];
    unsigned char mColour[9][3];
    unsigned char mEmote;
    unsigned char mPreset;
    CharacterData()
    {
        mID = 0;
        mName = "";
        mSpecies = 0;
        for(int i=0;i<5;i++)mMesh[i] = 0;
        for(int i=0;i<4;i++)mMat[i] = 0;
        for(int i=0;i<10;i++)mScale[i] = 0;
        for(int i=0;i<9;i++)
            for(int j=0;j<3;j++)mColour[i][j] = 0;
        mEmote = 0;
        mPreset = 0;
    }
};

struct GroupMemberInfo
{
    unsigned int mCharID;
    std::string mCharName;
    unsigned char mRank;
    std::string mTitle;
    bool mIsOnline;
    GroupMemberInfo(const unsigned int &charID, const std::string &charname)
    {
        mCharID = charID;
        mCharName = charname;
        mRank = 255;
        mTitle = "";
        mIsOnline = false;
    }
};

struct PartyMemberInfo
{
    unsigned int mCharID;
    std::string mCharName;
    PartyMemberInfo(const unsigned int &charID, const std::string &charname)
    {
        mCharID = charID;
        mCharName = charname;
    }
};

#endif
