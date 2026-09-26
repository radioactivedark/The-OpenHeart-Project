#ifndef _ZIPMANAGER_H_
#define _ZIPMANAGER_H_

#define ZIP_TEMP_DIR "media/overlays/overlay1/"
#define ZIP_TEMP_EXTENSION ".dll"
#define EXPORT_DIR "exports/"

#include "SaveFile.h"
#include "ZipOutput.h"
#include "LoggerManager.h"
#include "LoadingBarManager.h"

class ZipManager : public Singleton<ZipManager>
{
private:
    std::vector<String> mDecryptedFilenames;
    std::vector<String> mTempFilenames;
public:
    ZipManager()
    {
    }
    ~ZipManager()
    {
        deleteTempFiles();
    }
    static ZipManager* getSingletonPtr();
    static ZipManager& getSingleton();
    void getTempFilenames()
    {
        mTempFilenames.clear();
        try
        {
            Archive *archive = ArchiveManager::getSingletonPtr()->load(ZIP_TEMP_DIR,"FileSystem");
            if(archive)
            {
                StringVectorPtr zipFilenames = archive->find("*"+String(ZIP_TEMP_EXTENSION));
                if(!zipFilenames.isNull())
                {
                    for(StringVector::const_iterator it=zipFilenames->begin(); it!=zipFilenames->end(); it++)
                    {
                        const String zipFilename = *it;
                        mTempFilenames.push_back(zipFilename);
                    }
                }
            }
        }
        catch( Ogre::Exception& e )
        {
            LoggerManager::getSingleton().logMessage("ERROR::GetFilenames failed!");
        }
    }
    void deleteTempFiles()
    {
        while(!mTempFilenames.empty())
        {
            const String filename = ZIP_TEMP_DIR + mTempFilenames.back();
            mTempFilenames.pop_back();
            _unlink(filename.c_str());
        }
    }
    const bool defhz(const String &filename, const bool &registerResource=true, const bool &redeclare=false)
    {
        const String infilename = filename;
        for(std::vector<String>::iterator it=mDecryptedFilenames.begin(); it!=mDecryptedFilenames.end(); it++)
        {
            const String decryptedFilename = *it;
            if(decryptedFilename==filename)
            {
                if(redeclare)
                {
                    if(ResourceGroupManager::getSingleton().isResourceGroupInitialised("Temp/"+filename))ResourceGroupManager::getSingleton().destroyResourceGroup("Temp/"+filename);
                    _unlink(filename.c_str());
                    mDecryptedFilenames.erase(it);
                    break;
                }
                else return true;
            }
        }
        string editedFilename = filename;
        const size_t chopPos = editedFilename.find_last_of('/');
        if(chopPos!=string::npos)editedFilename.erase(0,chopPos+1);
        const String outfilename = ZIP_TEMP_DIR+editedFilename+ZIP_TEMP_EXTENSION;

        /*ifstream infile(infilename.c_str(),ios::binary);
        if(!infile.good())return false;
        ofstream outfile(outfilename.c_str(),ios::binary);

        unsigned char i = 0;
        const char hex[] = {80,75,3,4};
        while(infile.good())
        {
            if(i<4)
            {
                outfile.put(hex[i]);
                i++;
            }
            else outfile.put(infile.get());
        }

        infile.close();
        outfile.close();*/

        ByteArray bArr;
        if(!bArr.writeFromFile(infilename.c_str(),true))return false;
        if(!bArr.isZipFile())return false;
        if(!bArr.writeToFile(outfilename.c_str()))return false;

        if(registerResource)
        {
            try
            {
                //Save this as bad file until it succeeds
                setBadFile(filename);

                //Overwrite particles to avoid clash
                Archive *archive = ArchiveManager::getSingletonPtr()->load(outfilename,"Zip");
                if(archive)
                {
                    StringVectorPtr zipFilenames = archive->find("*.particle");
                    if(!zipFilenames.isNull())
                    {
                        for(StringVector::const_iterator it=zipFilenames->begin(); it!=zipFilenames->end(); it++)
                        {
                            const String zipFilename = *it;
                            DataStreamPtr data = archive->open(zipFilename);
                            if(!data.isNull())
                            {
                                while(!data->eof())
                                {
                                    const String buffer = data->getLine();
                                    if(StringUtil::startsWith(buffer,"particle_system"))
                                    {
                                        const StringVector part = StringUtil::split(buffer," {}");
                                        if(part.size()>1)
                                        {
                                            const String paricleName = part[1];
                                            if(ParticleSystemManager::getSingletonPtr()->getTemplate(paricleName))
                                            {
                                                LoggerManager::getSingleton().logMessage("WARNING::LoadFile:Overwriting Particle Template: " + paricleName);
                                                ParticleSystemManager::getSingletonPtr()->removeTemplate(paricleName);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                #if OGRE_PLATFORM == OGRE_PLATFORM_APPLE
                ResourceGroupManager::getSingleton().addResourceLocation(String(macBundlePath()+"/"+outfilename),"Zip","Temp/"+filename);
                #else
                ResourceGroupManager::getSingleton().addResourceLocation(outfilename,"Zip","Temp/"+filename);
                #endif
                ResourceGroupManager::getSingleton().initialiseResourceGroup("Temp/"+filename);

                //File load success, remove as bad file
                clearBadFile();
            }

            catch( Ogre::Exception& e )
            {
                LoggerManager::getSingleton().logMessage("ERROR::LoadFile:defhz: " + filename + " failed!");
                clearBadFile();

                return false;
            }
        }

        mDecryptedFilenames.push_back(filename);
        return true;
    }
    const bool fhz(const std::vector<pair<String,String> > &filenames, const String &zipName)
    {
        ZipOutput zipOut;

        LoadingBarManager *loadBarMgr = LoadingBarManager::getSingletonPtr();
        const unsigned short totalProgress = filenames.size() + 1;
        unsigned short progress = 0;

        //Loading bar start
        loadBarMgr->show();

        for(std::vector<pair<String,String> >::const_iterator it=filenames.begin(); it!=filenames.end(); it++)
        {
            const pair<String,String> filename = *it;

            if(filename.first=="")
            {
                LoggerManager::getSingleton().logMessage("ERROR::ExportFile: " + filename.second + " cannot be found for " + zipName);
                continue;
            }

            loadBarMgr->setTitle(filename.second);

            if(!addToZip(&zipOut,filename.first,filename.second))
            {
                loadBarMgr->hide();
                return false;
            }
            //Loading bar progress
            progress++;
            loadBarMgr->progress(float(progress)/totalProgress);
        }

        loadBarMgr->setTitle(zipName);
        if(!zipOut.finish())
        {
            loadBarMgr->hide();
            return false;
        }
        loadBarMgr->progress(1.0f);

        const bool success = zipOut.byteArray()->writeToFile(zipName,true);

        //Loading bar end
        loadBarMgr->hide();

        return success;
    }
    const bool addToZip(ZipOutput *zipOut, const string &inFilename, const string &outFilename)
    {
        ByteArray fileData;
        if(!fileData.writeFromFile(inFilename))
        {
            LoggerManager::getSingleton().logMessage("ERROR::LoadFile:writeFromFile: " + inFilename + " failed!");
            return false;
        }
        if(fileData.length()<=0)
        {
            LoggerManager::getSingleton().logMessage("WARNING::LoadFile:writeFromFile: " + inFilename + " has 0 bytes.");
            return true;
        }
        zipOut->putNextEntry(outFilename);
        zipOut->write(&fileData);
        zipOut->closeEntry();
        return true;
    }
    static void salvageMaterialAndTextures(MaterialPtr mat, std::vector<String> *materialNames, std::vector<String> *texNames)
    {
        const String scriptName = mat->getOrigin();

        bool exists = false;
        //.material
        for(std::vector<String>::iterator jt=materialNames->begin(); jt!=materialNames->end(); jt++)
        {
            String cmpName = *jt;
            String cmpName2 = scriptName;
            StringUtil::toLowerCase(cmpName);
            StringUtil::toLowerCase(cmpName2);
            if(cmpName==cmpName2)
            {
                exists = true;
                break;
            }
        }
        if(!exists)materialNames->push_back(scriptName);

        //.jpg/png/etc
        Material::TechniqueIterator mit = mat->getTechniqueIterator();
        while(mit.hasMoreElements())
        {
            Technique *tech = mit.getNext();
            Technique::PassIterator pit = tech->getPassIterator();
            while(pit.hasMoreElements())
            {
                Pass *pass = pit.getNext();
                Pass::TextureUnitStateIterator tit = pass->getTextureUnitStateIterator();
                while(tit.hasMoreElements())
                {
                    TextureUnitState *tex = tit.getNext();
                    for(int i=0; i<(int)tex->getNumFrames(); i++)
                    {
                        const String texName = tex->getFrameTextureName(i);
                        if(!SaveFile::resourceExists(texName))continue;
                        exists = false;
                        for(std::vector<String>::iterator it=texNames->begin(); it!=texNames->end(); it++)
                        {
                            String cmpName = *it;
                            String cmpName2 = texName;
                            StringUtil::toLowerCase(cmpName);
                            StringUtil::toLowerCase(cmpName2);
                            if(cmpName==cmpName2)
                            {
                                exists = true;
                                break;
                            }
                        }
                        if(!exists)texNames->push_back(texName);
                    }
                }
            }
        }
    }
    const bool loadByAnyMeans(SaveFile *sf, const String &filename, const String &fhzExtension, const String &extension)
    {
        const String fhzFilename = SaveFile::findDirectoryOf(filename+fhzExtension);
        if(fhzFilename=="" || !defhz(fhzFilename))
        {
            if(!sf->loadAnywhere(filename+extension))
            {
                LoggerManager::getSingleton().logMessage("ERROR::LoadFile::cannot find "+filename+extension);
                return false;
            }
        }
        else if(!sf->loadZip(filename+extension,ZIP_TEMP_DIR+filename+fhzExtension+ZIP_TEMP_EXTENSION))
        {
            LoggerManager::getSingleton().logMessage("ERROR::LoadFile::cannot load "+filename+extension);
            return false;
        }
        return true;
    }
    void setBadFile(const String &szFilename)
    {
        std::ofstream cOutfile("badfile.reminder",std::ios::binary);
        cOutfile.write(szFilename.c_str(), szFilename.length());
        cOutfile.close();
    }
    void clearBadFile()
    {
        _unlink("badfile.reminder");
    }
    const String getBadFile()
    {
        String szFilename = "";
        std::ifstream cInfile("badfile.reminder",std::ios::binary);
        if(cInfile.good())
        {
            char szBuffer[256] = "";
            cInfile.read(szBuffer,256);
            szFilename = szBuffer;
        }
        cInfile.close();

        clearBadFile();

        return szFilename;
    }
};

template<> ZipManager* Singleton<ZipManager>::ms_Singleton = 0;

ZipManager* ZipManager::getSingletonPtr()
{
	return ms_Singleton;
}

ZipManager& ZipManager::getSingleton()
{
	assert(ms_Singleton);
	return *ms_Singleton;
}

#endif
