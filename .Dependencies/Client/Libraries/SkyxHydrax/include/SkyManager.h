#ifndef _SKYMANAGER_H_
#define _SKYMANAGER_H_

#include <SkyX.h>

class SkyManager
{
private:
    SkyX::SkyX *m_pSkyX;
    SkyX::BasicController* m_pBasicController;
    Ogre::Camera *m_pCamera;

public:
    SkyManager()
    {
        m_pBasicController = 0;
		m_pSkyX = 0;
    }
    ~SkyManager()
    {
        clear();
    }
    void init(Ogre::SceneManager *pSceneMgr, Ogre::Camera *pCamera)
    {
        clear();

        m_pBasicController = new SkyX::BasicController();
		m_pSkyX = new SkyX::SkyX(pSceneMgr, m_pBasicController);
		m_pCamera = pCamera;

		SkyX::AtmosphereManager::Options cOptions = getOptions();
		cOptions.RayleighMultiplier = 0.003075f;
		cOptions.MieMultiplier = 0.00125f;
		cOptions.InnerRadius = 9.92f;
		cOptions.OuterRadius = 10.3311f;
		setOptions(cOptions);

		m_pSkyX->create();

		m_pSkyX->getCloudsManager()->add(SkyX::CloudLayer::Options());

		m_pBasicController->setMoonPhase(0.75f);

		/*
		mSkyX->getGPUManager()->addGroundPass(
			static_cast<Ogre::MaterialPtr>(Ogre::MaterialManager::getSingleton().
			getByName("Terrain"))->getTechnique(0)->createPass(), 5000, Ogre::SBT_TRANSPARENT_COLOUR);
			*/
    }
    void clear()
    {
        if(m_pSkyX)
        {
            delete m_pSkyX;
            m_pSkyX = 0;
        }
        if(m_pBasicController)
        {
            delete m_pBasicController;
            m_pBasicController = 0;
        }
        m_pCamera = 0;
    }
    void update(const float &fTimeElapsed)
    {
        m_pSkyX->update(fTimeElapsed);
		m_pSkyX->notifyCameraRender(m_pCamera);

		/*
		static_cast<Ogre::MaterialPtr>(Ogre::MaterialManager::getSingleton().getByName("Terrain"))->getTechnique(0)->getPass(0)
			->getFragmentProgramParameters()->setNamedConstant("uLightY", m_pBasicController->getSunDirection().y);
		*/
    }
    SkyX::AtmosphereManager::Options getOptions()
    {
        return m_pSkyX->getAtmosphereManager()->getOptions();
    }
    void setOptions(const SkyX::AtmosphereManager::Options &cOptions)
    {
        m_pSkyX->getAtmosphereManager()->setOptions(cOptions);
    }
    void showSky(const bool &bFlag)
    {
        m_pSkyX->getMeshManager()->getEntity()->setVisible(bFlag);
        m_pSkyX->getMoonManager()->getMoonBillboard()->setVisible(bFlag);
    }
    void showStars(const bool &bFlag)
    {
        m_pSkyX->setStarfieldEnabled(bFlag);
    }
    SkyX::SkyX* getSky()
    {
        return m_pSkyX;
    }
    SkyX::BasicController* getBasicController()
    {
        return m_pBasicController;
    }
    Ogre::Vector3 getTime()
    {
        return m_pBasicController->getTime();
    }
};

#endif
