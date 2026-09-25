#ifndef _SOUNDMANAGER_H_
#define _SOUNDMANAGER_H_

#define SOUND_CLICK "media/sounds/click.wav"
#define SOUND_RUNGRASS "media/sounds/run_grass.wav"
#define SOUND_WALKGRASS "media/sounds/walk_grass.wav"
#define SOUND_WATERWADE "media/sounds/water_wade.wav"
#define SOUND_ROAR "media/sounds/roar1.mp3"
#define SOUND_HOWL "media/sounds/howl1.mp3"

#define MUSIC_ALL "All"
#define MUSIC_TITLE "Title"
#define MUSIC_PLAINS "Plains"
#define MUSIC_CAVES "Caves"
#define MUSIC_SACRED "Sacred"

#include <irrKlang.h>
#include <LoggerManager.h>

using namespace irrklang;

class SoundManager : public Singleton<SoundManager>, public ISoundStopEventReceiver
{
private:
    ISoundEngine *mSoundEng;
    std::vector<ISound*> mSoundList;
    ISound *mMusic;
    Real mEffectsVolume;
    Real mMusicVolume;
    Real mGUIVolume;
    std::vector<String> mMusicList;
    char mCurrentMusic;
    String mCurrentMusicType;
public:
    SoundManager()
    {
        mSoundEng = createIrrKlangDevice();
        LoggerManager::getSingleton().logMessage("Sound:: Initialized: " + String(mSoundEng?"YES":"NO"));
        if(mSoundEng)mSoundEng->setDefault3DSoundMinDistance(200.0f);
        mMusic = 0;
        mCurrentMusic = -1;
        mCurrentMusicType = "";
    }
    ~SoundManager()
    {
        clear();
        stopMusic();
        if(mSoundEng)mSoundEng->drop();
    }
    static SoundManager* getSingletonPtr();
    static SoundManager& getSingleton();
    void clear()
    {
        while(!mSoundList.empty())
        {
            ISound *sound = mSoundList.back();
            mSoundList.pop_back();
            sound->stop();
            sound->drop();
        }
    }
    void update(const Vector3 &camPosition, const Vector3 &camDirection)
    {
        if(!mSoundEng)return;
        //Update 3D sounds
        const vec3df camPos = parseVec3df(camPosition);
        const vec3df camDir = parseVec3df(camDirection);
        mSoundEng->setListenerPosition(camPos,camDir);
    }
    ISound* play3DSound(const char *filename, const Vector3 &position, const bool &looped=false, const bool &keepRef=false, const Real &range=200.0f)
    {
        if(!mSoundEng)return 0;
        ISound *sound = mSoundEng->play3D(filename,parseVec3df(position),looped,true,true);
        //Search sound in all resource groups
        if(!sound)
        {
            const String filenamedir = SaveFile::findDirectoryOf(filename);
            if(filenamedir!="")sound = mSoundEng->play3D(filenamedir.c_str(),parseVec3df(position),looped,true,true);
        }
        if(sound)
        {
            if(looped||keepRef)mSoundList.push_back(sound);
            else sound->setSoundStopEventReceiver(this);
            sound->setMinDistance(range);
            sound->setVolume(mEffectsVolume);
            sound->setIsPaused(false);
        }
        return sound;
    }
    ISound* play2DSound(const char *filename, const bool &looped=false)
    {
        if(!mSoundEng)return 0;
        ISound *sound = mSoundEng->play2D(filename,looped,true,true);
        if(sound)
        {
            if(looped)mSoundList.push_back(sound);
            else sound->setSoundStopEventReceiver(this);
            sound->setVolume(mGUIVolume);
            sound->setIsPaused(false);
        }
        return sound;
    }
    void stopLoopedSound(ISound *sound)
    {
        for(std::vector<ISound*>::iterator it=mSoundList.begin(); it!=mSoundList.end(); it++)
        {
            ISound *snd = *it;
            if(snd==sound)
            {
                snd->stop();
                snd->drop();
                mSoundList.erase(it);
                break;
            }
        }
    }
    void OnSoundStopped(ISound *sound, E_STOP_EVENT_CAUSE reason, void *userData)
	{
	    if(sound && reason==ESEC_SOUND_FINISHED_PLAYING)
	    {
	        if(sound==mMusic)
            {
                playNextMusic();
                return;
            }
	        sound->stop();
            sound->drop();
	    }
	}
    void playMusic(const String &filename, const bool &loop=true)
    {
        if(!mSoundEng)return;
        if(mMusic)stopMusic(false);
        mMusic = mSoundEng->play2D(filename.c_str(),loop,true,true);
        if(mMusic)
        {
            mMusic->setVolume(mMusicVolume);
            mMusic->setIsPaused(false);
            mMusic->setSoundStopEventReceiver(this);
        }
    }
    void stopMusic(const bool &clearPrevIndex=true)
    {
        if(mMusic)
        {
            mMusic->stop();
            mMusic->drop();
            mMusic = 0;
        }
        if(clearPrevIndex)mCurrentMusic = -1;
    }
    static const vec3df parseVec3df(const Vector3 &pos)
    {
        return vec3df(pos.x,pos.y,pos.z);
    }
    void setEffectsVolume(const Real &value)
    {
        mEffectsVolume = value;
        for(std::vector<ISound*>::iterator it=mSoundList.begin(); it!=mSoundList.end(); it++)
        {
            ISound *snd = *it;
            snd->setVolume(mEffectsVolume);
        }
    }
    void setMusicVolume(const Real &value)
    {
        mMusicVolume = value;
        if(mMusic)mMusic->setVolume(mMusicVolume);
    }
    void setGUIVolume(const Real &value)
    {
        mGUIVolume = value;
    }
    const Real getVolume(const unsigned char &index)
    {
        switch(index)
        {
            case 0: return mEffectsVolume;
            case 1: return mMusicVolume;
            case 2: return mGUIVolume;
            default: break;
        }
        return 0;
    }
    const Real getMusicVolume()
    {
        return mMusicVolume;
    }
    const Real getGUIVolume()
    {
        return mGUIVolume;
    }
    void playNextMusic()
    {
        if(mMusicList.empty())return;

        if(mMusicList.size()<=1)mCurrentMusic = 0;
        else
        {
            char index = -1;
            do
            {
                index = (char)Math::RangeRandom(0,(char)mMusicList.size());
                if(index >= (char)mMusicList.size())index = (char)mMusicList.size()-1;
            }
            while(index==mCurrentMusic);
            mCurrentMusic = index;
        }

        playMusic(mMusicList[mCurrentMusic],mMusicList.size()<=1);
    }
    const bool initMusic(const String &type=MUSIC_ALL)
    {
        if(mCurrentMusicType == type)return true;
        mCurrentMusicType = type;
        stopMusic();
        mMusicList.clear();

        SaveFile sf;
        if(!sf.load("music.cfg"))return false;

        bool found = false;
        for(sf.beginSectionIterator(); sf.hasMoreSections(); sf.nextSection())
        {
            const String section = sf.peekNextSectionName();
            if(type==section || type==MUSIC_ALL)
            {
                found = true;
                for(sf.beginSettingIterator(); sf.hasMoreSettings(); sf.nextSetting())
                {
                    const String value = sf.getSettingValue();
                    mMusicList.push_back(value);
                }
                if(type!=MUSIC_ALL)break;
            }
        }

        playNextMusic();
        return found;
    }
    void initSingleMusic(const String &filename)
    {
        mCurrentMusicType = "";
        stopMusic();
        mMusicList.clear();

        const String music = SaveFile::findDirectoryOf(filename);
        if(music=="")return;

        mMusicList.push_back(music);
        playNextMusic();
    }
};

template<> SoundManager* Singleton<SoundManager>::ms_Singleton = 0;

SoundManager* SoundManager::getSingletonPtr()
{
	return ms_Singleton;
}

SoundManager& SoundManager::getSingleton()
{
	assert(ms_Singleton);
	return *ms_Singleton;
}

#endif
