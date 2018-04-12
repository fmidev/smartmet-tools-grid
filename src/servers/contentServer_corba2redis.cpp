#include "contentServer/redis/RedisImplementation.h"
#include "contentServer/corba/client/ClientImplementation.h"
#include "contentServer/corba/server/Server.h"
#include "grid-files/common/Exception.h"
#include "contentServer/corba/server/ServerInterface.h"
#include "grid-files/identification/GridDef.h"
#include <signal.h>

using namespace SmartMet;


ContentServer::Corba::Server *server = NULL;
ContentServer::RedisImplementation *redisImplementation = NULL;
bool shutdownRequested = false;


void sig_handler(int signum)
{
  {
    try
    {
      if (shutdownRequested)
        sprintf(NULL,"Crashing the system for the core dump");

      if (redisImplementation != NULL)
      {
        printf("\n**** SHUTTING DOWN ****\n");
        shutdownRequested = true;
        sleep(2);
        redisImplementation->shutdown();
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
  return 0;
#if 0
  try
  {
    if (argc < 6)
    {
      printf("\n");
      printf("##################################################################################\n");
      printf("                        contentServer_corba2redis\n");
      printf("##################################################################################\n");
      printf(" DESCRIPTION:\n");
      printf("   This is a contentServer that offers a CORBA service interface for accessing\n");
      printf("   its services. The actual content information is stored into the Redis database.\n");
      printf("\n");
      printf("   When the server starts, it prints its IOR (International Object Reference) \n");
      printf("   on the screen. You should store this into the environment variable SMARTMET_CS_IOR\n");
      printf("   so that client programs can access this service easily. Notice that the IOR\n");
      printf("   does not change as long as the server address and ports do not change. I.e. you\n");
      printf("   can store it in your shell start-up scripts (like '.bashrc').\n");
      printf("\n");
      printf(" USAGE:\n");
      printf("   contentServer_corba2redis <corbaAddress> <corbaPort> <redisAddress> <redisPort> <tablePrefix>\n");
      printf("     [-plog processingLogFile] [-dlog debugLogFile]\n");
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

    char *corbaAddress = (char*)argv[1];
    char *corbaPort = (char*)argv[2];
    char *redisAddress = (char*)argv[3];
    int redisPort = atoi(argv[4]);
    char *tablePrefix = (char*)argv[5];

    redisImplementation = new ContentServer::RedisImplementation();
    redisImplementation->init(redisAddress,redisPort,tablePrefix);

    server = new ContentServer::Corba::Server(corbaAddress,corbaPort);
    server->init(redisImplementation);

    Log processingLog;
    Log debugLog;

    for (int t=6; t<argc; t++)
    {
      if (strcmp(argv[t],"-plog") == 0  &&  (t+1 < argc))
      {
        processingLog.init(true,argv[t+1],10000000,5000000);
        redisImplementation->setProcessingLog(&processingLog);
      }

      if (strcmp(argv[t],"-dlog") == 0  &&  (t+1 < argc))
      {
        debugLog.init(true,argv[t+1],10000000,5000000);
        redisImplementation->setDebugLog(&debugLog);
      }
    }

    // Let's print the service IOR. This is necessary for accessing the service. Usually the best way
    // to handle an IOR is to store it into an environment variable.

    std::string ior = server->getServiceIor();
    printf("\n%s\n",ior.c_str());

    server->run();

    delete redisImplementation;

    return 0;
  }
  catch (SmartMet::Spine::Exception& e)
  {
    SmartMet::Spine::Exception exception(BCP,exception_operation_failed,NULL);
    exception.printError();
    return -1;
  }
#endif
}
