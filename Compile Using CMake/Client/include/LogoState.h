#ifndef _LOGOSTATE_H_
#define _LOGOSTATE_H_

#include <LoginState.h>

class LogoState : public GameState
{
private:
    bool mSkipped;
    Real mLogoTimer;
public:
    String mPanelName;
    LogoState()
    {
        mSkipped = false;
        mLogoTimer = 4.0f;
        mPanelName = "LogoScreen";
    }
    ~LogoState()
    {
        exit();
    }
    void enter()
    {
        init();
        mGui->addPanel(mPanelName,true);
        mScreenFXMgr->doFadeIn(2.0f,mPanelName=="LogoScreen2"?ColourValue::White:ColourValue::Black);
    }
    void exit()
    {
        uninit();
    }
    void update(const Real &timeElapsed)
    {
        updateIntroSequence(timeElapsed);
        mScreenFXMgr->update(timeElapsed);

        mRoot->renderOneFrame();
    }
    void updateIntroSequence(const Real &timeElapsed)
    {
        mLogoTimer -= timeElapsed;
        if(mLogoTimer<=0)
        {
            if(mSkipped)
            {
                changeState = true;
                replaceState = true;
                if(mPanelName=="LogoScreen2")mNextState = new LoginState();
                else
                {
                    LogoState *state = new LogoState();
                    state->mPanelName = "LogoScreen2";
                    mNextState = state;
                }
            }
            else doSkip();
        }
    }
    const bool keyPressed(const char &index)
    {
        doSkip(true);
        return true;
    }
    bool mouseReleased(const OIS::MouseEvent &arg, OIS::MouseButtonID id)
    {
        doSkip(true);
        return true;
    }
    void doSkip(const bool &fast=false)
    {
        if(mSkipped)return;
        mSkipped = true;
        mLogoTimer = (fast?0.25f:1.0f);
        mScreenFXMgr->doFadeOut(mLogoTimer);
    }
};

#endif
