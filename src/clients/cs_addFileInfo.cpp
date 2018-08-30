#include "grid-content/contentServer/corba/client/ClientImplementation.h"
#include "grid-content/contentServer/http/client/ClientImplementation.h"
#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"
#include "grid-content/contentServer/redis/RedisImplementation.h"


using namespace SmartMet;


int main(int argc, char *argv[])
{
  try
  {
    if (argc < 9)
    {
      fprintf(stdout,"USAGE: cs_addFileInfo <sessionId> <groupFlags> <producerId> <generationId> <fileType> <sourceId> <flags> <filename> [[-http <url>]|[-redis <address> <port> <tablePrefix>]]\n");
      return -1;
    }

    T::FileInfo info;
    T::SessionId sessionId = toInt64(argv[1]);
    info.mGroupFlags  = toInt64(argv[2]);
    info.mProducerId = toInt64(argv[3]);
    info.mGenerationId = toInt64(argv[4]);
    info.mFileType = toInt64(argv[5]);
    info.mSourceId = toInt64(argv[6]);
    info.mFlags = toInt64(argv[7]);
    info.mName = argv[8];

    int result = 0;
    unsigned long long startTime = 0;
    unsigned long long endTime = 0;

    if (argc == 11  &&  strcmp(argv[9],"-http") == 0)
    {
      ContentServer::HTTP::ClientImplementation service;
      service.init(argv[10]);

      startTime = getTime();
      result = service.addFileInfo(sessionId,info);
      endTime = getTime();
    }
    else
    if (argc > 4  &&  strcmp(argv[argc-4],"-redis") == 0)
    {
      ContentServer::RedisImplementation service;
      service.init(argv[argc-3],toInt64(argv[argc-2]),argv[argc-1]);

      startTime = getTime();
      result = service.addFileInfo(sessionId,info);
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
      result = service.addFileInfo(sessionId,info);
      endTime = getTime();
    }

    if (result != 0)
    {
      fprintf(stdout,"ERROR (%d) : %s\n",result,ContentServer::getResultString(result).c_str());
      return -3;
    }

    // ### Result:
    info.print(std::cout,0,0);

    printf("\nTIME : %f sec\n\n",(float)(endTime-startTime)/1000000);

    return info.mFileId;
  }
  catch (SmartMet::Spine::Exception& e)
  {
    SmartMet::Spine::Exception exception(BCP,"Service call failed!",nullptr);
    exception.printError();
    return -4;
  }
}

