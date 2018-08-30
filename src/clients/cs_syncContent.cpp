#include "grid-content/contentServer/corba/client/ClientImplementation.h"
#include "grid-content/contentServer/http/client/ClientImplementation.h"
#include "grid-content/contentServer/sync/ContentSync.h"
#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"

using namespace SmartMet;


int main(int argc, char *argv[])
{
  try
  {
    if (argc != 6)
    {
      fprintf(stdout,"USAGE: cs_syncContent <sessionId> <sourceIor> <targetIor> <sourceId> <targetId>\n");
      return -1;
    }

    T::SessionId sessionId = toInt64(argv[1]);
    char *sourceIor = argv[2];
    char *targetIor = argv[3];
    uint sourceId = toInt64(argv[4]);
    uint targetId = toInt64(argv[5]);

    ContentServer::Corba::ClientImplementation source;
    source.init(sourceIor);

    ContentServer::Corba::ClientImplementation target;
    target.init(targetIor);

    ContentServer::ContentSync sync;

    unsigned long long startTime = getTime();
    sync.synchronize(sessionId,&source,&target,sourceId,targetId);
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

