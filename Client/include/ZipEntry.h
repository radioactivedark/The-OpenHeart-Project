#ifndef _ZIPENTRY_H_
#define _ZIPENTRY_H_

#include <ByteArray.h>
#include <time.h>
class ZipEntry
{
public:
    string name;
    int size;
    int compressedSize;
    unsigned int dostime;
    unsigned int crc;
    int method; // compression method
    ByteArray extra; // optional extra field data for entry
    int flag; // bit flags
    int version; // version needed to extract
    int offset; // offset of loc header
    string comment; // optional comment string for entry
    ZipEntry(const string &n)
    {
        name = n;
        size = -1;
        compressedSize = -1;
        crc = 0;
        method = -1;

        dostime = 0;
        flag = 0;
        version = 0;
        offset = 0;
    }

    void setDosTime()
    {
        time_t t_even;
        struct tm *s;

        time(&t_even);
        t_even = (t_even + 1) & (~1);     /* Round up to even seconds. */
        s = localtime(&t_even);
        s->tm_year += 1900;
        s->tm_mon += 1;

        dostime = (s->tm_year < 1980)
        ? ((s->tm_mon << 21) | (s->tm_mday << 16) | (s->tm_hour << 11) | (s->tm_min << 5) | (s->tm_sec >> 1))
        : (((s->tm_year - 1980) << 25) | (s->tm_mon << 21) | (s->tm_mday << 16) | (s->tm_hour << 11) | (s->tm_min << 5) | (s->tm_sec >> 1));
    }

    const bool isDirectory()
    {
        if(name.length()<=0)return false;
        return (name[name.length() - 1] == '/');
    }

    const string toString()
    {
        return name;
    }

};

#endif
