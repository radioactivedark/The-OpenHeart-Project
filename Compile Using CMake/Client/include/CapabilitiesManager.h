#ifndef _CAPABILITIESMANAGER_H_
#define _CAPABILITIESMANAGER_H_

#include <LoggerManager.h>

class CapabilitiesManager : public Singleton<CapabilitiesManager>
{
private:
    bool mHasVertexProgram;
    bool mHasFragmentProgram;
public:
    CapabilitiesManager(Root *root)
    {
        const RenderSystemCapabilities *capabilities = root->getRenderSystem()->getCapabilities();
        mHasVertexProgram = capabilities->hasCapability(Ogre::RSC_VERTEX_PROGRAM);
        mHasFragmentProgram = capabilities->hasCapability(Ogre::RSC_FRAGMENT_PROGRAM);

        LoggerManager::getSingleton().logMessage("Capabilities:: VertexProgram: " + String(mHasVertexProgram?"YES":"NO"));
        LoggerManager::getSingleton().logMessage("Capabilities:: FragmentProgram: " + String(mHasFragmentProgram?"YES":"NO"));
    }
    ~CapabilitiesManager()
    {
    }
    static CapabilitiesManager* getSingletonPtr();
    static CapabilitiesManager& getSingleton();
    const bool hasShaders()
    {
        return (mHasVertexProgram && mHasFragmentProgram);
    }
    const String assertCustomColour(Renderable *renderable, const unsigned short &part, const bool &clone=false, const String &cloneID="", std::vector<MaterialPtr> *cloneMatList=0)
    {
        //if(hasShaders())return "";

        //Fallback method
        MaterialPtr mat = renderable->getMaterial();
        if(mat.isNull())return "";

        Technique *tech = mat->getBestTechnique();
        if(!tech || tech->getName()!="2")return "";

        String cloneMatName = "";

        if(clone && cloneMatList)
        {
            bool alreadyCloned = false;
            for(std::vector<MaterialPtr>::iterator it=cloneMatList->begin(); it!=cloneMatList->end(); it++)
            {
                MaterialPtr matPtr = *it;
                if(matPtr==mat)
                {
                    cloneMatName = mat->getName();
                    alreadyCloned = true;
                    break;
                }
            }
            if(!alreadyCloned)
            {
                cloneMatName = "DY_"+cloneID+"_"+mat->getName();
                MaterialPtr cloneMat = MaterialManager::getSingleton().getByName(cloneMatName);
                if(cloneMat.isNull())
                {
                    cloneMat = mat->clone(cloneMatName);
                    if(!cloneMat.isNull())
                    {
                        mat = cloneMat;
                        cloneMatList->push_back(cloneMat);
                    }
                }
                else
                {
                    mat = cloneMat;
                }
            }
        }

        tech = mat->getBestTechnique();
        if(!tech || tech->getName()!="2")return "";

        unsigned short i = 0;

        Technique::PassIterator pit = tech->getPassIterator();
        while(pit.hasMoreElements())
        {
            Pass *pass = pit.getNext();
            Pass::TextureUnitStateIterator tit = pass->getTextureUnitStateIterator();
            while(tit.hasMoreElements())
            {
                TextureUnitState *tex = tit.getNext();
                i++;
                if(i==part)
                {
                    LayerBlendModeEx mode = tex->getColourBlendMode();
                    const Vector4 colourVect = renderable->getCustomParameter(part);
                    const ColourValue colour(colourVect.x,colourVect.y,colourVect.z);
                    tex->setColourOperationEx(mode.operation, mode.source1, mode.source2, colour, colour);
                    LayerBlendModeEx alphaMode = tex->getAlphaBlendMode();
                    if(alphaMode.operation==Ogre::LBX_ADD)tex->setAlphaOperation(alphaMode.operation, alphaMode.source1, alphaMode.source2, colourVect.w, colourVect.w);
                    return cloneMatName;
                }
            }
        }

        return cloneMatName;
    }
};

template<> CapabilitiesManager* Singleton<CapabilitiesManager>::ms_Singleton = 0;

CapabilitiesManager* CapabilitiesManager::getSingletonPtr()
{
	return ms_Singleton;
}

CapabilitiesManager& CapabilitiesManager::getSingleton()
{
	assert(ms_Singleton);
	return *ms_Singleton;
}

#endif
