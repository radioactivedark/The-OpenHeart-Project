#ifndef _BYTEARRAY_H_
#define _BYTEARRAY_H_

#include <vector>
#include <string.h>
#include <zlib/zlib.h>
#include <ZipConstants.h>

#define CHUNK 16384
// The crypt key below is dead-simple. As it stands, the original string used in Kovu's source was 68 characters. Yours should match.
// Unless you change the XOR length everywhere it appears, or replace the code entirely to define it based on the length of the 
// crypt key, it must equate to exactly 68 characters. Otherwise, it'll be unable to encrypt or decrypt assets. 
// I.E., game no worky. 
// Also, choice words for whoever put that L33T as the crypt key. Maybe don't put death threats in your code next time, bozo.
#define CRYPT_KEY "INSERT_YOUR_CRYPT_KEY_HERE"

using namespace std;

class ByteArray
{
private:
    std::vector<unsigned char> mBytes;
public:
    bool mLittleEndian;
    unsigned int mPosition;
    ByteArray()
    {
        mBytes.clear();
        mPosition = 0;
        mLittleEndian = false;
    }
    ~ByteArray()
    {
        mBytes.clear();
    }
    void clear()
    {
        mBytes.clear();
        mPosition = 0;
    }
    const unsigned int length()
    {
        return mBytes.size();
    }
    const int compress()
    {
        int ret, flush;
        unsigned int have;
        z_stream strm;
        unsigned char in[CHUNK];
        unsigned char out[CHUNK];

        /* allocate deflate state */
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        ret = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
        if (ret != Z_OK)return ret;

        std::vector<unsigned char> newBytes;
        unsigned int readBytes = 0;
        unsigned int readBuffer = 0;

        for(std::vector<unsigned char>::const_iterator it=mBytes.begin(); it!=mBytes.end(); it++)
        {
            const unsigned char c = *it;
            in[readBuffer] = c;
            readBytes++;
            readBuffer++;
            flush = (readBytes==(unsigned int)mBytes.size()) ? Z_FINISH : Z_NO_FLUSH;
            if(readBuffer>=CHUNK || flush==Z_FINISH)
            {
                strm.avail_in = readBuffer;
                strm.next_in = in;
                do
                {
                    strm.avail_out = CHUNK;
                    strm.next_out = out;

                    ret = deflate(&strm, flush);

                    have = CHUNK - strm.avail_out;
                    for(unsigned int i=0;i<have;i++)
                    {
                        newBytes.push_back(out[i]);
                    }

                } while (strm.avail_out == 0);

                readBuffer = 0;
            }
        }

        mBytes.clear();
        mBytes = newBytes;

        (void)deflateEnd(&strm);
        return Z_OK;
    }
    const unsigned char at(const unsigned int &pos)
    {
        if(pos<(unsigned int)mBytes.size())return mBytes[pos];
        return 0;
    }
    void writeUnsignedInt(const unsigned int &value)
    {
        if(mLittleEndian)
        {
            mBytes.push_back(value & 0x000000FF);
            mBytes.push_back((value>>8) & 0x000000FF);
            mBytes.push_back((value>>16) & 0x000000FF);
            mBytes.push_back((value>>24) & 0x000000FF);
        }
        else
        {
            mBytes.push_back((value>>24) & 0x000000FF);
            mBytes.push_back((value>>16) & 0x000000FF);
            mBytes.push_back((value>>8) & 0x000000FF);
            mBytes.push_back(value & 0x000000FF);
        }
        mPosition += 4;
    }
    void writeShort(const short &value)
    {
        if(mLittleEndian)
        {
            mBytes.push_back(value & 0x00FF);
            mBytes.push_back((value>>8) & 0x00FF);
        }
        else
        {
            mBytes.push_back((value>>8) & 0x00FF);
            mBytes.push_back(value & 0x00FF);
        }
        mPosition += 2;
    }
    void writeUnsignedShort(const unsigned short &value)
    {
        if(mLittleEndian)
        {
            mBytes.push_back((value>>8) & 0x00FF);
            mBytes.push_back(value & 0x00FF);
        }
        else
        {
            mBytes.push_back((value>>8) & 0x00FF);
            mBytes.push_back(value & 0x00FF);
        }
        mPosition += 2;
    }
    void writeUTFBytes(const string &value)
    {
        for(int i=0;i<(int)value.length();i++)
        {
            mBytes.push_back(value[i]);
            mPosition++;
        }
    }
    void writeByte(const char &value)
    {
        mBytes.push_back(value);
        mPosition++;
    }
    void writeUTF(const string &value)
    {
        writeUnsignedShort(value.length());
        writeUTFBytes(value);
    }
    void writeBytes(const ByteArray &value, const unsigned int &offset=0, const unsigned int &length=0xffffffff)
    {
        unsigned int i=0;
        unsigned int written = 0;
        for(std::vector<unsigned char>::const_iterator it=value.mBytes.begin(); it!=value.mBytes.end(); it++)
        {
            if(i<offset)
            {
                i++;
                continue;
            }
            const char c = *it;
            mBytes.push_back(c);
            mPosition++;
            written++;
            if(written>=length)return;
        }
    }
    const bool writeFromFile(const string &filename, const bool &decrypt=false)
    {
        ifstream inFile(filename.c_str(),ios::in|ios::binary);
        if(!inFile.good())
        {
            inFile.close();
            return false;
        }
        const string key = CRYPT_KEY;
        unsigned char i = 0;
        while(inFile.good())
        {
            const unsigned char c = inFile.get();
            if(inFile.eof())break;
            mBytes.push_back(decrypt ? c^key[i] : c);
            mPosition++;
            i++;
            if(i>=68)i=0;
        }
        inFile.close();
        return true;
    }
    const bool writeToFile(const string &filename, const bool &encrypt=false)
    {
        ofstream outFile(filename.c_str(),ios::out|ios::binary);
        if(!outFile.good())
        {
            outFile.close();
            return false;
        }
        const string key = CRYPT_KEY;
        unsigned char i = 0;
        for(std::vector<unsigned char>::const_iterator it=mBytes.begin(); it!=mBytes.end(); it++)
        {
            const unsigned char c = *it;
            outFile.put(encrypt ? c^key[i] : c);
            i++;
            if(i>=68)i=0;
        }
        outFile.close();
        return true;
    }
    const bool isZipFile()
    {
        if(mBytes.size()<4)return false;
        unsigned int sig = mBytes[0];
        sig += mBytes[1] << 8;
        sig += mBytes[2] << 16;
        sig += mBytes[3] << 24;
        return(sig==ZIPCONSTANTS_LOCSIG);
    }
    const string toString()
    {
        string buffer = "";
        for(std::vector<unsigned char>::const_iterator it=mBytes.begin(); it!=mBytes.end(); it++)
        {
            const char c = *it;
            buffer += c;
        }
        return buffer;
    }
    void replace(const string &from, const string &to)
    {
        if(from.length()<=0)return;

        std::vector<unsigned char> newBytes;
        unsigned short ptr = 0;
        string buffer = "";
        for(std::vector<unsigned char>::const_iterator it=mBytes.begin(); it!=mBytes.end(); it++)
        {
            const char c = *it;
            //Character match
            if(c==from[ptr])
            {
                buffer += c;
                ptr++;
                if(ptr>=(int)from.length())
                {
                    for(int i=0;i<(int)to.length();i++)newBytes.push_back(to[i]);
                    buffer = "";
                    ptr = 0;
                }
            }
            else
            {
                if(buffer!="")
                {
                    for(int i=0;i<(int)buffer.length();i++)newBytes.push_back(buffer[i]);
                    buffer = "";
                    ptr = 0;
                }
                newBytes.push_back(c);
            }
        }
        //Remaining buffer
        if(buffer!="")
        {
            for(int i=0;i<(int)buffer.length();i++)newBytes.push_back(buffer[i]);
        }
        mBytes = newBytes;
    }
    const std::vector<std::pair<String,String> > splitScript(const String &filename, const String &extension, const String &header)
    {
        std::vector<std::pair<String,String> > scriptFilenames;
        ByteArray currentScript;
        String currentName = "";
        String currentHeader = "";
        bool isComment = false;
        bool isHeaderName = false;
        bool isScript = false;
        bool endOfScript = false;
        short braceCount = 0;
        unsigned short scriptCount = 0;

        for(std::vector<unsigned char>::const_iterator it=mBytes.begin(); it!=mBytes.end(); it++)
        {
            const char c = *it;

            //Script
            if(isScript)
            {
                currentScript.writeByte(c);
                if(c=='{')braceCount++;
                else if(c=='}')
                {
                    braceCount--;
                    if(braceCount<0)braceCount = 0;
                    if(braceCount==0)
                    {
                        isScript = false;
                        endOfScript = true;
                    }
                }
                continue;
            }
            //Close and write script
            if(endOfScript)
            {
                currentScript.writeByte(c);
                //Final newline at end of script
                if(c=='\n')
                {
                    endOfScript = false;

                    //Write script to file
                    const String scriptFilename = filename+"_"+StringConverter::toString(scriptCount)+extension;
                    currentScript.writeToFile(scriptFilename);
                    scriptFilenames.push_back(pair<String,String>(currentName,scriptFilename));
                    scriptCount++;

                    currentScript.clear();
                }
                continue;
            }

            currentHeader.push_back(c);

            //Comments
            if(isComment)
            {
                if(c=='\n')
                {
                    currentScript.writeUTFBytes(currentHeader);
                    currentHeader = "";
                    isComment = false;
                }
                continue;
            }
            if(currentHeader=="//")
            {
                isComment = true;
                continue;
            }

            //Header and name
            if(isHeaderName)
            {
                if(c=='\n')
                {
                    currentScript.writeUTFBytes(currentHeader);
                    const StringVector part = StringUtil::split(currentHeader," \r\n");
                    if(part.size()>1)currentName = part[1];
                    else currentName = "";
                    currentHeader = "";
                    isHeaderName = false;
                    isScript = true;
                }
                continue;
            }
            if(currentHeader==header)
            {
                isHeaderName = true;
                continue;
            }
        }
        if(endOfScript)
        {
            //Write script to file
            const String scriptFilename = filename+"_"+StringConverter::toString(scriptCount)+extension;
            currentScript.writeToFile(scriptFilename);
            scriptFilenames.push_back(pair<String,String>(currentName,scriptFilename));
            scriptCount++;
            currentScript.clear();
        }

        return scriptFilenames;
    }
};

#endif
