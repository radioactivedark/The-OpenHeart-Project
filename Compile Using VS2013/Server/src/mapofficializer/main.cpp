#include <md5.h>
#include <SaveFile.h>
#include <stdlib.h>


const bool officializeMap(const String &szMapname)
{
    SaveFile sf;
    if(!sf.load(szMapname+".map"))
    {
        printf("%s.map not found!\n",szMapname.c_str());
        return false;
    }

    //Generate seed
    String seed = "";
    if(!sf.getSetting("Seed",seed,"Map"))
    {
        for(int i=0;i<10;i++)
        {
            switch(rand()%3)
            {
                case 0: seed.push_back(char(48+rand()%10)); break;
                case 1: seed.push_back(char(65+rand()%26)); break;
                default: seed.push_back(char(97+rand()%26)); break;
            }
        }
        sf.setSetting("Seed",seed,"Map");
    }

    sf.setSetting("Hash",md5(szMapname+seed),"Map");

    if(sf.save())
    {
        printf("Officialized %s.map.\n",szMapname.c_str());
    }
    else
    {
        printf("Failed to save %s.map!\n",szMapname.c_str());
    }

    return true;
}

int main()
{
    printf("Enter map to officialize (without .map): ");

    char szBuffer[32] = "";
    std::cin.get(szBuffer,32,'\n');
    std::cin.get();

    printf("\nOfficializing %s...\n", szBuffer);
    officializeMap(szBuffer);

    printf("\nPress any key to exit...");
    std::cin.get();

    return 0;
}
