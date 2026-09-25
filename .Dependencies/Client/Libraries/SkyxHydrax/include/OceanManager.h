#ifndef _OCEANMANAGER_H_
#define _OCEANMANAGER_H_

#include <SkyX.h>
#include "Hydrax/Hydrax.h"
#include "Hydrax/Noise/Perlin/Perlin.h"
#include "Hydrax/Modules/ProjectedGrid/ProjectedGrid.h"

class OceanManager// : public Hydrax::RttManager::RttListener
{
private:
    Hydrax::Hydrax *m_pHydrax;
	SkyManager *m_pSkyMgr;
	Ogre::Camera *m_pCamera;
	Ogre::Light *m_pLight;

	SkyX::ColorGradient m_cWaterGradient;
	SkyX::ColorGradient	m_cSunGradient;
	SkyX::ColorGradient	m_cAmbientGradient;

public:
    OceanManager()
    {
        m_pHydrax = 0;

        // Water
		m_cWaterGradient = SkyX::ColorGradient();
		m_cWaterGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(0.058209,0.535822,0.779105)*0.4, 1));
		m_cWaterGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(0.058209,0.535822,0.729105)*0.3, 0.8));
		m_cWaterGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(0.058209,0.535822,0.679105)*0.25, 0.6));
		m_cWaterGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(0.058209,0.535822,0.679105)*0.2, 0.5));
		m_cWaterGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(0.058209,0.535822,0.679105)*0.1, 0.45));
		m_cWaterGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(0.058209,0.535822,0.679105)*0.025, 0));
		// Sun
		m_cSunGradient = SkyX::ColorGradient();
		m_cSunGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(0.8,0.75,0.55)*1.5, 1.0f));
		m_cSunGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(0.8,0.75,0.55)*1.4, 0.75f));
		m_cSunGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(0.8,0.75,0.55)*1.3, 0.5625f));
		m_cSunGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(0.6,0.5,0.2)*1.5, 0.5f));
		m_cSunGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(0.5,0.5,0.5)*0.25, 0.45f));
		m_cSunGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(0.5,0.5,0.5)*0.01, 0.0f));
		// Ambient
		m_cAmbientGradient = SkyX::ColorGradient();
		m_cAmbientGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(1,1,1)*1, 1.0f));
		m_cAmbientGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(1,1,1)*1, 0.6f));
		m_cAmbientGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(1,1,1)*0.6, 0.5f));
		m_cAmbientGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(1,1,1)*0.3, 0.45f));
		m_cAmbientGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(1,1,1)*0.1, 0.35f));
		m_cAmbientGradient.addCFrame(SkyX::ColorGradient::ColorFrame(Ogre::Vector3(1,1,1)*0.05, 0.0f));
    }
    ~OceanManager()
    {
        clear();
    }
    void init(Ogre::SceneManager *pSceneMgr, Ogre::Camera *pCamera, Ogre::Viewport *pViewport)
    {
        clear();

        m_pHydrax = new Hydrax::Hydrax(pSceneMgr, pCamera, pViewport);
        m_pCamera = pCamera;

		Hydrax::Module::ProjectedGrid *pModule = new Hydrax::Module::ProjectedGrid(
                                                m_pHydrax,
                                                new Hydrax::Noise::Perlin(/*Generic one*/),
                                                Ogre::Plane(Ogre::Vector3(0,1,0), Ogre::Vector3(0,0,0)),
                                                Hydrax::MaterialManager::NM_VERTEX,
                                                Hydrax::Module::ProjectedGrid::Options(/*Generic one*/));

		m_pHydrax->setModule(static_cast<Hydrax::Module::Module*>(pModule));

		m_pHydrax->loadCfg("HydraxDemo.hdx");
        m_pHydrax->create();

        //m_pHydrax->getRttManager()->addRttListener(this);

       /*
		m_pHydrax->getMaterialManager()->addDepthTechnique(
			static_cast<Ogre::MaterialPtr>(Ogre::MaterialManager::getSingleton().getByName("Island"))
			->createTechnique());
        */
    }
    void clear()
    {
        if(m_pHydrax)
        {
            delete m_pHydrax;
            m_pHydrax = 0;
        }
        m_pSkyMgr = 0;
        m_pCamera = 0;
        m_pLight = 0;
    }
    void setSkyManager(SkyManager *pSkyMgr)
    {
        m_pSkyMgr = pSkyMgr;
    }
    void setLight(Ogre::Light *pLight)
    {
        m_pLight = pLight;
    }
    void update(const float &fTimeElapsed)
    {
        updateEnvironmentLighting();
        m_pHydrax->update(fTimeElapsed);
    }
    void updateEnvironmentLighting()
	{
	    if(!m_pSkyMgr)return;

	    const Ogre::Vector3 lightDir = m_pSkyMgr->getBasicController()->getSunDirection();

		const float point = (-lightDir.y + 1.0f) / 2.0f;
		m_pHydrax->setWaterColor(m_cWaterGradient.getColor(point));

		SkyX::AtmosphereManager::Options SkyXOptions = m_pSkyMgr->getOptions();

		if(!m_pCamera)return;

		Ogre::Vector3 sunPos = m_pCamera->getDerivedPosition() - lightDir*SkyXOptions.InnerRadius;
		m_pHydrax->setSunPosition(sunPos);

		if(m_pLight)
        {
            m_pLight->setPosition(m_pCamera->getDerivedPosition() - lightDir*SkyXOptions.InnerRadius);
            const Ogre::Vector3 sunCol = m_cSunGradient.getColor(point);
            m_pLight->setSpecularColour(sunCol.x, sunCol.y, sunCol.z);
            const Ogre::Vector3 ambientCol = m_cAmbientGradient.getColor(point);
            m_pLight->setDiffuseColour(ambientCol.x, ambientCol.y, ambientCol.z);
            m_pHydrax->setSunColor(sunCol);
        }
	}
	/*
	void preRenderTargetUpdate(const Hydrax::RttManager::RttType& Rtt)
	{
		const bool bIsUnderwater = m_pHydrax->_isCurrentFrameUnderwater();

		switch (Rtt)
		{
			case Hydrax::RttManager::RTT_REFLECTION:
			{
				// No stars in the reflection map
				if(m_pSkyMgr)m_pSkyMgr->showStars(false);
			}
			break;

			case Hydrax::RttManager::RTT_REFRACTION:
			{
			}
			break;

			case Hydrax::RttManager::RTT_DEPTH: case Hydrax::RttManager::RTT_DEPTH_REFLECTION:
			{
				// Hide SkyX components in depth maps
				if(m_pSkyMgr)m_pSkyMgr->showSky(false);
			}
			break;
		}
	}
	void postRenderTargetUpdate(const Hydrax::RttManager::RttType& Rtt)
	{
		const bool bIsUnderwater = m_pHydrax->_isCurrentFrameUnderwater();

		switch (Rtt)
		{
			case Hydrax::RttManager::RTT_REFLECTION:
			{
				if(m_pSkyMgr)m_pSkyMgr->showStars(true);
			}
			break;

			case Hydrax::RttManager::RTT_REFRACTION:
			{
			}
			break;

			case Hydrax::RttManager::RTT_DEPTH: case Hydrax::RttManager::RTT_DEPTH_REFLECTION:
			{
				if(m_pSkyMgr)m_pSkyMgr->showSky(true);
			}
			break;
		}
	}
	*/
};

#endif
