#include "contentServer/corba/client/ClientImplementation.h"
#include "contentServer/http/client/ClientImplementation.h"
#include "contentServer/sync/ContentSync.h"
#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"

using namespace SmartMet;


int main(int argc, char *argv[])
{
  try
  {
    if (argc != 6)
    {
      fprintf(stdout,"USAGE: cs_syncContentFromCsv <sessionId> <sourceDir> <targetIor> <sourceId> <targetId>\n");
      return -1;
    }

    T::SessionId sessionId = (SmartMet::T::SessionId)atoll(argv[1]);
    char *sourceDir = argv[2];
    char *targetIor = argv[3];
    uint sourceId = (uint)atoll(argv[4]);
    uint targetId = (uint)atoll(argv[5]);

    ContentServer::Corba::ClientImplementation target;
    target.init(targetIor);

    ContentServer::ContentSync sync;

    unsigned long long startTime = getTime();
    sync.synchronize(sessionId,sourceDir,&target,sourceId,targetId);
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

