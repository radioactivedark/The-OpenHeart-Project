#ifndef _DEFLATER_H_
#define _DEFLATER_H_

#include <ByteArray.h>

class Deflater
{
private:
    ByteArray buf;
    bool compressed;
    unsigned int totalIn;
    unsigned int totalOut;

public:
    Deflater()
    {
        reset();
    }
   void reset()
   {
        buf.clear();
        buf.mLittleEndian = true;
        compressed = false;
        totalOut = totalIn = 0;
    }
    void setInput(const ByteArray &input)
    {
        buf.writeBytes(input);
        totalIn = buf.length();
    }
    const unsigned int deflate(ByteArray *output)
    {
        if(!compressed)
        {
            buf.compress();
            compressed = true;
        }
        output->writeBytes(buf, 2, buf.length() - 6); // remove 2-byte header and last 4-byte addler32 checksum
        totalOut = output->length();
        return 0;
    }
    const unsigned int getBytesRead()
    {
        return totalIn;
    }
    const unsigned int getBytesWritten()
    {
        return totalOut;
    }

};

#endif
