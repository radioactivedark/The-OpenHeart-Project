#ifndef _PACKETREADER_H_
#define _PACKETREADER_H_

#include "MessageIdentifiers.h"
#include "RakPeerInterface.h"
#include "RakNetTypes.h"
#include "BitStream.h"
#include "StringCompressor.h"
#include <string>

#define stringCompressor StringCompressor::Instance()

using namespace std;
using namespace RakNet;

class PacketReader
{
private:
    BitStream mBitStream;
    MessageID mID;
public:
    PacketReader()
    {
        mID = 0;
    }
    ~PacketReader()
    {
    }
    void load(Packet *p)
    {
        BitStream copyBit(p->data, p->length, false);
        mBitStream.Reset();
        mBitStream.Write(&copyBit);
        mBitStream.Read(mID);
    }
    const string readString(const unsigned short &length=256)
    {
        char *buffer = new char[length+1];
        stringCompressor->DecodeString(buffer, length+1, &mBitStream);
        const string value = string(buffer);
        delete[] buffer;
        return value;
    }
    const int readInt()
    {
        int buffer = 0;
        mBitStream.Read(buffer);
        return buffer;
    }
    const int readUInt()
    {
        unsigned int buffer = 0;
        mBitStream.Read(buffer);
        return buffer;
    }
    const bool readBool()
    {
        bool buffer = false;
        mBitStream.Read(buffer);
        return buffer;
    }
    const unsigned char readUChar()
    {
        unsigned char buffer = 0;
        mBitStream.Read(buffer);
        return buffer;
    }
    const char readChar()
    {
        char buffer = 0;
        mBitStream.Read(buffer);
        return buffer;
    }
    const unsigned short readUShort()
    {
        unsigned short buffer = 0;
        mBitStream.Read(buffer);
        return buffer;
    }
    const float readFloat()
    {
        float buffer = 0;
        mBitStream.Read(buffer);
        return buffer;
    }
};

class PacketWriter
{
private:
    RakPeerInterface *mPeer;
    BitStream mBitStream;
    MessageID mID;
public:
    PacketWriter()
    {
        mPeer = 0;
        mID = 0;
    }
    ~PacketWriter()
    {
    }
    void setPeer(RakPeerInterface *peer)
    {
        mPeer = peer;
    }
    void create(const MessageID &id)
    {
        mBitStream.Reset();
        mBitStream.Write(id);
        mID = id;
    }
    void writeString(const string &value)
    {
        stringCompressor->EncodeString(value.c_str(), (int)value.length()+1, &mBitStream);
    }
    void writeInt(const int &value)
    {
        mBitStream.Write(value);
    }
    void writeUInt(const unsigned int &value)
    {
        mBitStream.Write(value);
    }
    void writeBool(const bool &value)
    {
        mBitStream.Write(value);
    }
    void writeChar(const char &value)
    {
        mBitStream.Write(value);
    }
    void writeUChar(const unsigned char &value)
    {
        mBitStream.Write(value);
    }
    void writeUShort(const unsigned short &value)
    {
        mBitStream.Write(value);
    }
    void writeFloat(const float &value)
    {
        mBitStream.Write(value);
    }
    void send(const SystemAddress &add, const bool &bAddToQueue=false)
    {
        mPeer->Send(&mBitStream,HIGH_PRIORITY, RELIABLE_ORDERED, 0, add, false);
    }
    void sendUnreliable(const SystemAddress &add)
    {
        mPeer->Send(&mBitStream,LOW_PRIORITY, UNRELIABLE, 0, add, false);
    }
    void sendAll()
    {
        mPeer->Send(&mBitStream,HIGH_PRIORITY, RELIABLE_ORDERED, 0, UNASSIGNED_SYSTEM_ADDRESS, true);
    }
    void sendAllBut(const SystemAddress &add)
    {
        mPeer->Send(&mBitStream,HIGH_PRIORITY, RELIABLE_ORDERED, 0, add, true);
    }
    void updateQueue(const float &fTimeElapsed, const SystemAddress &cAdd)
    {

    }
    void clearQueue()
    {

    }
};

#endif
