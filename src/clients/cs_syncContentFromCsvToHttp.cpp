#include "grid-content/contentServer/http/client/ClientImplementation.h"
#include "grid-content/contentServer/sync/ContentSync.h"
#include <macgyver/Exception.h>
#include "grid-files/common/GeneralFunctions.h"

using namespace SmartMet;


int main(int argc, char *argv[])
{
  try
  {
    if (argc != 6)
    {
      fprintf(stdout,"USAGE: cs_syncContentFromCsv <sessionId> <sourceDir> <targetUrl> <sourceId> <targetId>\n");
      return -1;
    }

    T::SessionId sessionId = toInt64(argv[1]);
    char *sourceDir = argv[2];
    char *targetUrl = argv[3];
    uint sourceId = toInt64(argv[4]);
    uint targetId = toInt64(argv[5]);

    ContentServer::HTTP::ClientImplementation target;
    target.init(targetUrl);


    ContentServer::ContentSync sync;

    unsigned long long startTime = getTime();
    sync.synchronize(sessionId,sourceDir,&target,sourceId,targetId);
    unsigned long long endTime = getTime();


    printf("\nTIME : %f sec\n\n",(float)(endTime-startTime)/1000000);

    return 0;
  }
  catch (Fmi::Exception& e)
  {
    Fmi::Exception exception(BCP,"Service call failed!",nullptr);
    exception.printError();
    return -4;
  }
}

