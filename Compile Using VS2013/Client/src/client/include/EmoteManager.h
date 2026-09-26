#ifndef _EMOTEMANAGER_H_
#define _EMOTEMANAGER_H_

#include <SaveFile.h>
#include <GuiList.h>

struct Emote
{
    String mName;
    std::vector<pair<String,Real> > mList;
    Real mEyelidL;
    Real mEyelidR;
    Emote(const String &name)
    {
        mName = name;
        mEyelidL = 1;
        mEyelidR = 1;
    }
    void add(const String &anim, const Real &ratio)
    {
        mList.push_back(pair<String,Real>(anim,ratio));
    }
};

class EmoteManager : public Singleton<EmoteManager>
{
private:
    std::vector<Emote*> mEmotes;
public:
    EmoteManager()
    {
        mEmotes.clear();
    }
    ~EmoteManager()
    {
        while(!mEmotes.empty())
        {
            Emote *emote = mEmotes.back();
            mEmotes.pop_back();
            delete emote;
        }
    }
    static EmoteManager* getSingletonPtr();
    static EmoteManager& getSingleton();
    void load(const String &filename)
    {
        SaveFile sf(filename);
        for(sf.beginSectionIterator(); sf.hasMoreSections(); sf.nextSection())
        {
            Emote *emote = new Emote(sf.peekNextSectionName());

            for(sf.beginSettingIterator(); sf.hasMoreSettings(); sf.nextSetting())
            {
                const String anim = sf.getSettingTag();
                if(anim=="Eyelids")
                {
                    const Real ratio = StringConverter::parseReal(sf.getSettingValue());
                    emote->mEyelidL = emote->mEyelidR = ratio;
                }
                else if(anim=="EyelidL")emote->mEyelidL = StringConverter::parseReal(sf.getSettingValue());
                else if(anim=="EyelidR")emote->mEyelidR = StringConverter::parseReal(sf.getSettingValue());
                else emote->add(anim,StringConverter::parseReal(sf.getSettingValue()));
            }

            mEmotes.push_back(emote);
        }
    }
    Emote* getEmote(const unsigned char &index)
    {
        if(index>=mEmotes.size())return 0;
        return mEmotes[index];
    }
    const unsigned char getEmoteIndex(const String &name)
    {
        unsigned char index = 0;
        for(std::vector<Emote*>::iterator i=mEmotes.begin(); i!=mEmotes.end(); i++)
        {
            Emote* emote = *i;
            if(emote->mName==name)return index;
            index++;
        }
        return 0;
    }
    void fillList(GuiList *list)
    {
        for(std::vector<Emote*>::iterator i=mEmotes.begin(); i!=mEmotes.end(); i++)
        {
            Emote* emote = *i;
            list->pushLine(emote->mName);
        }
    }
};

template<> EmoteManager* Singleton<EmoteManager>::ms_Singleton = 0;

EmoteManager* EmoteManager::getSingletonPtr()
{
	return ms_Singleton;
}

EmoteManager& EmoteManager::getSingleton()
{
	assert(ms_Singleton);
	return *ms_Singleton;
}

#endif
