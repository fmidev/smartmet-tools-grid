#include "contentServer/corba/client/ClientImplementation.h"
#include "contentServer/http/client/ClientImplementation.h"
#include "contentServer/redis/RedisImplementation.h"
#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"

using namespace SmartMet;


int main(int argc, char *argv[])
{
  try
  {
    if (argc < 3)
    {
      fprintf(stdout,"USAGE: cs_addFileAndContentListFromFile <sessionId> <filename> [[-http <url>]|[-redis <address> <port> <tablePrefix>]]\n");
      return -1;
    }

    T::SessionId sessionId = (SmartMet::T::SessionId)atoll(argv[1]);
    char* filename = argv[2];


    FILE *file = fopen(filename,"r");
    if (file == NULL)
    {
      fprintf(stdout,"Cannot open file (%s) for reading!\n",filename);
      return -3;
    }


    ContentServer::ServiceInterface *service = NULL;

    if (argc == 5  &&  strcmp(argv[3],"-http") == 0)
    {
      ContentServer::HTTP::ClientImplementation *httpService = new ContentServer::HTTP::ClientImplementation();
      httpService->init(argv[4]);
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

      if (strcmp(argv[argc-2],"-ior") == 0)
        serviceIor = argv[argc-1];

      if (serviceIor == NULL)
      {
        fprintf(stdout,"Service IOR not defined!\n");
        return -2;
      }

      ContentServer::Corba::ClientImplementation *corbaService = new ContentServer::Corba::ClientImplementation();
      corbaService->init(serviceIor);
      service = corbaService;
    }


    T::FileInfo prevFileInfo;
    T::ContentInfoList contentList;
    uint count = 0;

    char st[1000];
    while (!feof(file))
    {
      if (fgets(st,1000,file) != NULL)
      {
        count++;
        if ((count % 1000) == 0)
          printf("%u\n",count);

        char *field[100];
        uint c = 0;
        field[0] = st;
        char *p = st;
        while (*p != '\0'  &&  c < 100)
        {
          if (*p == ';'  ||  *p == '\n')
          {
            *p = '\0';
            p++;
            c++;
            field[c] = p;
          }
          else
          {
            p++;
          }
        }

        if (c >= 17)
        {
          T::FileInfo fileInfo;
          fileInfo.mFileId = 0;
          fileInfo.mFileType = (T::FileType)atoll(field[0]);
          fileInfo.mGroupFlags = (uint)atoll(field[1]);
          fileInfo.mProducerId = (uint)atoll(field[2]);
          fileInfo.mGenerationId = (uint)atoll(field[3]);
          fileInfo.mName = field[4];
          fileInfo.mFlags = (uint)atoll(field[5]);

          T::ContentInfo *contentInfo = new T::ContentInfo();
          contentInfo->mFileId = 0;
          contentInfo->mFileType = fileInfo.mFileType;
          contentInfo->mGroupFlags = fileInfo.mGroupFlags;
          contentInfo->mProducerId = fileInfo.mProducerId;
          contentInfo->mGenerationId = fileInfo.mGenerationId;
          contentInfo->mMessageIndex = (uint)atoll(field[6]);
          contentInfo->mForecastTime = field[7];
          contentInfo->mFmiParameterId = field[8];
          contentInfo->mFmiParameterName = field[9];
          contentInfo->mGribParameterId = field[10];
          contentInfo->mFmiParameterLevelId = (T::ParamLevelId)atoll(field[11]);
          contentInfo->mGrib1ParameterLevelId = (T::ParamLevelId)atoll(field[12]);
          contentInfo->mGrib2ParameterLevelId = (T::ParamLevelId)atoll(field[13]);
          contentInfo->mParameterLevel = (T::ParamLevel)atoll(field[14]);
          contentInfo->mFmiParameterUnits = field[15];
          contentInfo->mGribParameterUnits = field[16];
          contentInfo->mFlags = (uint)atoll(field[17]);


          if (prevFileInfo.mName != fileInfo.mName)
          {
            //printf("** File %s %s\n",prevFileInfo.mName.c_str(),fileInfo.mName.c_str());
            if (contentList.getLength() > 0)
            {
              //printf("-- Add file %s\n",prevFileInfo.mName.c_str());
              /*int result = */service->addFileInfoWithContentList(sessionId,prevFileInfo,contentList);
              prevFileInfo = fileInfo;
              contentList.clear();
              contentList.addContentInfo(contentInfo);
            }
            else
            {
              prevFileInfo = fileInfo;
              contentList.addContentInfo(contentInfo);
            }
          }
          else
          {
            //printf("-- add content %u\n",contentList.getLength());
            contentList.addContentInfo(contentInfo);
          }
        }
      }
    }

    fclose(file);

    if (contentList.getLength() > 0)
    {
      /*int result = */service->addFileInfoWithContentList(sessionId,prevFileInfo,contentList);
    }

    delete service;

    return 0;
  }
  catch (SmartMet::Spine::Exception& e)
  {
    SmartMet::Spine::Exception exception(BCP,"Service call failed!",NULL);
    exception.printError();
    return -4;
  }
}

