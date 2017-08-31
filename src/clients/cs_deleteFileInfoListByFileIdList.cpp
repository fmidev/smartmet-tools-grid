#include "contentServer/corba/client/ClientImplementation.h"
#include "contentServer/http/client/ClientImplementation.h"
#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"

using namespace SmartMet;


int main(int argc, char *argv[])
{
  try
  {
    if (argc < 3)
    {
      fprintf(stdout,"USAGE: cs_deleteFileInfoListByFileIdList <sessionId> <fileId1> [<fileId2> .. <fileIdN>] [-http <url>]\n");
      return -1;
    }

    T::SessionId sessionId = (SmartMet::T::SessionId)atoll(argv[1]);

    int result = 0;
    unsigned long long startTime = 0;
    unsigned long long endTime = 0;
    std::set<uint> fileIdList;

    for (int t=2; t<argc-2; t++)
      fileIdList.insert((uint)atoll(argv[t]));

    if (strcmp(argv[argc-2],"-http") == 0)
    {
      ContentServer::HTTP::ClientImplementation service;
      service.init(argv[argc-1]);

      startTime = getTime();
      result = service.deleteFileInfoListByFileIdList(sessionId,fileIdList);
      endTime = getTime();
    }
    else
    {
      char *serviceIor = getenv("SMARTMET_CS_IOR");
      if (serviceIor == NULL)
      {
        fprintf(stdout,"SMARTMET_CS_IOR not defined!\n");
        return -2;
      }

      ContentServer::Corba::ClientImplementation service;
      service.init(serviceIor);

      startTime = getTime();
      result = service.deleteFileInfoListByFileIdList(sessionId,fileIdList);
      endTime = getTime();
    }

    if (result != 0)
    {
      fprintf(stdout,"ERROR (%d) : %s\n",result,ContentServer::getResultString(result).c_str());
      return -3;
    }

    // ### Result:
    printf("OK\n");

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

