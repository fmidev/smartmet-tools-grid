#include "grid-content/contentServer/corba/client/ClientImplementation.h"
#include "grid-content/contentServer/http/client/ClientImplementation.h"
#include "grid-content/contentServer/redis/RedisImplementation.h"
#include "grid-content/contentServer/sync/ContentSync.h"
#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"

using namespace SmartMet;


int main(int argc, char *argv[])
{
  try
  {
    if (argc < 5)
    {
      fprintf(stdout,"USAGE: cs_syncContentFromCsv <sessionId> <sourceDir> <sourceId> <targetId>\n");
      fprintf(stdout,"  [[-http <url>]|[-redis <address> <port> <tablePrefix>]|[-ior <ior>]]\n");
      return -1;
    }


    T::SessionId sessionId = (SmartMet::T::SessionId)atoll(argv[1]);
    char *sourceDir = argv[2];
    uint sourceId = (uint)atoll(argv[3]);
    uint targetId = (uint)atoll(argv[4]);

    ContentServer::ServiceInterface *service = NULL;
    ContentServer::HTTP::ClientImplementation httpClient;
    ContentServer::RedisImplementation redisClient;
    ContentServer::Corba::ClientImplementation corbaClient;

    if (strcmp(argv[argc-2],"-http") == 0)
    {
      httpClient.init(argv[argc-1]);
      service = &httpClient;
    }
    else
    if (argc > 4  &&  strcmp(argv[argc-4],"-redis") == 0)
    {
      redisClient.init(argv[argc-3],atoi(argv[argc-2]),argv[argc-1]);
      service = &redisClient;
    }
    else
    {
      char *serviceIor = getenv("SMARTMET_CS_IOR");

      if (strcmp(argv[argc-2],"-ior") == 0)
        serviceIor = argv[argc-1];

      if (serviceIor == NULL)
      {
        fprintf(stdout,"Service IOR not defined!\n");
        return -2;
      }

      corbaClient.init(serviceIor);
      service = &corbaClient;
    }

    if (service == NULL)
    {
      fprintf(stdout,"Service not defined!\n");
      return -2;
    }


    ContentServer::ContentSync sync;

    unsigned long long startTime = getTime();
    sync.synchronize(sessionId,sourceDir,service,sourceId,targetId);
    unsigned long long endTime = getTime();


    printf("\nTIME : %f sec\n\n",(float)(endTime-startTime)/1000000);

    return 0;
  }
  catch (SmartMet::Spine::Exception& e)
  {
    SmartMet::Spine::Exception exception(BCP,"Service call failed!",NULL);
    exception.printError();
    return -4;
  }
}

