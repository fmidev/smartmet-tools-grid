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


    T::SessionId sessionId = toInt64(argv[1]);
    char *sourceDir = argv[2];
    uint sourceId = toInt64(argv[3]);
    uint targetId = toInt64(argv[4]);

    ContentServer::ServiceInterface *service = nullptr;
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
      redisClient.init(argv[argc-3],toInt64(argv[argc-2]),argv[argc-1]);
      service = &redisClient;
    }
    else
    {
      char *serviceIor = getenv("SMARTMET_CS_IOR");

      if (strcmp(argv[argc-2],"-ior") == 0)
        serviceIor = argv[argc-1];

      if (serviceIor == nullptr)
      {
        fprintf(stdout,"Service IOR not defined!\n");
        return -2;
      }

      corbaClient.init(serviceIor);
      service = &corbaClient;
    }

    if (service == nullptr)
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
    SmartMet::Spine::Exception exception(BCP,"Service call failed!",nullptr);
    exception.printError();
    return -4;
  }
}

