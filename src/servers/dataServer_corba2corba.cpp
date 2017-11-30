#include "contentServer/cache/CacheImplementation.h"
#include "contentServer/corba/client/ClientImplementation.h"
#include "dataServer/implementation/ServiceImplementation.h"
#include "dataServer/implementation/VirtualContentFactory_type1.h"
#include "dataServer/corba/server/ServerInterface.h"
#include "dataServer/corba/server/Server.h"
#include "grid-files/common/Exception.h"
#include "grid-files/grid/ValueCache.h"
#include "grid-files/identification/GribDef.h"
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
ContentServer::CacheImplementation *cacheContentServer = new ContentServer::CacheImplementation();

bool shutdownRequested = false;



void sig_handler(int signum)
{
  {
    try
    {
      if (shutdownRequested)
        sprintf(NULL,"Crashing the system for the core dump");

      if (dataServer != NULL)
      {
        printf("\n**** SHUTTING DOWN ****\n");
        shutdownRequested = true;
        sleep(2);
        cacheContentServer->shutdown();
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





void init()
{
  try
  {
    // ### Initializing global structures.


    // 1. Initializing information that is needed for identifying the content of the grid files.

    char *configDir = getenv("SMARTMET_GRID_CONFIG_DIR");
    if (configDir == NULL)
    {
      printf("SMARTMET_GRID_CONFIG_DIR not defined!\n");
      exit(-1);
    }

    SmartMet::Identification::gribDef.init(configDir);


    // 2. Initializing the size of the grid value cache.

    GRID::valueCache.init(8000,10000,10000);

  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,NULL);
  }
}





int main(int argc, char *argv[])
{
  try
  {
    if (argc < 8)
    {
      printf("\n");
      printf("##################################################################################\n");
      printf("                        dataServer_corba2corba\n");
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
      printf("   dataServer_corba2corba <serverId> <serverName> <gridFileDir> <corbaAddress> <corbaPort> <contentServerIor> <conversionFile> [-log logFile] [-lua luaFile]\n");
      printf("\n");
      printf(" WHERE:\n");
      printf("   <serverId>         => Unique server identifier used for content registration,\n");
      printf("                         which value should be in range [1..64].\n");
      printf("   <serverName>       => Unique server name used when the server is registered to\n");
      printf("                         the contentServer.\n");
      printf("   <gridFileDir>      => The main directory of the grid file. The filenames used\n");
      printf("                         the contentServer should not contain this directory path.\n");
      printf("   <corbaAddress>     => The IP address of the server.\n");
      printf("   <corbaPort>        => The TCP port of the server.\n");
      printf("   <contentServerIor> => The IOR of the ContentServer.\n");
      printf("   <conversionFile>   => Conversion definitions.\n");
      printf("##################################################################################\n");
      printf("\n");
      return -1;
    }

    init();

    signal(SIGINT, sig_handler);

    uint serverId = (uint)atoi(argv[1]);
    T::SessionId sessionId = 0;
    char *serverName = (char*)argv[2];
    char *gridFileDir = (char*)argv[3];
    char *corbaAddress = (char*)argv[4];
    char *corbaPort = (char*)argv[5];
    char *contentServerIor = (char*)argv[6];
    char *conversionFile = (char*)argv[7];
    std::vector<std::string> luaFiles;
    Log processingLog;

    dataServer = new DataServer::ServiceImplementation();

    corbaServer = new DataServer::Corba::Server(corbaAddress,corbaPort);
    corbaServer->init(dataServer);


    for (int t=7; t<argc; t++)
    {
      if (strcmp(argv[t],"-log") == 0  && (t+1) < argc)
      {
        processingLog.init(true,argv[t+1],10000000,5000000);
        dataServer->setProcessingLog(&processingLog);
      }

      if (strcmp(argv[t],"-lua") == 0  && (t+1) < argc)
      {
        luaFiles.push_back(std::string(argv[t+1]));
      }
    }



    ContentServer::Corba::ClientImplementation contentServerClient;
    contentServerClient.init(contentServerIor);

    cacheContentServer = new ContentServer::CacheImplementation();
    //cacheContentServer->init(sessionId,&contentServerClient);
    //cacheContentServer->startEventProcessing();


    dataServer->init(sessionId,serverId,serverName,corbaServer->getServiceIor().c_str(),gridFileDir,&contentServerClient,luaFiles);

    DataServer::VirtualContentFactory_type1 *factory = new DataServer::VirtualContentFactory_type1();
    factory->init(conversionFile);

    dataServer->addVirtualContentFactory(factory);


    dataServer->startEventProcessing();

    std::string ior = corbaServer->getServiceIor();
    printf("\n%s\n",ior.c_str());

    corbaServer->run();

    delete dataServer;
    return 0;
  }
  catch (...)
  {
    SmartMet::Spine::Exception exception(BCP,exception_operation_failed,NULL);
    exception.printError();
    return -1;
  }
}
