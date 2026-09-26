// ##################################################################################################
// ##------------------------------------------FILE START------------------------------------------##
// ##################################################################################################


#ifndef _INPUTMANAGER_H_
#define _INPUTMANAGER_H_

#include <string>
#include <iostream>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

using namespace std;

class InputListener
{
public:
    InputListener(){}
    virtual ~InputListener(){}
    virtual void inputEvent(const string &input) = 0;
};

class InputManager
{
private:
    InputListener *mListener;
    string mInput;
public:
    InputManager(InputListener *listener)
    {
        mListener = listener;
    }
    ~InputManager()
    {
    }
    void update()
    {
        if(kbhit())
        {
            const int input = getch();
            //Return
            if(input==10)
            {
                cout << "=>" << mInput.c_str() << endl;
                mListener->inputEvent(mInput);
                mInput = "";
            }
            //Backspace
            else if(input==127)
            {
                if(mInput.length()>0)mInput.erase(mInput.end()-1);
                cout << "->" << mInput.c_str() << endl;
            }
            else
            {
                mInput.push_back(input);
                cout << "->" << mInput.c_str() << endl;
            }
        }
    }
    int kbhit(void)
    {
      struct termios oldt, newt;
      int ch;
      int oldf;

      tcgetattr(STDIN_FILENO, &oldt);
      newt = oldt;
      newt.c_lflag &= ~(ICANON | ECHO);
      tcsetattr(STDIN_FILENO, TCSANOW, &newt);
      oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
      fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

      ch = getchar();

      tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
      fcntl(STDIN_FILENO, F_SETFL, oldf);

      if(ch != EOF)
      {
          ungetc(ch, stdin);
          return 1;
      }

      return 0;
    }

    int getch( )
    {
      struct termios oldt, newt;
      int ch;
      tcgetattr( STDIN_FILENO, &oldt );
      newt = oldt;
      newt.c_lflag &= ~( ICANON | ECHO );
      tcsetattr( STDIN_FILENO, TCSANOW, &newt );
      ch = getchar();
      tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
      return ch;
    }

};

#endif


// ##################################################################################################
// ##-------------------------------------------FILE END-------------------------------------------##
// ##################################################################################################
// ##-------------------------------------------MODIFIED-------------------------------------------##
// ## DATE: 9/25/2026                                                                              ##
// ## TIME: 6:18 PM (UTC-5:00)                                                                     ##
// ## BY: radioactivedark                                                                          ##
// ##################################################################################################