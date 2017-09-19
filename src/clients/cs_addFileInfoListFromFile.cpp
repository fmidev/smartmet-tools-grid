#include "contentServer/corba/client/ClientImplementation.h"
#include "contentServer/http/client/ClientImplementation.h"
#include "contentServer/redis/RedisImplementation.h"
#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"
#include "string.h"

using namespace SmartMet;


int main(int argc, char *argv[])
{
  try
  {
    char *serviceIor = getenv("SMARTMET_CS_IOR");
    if (serviceIor == NULL)
    {
      fprintf(stdout,"SMARTMET_CS_IOR not defined!\n");
      return -2;
    }


    if (argc < 9)
    {
      fprintf(stdout,"USAGE: cs_addFileInfoListFromFile <sessionId> <groupFlags> <producerId> <generationId> <fileType> <sourceId> <flags> <filename> [[-http <url>]|[-redis <address> <port> <tablePrefix>]]\n");
      return -1;
    }


    T::FileInfo info;
    T::SessionId sessionId = (SmartMet::T::SessionId)atoll(argv[1]);
    info.mGroupFlags  = (uint)atoll(argv[2]);
    info.mProducerId = (uint)atoll(argv[3]);
    info.mGenerationId = (uint)atoll(argv[4]);
    info.mFileType = (T::FileType)atoll(argv[5]);
    info.mSourceId = (uint)atoll(argv[6]);
    info.mFlags = (uint)atoll(argv[7]);

    char* filename = argv[8];

    FILE *file = fopen(filename,"r");
    if (file == NULL)
    {
      fprintf(stdout,"Cannot open file (%s) for reading!\n",filename);
      return -2;
    }

    ContentServer::ServiceInterface *service = NULL;

    if (argc == 11  &&  strcmp(argv[9],"-http") == 0)
    {
      ContentServer::HTTP::ClientImplementation *httpService = new ContentServer::HTTP::ClientImplementation();
      httpService->init(argv[10]);
      service = httpService;
    }
    else
    if (argc > 4  &&  strcmp(argv[argc-4],"-redis") == 0)
    {
      ContentServer::RedisImplementation *redis = new ContentServer::RedisImplementation();
      redis->init(argv[argc-3],atoi(argv[argc-2]),argv[argc-1]);
      service = redis;
    }
    else
    {
      char *serviceIor = getenv("SMARTMET_CS_IOR");
      if (serviceIor == NULL)
      {
        fprintf(stdout,"SMARTMET_CS_IOR not defined!\n");
        return -3;
      }

      ContentServer::Corba::ClientImplementation *corbaService = new ContentServer::Corba::ClientImplementation();
      corbaService->init(serviceIor);
      service = corbaService;
    }


    unsigned long long startTime = getTime();

    char st[1000];
    while (!feof(file))
    {
      if (fgets(st,1000,file) != NULL)
      {
        char *p = strstr(st,"\n");
        if (p != NULL)
          *p = '\0';

        info.mName = st;
        int result = service->addFileInfo(sessionId,info);
        if (result != 0)
        {
          fprintf(stdout,"ERROR (%d) : %s\n",result,ContentServer::getResultString(result).c_str());
          delete service;
          return -4;
        }
      }
    }

    fclose(file);

    unsigned long long endTime = getTime();


    printf("\nTIME : %f sec\n\n",(float)(endTime-startTime)/1000000);

    delete service;

    return 0;
  }
  catch (SmartMet::Spine::Exception& e)
  {
    SmartMet::Spine::Exception exception(BCP,"Service call failed!",NULL);
    exception.printError();
    return -5;
  }
}

