#include "grid-content/contentServer/cache/CacheImplementation.h"
#include "grid-content/contentServer/redis/RedisImplementation.h"
#include "grid-content/contentServer/corba/client/ClientImplementation.h"
#include "grid-content/contentServer/http/client/ClientImplementation.h"
#include "grid-content/contentServer/memory/MemoryImplementation.h"
#include "grid-content/contentServer/corba/server/Server.h"
#include "grid-files/common/Exception.h"
#include "grid-content/contentServer/corba/server/ServerInterface.h"
#include "grid-files/identification/GridDef.h"
#include <signal.h>

using namespace SmartMet;


ContentServer::Corba::Server *corbaServer = nullptr;
ContentServer::ServiceInterface *contentSource = nullptr;
ContentServer::Corba::ClientImplementation *corbaClient = nullptr;
ContentServer::HTTP::ClientImplementation *httpClient = nullptr;
ContentServer::RedisImplementation *redisImplementation = nullptr;
ContentServer::MemoryImplementation *memoryImplementation = nullptr;
ContentServer::CacheImplementation *cacheImplementation = nullptr;

bool mShutdownRequested = false;

ConfigurationFile   mConfigurationFile;
std::string         mServerAddress;
std::string         mServerPort;
std::string         mServerIorFile;
bool                mCacheEnabled = false;
uint                mCacheContentSortingFlags = 0;
std::string         mContentSourceType;
bool                mDataLoadEnabled = false;
bool                mDataSaveEnabled = false;
std::string         mDataDir;
uint                mDataSaveInterval;
uint                mMemoryContentSortingFlags;
bool                mProcessingLogEnabled = false;
std::string         mProcessingLogFile;
int                 mProcessingLogMaxSize = 100000000;
int                 mProcessingLogTruncateSize = 20000000;
Log                 mProcessingLog;
bool                mDebugLogEnabled = false;
std::string         mDebugLogFile;
int                 mDebugLogMaxSize = 100000000;
int                 mDebugLogTruncateSize = 20000000;
Log                 mDebugLog;
std::string         mRedisAddress;
int                 mRedisPort;
std::string         mRedisTablePrefix;
std::string         mCorbaIor;
std::string         mHttpUrl;




void sig_handler(int signum)
{
  {
    try
    {
      if (mShutdownRequested)
        sprintf(nullptr,"Crashing the system for the core dump");

      if (corbaServer != nullptr)
      {
        printf("\n**** SHUTTING DOWN ****\n");
        mShutdownRequested = true;
        sleep(2);
        corbaServer->shutdown();
      }
      else
        exit(-1);
    }
    catch (...)
    {
      SmartMet::Spine::Exception exception(BCP,exception_operation_failed,nullptr);
      exception.printError();
      exit(-1);
    }
  }
}





void readConfigFile(const char* configFile)
{
  try
  {
    const char *configAttribute[] =
    {
        "smartmet.tools.grid.content-server.address",
        "smartmet.tools.grid.content-server.port",
        "smartmet.tools.grid.content-server.iorFile",
        "smartmet.tools.grid.content-server.cache.enabled",
        "smartmet.tools.grid.content-server.cache.contentSortingFlags",
        "smartmet.tools.grid.content-server.content-source.type",
        "smartmet.tools.grid.content-server.content-source.redis.address",
        "smartmet.tools.grid.content-server.content-source.redis.port",
        "smartmet.tools.grid.content-server.content-source.redis.tablePrefix",
        "smartmet.tools.grid.content-server.content-source.corba.ior",
        "smartmet.tools.grid.content-server.content-source.http.url",
        "smartmet.tools.grid.content-server.content-source.memory.contentLoadEnabled",
        "smartmet.tools.grid.content-server.content-source.memory.contentSaveEnabled",
        "smartmet.tools.grid.content-server.content-source.memory.contentDir",
        "smartmet.tools.grid.content-server.content-source.memory.contentSaveInterval",
        "smartmet.tools.grid.content-server.content-source.memory.contentSortingFlags",
        "smartmet.tools.grid.content-server.processing-log.enabled",
        "smartmet.tools.grid.content-server.processing-log.file",
        "smartmet.tools.grid.content-server.processing-log.maxSize",
        "smartmet.tools.grid.content-server.processing-log.truncateSize",
        "smartmet.tools.grid.content-server.debug-log.enabled",
        "smartmet.tools.grid.content-server.debug-log.file",
        "smartmet.tools.grid.content-server.debug-log.maxSize",
        "smartmet.tools.grid.content-server.debug-log.truncateSize",
        nullptr
    };


    mConfigurationFile.readFile(configFile);
    //mConfigurationFile.print(std::cout,0,0);

    uint t=0;
    while (configAttribute[t] != nullptr)
    {
      if (!mConfigurationFile.findAttribute(configAttribute[t]))
      {
        SmartMet::Spine::Exception exception(BCP, "Missing configuration attribute!");
        exception.addParameter("File",configFile);
        exception.addParameter("Attribute",configAttribute[t]);
        throw exception;
      }
      t++;
    }

    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.address", mServerAddress);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.port", mServerPort);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.iorFile", mServerIorFile);

    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.cache.enabled", mCacheEnabled);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.cache.contentSortingFlags", mCacheContentSortingFlags);

    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.content-source.type", mContentSourceType);

    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.content-source.redis.address", mRedisAddress);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.content-source.redis.port", mRedisPort);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.content-source.redis.tablePrefix", mRedisTablePrefix);

    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.content-source.corba.ior", mCorbaIor);

    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.content-source.http.url", mHttpUrl);

    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.content-source.memory.contentLoadEnabled", mDataLoadEnabled);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.content-source.memory.contentSaveEnabled", mDataSaveEnabled);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.content-source.memory.contentDir", mDataDir);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.content-source.memory.contentSaveInterval", mDataSaveInterval);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.content-source.memory.contentSortingFlags", mMemoryContentSortingFlags);

    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.processing-log.enabled", mProcessingLogEnabled);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.processing-log.file", mProcessingLogFile);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.processing-log.maxSize", mProcessingLogMaxSize);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.processing-log.truncateSize", mProcessingLogTruncateSize);

    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.debug-log.enabled", mDebugLogEnabled);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.debug-log.file", mDebugLogFile);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.debug-log.maxSize", mDebugLogMaxSize);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.debug-log.truncateSize", mDebugLogTruncateSize);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Constructor failed!", nullptr);
  }
}





int main(int argc, char *argv[])
{
  try
  {
    if (argc != 2)
    {
      printf("\n");
      printf("##################################################################################\n");
      printf("                            corbaContentServer\n");
      printf("##################################################################################\n");
      printf(" DESCRIPTION:\n");
      printf("   This is a contentServer that offers a CORBA service interface for accessing\n");
      printf("   content information in the content source. Notice that this is just an interface");
      printf("   and it does not contain any caching capabilities.");
      printf("\n");
      printf("   When the server starts, it prints its IOR (International Object Reference) \n");
      printf("   on the screen or into the file specified in the configuration file. You should\n");
      printf("   store this into the environment variable SMARTMET_CS_IOR so that client programs\n");
      printf("   can access this service easily (i.e. it does not make sense to write own configuration\n");
      printf("   files for tens of client programs). Notice that the IOR does not change as long as\n");
      printf("   the server address and the port do not change. I.e. you can store it in your shell\n");
      printf("   start-up scripts (like '.bashrc').\n");
      printf("\n");
      printf(" USAGE:\n");
      printf("   corbaContentServer <configFile>\n");
      printf("\n");
      printf("##################################################################################\n");
      printf("\n");
      return -1;
    }

    signal(SIGINT, sig_handler);

    readConfigFile(argv[1]);

    corbaServer = new ContentServer::Corba::Server(mServerAddress.c_str(),mServerPort.c_str());


    if (strcasecmp(mContentSourceType.c_str(),"redis") == 0)
    {
      redisImplementation = new ContentServer::RedisImplementation();
      redisImplementation->init(mRedisAddress.c_str(),mRedisPort,mRedisTablePrefix.c_str());
      contentSource = redisImplementation;
    }
    else
    if (strcasecmp(mContentSourceType.c_str(),"corba") == 0)
    {
      corbaClient = new ContentServer::Corba::ClientImplementation();
      corbaClient->init(mCorbaIor.c_str());
      contentSource = corbaClient;
    }
    else
    if (strcasecmp(mContentSourceType.c_str(),"http") == 0)
    {
      httpClient = new ContentServer::HTTP::ClientImplementation();
      httpClient->init(mHttpUrl.c_str());
      contentSource = httpClient;
    }
    else
    if (strcasecmp(mContentSourceType.c_str(),"memory") == 0)
    {
      memoryImplementation = new ContentServer::MemoryImplementation();
      memoryImplementation->init(mDataLoadEnabled,mDataSaveEnabled,mDataDir,mDataSaveInterval,mMemoryContentSortingFlags);
      contentSource = memoryImplementation;
    }

    if (contentSource == nullptr)
    {
      SmartMet::Spine::Exception exception(BCP,"No acceptable content source defined!");
      throw exception;
    }

    if (mCacheEnabled)
    {
      cacheImplementation = new ContentServer::CacheImplementation();
      if (mProcessingLogEnabled)
      {
        mProcessingLog.init(true,mProcessingLogFile.c_str(),mProcessingLogMaxSize,mProcessingLogTruncateSize);
        cacheImplementation->setProcessingLog(&mProcessingLog);
      }

      if (mDebugLogEnabled)
      {
        mDebugLog.init(true,mDebugLogFile.c_str(),mDebugLogMaxSize,mDebugLogTruncateSize);
        cacheImplementation->setDebugLog(&mDebugLog);
      }

      cacheImplementation->init(0,contentSource,mCacheContentSortingFlags);
      cacheImplementation->startEventProcessing();
      contentSource = cacheImplementation;
    }
    else
    {
      if (mProcessingLogEnabled)
      {
        mProcessingLog.init(true,mProcessingLogFile.c_str(),mProcessingLogMaxSize,mProcessingLogTruncateSize);
        contentSource->setProcessingLog(&mProcessingLog);
      }

      if (mDebugLogEnabled)
      {
        mDebugLog.init(true,mDebugLogFile.c_str(),mDebugLogMaxSize,mDebugLogTruncateSize);
        contentSource->setDebugLog(&mDebugLog);
      }
    }


    corbaServer->init(contentSource);


    // Let's print the service IOR. This is necessary for accessing the service. Usually the best way
    // to handle an IOR is to store it into an environment variable.

    std::string ior = corbaServer->getServiceIor();
    if (mServerIorFile.length() == 0)
    {
      printf("\n%s\n",ior.c_str());
    }
    else
    {
      FILE *file = fopen(mServerIorFile.c_str(),"we");
      if (file == nullptr)
      {
        SmartMet::Spine::Exception exception(BCP,"Cannot create file for IOR!");
        exception.addParameter("IorFile",mServerIorFile);
        throw exception;
      }
      fprintf(file,"%s\n",ior.c_str());
      fclose(file);
    }

    corbaServer->run();

    if (redisImplementation != nullptr)
      delete redisImplementation;

    if (corbaClient != nullptr)
      delete corbaClient;

    if (httpClient != nullptr)
      delete httpClient;

    if (memoryImplementation != nullptr)
      delete memoryImplementation;

    delete corbaServer;

    return 0;
  }
  catch (SmartMet::Spine::Exception& e)
  {
    SmartMet::Spine::Exception exception(BCP,exception_operation_failed,nullptr);
    exception.printError();
    return -1;
  }
}
