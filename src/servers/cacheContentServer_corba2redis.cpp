#include "contentServer/cache/CacheImplementation.h"
#include "contentServer/redis/RedisImplementation.h"
#include "contentServer/corba/client/ClientImplementation.h"
#include "contentServer/corba/server/Server.h"
#include "grid-files/common/Exception.h"
#include "contentServer/corba/server/ServerInterface.h"
#include "grid-files/identification/GribDef.h"
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
    if (argc < 6)
    {
      printf("\n");
      printf("##################################################################################\n");
      printf("                        cacheContentServer_corba2redis\n");
      printf("##################################################################################\n");
      printf(" DESCRIPTION:\n");
      printf("   This is a contentServer that offers a CORBA service interface for accessing\n");
      printf("   its services. The actual content information is stored into the Redis database\n");
      printf("   and this server acts like a memory-based cache on the front of the Redis.\n");
      printf("   In spite of that the Redis database is quite fast this service makes the content\n");
      printf("   information queries much faster, because the cache sorts content information\n");
      printf("   so that it can be fetched as fast possible.\n");
      printf("\n");
      printf("   When the server starts, it prints its IOR (International Object Reference) \n");
      printf("   on the screen. You should store this into the environment variable SMARTMET_CS_IOR\n");
      printf("   so that client programs can access this service easily. Notice that the IOR\n");
      printf("   does not change as long as the server address and ports do not change. I.e. you\n");
      printf("   can store it in your shell start-up scripts (like '.bashrc').\n");
      printf("\n");
      printf(" USAGE:\n");
      printf("   cacheContentServer_corba2redis <corbaAddress> <corbaPort> <redisAddress> <redisPort> <tablePrefix> [-log logFile]\n");
      printf("\n");
      printf(" WHERE:\n");
      printf("   <corbaAddress>    => The IP address of the server.\n");
      printf("   <corbaPort>       => The TCP port of the server.\n");
      printf("   <redisAddress>    => The IP address of the Redis database.\n");
      printf("   <redisPort>       => The TCP port of the Redis database.\n");
      printf("   <tablePrefix>     => The table prefix (=> separates different instances).\n");
      printf("##################################################################################\n");
      printf("\n");
      return -1;
    }

    signal(SIGINT, sig_handler);

    T::SessionId sessionId = 0;
    char *corbaAddress = (char*)argv[1];
    char *corbaPort = (char*)argv[2];
    char *redisAddress = (char*)argv[3];
    int redisPort = atoi(argv[4]);
    char *tablePrefix = (char*)argv[5];

    cacheImplementation = new ContentServer::CacheImplementation();

    server = new ContentServer::Corba::Server(corbaAddress,corbaPort);
    server->init(cacheImplementation);

    ContentServer::RedisImplementation redis;
    redis.init(redisAddress,redisPort,tablePrefix);

    cacheImplementation->init(sessionId,&redis);
    cacheImplementation->startEventProcessing();

    Log processingLog;
    if (argc == 8  && strcmp(argv[6],"-log") == 0)
    {
      processingLog.init(true,argv[7],10000000,5000000);
      cacheImplementation->setProcessingLog(&processingLog);
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
