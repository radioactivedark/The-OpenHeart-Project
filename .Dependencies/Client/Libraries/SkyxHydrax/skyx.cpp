// ----------------------------------------------------------------------------
// Include the main OGRE header files
// Ogre.h just expands to including lots of individual OGRE header files
// ----------------------------------------------------------------------------
#include <OGRE/Ogre.h>
#include <OGRE/OgreTextAreaOverlayElement.h>


// ---------------------------------------------------------------------------
// Include SkyX header files
// ---------------------------------------------------------------------------
#include <SkyManager.h>
#include <OceanManager.h>
#include <TerrainManager.h>

// ----------------------------------------------------------------------------
// Include the OGRE example framework
// This includes the classes defined to make getting an OGRE application running
// a lot easier. It automatically sets up all the main objects and allows you to
// just override the bits you want to instead of writing it all from scratch.
// ----------------------------------------------------------------------------
#include <OGRE/ExampleApplication.h>

// ----------------------------------------------------------------------------
// SkyX-related ptrs
// ----------------------------------------------------------------------------
SkyManager *m_pSkyManager = 0;
OceanManager *m_pOceanManager = 0;
TerrainManager *m_pTerrainManager = 0;

// ----------------------------------------------------------------------------
// Misc
// ----------------------------------------------------------------------------
Ogre::TextAreaOverlayElement* mTextArea = 0;
bool mShowInformation = false;

Ogre::String getConfigStringFromSkyXAtmosphereOptions(const SkyX::AtmosphereManager::Options& Options)
{
	Ogre::Vector3 time = m_pSkyManager->getTime();

	int hour = static_cast<int>(time.x);
	int min  = static_cast<int>((time.x - hour)*60);

	Ogre::String timeStr = Ogre::StringConverter::toString(hour) + ":" + Ogre::StringConverter::toString(min);
	Ogre::String str = "SkyX Plugin demo (Press F1 to show/hide information)" + static_cast<Ogre::String>(mShowInformation ? " - Simuation paused - \n" : "\n-------------------------------------------------------------\nTime: " + timeStr + "\n");

	if (mShowInformation)
	{
		str += "-------------------------------------------------------------\n";
		str += "Time: " + timeStr + " [1, Shift+1] (+/-).\n";
		str += "Rayleigh multiplier: " + Ogre::StringConverter::toString(Options.RayleighMultiplier) + " [2, Shift+2] (+/-).\n";
		str += "Mie multiplier: " + Ogre::StringConverter::toString(Options.MieMultiplier) + " [3, Shift+3] (+/-).\n";
		str += "Exposure: " + Ogre::StringConverter::toString(Options.Exposure) + " [4, Shift+4] (+/-).\n";
		str += "Inner radius: " + Ogre::StringConverter::toString(Options.InnerRadius) + " [5, Shift+5] (+/-).\n";
		str += "Outer radius: " + Ogre::StringConverter::toString(Options.OuterRadius) + " [6, Shift+6] (+/-).\n";
		str += "Number of samples: " + Ogre::StringConverter::toString(Options.NumberOfSamples) + " [7, Shift+7] (+/-).\n";
		str += "Height position: " + Ogre::StringConverter::toString(Options.HeightPosition) + " [8, Shift+8] (+/-).\n";
		str += "Moon phase: " + Ogre::StringConverter::toString(m_pSkyManager->getBasicController()->getMoonPhase()) + " [9, Shift+9] (+/-).\n";
	}
	return str;
}

// ----------------------------------------------------------------------------
// Define the application object
// This is derived from ExampleApplication which is the class OGRE provides to
// make it easier to set up OGRE without rewriting the same code all the time.
// You can override extra methods of ExampleApplication if you want to further
// specialise the setup routine, otherwise the only mandatory override is the
// 'createScene' method which is where you set up your own personal scene.
// ----------------------------------------------------------------------------
class SkyXDemoFrameListener : public ExampleFrameListener
{
public:
    SceneManager *mSceneMgr;
    Real mKeyBuffer;
    Real mLastPositionLength;

    SkyXDemoFrameListener(RenderWindow* win, Camera* cam, SceneManager *sm)
            : ExampleFrameListener(win,cam)
            , mSceneMgr(sm)
            , mKeyBuffer(-1)
    {
    }

    bool frameStarted(const FrameEvent &e)
    {
        mKeyboard->capture();

        /*
		// Check camera height
		Ogre::RaySceneQuery * raySceneQuery =
			mSceneMgr->
			     createRayQuery(Ogre::Ray(mCamera->getPosition() + Ogre::Vector3(0,1000000,0),
				                Vector3::NEGATIVE_UNIT_Y));
		Ogre::RaySceneQueryResult& qryResult = raySceneQuery->execute();
        Ogre::RaySceneQueryResult::iterator i = qryResult.begin();
        if (i != qryResult.end() && i->worldFragment)
        {
			if (mCamera->getDerivedPosition().y < i->worldFragment->singleIntersection.y + 30)
			{
                mCamera->
				     setPosition(mCamera->getPosition().x,
                                 i->worldFragment->singleIntersection.y + 30,
                                 mCamera->getPosition().z);
			}
        }
        */

        Ogre::Vector3 camPos = mCamera->getPosition();
        const Ogre::Real fHeight = m_pTerrainManager->getHeightAtPosition(camPos);
        if (camPos.y < fHeight + 10.0f)
        {
            mCamera->setPosition(camPos.x, fHeight + 10.0f, camPos.z);
        }

        m_pTerrainManager->frameRenderingQueued();


		SkyX::AtmosphereManager::Options SkyXOptions = m_pSkyManager->getOptions();

		// Time
		if (!mShowInformation)
		{
			m_pSkyManager->getSky()->setTimeMultiplier(1);
		}
		else
		{
			m_pSkyManager->getSky()->setTimeMultiplier(0.0f);
		}

		if (mKeyboard->isKeyDown(OIS::KC_1) && !(mKeyboard->isKeyDown(OIS::KC_LSHIFT) || mKeyboard->isKeyDown(OIS::KC_RSHIFT)))
			m_pSkyManager->getSky()->setTimeMultiplier(1.0f);
		if (mKeyboard->isKeyDown(OIS::KC_1) &&  (mKeyboard->isKeyDown(OIS::KC_LSHIFT) || mKeyboard->isKeyDown(OIS::KC_RSHIFT)))
			m_pSkyManager->getSky()->setTimeMultiplier(-1.0f);

		// Rayleigh multiplier
		if (mKeyboard->isKeyDown(OIS::KC_2) && !(mKeyboard->isKeyDown(OIS::KC_LSHIFT) || mKeyboard->isKeyDown(OIS::KC_RSHIFT)))
			SkyXOptions.RayleighMultiplier += e.timeSinceLastFrame*0.025f;
		if (mKeyboard->isKeyDown(OIS::KC_2) &&  (mKeyboard->isKeyDown(OIS::KC_LSHIFT) || mKeyboard->isKeyDown(OIS::KC_RSHIFT)))
			SkyXOptions.RayleighMultiplier -= e.timeSinceLastFrame*0.025f;

		// Mie multiplier
		if (mKeyboard->isKeyDown(OIS::KC_3) && !(mKeyboard->isKeyDown(OIS::KC_LSHIFT) || mKeyboard->isKeyDown(OIS::KC_RSHIFT)))
			SkyXOptions.MieMultiplier += e.timeSinceLastFrame*0.025f;
		if (mKeyboard->isKeyDown(OIS::KC_3) &&  (mKeyboard->isKeyDown(OIS::KC_LSHIFT) || mKeyboard->isKeyDown(OIS::KC_RSHIFT)))
			SkyXOptions.MieMultiplier -= e.timeSinceLastFrame*0.025f;

		// Exposure
		if (mKeyboard->isKeyDown(OIS::KC_4) && !(mKeyboard->isKeyDown(OIS::KC_LSHIFT) || mKeyboard->isKeyDown(OIS::KC_RSHIFT)))
			SkyXOptions.Exposure += e.timeSinceLastFrame*0.5f;
		if (mKeyboard->isKeyDown(OIS::KC_4) &&  (mKeyboard->isKeyDown(OIS::KC_LSHIFT) || mKeyboard->isKeyDown(OIS::KC_RSHIFT)))
			SkyXOptions.Exposure -= e.timeSinceLastFrame*0.5f;

		// Inner radius
		if (mKeyboard->isKeyDown(OIS::KC_5) && !(mKeyboard->isKeyDown(OIS::KC_LSHIFT) || mKeyboard->isKeyDown(OIS::KC_RSHIFT)))
			SkyXOptions.InnerRadius += e.timeSinceLastFrame*0.25f;
		if (mKeyboard->isKeyDown(OIS::KC_5) &&  (mKeyboard->isKeyDown(OIS::KC_LSHIFT) || mKeyboard->isKeyDown(OIS::KC_RSHIFT)))
			SkyXOptions.InnerRadius -= e.timeSinceLastFrame*0.25f;

		// Outer radius
		if (mKeyboard->isKeyDown(OIS::KC_6) && !(mKeyboard->isKeyDown(OIS::KC_LSHIFT) || mKeyboard->isKeyDown(OIS::KC_RSHIFT)))
			SkyXOptions.OuterRadius += e.timeSinceLastFrame*0.25f;
		if (mKeyboard->isKeyDown(OIS::KC_6) &&  (mKeyboard->isKeyDown(OIS::KC_LSHIFT) || mKeyboard->isKeyDown(OIS::KC_RSHIFT)))
			SkyXOptions.OuterRadius -= e.timeSinceLastFrame*0.25f;

		// Number of samples
		if (mKeyboard->isKeyDown(OIS::KC_7) && mKeyBuffer < 0 &&!(mKeyboard->isKeyDown(OIS::KC_LSHIFT) || mKeyboard->isKeyDown(OIS::KC_RSHIFT)))
		{
			SkyXOptions.NumberOfSamples ++;
			mKeyBuffer = 0.25f;
		}
		if (mKeyboard->isKeyDown(OIS::KC_7) && mKeyBuffer < 0 && (mKeyboard->isKeyDown(OIS::KC_LSHIFT) || mKeyboard->isKeyDown(OIS::KC_RSHIFT)))
		{
			SkyXOptions.NumberOfSamples --;
			mKeyBuffer = 0.25f;
		}

		// Height position
		if (mKeyboard->isKeyDown(OIS::KC_8) && !(mKeyboard->isKeyDown(OIS::KC_LSHIFT) || mKeyboard->isKeyDown(OIS::KC_RSHIFT)))
			SkyXOptions.HeightPosition += e.timeSinceLastFrame*0.05f;
		if (mKeyboard->isKeyDown(OIS::KC_8) &&  (mKeyboard->isKeyDown(OIS::KC_LSHIFT) || mKeyboard->isKeyDown(OIS::KC_RSHIFT)))
			SkyXOptions.HeightPosition -= e.timeSinceLastFrame*0.05f;

		// Moon phase
		if (mKeyboard->isKeyDown(OIS::KC_9) && !(mKeyboard->isKeyDown(OIS::KC_LSHIFT) || mKeyboard->isKeyDown(OIS::KC_RSHIFT)))
		{
			m_pSkyManager->getBasicController()->setMoonPhase(m_pSkyManager->getBasicController()->getMoonPhase() + e.timeSinceLastFrame*0.25f);

			if (m_pSkyManager->getBasicController()->getMoonPhase() > 1)
			{
				m_pSkyManager->getBasicController()->setMoonPhase(1);
			}
		}
		if (mKeyboard->isKeyDown(OIS::KC_9) &&  (mKeyboard->isKeyDown(OIS::KC_LSHIFT) || mKeyboard->isKeyDown(OIS::KC_RSHIFT)))
		{
			m_pSkyManager->getBasicController()->setMoonPhase(m_pSkyManager->getBasicController()->getMoonPhase() - e.timeSinceLastFrame*0.25f);

			if (m_pSkyManager->getBasicController()->getMoonPhase() < -1)
			{
				m_pSkyManager->getBasicController()->setMoonPhase(-1);
			}
		}

		m_pSkyManager->setOptions(SkyXOptions);

		// Show/Hide information
		if (mKeyboard->isKeyDown(OIS::KC_F1) && mKeyBuffer < 0)
        {
			mShowInformation = !mShowInformation;

			mKeyBuffer = 0.25f;
		}

		mTextArea->setCaption(getConfigStringFromSkyXAtmosphereOptions(SkyXOptions));

        // Update SkyX
		m_pSkyManager->update(e.timeSinceLastFrame);
		// Update Hydrax
        m_pOceanManager->update(e.timeSinceLastFrame);


		mKeyBuffer -= e.timeSinceLastFrame;

        return true;
    }


};

/** Just to locate palmiers with a pseudo-random algoritm
 */
float seed_ = 801;
float rnd_(const float& min, const float& max)
{
	seed_ += Ogre::Math::PI*2.8574f + seed_*(0.3424f - 0.12434f + 0.452345f);
	if (seed_ > 10000000000) seed_ -= 10000000000;
	return ((max-min)*Ogre::Math::Abs(Ogre::Math::Sin(Ogre::Radian(seed_))) + min);
}

void createPalms(Ogre::SceneManager *mSceneMgr)
{
	const int NumberOfPalms = 12;

	Ogre::SceneNode* mPalmsSceneNode = mSceneMgr->getRootSceneNode()->createChildSceneNode();

	for (int k = 0; k < NumberOfPalms; k++)
	{
		Ogre::Vector3 RandomPos = Ogre::Vector3(rnd_(500,2500),
			0,
			rnd_(500,2500));

        /*
		Ogre::RaySceneQuery * raySceneQuery = mSceneMgr->
			createRayQuery(Ogre::Ray(RandomPos + Ogre::Vector3(0,1000000,0),
			Ogre::Vector3::NEGATIVE_UNIT_Y));

		Ogre::RaySceneQueryResult& qryResult = raySceneQuery->execute();
		Ogre::RaySceneQueryResult::iterator i = qryResult.begin();

		if (i != qryResult.end() && i->worldFragment)
		{
			if (i->worldFragment->singleIntersection.y>105 || i->worldFragment->singleIntersection.y<20)
			{
				k--;
				continue;
			}

			RandomPos.y = i->worldFragment->singleIntersection.y;
		}
		else
		{
			k--;
			continue;
		}
		*/

		RandomPos.y = m_pTerrainManager->getHeightAtPosition(RandomPos);

		Ogre::Entity *mPalmEnt = mSceneMgr->createEntity("Palm"+Ogre::StringConverter::toString(k), "Palm.mesh");
		Ogre::SceneNode *mPalmSN = mPalmsSceneNode->createChildSceneNode();

		mPalmSN->rotate(Ogre::Vector3(-1,0,rnd_(-0.3,0.3)), Ogre::Degree(90));
		mPalmSN->attachObject(mPalmEnt);
		Ogre::Real Scale = rnd_(50,75);
		mPalmSN->scale(Scale,Scale,Scale);
		mPalmSN->setPosition(RandomPos);
	}
}

class SampleApp : public ExampleApplication
{
public:
    // Basic constructor
    SampleApp()
    {}

protected:

    // Just override the mandatory create scene method
    void createScene(void)
    {
		// Set default ambient light
		mSceneMgr->setAmbientLight(ColourValue(1, 1, 1));

		// Set some camera params
        mCamera->setFarClipDistance(99999*6);
		mCamera->setPosition(311.902,128.419,1539.02);
		mCamera->setDirection(0.155, 0.1808, -0.97);

	    // Light
		Ogre::Light *mLight0 = mSceneMgr->createLight("Light0");
		mLight0->setDiffuseColour(1, 1, 1);
		mLight0->setCastShadows(false);

		m_pSkyManager = new SkyManager();
		m_pSkyManager->init(mSceneMgr, mCamera);

		m_pOceanManager = new OceanManager();
		m_pOceanManager->init(mSceneMgr, mCamera, mWindow->getViewport(0));
		m_pOceanManager->setSkyManager(m_pSkyManager);
		m_pOceanManager->setLight(mLight0);

        m_pTerrainManager = new TerrainManager();
        m_pTerrainManager->init(mSceneMgr, mLight0);

		// Create palmiers
		createPalms(mSceneMgr);

		// Create text area to show skyboxes information
		createTextArea();
    }

	void createFrameListener()
	{
		// Add frame listener
		mRoot->addFrameListener(new SkyXDemoFrameListener(mWindow, mCamera, mSceneMgr));
	}

	void chooseSceneManager()
    {
        // Create the SceneManager, in this case terrain scene manager
        //mSceneMgr = mRoot->createSceneManager("TerrainSceneManager");
        mSceneMgr = mRoot->createSceneManager(ST_EXTERIOR_CLOSE);
    }

	// Create text area for SkyX parameters
	void createTextArea()
	{
		// Create a panel
		Ogre::OverlayContainer* panel = static_cast<Ogre::OverlayContainer*>(
			OverlayManager::getSingleton().createOverlayElement("Panel", "SkyXParametersPanel"));
		panel->setMetricsMode(Ogre::GMM_PIXELS);
		panel->setPosition(10, 10);
		panel->setDimensions(400, 400);

		// Create a text area
		mTextArea = static_cast<Ogre::TextAreaOverlayElement*>(
			OverlayManager::getSingleton().createOverlayElement("TextArea", "SkyXParametersTextArea"));
		mTextArea->setMetricsMode(Ogre::GMM_PIXELS);
		mTextArea->setPosition(0, 0);
		mTextArea->setDimensions(100, 100);
		mTextArea->setCaption("SkyX plugin demo");
		mTextArea->setCharHeight(16);
		mTextArea->setFontName("BlueHighway");
		mTextArea->setColourBottom(ColourValue(0.3, 0.5, 0.3));
		mTextArea->setColourTop(ColourValue(0.5, 0.7, 0.5));

		// Create an overlay, and add the panel
		Ogre::Overlay* overlay = OverlayManager::getSingleton().create("OverlayName");
		overlay->add2D(panel);

		// Add the text area to the panel
		panel->addChild(mTextArea);

		// Show the overlay
		overlay->show();
	}
};

// ----------------------------------------------------------------------------
// Main function, just boots the application object
// ----------------------------------------------------------------------------
int main(int argc, char** argv)
{
	try
	{
		SampleApp app;
		app.go();
	}
	catch (Ogre::Exception &e)
	{
		std::cerr << "An exception has occured: " << e.getFullDescription();

		return -1;
	}

	return 0;
}
