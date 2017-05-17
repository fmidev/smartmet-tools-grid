#include "contentServer/corba/client/ClientImplementation.h"
#include "contentServer/http/client/ClientImplementation.h"
#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"

using namespace SmartMet;


int main(int argc, char *argv[])
{
  try
  {
    if (argc < 5)
    {
      fprintf(stdout,"USAGE: cs_getFileInfoListByGenerationName <sessionId> <generationName> <startFileId> <maxRecords> [-http <url>]\n");
      return -1;
    }

    T::SessionId sessionId = (SmartMet::T::SessionId)atoll(argv[1]);
    T::FileInfoList infoList;
    std::string generationName = argv[2];
    uint startFileId = (uint)atoll(argv[3]);
    uint maxRecords = (uint)atoll(argv[4]);

    int result = 0;
    unsigned long long startTime = 0;
    unsigned long long endTime = 0;

    if (argc == 7  &&  strcmp(argv[5],"-http") == 0)
    {
      ContentServer::HTTP::ClientImplementation service;
      service.init(argv[6]);

      startTime = getTime();
      result = service.getFileInfoListByGenerationName(sessionId,generationName,startFileId,maxRecords,infoList);
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
      result = service.getFileInfoListByGenerationName(sessionId,generationName,startFileId,maxRecords,infoList);
      endTime = getTime();
    }

    if (result != 0)
    {
      fprintf(stdout,"ERROR (%d) : %s\n",result,ContentServer::getResultString(result).c_str());
      return -3;
    }

    // ### Result:
    infoList.print(std::cout,0,0);

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

