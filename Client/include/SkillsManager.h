#ifndef _SKILLSMANAGER_H_
#define _SKILLSMANAGER_H_

#define DASH_FORCE 5000.0f

#include <UnitManager.h>
#include <NetworkManager.h>
#include <MapManager.h>

class SkillUpdate
{
private:
    Unit *m_pUser;
    unsigned char m_nType;
public:
    Real m_fTimeLeft;
    SkillUpdate(Unit *pUser, const unsigned char &nType, const Real &fTimeLeft)
        : m_pUser(pUser), m_nType(nType), m_fTimeLeft(fTimeLeft)
    {
    }
    ~SkillUpdate()
    {
    }
    Unit* getUser()
    {
        return m_pUser;
    }
    const unsigned char getType()
    {
        return m_nType;
    }
};

class SkillsListener
{
public:
    virtual void disableControl(const bool &bFlag)=0;
};

class SkillsManager : public Singleton<SkillsManager>, public NetworkListener, public ObjectListener
{
private:
    UnitManager *m_pUnitMgr;
    NetworkManager *m_pNetMgr;
    ChatManager *m_pChatMgr;
    Unit *m_pPlayer;
    std::vector<SkillUpdate*> m_vpSkillUpdates;

    std::vector<SkillsListener*> m_vpListeners;
    bool m_bNoFlying;
public:
    SkillsManager()
    {
        clear();
    }
    ~SkillsManager()
    {
        clear();
    }
    enum
    {
        SKILL_NONE,
        SKILL_DASH,
        SKILL_FLY,
        SKILL_WEAPON
    };
    static SkillsManager* getSingletonPtr();
    static SkillsManager& getSingleton();
    void init(Unit *pPlayer)
    {
        m_pUnitMgr = UnitManager::getSingletonPtr();
        m_pNetMgr = NetworkManager::getSingletonPtr();
        m_pChatMgr = ChatManager::getSingletonPtr();

        m_pPlayer = pPlayer;

        m_bNoFlying = MapManager::getSingletonPtr()->getNoFlying();
    }
    void clear()
    {
        m_vpListeners.clear();
        m_pPlayer = 0;
        while(!m_vpSkillUpdates.empty())
        {
            SkillUpdate *pSkill = m_vpSkillUpdates.back();
            m_vpSkillUpdates.pop_back();
            deleteSkillUpdate(pSkill);
        }
        m_bNoFlying = false;
    }
    void update(const Real &fTimeElapsed)
    {
        std::vector<SkillUpdate*>::iterator it=m_vpSkillUpdates.begin();
        while(it!=m_vpSkillUpdates.end())
        {
            SkillUpdate *pSkill = *it;

            //Update skill
            if(updateSkill(pSkill, fTimeElapsed))
            {
                //Skill finished, delete
                it = m_vpSkillUpdates.erase(it);
                deleteSkillUpdate(pSkill);
                continue;
            }

            it++;
        }
    }
    void deleteSkillUpdate(SkillUpdate *pSkill)
    {
        delete pSkill;
    }
    void deleteSkillUpdate(Unit *pUser, const unsigned char &nType)
    {
        std::vector<SkillUpdate*>::iterator it=m_vpSkillUpdates.begin();
        while(it!=m_vpSkillUpdates.end())
        {
            SkillUpdate *pSkill = *it;

            if(pSkill->getUser()==pUser && (pSkill->getType()==nType || 0==nType))
            {
                it = m_vpSkillUpdates.erase(it);
                deleteSkillUpdate(pSkill);
                continue;
            }

            it++;
        }
    }
    void setPlayer(Unit *pPlayer)
    {
        m_pPlayer = pPlayer;
    }
    void addListener(SkillsListener *pListener)
    {
        m_vpListeners.push_back(pListener);
    }
    void keyPressed(const char &index)
    {
        using namespace KeyboardIndex;
        switch(index)
        {
            case KB_EQUIP:
                if(m_pPlayer)useEquip(m_pPlayer);
                break;
        }
    }
    void useEquip(Unit *pUnit)
    {
        const unsigned char nEquip = pUnit->getEquip();

        //No equip
        if(!nEquip)
        {
            prepareSkill(pUnit, SKILL_DASH);
            m_pNetMgr->sendCharSkill(SKILL_DASH);
        }
        else switch(nEquip/10)
        {
            //Wings
            case 0:
                {
                    const bool bFlag = !pUnit->mAntiGravity;
                    prepareSkill(pUnit, SKILL_FLY,bFlag);
                    m_pNetMgr->sendCharSkill(SKILL_FLY,bFlag,true);
                }
                break;
            //Weapon
            case 1:
                prepareSkill(pUnit, SKILL_WEAPON);
                break;
            default: break;
        }
    }
    void prepareSkill(Unit *pUser, const unsigned char &nType, const bool &bFlag=true)
    {
        Real fTimeLeft = 0.0f;

        switch(nType)
        {
            case SKILL_DASH:
                fTimeLeft = 0.3f;
                pUser->mApplyMaxVelocity = false;
                if(pUser==m_pPlayer)disableControl(true);
                pUser->setAction(Unit::ACT_DASH);
                break;
            case SKILL_FLY:
                if(m_bNoFlying)
                {
                    if(pUser==m_pPlayer)m_pChatMgr->systemMessage("You cannot fly here.");
                    return;
                }
                pUser->mAntiGravity = bFlag;
                if(bFlag)fTimeLeft = 2.0f;
                else deleteSkillUpdate(pUser,nType);
                break;
            case SKILL_WEAPON: fTimeLeft = 2.0f; break;
            default: break;
        }

        if(fTimeLeft > 0.0f)
        {
            SkillUpdate *pSkill = new SkillUpdate(pUser,nType,fTimeLeft);

            m_vpSkillUpdates.push_back(pSkill);
        }
    }
    const bool updateSkill(SkillUpdate *pSkill, const Real &fTimeElapsed)
    {
        Unit *pUser = pSkill->getUser();
        bool bFinished = false;
        const bool bPositiveTimeLeft = (pSkill->m_fTimeLeft >= 0.0f);
        if(bPositiveTimeLeft)
        {
            pSkill->m_fTimeLeft -= fTimeElapsed;
            bFinished = (pSkill->m_fTimeLeft<=0.0f);
            if(bFinished)pSkill->m_fTimeLeft = 0.0f;
        }
        else
        {
            pSkill->m_fTimeLeft += fTimeElapsed;
            bFinished = (pSkill->m_fTimeLeft>=0.0f);
            if(bFinished)pSkill->m_fTimeLeft = 0.0f;
        }

        switch(pSkill->getType())
        {
            case SKILL_DASH:
                pUser->addVelocity(pUser->getOrientation(true)*Vector3::UNIT_Z*DASH_FORCE*fTimeElapsed);
                if(bFinished)
                {
                    pUser->mApplyMaxVelocity = true;
                    if(pUser==m_pPlayer)disableControl(false);
                    pUser->setAction(Unit::ACT_IDLE);
                }
                break;
            case SKILL_FLY:
                pUser->setPosition(pUser->getPosition() + Vector3(0,10.0f*(pSkill->m_fTimeLeft-(pSkill->m_fTimeLeft>=0.0f?1.0f:-1.0f))*fTimeElapsed,0));
                if(bFinished)
                {
                    bFinished = false;
                    pSkill->m_fTimeLeft = 2.0f * (bPositiveTimeLeft?-1:1);
                }
                break;

            default: break;
        }

        return bFinished;
    }
    void disableControl(const bool &bFlag)
    {
        for(std::vector<SkillsListener*>::iterator it=m_vpListeners.begin(); it!=m_vpListeners.end(); it++)
        {
            SkillsListener *pListener = *it;
            pListener->disableControl(bFlag);
        }
    }
    void charSkillEvent(const unsigned int &nCharID, const unsigned char &nSkill, const bool &bFlag)
    {
        Unit *pUnit = m_pUnitMgr->getUnitByCharID(nCharID);
        if(!pUnit)return;
        prepareSkill(pUnit,nSkill,bFlag);
    }
    const bool objectEvent(ObjectListenerObject *object, const unsigned char &event)
    {
        if(!object)return false;

        if(event==ObjectListener::OBJECT_DESTROYED/* && object->getType()==ObjectListenerObject::OBJECTTYPE_UNIT*/)
        {
            deleteSkillUpdate(static_cast<Unit*>(object),0);
        }

        return true;
    }
    const bool controllableEvent(ObjectListenerObject *object, const unsigned char &event)
    {
        return false;
    }
    const bool unitEvent(ObjectListenerObject *object, const unsigned char &event)
    {
        return false;
    }
};

template<> SkillsManager* Singleton<SkillsManager>::ms_Singleton = 0;

SkillsManager* SkillsManager::getSingletonPtr()
{
	return ms_Singleton;
}

SkillsManager& SkillsManager::getSingleton()
{
	assert(ms_Singleton);
	return *ms_Singleton;
}

#endif
