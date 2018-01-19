#include "contentServer/cache/CacheImplementation.h"
#include "contentServer/redis/RedisImplementation.h"
#include "contentServer/corba/client/ClientImplementation.h"
#include "contentServer/corba/server/Server.h"
#include "grid-files/common/Exception.h"
#include "contentServer/corba/server/ServerInterface.h"
#include "grid-files/identification/GridDef.h"
#include <signal.h>

using namespace SmartMet;


ContentServer::Corba::Server *server = NULL;
ContentServer::CacheImplementation *cacheImplementation = NULL;

bool shutdownRequested = false;


void sig_handler(int signum)
{
  {
    try
    {
      if (shutdownRequested)
        sprintf(NULL,"Crashing the system for the core dump");

      if (cacheImplementation != NULL)
      {
        printf("\n**** SHUTTING DOWN ****\n");
        shutdownRequested = true;
        sleep(2);
        cacheImplementation->shutdown();
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




int main(int argc, char *argv[])
{
  try
  {
    if (argc < 4)
    {
      printf("\n");
      printf("##################################################################################\n");
      printf("                        cacheContentServer_corba2corba\n");
      printf("##################################################################################\n");
      printf(" DESCRIPTION:\n");
      printf("   This is a contentServer that offers a CORBA service interface for accessing\n");
      printf("   its services. The actual content information is stored into another content\n");
      printf("   server which is also accessed with CORBA. This server acts like a memory-based\n");
      printf("   cache on the front of the other contentServer.This service makes the content\n");
      printf("   information queries much faster, because the cache sorts content information\n");
      printf("   so that it can be fetched as fast possible.\n");
      printf("\n");
      printf("   Notice that this service does not care how the other contentServer is implemented\n");
      printf("   i.e. it only sees its CORBA service interface. This means that the other content\n");
      printf("   server can be for example a Redis-based contentServer or just another caching\n");
      printf("   contentServer. This makes it possible to build a contentServer chain or hierarchy.\n");
      printf("\n");
      printf("   When the server starts, it prints its IOR (International Object Reference) \n");
      printf("   on the screen. You should store this into the environment variable SMARTMET_CS_IOR\n");
      printf("   so that client programs can access this service easily. Notice that the IOR\n");
      printf("   does not change as long as the server address and ports do not change. I.e. you\n");
      printf("   can store it in your shell start-up scripts (like '.bashrc').\n");
      printf("\n");
      printf(" USAGE:\n");
      printf("   cacheContentServer_corba2corba <corbaAddress> <corbaPort> <contentServerIor>\n");
      printf("     [-plog processingLogFile] [-dlog debugLogFile] [-lua luaFile]\n");
      printf("\n");
      printf(" WHERE:\n");
      printf("   <corbaAddress>     => The IP address of the server.\n");
      printf("   <corbaPort>        => The TCP port of the server.\n");
      printf("   <contentServerIor> => The IOR of the ContentServer.\n");
      printf("##################################################################################\n");
      printf("\n");
      return -1;
    }

    signal(SIGINT, sig_handler);

    T::SessionId sessionId = 0;
    char *corbaAddress = (char*)argv[1];
    char *corbaPort = (char*)argv[2];
    char *contentServerIor = (char*)argv[3];

    cacheImplementation = new ContentServer::CacheImplementation();

    server = new ContentServer::Corba::Server(corbaAddress,corbaPort);
    server->init(cacheImplementation);

    ContentServer::Corba::ClientImplementation contentServerClient;
    contentServerClient.init(contentServerIor);

    cacheImplementation->init(sessionId,&contentServerClient);
    cacheImplementation->startEventProcessing();

    Log processingLog;
    Log debugLog;

    for (int t=4; t<argc; t++)
    {
      if (strcmp(argv[t],"-plog") == 0  &&  (t+1 < argc))
      {
        processingLog.init(true,argv[t+1],10000000,5000000);
        cacheImplementation->setProcessingLog(&processingLog);
      }

      if (strcmp(argv[t],"-dlog") == 0  &&  (t+1 < argc))
      {
        debugLog.init(true,argv[t+1],10000000,5000000);
        cacheImplementation->setDebugLog(&debugLog);
      }
    }

    // Let's print the service IOR. This is necessary for accessing the service. Usually the best way
    // to handle an IOR is to store it into an environment variable.

    std::string ior = server->getServiceIor();
    printf("\n%s\n",ior.c_str());

    server->run();

    delete cacheImplementation;

    return 0;
  }
  catch (SmartMet::Spine::Exception& e)
  {
    SmartMet::Spine::Exception exception(BCP,exception_operation_failed,NULL);
    exception.printError();
    return -1;
  }
}
