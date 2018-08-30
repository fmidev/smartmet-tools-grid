#include "grid-content/contentServer/redis/RedisImplementation.h"
#include "grid-content/contentServer/sync/ContentSync.h"
#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"

using namespace SmartMet;


int main(int argc, char *argv[])
{
  try
  {
    if (argc != 8)
    {
      fprintf(stdout,"USAGE: cs_syncContentFromCsvToRedis <sessionId> <sourceDir> <redisAddress> <tablePrefix> <redisPort> <sourceId> <targetId>\n");
      return -1;
    }

    T::SessionId sessionId = toInt64(argv[1]);
    char *sourceDir = argv[2];
    char *redisAddress = argv[3];
    uint redisPort = toInt64(argv[4]);
    char *tablePrefix = argv[5];
    uint sourceId = toInt64(argv[6]);
    uint targetId = toInt64(argv[7]);

    ContentServer::RedisImplementation redis;
    redis.init(redisAddress,redisPort,tablePrefix);

    ContentServer::ContentSync sync;

    unsigned long long startTime = getTime();
    sync.synchronize(sessionId,sourceDir,&redis,sourceId,targetId);
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

