#include "grid-content/contentServer/corba/client/ClientImplementation.h"
#include "grid-content/contentServer/http/client/ClientImplementation.h"
// #include "grid-content/contentServer/postgres/PostgresImplementation.h"
#include "grid-content/contentServer/redis/RedisImplementation.h"
#include <macgyver/Exception.h>
#include "grid-files/common/GeneralFunctions.h"

using namespace SmartMet;



int main(int argc, char *argv[])
{
  try
  {
    if (argc < 2)
    {
      fprintf(stdout,"USAGE: cs_getFileInfoCountsByProducers <sessionId> [[-http <url>]|[-redis <address> <port> <tablePrefix>]]\n");
      return -1;
    }

    T::SessionId sessionId = toInt64(argv[1]);
    T::ProducerInfoList infoList;
    int result = 0;
    unsigned long long startTime = 0;
    unsigned long long endTime = 0;

    if (strcmp(argv[argc-2],"-http") == 0)
    {
      ContentServer::HTTP::ClientImplementation service;
      service.init(argv[argc-1]);

      startTime = getTime();
      result = service.getProducerInfoList(sessionId,infoList);

      uint len = infoList.getLength();
      for (uint t=0; t<len; t++)
      {
        T::ProducerInfo *info = infoList.getProducerInfoByIndex(t);
        uint count = 0;
        if (service.getFileInfoCountByProducerId(sessionId,info->mProducerId,count) == 0)
        {
          printf("%s:%u\n",info->mName.c_str(),count);
        }
      }
      endTime = getTime();
    }
    else
    if (argc == 6  &&  strcmp(argv[argc-4],"-redis") == 0)
    {
      ContentServer::RedisImplementation service;
      service.init(argv[argc-3],toInt64(argv[argc-2]),argv[argc-1]);

      startTime = getTime();
      result = service.getProducerInfoList(sessionId,infoList);

      uint len = infoList.getLength();
      for (uint t=0; t<len; t++)
      {
        T::ProducerInfo *info = infoList.getProducerInfoByIndex(t);
        uint count = 0;
        if (service.getFileInfoCountByProducerId(sessionId,info->mProducerId,count) == 0)
        {
          printf("%s:%u\n",info->mName.c_str(),count);
        }
      }
      endTime = getTime();
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

      ContentServer::Corba::ClientImplementation service;
      service.init(serviceIor);

      startTime = getTime();
      result = service.getProducerInfoList(sessionId,infoList);

      uint len = infoList.getLength();
      for (uint t=0; t<len; t++)
      {
        T::ProducerInfo *info = infoList.getProducerInfoByIndex(t);
        uint count = 0;
        if (service.getFileInfoCountByProducerId(sessionId,info->mProducerId,count) == 0)
        {
          printf("%s:%u\n",info->mName.c_str(),count);
        }
      }
      endTime = getTime();
    }

    if (result != 0)
    {
      fprintf(stdout,"ERROR (%d) : %s\n",result,ContentServer::getResultString(result).c_str());
      return -3;
    }

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

