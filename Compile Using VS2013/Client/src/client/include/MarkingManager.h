#ifndef _MARKINGMANAGER_H_
#define _MARKINGMANAGER_H_

#define NUM_EYEMATS 22

#include <SaveFile.h>

class MarkingManager : public Singleton<MarkingManager>
{
private:
    std::vector<String> mMarking[2][3];
public:
    MarkingManager()
    {
        for(int i=0;i<3;i++)mMarking[0][i].clear();
        for(int i=0;i<3;i++)mMarking[1][i].clear();
    }
    ~MarkingManager()
    {
    }
    static MarkingManager* getSingletonPtr();
    static MarkingManager& getSingleton();
    void load(const String &filename)
    {
        SaveFile sf(filename);
        for(sf.beginSectionIterator(); sf.hasMoreSections(); sf.nextSection())
        {
            const String section = sf.peekNextSectionName();
            String part = section;
            const char species = part.length()>0? part[0] : 'F';
            if(part.length()>1)part.erase(0,1);
            const unsigned char speciesIndex = species=='F'? 0 : 1;
            const unsigned char partIndex = (part=="body"? 0 : (part=="head"?1:2));

            for(sf.beginSettingIterator(); sf.hasMoreSettings(); sf.nextSetting())
            {
                const StringVector param = StringUtil::split(sf.getSettingValue(),";",1);
                if(param.size()<2)continue;
                mMarking[speciesIndex][partIndex].push_back(param[0]);
                switch(partIndex)
                {
                    //Body
                    case 0: createMarkingMaterial(section,(int)mMarking[speciesIndex][partIndex].size(),param[1],2); break;
                    //Head
                    case 1: createMarkingMaterial(section,(int)mMarking[speciesIndex][partIndex].size(),param[1],5); break;
                    //Tail
                    case 2: createMarkingMaterial(section,(int)mMarking[speciesIndex][partIndex].size(),param[1],1,2); break;
                    default : break;
                }
            }
        }
    }
    void initEyeMaterials()
    {
        for(int i=1;i<NUM_EYEMATS;i++)
        {
            createEyeMaterial("F",i);
            createEyeMaterial("C",i);
        }
    }
    void createMarkingMaterial(const String &speciesPart, const unsigned short &index, const String &texture, const unsigned char &textureUnitState, const char &fallbackTextureUnitState=-1)
    {
        const String markName = speciesPart+"Mark"+StringConverter::toString(index);
        MaterialPtr cloneMat = static_cast<MaterialPtr>(MaterialManager::getSingleton().getByName(speciesPart+"MarkBase"))->clone(markName);
        cloneMat->getTechnique(0)->getPass(0)->getTextureUnitState(textureUnitState)->setTextureName(texture);

        Technique *fallbackTech = cloneMat->getTechnique(1);
        Technique::PassIterator pit = fallbackTech->getPassIterator();
        unsigned short i = 0;
        while(pit.hasMoreElements())
        {
            Pass *pass = pit.getNext();
            Pass::TextureUnitStateIterator tit = pass->getTextureUnitStateIterator();
            while(tit.hasMoreElements())
            {
                TextureUnitState *tex = tit.getNext();
                if(i == (fallbackTextureUnitState>=0 ? fallbackTextureUnitState : textureUnitState))
                {
                    tex->setTextureName(texture);
                    return;
                }
                i++;
            }
        }
    }
    const String getMarking(const char &species, const unsigned char &part, const unsigned char &index)
    {
        const unsigned char speciesIndex = species=='f'? 0 : 1;
        if(index>mMarking[speciesIndex][part].size())return "";
        return mMarking[speciesIndex][part][index];
    }
    std::vector<String>* getMarkingList(const char &species, const unsigned char &part)
    {
        const unsigned char speciesIndex = species=='f'? 0 : 1;
        return &mMarking[speciesIndex][part];
    }
    void createEyeMaterial(const String &speciesPart, const unsigned short &index)
    {
        const String matNameL = speciesPart+"eyeMatL"+StringConverter::toString(index);
        const String matNameR = speciesPart+"eyeMatR"+StringConverter::toString(index);
        MaterialPtr cloneMat = static_cast<MaterialPtr>(MaterialManager::getSingleton().getByName(speciesPart+"eyeMatL"))->clone(matNameL);
        cloneMat->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName("feyeMask1_"+StringConverter::toString(index)+".png");
        cloneMat->getTechnique(1)->getPass(0)->getTextureUnitState(0)->setTextureName("feyeMask1_"+StringConverter::toString(index)+".png");
        cloneMat = static_cast<MaterialPtr>(MaterialManager::getSingleton().getByName(speciesPart+"eyeMatR"))->clone(matNameR);
        cloneMat->getTechnique(0)->getPass(0)->getTextureUnitState(0)->setTextureName("feyeMask1_"+StringConverter::toString(index)+".png");
        cloneMat->getTechnique(1)->getPass(0)->getTextureUnitState(0)->setTextureName("feyeMask1_"+StringConverter::toString(index)+".png");
    }
};

template<> MarkingManager* Singleton<MarkingManager>::ms_Singleton = 0;

MarkingManager* MarkingManager::getSingletonPtr()
{
	return ms_Singleton;
}

MarkingManager& MarkingManager::getSingleton()
{
	assert(ms_Singleton);
	return *ms_Singleton;
}

#endif
