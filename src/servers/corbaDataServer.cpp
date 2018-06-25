#include "grid-content/contentServer/cache/CacheImplementation.h"
#include "grid-content/contentServer/corba/client/ClientImplementation.h"
#include "grid-content/dataServer/implementation/ServiceImplementation.h"
#include "grid-content/dataServer/implementation/VirtualContentFactory_type1.h"
#include "grid-content/dataServer/corba/server/ServerInterface.h"
#include "grid-content/dataServer/corba/server/Server.h"
#include "grid-files/common/Exception.h"
#include "grid-files/common/Typedefs.h"
#include "grid-files/grid/ValueCache.h"
#include "grid-files/identification/GridDef.h"
#include <iostream>
#include <stdexcept>
#include <string.h>
#include <vector>
#include <pthread.h>
#include <sys/time.h>
#include <signal.h>


using namespace SmartMet;

DataServer::Corba::Server *corbaServer = NULL;
DataServer::ServiceImplementation *dataServer = NULL;

ConfigurationFile   mConfigurationFile;
std::string         mServerName;
uint                mServerId = 0;
std::string         mServerAddress;
std::string         mServerPort;
std::string         mServerIorFile;
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
std::string         mContentSourceIor;
bool                mVirtualFilesEnabled;
std::string         mVirtualFileDefinitions;
bool                mGridPreloadEnabled;
string_vec          mLuaFiles;
std::string         mGridDirectory;
std::string         mGridConfigFile;
uint                mNumOfCachedGrids = 8000;
uint                mMaxCompressedMegaBytesOfCachedGrids = 10000;
uint                mMaxUncompressedMegaBytesOfCachedGrids = 10000;


bool mShutdownRequested = false;



void sig_handler(int signum)
{
  {
    try
    {
      if (mShutdownRequested)
        sprintf(NULL,"Crashing the system for the core dump");

      if (dataServer != NULL)
      {
        printf("\n**** SHUTTING DOWN ****\n");
        mShutdownRequested = true;
        sleep(2);
        dataServer->shutdown();
        corbaServer->shutdown();
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
        "smartmet.library.grid-files.configFile",
        "smartmet.library.grid-files.cache.numOfGrids",
        "smartmet.library.grid-files.cache.maxUncompressedSizeInMegaBytes",
        "smartmet.library.grid-files.cache.maxCompressedSizeInMegaBytes",
        "smartmet.tools.grid.data-server.name",
        "smartmet.tools.grid.data-server.id",
        "smartmet.tools.grid.data-server.address",
        "smartmet.tools.grid.data-server.port",
        "smartmet.tools.grid.data-server.iorFile",
        "smartmet.tools.grid.data-server.content-source.ior",
        "smartmet.tools.grid.data-server.grid-storage.directory",
        "smartmet.tools.grid.data-server.grid-storage.preloadEnabled",
        "smartmet.tools.grid.data-server.virtualFiles.enabled",
        "smartmet.tools.grid.data-server.virtualFiles.definitionFile",
        "smartmet.tools.grid.data-server.luaFiles",
        "smartmet.tools.grid.data-server.processing-log.enabled",
        "smartmet.tools.grid.data-server.processing-log.file",
        "smartmet.tools.grid.data-server.processing-log.maxSize",
        "smartmet.tools.grid.data-server.processing-log.truncateSize",
        "smartmet.tools.grid.data-server.debug-log.enabled",
        "smartmet.tools.grid.data-server.debug-log.file",
        "smartmet.tools.grid.data-server.debug-log.maxSize",
        "smartmet.tools.grid.data-server.debug-log.truncateSize",
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

    mConfigurationFile.getAttributeValue("smartmet.library.grid-files.configFile", mGridConfigFile);
    mConfigurationFile.getAttributeValue("smartmet.library.grid-files.cache.numOfGrids", mNumOfCachedGrids);
    mConfigurationFile.getAttributeValue("smartmet.library.grid-files.cache.maxUncompressedSizeInMegaBytes", mMaxUncompressedMegaBytesOfCachedGrids);
    mConfigurationFile.getAttributeValue("smartmet.library.grid-files.cache.maxCompressedSizeInMegaBytes", mMaxCompressedMegaBytesOfCachedGrids);

    mConfigurationFile.getAttributeValue("smartmet.tools.grid.data-server.name", mServerName);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.data-server.id", mServerId);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.data-server.address", mServerAddress);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.data-server.port", mServerPort);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.data-server.iorFile", mServerIorFile);

    mConfigurationFile.getAttributeValue("smartmet.tools.grid.data-server.content-source.ior", mContentSourceIor);

    mConfigurationFile.getAttributeValue("smartmet.tools.grid.data-server.grid-storage.directory",mGridDirectory);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.data-server.grid-storage.preloadEnabled",mGridPreloadEnabled);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.data-server.virtualFiles.enabled",mVirtualFilesEnabled);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.data-server.virtualFiles.definitionFile",mVirtualFileDefinitions);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.data-server.luaFiles",mLuaFiles);

    mConfigurationFile.getAttributeValue("smartmet.tools.grid.data-server.processing-log.enabled", mProcessingLogEnabled);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.data-server.processing-log.file", mProcessingLogFile);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.data-server.processing-log.maxSize", mProcessingLogMaxSize);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.data-server.processing-log.truncateSize", mProcessingLogTruncateSize);

    mConfigurationFile.getAttributeValue("smartmet.tools.grid.data-server.debug-log.enabled", mDebugLogEnabled);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.data-server.debug-log.file", mDebugLogFile);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.data-server.debug-log.maxSize", mDebugLogMaxSize);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.data-server.debug-log.truncateSize", mDebugLogTruncateSize);
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
      printf("                            corbaDataServer\n");
      printf("##################################################################################\n");
      printf(" DESCRIPTION:\n");
      printf("   This is a dataServer that offers a CORBA service interface for accessing\n");
      printf("   data in grid files. The content of the grid files is managed in the contentServer,\n");
      printf("   which means that the dataServer needs an access to the contentServer. In this case\n");
      printf("   this access is CORBA-based (i.e. we are using CORBA-client for fetching information\n");
      printf("   from the contentServer).\n");
      printf("\n");
      printf("   The basic idea of the dataServer is that it polls the contentServer in order to\n");
      printf("   find out which grid files it should keep available. For example, when a new file \n");
      printf("   information is added to the contentServer, the dataServer gets an event for this\n");
      printf("   operation. The dataServer reads the content information of the this new grid file\n");
      printf("   and registers it to the contentServer. After that all contentServer users should\n");
      printf("   know that the current grid file data is available in the current dataServer.\n");
      printf("   This grid data can be fetched from the dataServer by using its CORBA interface.\n");
      printf("\n");
      printf("   When the dataServer is started, it register itself to the contentServer. When the\n");
      printf("   the dataServer is stopped it removes this registration and all content registrations.\n");
      printf("   made for this server.\n");
      printf("\n");
      printf(" USAGE:\n");
      printf("   corbaDataServer <configurationFile>\n");
      printf("\n");
      printf("##################################################################################\n");
      printf("\n");
      return -1;
    }

    readConfigFile(argv[1]);

    Identification::gridDef.init(mGridConfigFile.c_str());
    GRID::valueCache.init(mNumOfCachedGrids,mMaxCompressedMegaBytesOfCachedGrids,mMaxUncompressedMegaBytesOfCachedGrids);


    signal(SIGINT, sig_handler);

    std::string virtualFileDef;
    std::vector<std::string> luaFiles;
    Log debugLog;
    Log processingLog;

    dataServer = new DataServer::ServiceImplementation();

    corbaServer = new DataServer::Corba::Server(mServerAddress.c_str(),mServerPort.c_str());
    corbaServer->init(dataServer);

    ContentServer::Corba::ClientImplementation contentServerClient;
    contentServerClient.init(mContentSourceIor.c_str());

    if (mProcessingLogEnabled)
    {
      mProcessingLog.init(true,mProcessingLogFile.c_str(),mProcessingLogMaxSize,mProcessingLogTruncateSize);
      dataServer->setProcessingLog(&mProcessingLog);
    }

    if (mDebugLogEnabled)
    {
      mDebugLog.init(true,mDebugLogFile.c_str(),mDebugLogMaxSize,mDebugLogTruncateSize);
      dataServer->setDebugLog(&mDebugLog);
    }


    dataServer->init(0,mServerId,mServerName.c_str(),corbaServer->getServiceIor().c_str(),mGridDirectory.c_str(),&contentServerClient,mLuaFiles);
    dataServer->enableContentPreload(mGridPreloadEnabled);
    dataServer->enableVirtualContent(mVirtualFilesEnabled);

    if (mVirtualFileDefinitions.length() > 0)
    {
      DataServer::VirtualContentFactory_type1 *factory = new DataServer::VirtualContentFactory_type1();
      factory->init(mVirtualFileDefinitions);

      dataServer->addVirtualContentFactory(factory);
    }

    dataServer->startEventProcessing();

    // Let's print the service IOR. This is necessary for accessing the service. Usually the best way
    // to handle an IOR is to store it into an environment variable.

    std::string ior = corbaServer->getServiceIor();
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

    corbaServer->run();

    delete dataServer;
    delete corbaServer;

    return 0;
  }
  catch (...)
  {
    SmartMet::Spine::Exception exception(BCP,exception_operation_failed,NULL);
    exception.printError();
    return -1;
  }
}
