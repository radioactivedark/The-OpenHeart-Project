#ifndef _ITEMSMANAGER_H_
#define _ITEMSMANAGER_H_

struct Item
{
    String m_szName;
    String m_szMesh;
    String m_szBone;
    String m_szMaterial;
    std::map<char, Vector3> m_cOffset;
    std::map<char, Vector3> m_cRotation;
    Item(const String &szName, const String &szMesh = "", const String &szBone = "", const String &szMaterial = "")
        : m_szName(szName), m_szMesh(szMesh), m_szBone(szBone), m_szMaterial(szMaterial)
    {
    }
    const Vector3 getOffset(const char &nSpecies)
    {
        std::map<char, Vector3>::iterator it = m_cOffset.find(nSpecies);
        if(it==m_cOffset.end())return Vector3::ZERO;
        return it->second;
    }
    const Vector3 getRotation(const char &nSpecies)
    {
        std::map<char, Vector3>::iterator it = m_cRotation.find(nSpecies);
        if(it==m_cRotation.end())return Vector3::ZERO;
        return it->second;
    }
};

class ItemsManager : public Singleton<ItemsManager>
{
private:
    std::vector<Item*> m_vpItems;
public:
    ItemsManager()
    {
        m_vpItems.clear();
    }
    ~ItemsManager()
    {
        while(!m_vpItems.empty())
        {
            Item *pItem = m_vpItems.back();
            m_vpItems.pop_back();
            delete pItem;
        }
    }
    static ItemsManager* getSingletonPtr();
    static ItemsManager& getSingleton();
    void load(const String &filename)
    {
        SaveFile sf(filename);
        for(sf.beginSectionIterator(); sf.hasMoreSections(); sf.nextSection())
        {
            Item *pItem = new Item(sf.peekNextSectionName());

            for(sf.beginSettingIterator(); sf.hasMoreSettings(); sf.nextSetting())
            {
                const String szTag = sf.getSettingTag();
                if(szTag=="mesh")
                {
                    pItem->m_szMesh = sf.getSettingValue();
                }
                else if(szTag=="bone")
                {
                    pItem->m_szBone = sf.getSettingValue();
                }
                else if(szTag=="material")
                {
                    pItem->m_szMaterial = sf.getSettingValue();
                }
                else if(szTag=="offset_f")
                {
                    pItem->m_cOffset['f'] = StringConverter::parseVector3(sf.getSettingValue());
                }
                else if(szTag=="rotation_f")
                {
                    pItem->m_cRotation['f'] = StringConverter::parseVector3(sf.getSettingValue());
                }
                else if(szTag=="offset_c")
                {
                    pItem->m_cOffset['c'] = StringConverter::parseVector3(sf.getSettingValue());
                }
                else if(szTag=="rotation_c")
                {
                    pItem->m_cRotation['c'] = StringConverter::parseVector3(sf.getSettingValue());
                }
            }

            m_vpItems.push_back(pItem);
        }
    }
    Item* getItem(const unsigned short &nIndex)
    {
        if(nIndex<=0 || nIndex>m_vpItems.size())return 0;
        return m_vpItems[nIndex-1];
    }
    void fillList(GuiList *pList)
    {
        for(std::vector<Item*>::iterator i=m_vpItems.begin(); i!=m_vpItems.end(); i++)
        {
            Item* pItem = *i;
            pList->pushLine(pItem->m_szName);
        }
    }
};

template<> ItemsManager* Singleton<ItemsManager>::ms_Singleton = 0;

ItemsManager* ItemsManager::getSingletonPtr()
{
	return ms_Singleton;
}

ItemsManager& ItemsManager::getSingleton()
{
	assert(ms_Singleton);
	return *ms_Singleton;
}

#endif
