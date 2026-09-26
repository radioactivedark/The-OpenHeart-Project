// ##################################################################################################
// ##------------------------------------------FILE START------------------------------------------##
// ##################################################################################################


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
	unsigned char mMesh[MAX_MESHES];
	unsigned char mMat[MAX_MATS];
	char mScale[NUM_SCALABLES];
	unsigned char mColour[NUM_COLOURABLES][3];
	unsigned char mEmote;
	unsigned char mPreset;
	unsigned char m_nEquip;
	std::string m_szItems;
	CharacterData()
	{
		mID = 0;
		mName = "";
		mSpecies = 0;
		for (int i = 0; i<MAX_MESHES; i++)mMesh[i] = 0;
		for (int i = 0; i<MAX_MATS; i++)mMat[i] = 0;
		for (int i = 0; i<NUM_SCALABLES; i++)mScale[i] = 0;
		for (int i = 0; i<NUM_COLOURABLES; i++)
			for (int j = 0; j<3; j++)mColour[i][j] = 0;
		mEmote = 0;
		mPreset = 0;
		m_nEquip = 0;
		m_szItems = "";
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


// ##################################################################################################
// ##-------------------------------------------FILE END-------------------------------------------##
// ##################################################################################################
// ##-------------------------------------------MODIFIED-------------------------------------------##
// ## DATE: 9/25/2026                                                                              ##
// ## TIME: 6:07 PM (UTC-5:00)                                                                     ##
// ## BY: radioactivedark                                                                          ##
// ##################################################################################################