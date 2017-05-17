#include "contentServer/corba/client/ClientImplementation.h"
#include "contentServer/http/client/ClientImplementation.h"
#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"

using namespace SmartMet;


int main(int argc, char *argv[])
{
  try
  {
    if (argc < 6)
    {
      fprintf(stdout,"USAGE: cs_getContentListByProducerName <sessionId> <producerName> <startFileId> <startMessageIndex> <maxRecords> [-http <url>]\n");
      return -1;
    }

    T::SessionId sessionId = (SmartMet::T::SessionId)atoll(argv[1]);
    T::ContentInfoList infoList;
    std::string producerName = argv[2];
    uint startFileId = (uint)atoll(argv[3]);
    uint startMessageIndex = (uint)atoll(argv[4]);
    uint maxRecords = (uint)atoll(argv[5]);

    int result = 0;
    unsigned long long startTime = 0;
    unsigned long long endTime = 0;

    if (argc == 8  &&  strcmp(argv[6],"-http") == 0)
    {
      ContentServer::HTTP::ClientImplementation service;
      service.init(argv[7]);

      startTime = getTime();
      result = service.getContentListByProducerName(sessionId,producerName,startFileId,startMessageIndex,maxRecords,infoList);
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
      result = service.getContentListByProducerName(sessionId,producerName,startFileId,startMessageIndex,maxRecords,infoList);
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

