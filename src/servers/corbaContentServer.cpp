#include "contentServer/cache/CacheImplementation.h"
#include "contentServer/redis/RedisImplementation.h"
#include "contentServer/corba/client/ClientImplementation.h"
#include "contentServer/http/client/ClientImplementation.h"
#include "contentServer/memory/MemoryImplementation.h"
#include "contentServer/corba/server/Server.h"
#include "grid-files/common/Exception.h"
#include "contentServer/corba/server/ServerInterface.h"
#include "grid-files/identification/GridDef.h"
#include <signal.h>

using namespace SmartMet;


ContentServer::Corba::Server *server = NULL;
ContentServer::ServiceInterface *dataSource = NULL;
ContentServer::Corba::ClientImplementation *corbaClient = NULL;
ContentServer::HTTP::ClientImplementation *httpClient = NULL;
ContentServer::RedisImplementation *redisImplementation = NULL;
ContentServer::MemoryImplementation *memoryImplementation = NULL;
ContentServer::CacheImplementation *cacheImplementation = NULL;

bool shutdownRequested = false;

ConfigurationFile   mConfigurationFile;
std::string         mServerAddress;
std::string         mServerPort;
std::string         mServerIorFile;
bool                mCacheEnabled = false;
uint                mCacheContentSortingFlags = 0;
std::string         mDataSourceType;
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
      if (shutdownRequested)
        sprintf(NULL,"Crashing the system for the core dump");

      if (server != NULL)
      {
        printf("\n**** SHUTTING DOWN ****\n");
        shutdownRequested = true;
        sleep(2);
        server->shutdown();
      }
      else
        exit(-1);
    }
    catch (...)
    {
      SmartMet::Spine::Exception exception(BCP,exception_operation_failed,NULL);
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
        "smartmet.service.grid.content-server.address",
        "smartmet.service.grid.content-server.port",
        "smartmet.service.grid.content-server.iorFile",
        "smartmet.service.grid.content-server.cache.enabled",
        "smartmet.service.grid.content-server.cache.contentSortingFlags",
        "smartmet.service.grid.content-server.data-source.type",
        "smartmet.service.grid.content-server.data-source.redis.address",
        "smartmet.service.grid.content-server.data-source.redis.port",
        "smartmet.service.grid.content-server.data-source.redis.tablePrefix",
        "smartmet.service.grid.content-server.data-source.corba.ior",
        "smartmet.service.grid.content-server.data-source.http.url",
        "smartmet.service.grid.content-server.data-source.memory.dataLoadEnabled",
        "smartmet.service.grid.content-server.data-source.memory.dataSaveEnabled",
        "smartmet.service.grid.content-server.data-source.memory.dataDir",
        "smartmet.service.grid.content-server.data-source.memory.dataSaveInterval",
        "smartmet.service.grid.content-server.data-source.memory.contentSortingFlags",
        "smartmet.service.grid.content-server.processing-log.enabled",
        "smartmet.service.grid.content-server.processing-log.file",
        "smartmet.service.grid.content-server.processing-log.maxSize",
        "smartmet.service.grid.content-server.processing-log.truncateSize",
        "smartmet.service.grid.content-server.debug-log.enabled",
        "smartmet.service.grid.content-server.debug-log.file",
        "smartmet.service.grid.content-server.debug-log.maxSize",
        "smartmet.service.grid.content-server.debug-log.truncateSize",
        NULL
    };


    mConfigurationFile.readFile(configFile);
    //mConfigurationFile.print(std::cout,0,0);

    uint t=0;
    while (configAttribute[t] != NULL)
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

    mConfigurationFile.getAttributeValue("smartmet.service.grid.content-server.address", mServerAddress);
    mConfigurationFile.getAttributeValue("smartmet.service.grid.content-server.port", mServerPort);
    mConfigurationFile.getAttributeValue("smartmet.service.grid.content-server.iorFile", mServerIorFile);

    mConfigurationFile.getAttributeValue("smartmet.service.grid.content-server.cache.enabled", mCacheEnabled);
    mConfigurationFile.getAttributeValue("smartmet.service.grid.content-server.cache.contentSortingFlags", mCacheContentSortingFlags);

    mConfigurationFile.getAttributeValue("smartmet.service.grid.content-server.data-source.type", mDataSourceType);

    mConfigurationFile.getAttributeValue("smartmet.service.grid.content-server.data-source.redis.address", mRedisAddress);
    mConfigurationFile.getAttributeValue("smartmet.service.grid.content-server.data-source.redis.port", mRedisPort);
    mConfigurationFile.getAttributeValue("smartmet.service.grid.content-server.data-source.redis.tablePrefix", mRedisTablePrefix);

    mConfigurationFile.getAttributeValue("smartmet.service.grid.content-server.data-source.corba.ior", mCorbaIor);

    mConfigurationFile.getAttributeValue("smartmet.service.grid.content-server.data-source.http.url", mHttpUrl);

    mConfigurationFile.getAttributeValue("smartmet.service.grid.content-server.data-source.memory.dataLoadEnabled", mDataLoadEnabled);
    mConfigurationFile.getAttributeValue("smartmet.service.grid.content-server.data-source.memory.dataSaveEnabled", mDataSaveEnabled);
    mConfigurationFile.getAttributeValue("smartmet.service.grid.content-server.data-source.memory.dataDir", mDataDir);
    mConfigurationFile.getAttributeValue("smartmet.service.grid.content-server.data-source.memory.dataSaveInterval", mDataSaveInterval);
    mConfigurationFile.getAttributeValue("smartmet.service.grid.content-server.data-source.memory.contentSortingFlags", mMemoryContentSortingFlags);

    mConfigurationFile.getAttributeValue("smartmet.service.grid.content-server.processing-log.enabled", mProcessingLogEnabled);
    mConfigurationFile.getAttributeValue("smartmet.service.grid.content-server.processing-log.file", mProcessingLogFile);
    mConfigurationFile.getAttributeValue("smartmet.service.grid.content-server.processing-log.maxSize", mProcessingLogMaxSize);
    mConfigurationFile.getAttributeValue("smartmet.service.grid.content-server.processing-log.truncateSize", mProcessingLogTruncateSize);

    mConfigurationFile.getAttributeValue("smartmet.service.grid.content-server.debug-log.enabled", mDebugLogEnabled);
    mConfigurationFile.getAttributeValue("smartmet.service.grid.content-server.debug-log.file", mDebugLogFile);
    mConfigurationFile.getAttributeValue("smartmet.service.grid.content-server.debug-log.maxSize", mDebugLogMaxSize);
    mConfigurationFile.getAttributeValue("smartmet.service.grid.content-server.debug-log.truncateSize", mDebugLogTruncateSize);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Constructor failed!", NULL);
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
      printf("   content information in the data source. Notice that this is just an interface");
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

    server = new ContentServer::Corba::Server(mServerAddress.c_str(),mServerPort.c_str());


    if (strcasecmp(mDataSourceType.c_str(),"redis") == 0)
    {
      redisImplementation = new ContentServer::RedisImplementation();
      redisImplementation->init(mRedisAddress.c_str(),mRedisPort,mRedisTablePrefix.c_str());
      dataSource = redisImplementation;
    }
    else
    if (strcasecmp(mDataSourceType.c_str(),"corba") == 0)
    {
      corbaClient = new ContentServer::Corba::ClientImplementation();
      corbaClient->init(mCorbaIor.c_str());
      dataSource = corbaClient;
    }
    else
    if (strcasecmp(mDataSourceType.c_str(),"http") == 0)
    {
      httpClient = new ContentServer::HTTP::ClientImplementation();
      httpClient->init(mHttpUrl.c_str());
      dataSource = httpClient;
    }
    else
    if (strcasecmp(mDataSourceType.c_str(),"memory") == 0)
    {
      memoryImplementation = new ContentServer::MemoryImplementation();
      memoryImplementation->init(mDataLoadEnabled,mDataSaveEnabled,mDataDir,mDataSaveInterval,mMemoryContentSortingFlags);
      dataSource = memoryImplementation;
    }

    if (dataSource == NULL)
    {
      SmartMet::Spine::Exception exception(BCP,"No acceptable data source defined!");
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

      cacheImplementation->init(0,dataSource,mCacheContentSortingFlags);
      cacheImplementation->startEventProcessing();
      dataSource = cacheImplementation;
    }
    else
    {
      if (mProcessingLogEnabled)
      {
        mProcessingLog.init(true,mProcessingLogFile.c_str(),mProcessingLogMaxSize,mProcessingLogTruncateSize);
        dataSource->setProcessingLog(&mProcessingLog);
      }

      if (mDebugLogEnabled)
      {
        mDebugLog.init(true,mDebugLogFile.c_str(),mDebugLogMaxSize,mDebugLogTruncateSize);
        dataSource->setDebugLog(&mDebugLog);
      }
    }


    server->init(dataSource);


    // Let's print the service IOR. This is necessary for accessing the service. Usually the best way
    // to handle an IOR is to store it into an environment variable.

    std::string ior = server->getServiceIor();
    if (mServerIorFile.length() == 0)
    {
      printf("\n%s\n",ior.c_str());
    }
    else
    {
      FILE *file = fopen(mServerIorFile.c_str(),"w");
      if (file == NULL)
      {
        SmartMet::Spine::Exception exception(BCP,"Cannot create file for IOR!");
        exception.addParameter("IorFile",mServerIorFile);
        throw exception;
      }
      fprintf(file,"%s\n",ior.c_str());
      fclose(file);
    }

    server->run();

    if (redisImplementation != NULL)
      delete redisImplementation;

    if (corbaClient != NULL)
      delete corbaClient;

    if (httpClient != NULL)
      delete httpClient;

    if (memoryImplementation != NULL)
      delete memoryImplementation;

    delete server;

    return 0;
  }
  catch (SmartMet::Spine::Exception& e)
  {
    SmartMet::Spine::Exception exception(BCP,exception_operation_failed,NULL);
    exception.printError();
    return -1;
  }
}
