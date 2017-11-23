#include "contentServer/corba/client/ClientImplementation.h"
#include "dataServer/corba/client/ClientImplementation.h"
#include "queryServer/corba/server/ServerInterface.h"
#include "queryServer/corba/server/Server.h"
#include "queryServer/implementation/ServiceImplementation.h"
#include "grid-files/common/Exception.h"
#include "grid-files/identification/GribDef.h"

#include <iostream>
#include <stdexcept>
#include <string.h>
#include <vector>
#include <pthread.h>
#include <sys/time.h>
#include <signal.h>


using namespace SmartMet;

QueryServer::Corba::Server *corbaServer = NULL;
QueryServer::ServiceImplementation *queryServer = NULL;



bool shutdownRequested = false;


void sig_handler(int signum)
{
  {
    try
    {
      if (shutdownRequested)
        sprintf(NULL,"Crashing the system for the core dump");

      if (queryServer != NULL)
      {
        printf("\n**** SHUTTING DOWN ****\n");
        queryServer->shutdown();
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





int main(int argc, char *argv[])
{
  try
  {
#if 0
    if (argc < 9)
    {
      printf("\n");
      printf("##################################################################################\n");
      printf("                             queryServer\n");
      printf("##################################################################################\n");
      printf(" DESCRIPTION:\n");
      printf("\n");
      printf(" USAGE:\n");
      printf("   queryServer <corbaAddress> <corbaPort> <contentServerIor> <dataServerIor> <parameterFile>\n");
      printf("               <parameterAliasFile> <producerFile> <luaFunctionFile> [-log logFile]\n");
      printf("\n");
      printf(" WHERE:\n");
      printf("   <corbaAddress>       => The IP address of the server.\n");
      printf("   <corbaPort>          => The TCP port of the server.\n");
      printf("   <contentServerIor>   => The IOR of the ContentServer.\n");
      printf("   <dataServerIor>      => The IOR of the DataServer.\n");
      printf("   <parameterFile>      =>\n");
      printf("   <parameterAliasFile> =>\n");
      printf("   <producerFile>       =>\n");
      printf("   <luaFunctionFile>    =>\n");

      printf("##################################################################################\n");
      printf("\n");
      return -1;
    }

    signal(SIGINT, sig_handler);

    char *corbaAddress = (char*)argv[1];
    char *corbaPort = (char*)argv[2];
    char *contentServerIor = (char*)argv[3];
    char *dataServerIor = (char*)argv[4];
    char *parameterFile = (char*)argv[5];
    char *parameterAliasFile = (char*)argv[6];
    char *producerFile = (char*)argv[7];
    char *luaFunctionFile = (char*)argv[8];

    char *configDir = getenv("SMARTMET_GRID_CONFIG_DIR");
    if (configDir == NULL)
    {
      printf("SMARTMET_GRID_CONFIG_DIR not defined!\n");
      exit(-1);
    }

    // Initializing the global structures. These are needed when
    // extracting information from GRIB files.

    SmartMet::Identification::gribDef.init(configDir);

    queryServer = new QueryServer::ServiceImplementation();

    corbaServer = new QueryServer::Corba::Server(corbaAddress,corbaPort);
    corbaServer->init(queryServer);

    ContentServer::Corba::ClientImplementation contentServerClient;
    contentServerClient.init(contentServerIor);

    DataServer::Corba::ClientImplementation dataServerClient;
    dataServerClient.init(dataServerIor);

    queryServer->init (&contentServerClient,&dataServerClient,configDir,parameterFile,parameterAliasFile,producerFile,luaFunctionFile);

    Log processingLog;
    if (argc == 6  && strcmp(argv[8],"-log") == 0)
    {
      processingLog.init(true,argv[9],10000000,5000000);
      queryServer->setProcessingLog(&processingLog);
    }

    // Let's print the service IOR. This is necessary for accessing the service. Usually the best way
    // to handle an IOR is to store it into an environment variable.

    std::string ior = corbaServer->getServiceIor();
    printf("\n%s\n",ior.c_str());

    corbaServer->run();

    delete queryServer;
#endif
    return 0;
  }
  catch (...)
  {
    SmartMet::Spine::Exception exception(BCP,exception_operation_failed,NULL);
    exception.printError();
    return -1;
  }
}
