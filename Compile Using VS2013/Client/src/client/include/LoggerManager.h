#ifndef _LOGGERMANAGER_H_
#define _LOGGERMANAGER_H_

#define LOGGER_FILENAME "FH.log"

class LoggerManager : public Singleton<LoggerManager>
{
private:
    std::ofstream mOutFile;
public:
    LoggerManager()
    {
        mOutFile.open(LOGGER_FILENAME);
        mOutFile.close();
    }
    ~LoggerManager()
    {
    }
    static LoggerManager* getSingletonPtr();
    static LoggerManager& getSingleton();
    void logMessage(const String &message)
    {
        mOutFile.open(LOGGER_FILENAME,std::ios::app);
        mOutFile.write(message.c_str(),message.length());
        mOutFile.write("\n",1);
        mOutFile.close();
    }
};

template<> LoggerManager* Singleton<LoggerManager>::ms_Singleton = 0;

LoggerManager* LoggerManager::getSingletonPtr()
{
	return ms_Singleton;
}

LoggerManager& LoggerManager::getSingleton()
{
	assert(ms_Singleton);
	return *ms_Singleton;
}

#endif
