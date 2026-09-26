#ifndef _CRC32_H_
#define _CRC32_H_

#include <ByteArray.h>

class CRC32
{
private:
    unsigned int crc;
    unsigned int crcTable[256];
public:
    CRC32()
    {
        crc = 0;
        makeCrcTable();
    }
    void makeCrcTable()
    {
        for(int n = 0; n < 256; n++)
        {
            unsigned int c = n;
            for(int k = 8; --k >= 0; )
            {
                if((c & 1) != 0) c = 0xedb88320 ^ (c >> 1);
                else c = c >> 1;
            }
            crcTable[n] = c;
        }
    }
    const unsigned int getValue()
    {
        return (crc & 0xffffffff);
    }
    void reset()
    {
        crc = 0;
    }
    void update(ByteArray *buf)
    {
        unsigned int off = 0;
        unsigned int len = buf->length();
        unsigned int c = ~crc;
        while(--len >= 0)
        {
            c = crcTable[(c ^ buf->at(off++)) & 0xff] ^ (c >> 8);
            if(len==0)break;
        }
        crc = ~c;
    }

};

#endif
