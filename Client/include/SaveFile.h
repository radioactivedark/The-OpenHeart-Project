#ifndef _SAVEFILE_H_
#define _SAVEFILE_H_

#include "LoggerManager.h"

struct SaveFileSetting
{
    String mTag;
    String mValue;
    SaveFileSetting()
    {
        mTag = "";
        mValue = "";
    }
};

class SaveFileSection
{
public:
    String mName;
    std::vector<SaveFileSetting*> mSettingList;
    SaveFileSection(const String &name)
    {
        mName = name;
    }
    ~SaveFileSection()
    {
        clear();
    }
    void clear()
    {
        while(!mSettingList.empty())
        {
            SaveFileSetting *set = mSettingList.back();
            mSettingList.pop_back();
            delete set;
        }
    }
};

class SaveFile
{
private:
    String mFilename;
    std::vector<SaveFileSection*> mSectionList;
    std::vector<SaveFileSection*>::iterator mSectionIterator;
    std::vector<SaveFileSetting*>::iterator mSettingIterator;
    std::vector<SaveFileSetting*>::iterator mSettingIteratorEnd;
public:
    SaveFile()
    {
        mFilename = "";
    }
    SaveFile(const String &filename)
    {
        load(filename);
    }
    ~SaveFile()
    {
        clear();
    }
    void clear()
    {
        while(!mSectionList.empty())
        {
            SaveFileSection *sec = mSectionList.back();
            mSectionList.pop_back();
            delete sec;
        }
    }
    const bool isEmpty()
    {
        return mSectionList.empty();
    }
    const bool load(const String &filename)
    {
        mFilename = filename;

        std::ifstream inFile(filename.c_str(),std::ios::in);
        if(!inFile.good())
        {
            inFile.close();
            return false;
        }

        SaveFileSection *currentSection = 0;
        while(inFile.good())
        {
            String nextLine = "";
            char lastChar = 0;
            while(inFile.good())
            {
                const char c = inFile.get();
                if(c=='\n')break;
                //Don't read last eof character
                if(inFile.good())
                {
                    nextLine.push_back(c);
                    lastChar = c;
                }
            }
            if(nextLine.length()<=0)continue;

            //Section
            if(nextLine[0]=='[' && lastChar==']')
            {
                nextLine.erase(0,1);
                nextLine.erase((int)nextLine.length()-1,1);
                currentSection = new SaveFileSection(nextLine);
                mSectionList.push_back(currentSection);
            }
            //Comment
            else if(nextLine[0]=='#')continue;
            //Setting
            else
            {
                //Nameless section
                if(!currentSection)
                {
                    currentSection = new SaveFileSection("");
                    mSectionList.push_back(currentSection);
                }
                SaveFileSetting *setting = new SaveFileSetting();

                const StringVector part = StringUtil::split(nextLine,"=",1);
                //No tag, just value
                if(part.size()<2)setting->mValue = nextLine;
                //Both tag and value
                else
                {
                    setting->mTag = part[0];
                    setting->mValue = part[1];
                }
                currentSection->mSettingList.push_back(setting);
            }
        }

        inFile.close();
        return true;
    }
    const bool loadZip(const String &filename, const String &zipFilename)
    {
        //Test existance of temp file
        std::ifstream infile(zipFilename.c_str());
        if(!infile.good())return false;
        infile.close();
        //Load temp zip file
        DataStream *inStream = 0;
        Archive *zipFile = 0;
        DataStreamPtr data;
        try
        {
            zipFile = ArchiveManager::getSingleton().load(zipFilename,"Zip");
            data = zipFile->open(filename);
            if(!data.isNull())inStream = data.get();
        }
        catch( Ogre::Exception& e )
        {
        }

        if(!inStream)
        {
            LoggerManager::getSingleton().logMessage("ERROR::SaveFile:loadz: " + filename + " failed!");
            return false;
        }

        SaveFileSection *currentSection = 0;
        while(!inStream->eof())
        {
            String nextLine = inStream->getLine(false);
            if(nextLine.length()<=0)continue;
            const char lastChar = nextLine[nextLine.length()-1];

            //Section
            if(nextLine[0]=='[' && lastChar==']')
            {
                nextLine.erase(0,1);
                nextLine.erase((int)nextLine.length()-1,1);
                currentSection = new SaveFileSection(nextLine);
                mSectionList.push_back(currentSection);
            }
            //Comment
            else if(nextLine[0]=='#')continue;
            //Setting
            else
            {
                //Nameless section
                if(!currentSection)
                {
                    currentSection = new SaveFileSection("");
                    mSectionList.push_back(currentSection);
                }
                SaveFileSetting *setting = new SaveFileSetting();

                const StringVector part = StringUtil::split(nextLine,"=",1);
                //No tag, just value
                if(part.size()<2)setting->mValue = nextLine;
                //Both tag and value
                else
                {
                    setting->mTag = part[0];
                    setting->mValue = part[1];
                }
                currentSection->mSettingList.push_back(setting);
            }
        }

        //inStream->close();
        return true;
    }
    const bool saveAs(const String &filename)
    {
        const String origFilename = mFilename;
        mFilename = filename;
        const bool res = save();
        mFilename = origFilename;
        return res;
    }
    const bool save()
    {
        std::ofstream outFile(mFilename.c_str(),std::ios::out);
        if(!outFile.good())
        {
            outFile.close();
            return false;
        }

        std::vector<SaveFileSection*>::iterator seci = mSectionList.begin();
        std::vector<SaveFileSection*>::iterator secEnd = mSectionList.end();

        while(seci!=secEnd)
        {
            SaveFileSection *sec = *seci;

            if(sec->mName!="")
            {
                const String secName = "[" + sec->mName + "]\n";
                outFile.write(secName.c_str(),secName.length());
            }

            std::vector<SaveFileSetting*>::iterator seti = sec->mSettingList.begin();
            std::vector<SaveFileSetting*>::iterator setEnd = sec->mSettingList.end();
            while(seti != setEnd)
            {
                SaveFileSetting *set = *seti;

                const String buffer = set->mTag + ((set->mTag=="")?"":"=") + set->mValue + "\n";
                outFile.write(buffer.c_str(),buffer.length());
                seti++;
            }
            seci++;
        }

        outFile.close();
        return true;
    }
    void setSetting(const String &tag, const String &value, const String &section = "")
    {
        std::vector<SaveFileSection*>::iterator seci = mSectionList.begin();
        std::vector<SaveFileSection*>::iterator secEnd = mSectionList.end();

        while(seci!=secEnd)
        {
            SaveFileSection *sec = *seci;

            //Section found
            if(sec->mName==section)
            {
                std::vector<SaveFileSetting*>::iterator seti = sec->mSettingList.begin();
                std::vector<SaveFileSetting*>::iterator setEnd = sec->mSettingList.end();

                if(tag!="")
                {
                    while(seti != setEnd)
                    {
                        SaveFileSetting *set = *seti;

                        //Tag found
                        if(set->mTag==tag)
                        {
                            set->mValue = value;
                            return;
                        }
                        seti++;
                    }
                }

                //Tag doesn't exist
                SaveFileSetting *set = new SaveFileSetting();
                set->mTag = tag;
                set->mValue = value;
                sec->mSettingList.push_back(set);
                return;
            }
            seci++;
        }

        //Section doesn't exist
        SaveFileSection *sec = new SaveFileSection(section);
        SaveFileSetting *set = new SaveFileSetting();
        set->mTag = tag;
        set->mValue = value;
        sec->mSettingList.push_back(set);
        mSectionList.push_back(sec);
    }
    const bool getSetting(const String &tag, String &value, const String &section = "")
    {
        std::vector<SaveFileSection*>::iterator seci = mSectionList.begin();
        std::vector<SaveFileSection*>::iterator secEnd = mSectionList.end();

        while(seci!=secEnd)
        {
            SaveFileSection *sec = *seci;

            //Section found
            if(sec->mName==section)
            {
                std::vector<SaveFileSetting*>::iterator seti = sec->mSettingList.begin();
                std::vector<SaveFileSetting*>::iterator setEnd = sec->mSettingList.end();

                while(seti != setEnd)
                {
                    SaveFileSetting *set = *seti;

                    //Tag found
                    if(set->mTag==tag)
                    {
                        value = set->mValue;
                        return true;
                    }
                    seti++;
                }

                //Tag doesn't exist
                return false;
            }
            seci++;
        }

        //Section doesn't exist
        return false;
    }
    void clearSection(const String &section)
    {
        std::vector<SaveFileSection*>::iterator seci = mSectionList.begin();
        std::vector<SaveFileSection*>::iterator secEnd = mSectionList.end();

        while(seci!=secEnd)
        {
            SaveFileSection *sec = *seci;

            //Section found
            if(sec->mName==section)
            {
                sec->clear();
                return;
            }
            seci++;
        }
    }
    const bool hasSection(const String &section)
    {
        std::vector<SaveFileSection*>::iterator seci = mSectionList.begin();
        std::vector<SaveFileSection*>::iterator secEnd = mSectionList.end();

        while(seci!=secEnd)
        {
            SaveFileSection *sec = *seci;

            //Section found
            if(sec->mName==section)return true;
            seci++;
        }

        return false;
    }
    void deleteSection(const String &section)
    {
        std::vector<SaveFileSection*>::iterator seci = mSectionList.begin();
        std::vector<SaveFileSection*>::iterator secEnd = mSectionList.end();

        while(seci!=secEnd)
        {
            SaveFileSection *sec = *seci;

            //Section found
            if(sec->mName==section)
            {
                delete sec;
                mSectionList.erase(seci);
                return;
            }
            seci++;
        }
    }
    void deleteSetting(const String &tag, const String &section)
    {
        std::vector<SaveFileSection*>::iterator seci = mSectionList.begin();
        std::vector<SaveFileSection*>::iterator secEnd = mSectionList.end();

        while(seci!=secEnd)
        {
            SaveFileSection *sec = *seci;

            //Section found
            if(sec->mName==section)
            {
                std::vector<SaveFileSetting*>::iterator seti = sec->mSettingList.begin();
                std::vector<SaveFileSetting*>::iterator setEnd = sec->mSettingList.end();

                while(seti != setEnd)
                {
                    SaveFileSetting *set = *seti;

                    //Tag found
                    if(set->mTag==tag)
                    {
                        delete set;
                        sec->mSettingList.erase(seti);
                        return;
                    }
                    seti++;
                }
            }
            seci++;
        }
    }
    void beginSectionIterator()
    {
        mSectionIterator = mSectionList.begin();
    }
    const bool hasMoreSections()
    {
        return (mSectionIterator != mSectionList.end());
    }
    const String peekNextSectionName()
    {
        SaveFileSection *sec = *mSectionIterator;
        return sec->mName;
    }
    void nextSection()
    {
        mSectionIterator++;
    }
    void beginSettingIterator()
    {
        SaveFileSection *sec = *mSectionIterator;
        mSettingIterator = sec->mSettingList.begin();
        mSettingIteratorEnd = sec->mSettingList.end();
    }
    const bool hasMoreSettings()
    {
        return (mSettingIterator != mSettingIteratorEnd);
    }
    void nextSetting()
    {
        mSettingIterator++;
    }
    const String getSettingTag()
    {
        SaveFileSetting *set = *mSettingIterator;
        return set->mTag;
    }
    const String getSettingValue()
    {
        SaveFileSetting *set = *mSettingIterator;
        return set->mValue;
    }
    static const StringVector findResourceNames(const String &group, const String &wildcard)
    {
        //-Wno-uninitialized added in build options to remove warning
        return *ResourceGroupManager::getSingletonPtr()->findResourceNames(group,wildcard);
    }
    static const std::vector<String> findResourceNames(const String &wildcard)
    {
        std::vector<String> nameList;
        ResourceGroupManager *resGrpMgr = ResourceGroupManager::getSingletonPtr();
        StringVector groupList = resGrpMgr->getResourceGroups();
        for(StringVector::iterator it=groupList.begin(); it!=groupList.end(); it++)
        {
            const String groupName = *it;
            if(StringUtil::startsWith(groupName,"Temp/",false))continue;
            StringVectorPtr list = resGrpMgr->findResourceNames(groupName,wildcard);
            if(!list.isNull())
            {
                for(StringVector::iterator lit=list->begin(); lit!=list->end(); lit++)
                {
                    const String name = *lit;
                    nameList.push_back(name);
                }
            }
        }
        return nameList;
    }
    static const bool resourceExists(const String &filename, String *groupname=0)
    {
        ResourceGroupManager *resGrpMgr = ResourceGroupManager::getSingletonPtr();
        StringVector groupList = resGrpMgr->getResourceGroups();
        for(StringVector::iterator it=groupList.begin(); it!=groupList.end(); it++)
        {
            const String groupName = *it;
            if(StringUtil::startsWith(groupName,"Temp/",false))continue;
            if(resGrpMgr->resourceExists(groupName,filename))
            {
                if(groupname)*groupname = groupName;
                return true;
            }
        }
        return false;
    }
    static const String findDirectoryOf(const String &filename)
    {
        if(filename=="")return"";
        ResourceGroupManager *resGrpMgr = ResourceGroupManager::getSingletonPtr();
        StringVector groupList = resGrpMgr->getResourceGroups();
        for(StringVector::iterator it=groupList.begin(); it!=groupList.end(); it++)
        {
            const String groupName = *it;
            FileInfoListPtr list = resGrpMgr->findResourceFileInfo(groupName,filename);
            if(!list.isNull())
            {
                for(FileInfoList::iterator fit=list->begin(); fit!=list->end(); fit++)
                {
                    const FileInfo info = *fit;
                    if(info.archive)return info.archive->getName()+"/"+filename;
                }
            }
        }
        return "";
    }
    const bool loadAnywhere(const String &filename)
    {
        const String filenamedir = findDirectoryOf(filename);
        if(filenamedir=="")return false;
        return load(filenamedir);
    }
};

#endif
