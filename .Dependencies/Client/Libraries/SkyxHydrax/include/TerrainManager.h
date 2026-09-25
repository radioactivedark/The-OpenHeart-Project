#ifndef _TERRAINMANAGER_H_
#define _TERRAINMANAGER_H_

#include <Terrain/OgreTerrain.h>
#include <Terrain/OgreTerrainGroup.h>

class TerrainManager
{
private:
    Ogre::TerrainGlobalOptions *m_pTerrainGlobals;
    Ogre::TerrainGroup *m_pTerrainGroup;
    Ogre::SceneManager *m_pSceneMgr;
    bool m_bTerrainsImported;

    void getTerrainImage(bool flipX, bool flipY, Ogre::Image& img)
    {
       img.load("terrain.png", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        if (flipX)
            img.flipAroundY();
        if (flipY)
            img.flipAroundX();
    }
    void defineTerrain(long x, long y)
    {
       Ogre::String szFilename = m_pTerrainGroup->generateFilename(x, y);
        if (Ogre::ResourceGroupManager::getSingleton().resourceExists(m_pTerrainGroup->getResourceGroup(), szFilename))
        {
            m_pTerrainGroup->defineTerrain(x, y);
        }
        else
        {
            Ogre::Image img;
            getTerrainImage(x % 2 != 0, y % 2 != 0, img);
            m_pTerrainGroup->defineTerrain(x, y, &img);
            m_bTerrainsImported = true;
        }
    }
    void initBlendMaps(Ogre::Terrain* pTerrain)
    {
        Ogre::TerrainLayerBlendMap* blendMap0 = pTerrain->getLayerBlendMap(1);
        Ogre::TerrainLayerBlendMap* blendMap1 = pTerrain->getLayerBlendMap(2);
        Ogre::Real minHeight0 = 70;
        Ogre::Real fadeDist0 = 40;
        Ogre::Real minHeight1 = 70;
        Ogre::Real fadeDist1 = 15;
        float* pBlend0 = blendMap0->getBlendPointer();
        float* pBlend1 = blendMap1->getBlendPointer();
        for (Ogre::uint16 y = 0; y < pTerrain->getLayerBlendMapSize(); ++y)
        {
            for (Ogre::uint16 x = 0; x < pTerrain->getLayerBlendMapSize(); ++x)
            {
                Ogre::Real tx, ty;

                blendMap0->convertImageToTerrainSpace(x, y, &tx, &ty);
                Ogre::Real height = pTerrain->getHeightAtTerrainPosition(tx, ty);
                Ogre::Real val = (height - minHeight0) / fadeDist0;
                val = Ogre::Math::Clamp(val, (Ogre::Real)0, (Ogre::Real)1);
                *pBlend0++ = val;

                val = (height - minHeight1) / fadeDist1;
                val = Ogre::Math::Clamp(val, (Ogre::Real)0, (Ogre::Real)1);
                *pBlend1++ = val;
            }
        }
        blendMap0->dirty();
        blendMap1->dirty();
        blendMap0->update();
        blendMap1->update();
    }
    void configureTerrainDefaults(Ogre::Light* pLight)
    {
       // Configure global
        m_pTerrainGlobals->setMaxPixelError(8); // decides how precise our terrain is going to be. A lower number will mean a more accurate terrain, at the cost of performance (because of more vertices).
        // testing composite map
        m_pTerrainGlobals->setCompositeMapDistance(3000); //decides how far the Ogre terrain will render the lightmapped terrain.

       // Important to set these so that the terrain knows what to use for derived (non-realtime) data
        m_pTerrainGlobals->setLightMapDirection(pLight->getDerivedDirection());
        m_pTerrainGlobals->setCompositeMapAmbient(m_pSceneMgr->getAmbientLight());
        m_pTerrainGlobals->setCompositeMapDiffuse(pLight->getDiffuseColour());

       // Configure default import settings for if we use imported image
        Ogre::Terrain::ImportData& defaultimp = m_pTerrainGroup->getDefaultImportSettings();
        defaultimp.terrainSize = 513;
        defaultimp.worldSize = 12000.0f;
        defaultimp.inputScale = 600; // due terrain.png is 8 bpp
        defaultimp.minBatchSize = 33;
        defaultimp.maxBatchSize = 65;

       // textures
        defaultimp.layerList.resize(3);
        defaultimp.layerList[0].worldSize = 100;
        defaultimp.layerList[0].textureNames.push_back("dirt_grayrocky_diffusespecular.dds");
        defaultimp.layerList[0].textureNames.push_back("dirt_grayrocky_normalheight.dds");
        defaultimp.layerList[1].worldSize = 30;
        defaultimp.layerList[1].textureNames.push_back("grass_green-01_diffusespecular.dds");
        defaultimp.layerList[1].textureNames.push_back("grass_green-01_normalheight.dds");
        defaultimp.layerList[2].worldSize = 200;
        defaultimp.layerList[2].textureNames.push_back("growth_weirdfungus-03_diffusespecular.dds");
        defaultimp.layerList[2].textureNames.push_back("growth_weirdfungus-03_normalheight.dds");
    }

public:
    TerrainManager()
    {
        m_pTerrainGlobals = 0;
        m_pTerrainGroup = 0;
    }
    ~TerrainManager()
    {
        clear();
    }
    void init(Ogre::SceneManager *pSceneMgr, Ogre::Light *pLight)
    {
        m_pSceneMgr = pSceneMgr;

        m_pTerrainGlobals = new Ogre::TerrainGlobalOptions();
        m_pTerrainGroup = new Ogre::TerrainGroup(m_pSceneMgr, Ogre::Terrain::ALIGN_X_Z, 513, 12000.0f);
        m_pTerrainGroup->setFilenameConvention(Ogre::String("SkyxHydraxTerrain"), Ogre::String("dat"));
        m_pTerrainGroup->setOrigin(Ogre::Vector3::ZERO);

        configureTerrainDefaults(pLight);

        // define our terrains and instruct the TerrainGroup to load them all:
        for (long x = 0; x <= 0; ++x)
            for (long y = 0; y <= 0; ++y)
                defineTerrain(x, y);

        // sync load since we want everything in place when we start
        m_pTerrainGroup->loadAllTerrains(true);

        // if we just imported our terrains, we would like our blendmaps to be calculated:
        if (m_bTerrainsImported)
        {
            Ogre::TerrainGroup::TerrainIterator ti = m_pTerrainGroup->getTerrainIterator();
          // loops through the available terrains and calls initBlendMaps on each.
            while(ti.hasMoreElements())
            {
                Ogre::Terrain* t = ti.getNext()->instance;
                initBlendMaps(t);
            }
        }
        // clean up after the initial terrain creation:
        m_pTerrainGroup->freeTemporaryResources();
    }
    void clear()
    {
        if(m_pTerrainGroup)
        {
            delete m_pTerrainGroup;
            m_pTerrainGroup = 0;
        }
        if(m_pTerrainGlobals)
        {
            delete m_pTerrainGlobals;
            m_pTerrainGlobals = 0;
        }
        m_pSceneMgr = 0;
    }
    void frameRenderingQueued()
    {
        if (m_pTerrainGroup->isDerivedDataUpdateInProgress())
        {
            if (m_bTerrainsImported)
            {
                //"Building terrain, please wait...");
            }
            else
            {
                //"Updating textures, patience...");
            }
        }
        else
        {
            if (m_bTerrainsImported)
            {
                m_pTerrainGroup->saveAllTerrains(true);
                m_bTerrainsImported = false;
            }
        }
    }
    const Ogre::Real getHeightAtPosition(const Ogre::Vector3 &cPos)
    {
        return m_pTerrainGroup->getHeightAtWorldPosition(cPos);
    }
};

#endif
