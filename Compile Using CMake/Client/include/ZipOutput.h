#ifndef _ZIPOUTPUT_H_
#define _ZIPOUTPUT_H_

#include <stdio.h>
#include <ZipConstants.h>
#include <ZipEntry.h>
#include <Deflater.h>
#include <CRC32.h>
#include <map>

class ZipOutput
{
private:
    ZipEntry *mZipEntry;
    std::vector<ZipEntry*> _entries;
    Deflater _def;
    CRC32 _crc;
    ByteArray _buf;
    string _comment;
public:
    ZipOutput()
    {
        mZipEntry = 0;
        _entries.clear();
        _buf.mLittleEndian = true;
    }
    ~ZipOutput()
    {
        while(!_entries.empty())
        {
           ZipEntry *entry = _entries.back();
           _entries.pop_back();
           delete entry;
        }
    }
    const unsigned int size()
    {
        return _entries.size();
    }
    ByteArray* byteArray()
    {
        _buf.mPosition = 0;
        return &_buf;
    }

    void setComment(const string &value)
    {
        _comment = value;
    }

    void putNextEntry(const string &n)
    {
        if(mZipEntry) closeEntry();
        ZipEntry *e = new ZipEntry(n);

        if(e->dostime == 0)e->setDosTime();
        if(e->method == -1) e->method = ZIPCONSTANTS_DEFLATED; // use default method
        switch(e->method)
        {
            case ZIPCONSTANTS_DEFLATED:
                if(e->size == -1 || e->compressedSize == -1 || e->crc == 0)
                {
                    // store size, compressed size, and crc-32 in data descriptor
                    // immediately following the compressed entry data
                    e->flag = 8;
                }
                else if(e->size != -1 && e->compressedSize != -1 && e->crc != 0)
                {
                    // store size, compressed size, and crc-32 in LOC header
                    e->flag = 0;
                }
                else
                {
                    //printf("DEFLATED entry missing size, compressed size, or crc-32\n");
                }
                e->version = 20;
                break;
            case ZIPCONSTANTS_STORED:
                // compressed size, uncompressed size, and crc-32 must all be
                // set for entries using STORED compression method
                if(e->size == -1)
                {
                    e->size = e->compressedSize;
                }
                else if(e->compressedSize == -1)
                {
                    e->compressedSize = e->size;
                }
                else if(e->size != e->compressedSize)
                {
                    //printf("STORED entry where compressed != uncompressed size\n");
                }
                if(e->size == -1 || e->crc == 0)
                {
                    //printf("STORED entry missing size, compressed size, or crc-32");
                }
                e->version = 10;
                e->flag = 0;
                break;
            default:
                //printf("unsupported compression method\n");
                break;
        }
        e->offset = _buf.mPosition;
        writeLOC(e);
        _entries.push_back(e);
        mZipEntry = e;
    }

    void write(ByteArray *b)
    {
        if(!mZipEntry)return;
        switch(mZipEntry->method)
        {
            case ZIPCONSTANTS_DEFLATED:
                {
                    ByteArray cb = ByteArray();
                    _def.setInput(*b);
                    _def.deflate(&cb);
                    _buf.writeBytes(cb);
                }
                break;
            case ZIPCONSTANTS_STORED:
                _buf.writeBytes(*b);
                break;
            default:
                //printf("invalid compression method\n");
                break;
        }
        _crc.update(b);
    }

    void closeEntry()
    {
        if(mZipEntry)
        {
            switch (mZipEntry->method)
            {
                case ZIPCONSTANTS_DEFLATED:
                    if ((mZipEntry->flag & 8) == 0)
                    {
                        // verify size, compressed size, and crc-32 settings
                        if ((unsigned)mZipEntry->size != _def.getBytesRead())
                        {
                            //printf("invalid entry size (expected %d but got %d bytes)\n",mZipEntry->size,_def.getBytesRead());
                        }
                        if ((unsigned)mZipEntry->compressedSize != _def.getBytesWritten())
                        {
                            //printf("invalid entry compressed size (expected %d but got %d bytes)\n",mZipEntry->compressedSize,_def.getBytesWritten());
                        }
                        if (mZipEntry->crc != _crc.getValue())
                        {
                            //printf( "invalid entry CRC-32 (expected 0x%d but got 0x%d)\n",mZipEntry->crc,_crc.getValue());
                        }
                    }
                    else
                    {
                        mZipEntry->size = _def.getBytesRead();
                        mZipEntry->compressedSize = _def.getBytesWritten();
                        mZipEntry->crc = _crc.getValue();
                        writeEXT(mZipEntry);
                    }
                    _def.reset();
                    break;
                case ZIPCONSTANTS_STORED:
                    break;
                default:
                    //printf("invalid compression method\n");
                    break;
            }
            _crc.reset();
            mZipEntry = 0;
        }
    }

    const bool finish()
    {
        if(mZipEntry) closeEntry();
        if(_entries.size() < 1)return false;// printf("ZIP file must have at least one entry\n");
        const unsigned int off = _buf.mPosition;
        // write central directory
        for(std::vector<ZipEntry*>::iterator it=_entries.begin(); it!=_entries.end(); it++)
        {
            ZipEntry *entry = *it;
            writeCEN(entry);
        }
        writeEND(off, _buf.mPosition - off);
        return true;
    }

    void writeLOC(ZipEntry *e)
    {
        _buf.writeUnsignedInt(ZIPCONSTANTS_LOCSIG);
        _buf.writeShort(e->version);
        _buf.writeShort(e->flag);
        _buf.writeShort(e->method);
        _buf.writeUnsignedInt(e->dostime); // dostime
        if((e->flag & 8) == 8)
        {
            // store size, uncompressed size, and crc-32 in data descriptor
            // immediately following compressed entry data
            _buf.writeUnsignedInt(0);
            _buf.writeUnsignedInt(0);
            _buf.writeUnsignedInt(0);
        }
        else
        {
            _buf.writeUnsignedInt(e->crc); // crc-32
            _buf.writeUnsignedInt(e->compressedSize); // compressed size
            _buf.writeUnsignedInt(e->size); // uncompressed size
        }
        _buf.writeShort(e->name.length());
        _buf.writeShort(e->extra.length()>0 ? e->extra.length() : 0);
        _buf.writeUTFBytes(e->name);
        if(e->extra.length()>0)
        {
            _buf.writeBytes(e->extra);
        }
    }

    void writeEXT(ZipEntry *e)
    {
        _buf.writeUnsignedInt(ZIPCONSTANTS_EXTSIG); // EXT header signature
        _buf.writeUnsignedInt(e->crc); // crc-32
        _buf.writeUnsignedInt(e->compressedSize); // compressed size
        _buf.writeUnsignedInt(e->size); // uncompressed size
    }

    void writeCEN(ZipEntry *e)
    {
        _buf.writeUnsignedInt(ZIPCONSTANTS_CENSIG); // CEN header signature
        _buf.writeShort(e->version); // version made by
        _buf.writeShort(e->version); // version needed to extract
        _buf.writeShort(e->flag); // general purpose bit flag
        _buf.writeShort(e->method); // compression method
        _buf.writeUnsignedInt(e->dostime); // last modification time
        _buf.writeUnsignedInt(e->crc); // crc-32
        _buf.writeUnsignedInt(e->compressedSize); // compressed size
        _buf.writeUnsignedInt(e->size); // uncompressed size
        _buf.writeShort(e->name.length());
        _buf.writeShort(e->extra.length()>0 ? e->extra.length() : 0);
        _buf.writeShort(e->comment.length()>0 ? e->comment.length() : 0);
        _buf.writeShort(0); // starting disk number
        _buf.writeShort(0); // internal file attributes (unused)
        _buf.writeUnsignedInt(0); // external file attributes (unused)
        _buf.writeUnsignedInt(e->offset); // relative offset of local header
        _buf.writeUTFBytes(e->name);
        if(e->extra.length()>0)
        {
            _buf.writeBytes(e->extra);
        }
        if(e->comment.length()>0)
        {
            _buf.writeUTFBytes(e->comment);
        }
    }

    void writeEND(const unsigned int &off, const unsigned int &len)
    {
        _buf.writeUnsignedInt(ZIPCONSTANTS_ENDSIG); // END record signature
        _buf.writeShort(0); // number of this disk
        _buf.writeShort(0); // central directory start disk
        _buf.writeShort((short)_entries.size()); // number of directory entries on disk
        _buf.writeShort((short)_entries.size()); // total number of directory entries
        _buf.writeUnsignedInt(len); // length of central directory
        _buf.writeUnsignedInt(off); // offset of central directory
        _buf.writeUTF(_comment); // zip file comment
    }
};

#endif
