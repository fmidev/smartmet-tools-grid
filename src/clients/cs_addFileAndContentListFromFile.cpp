#include "grid-content/contentServer/corba/client/ClientImplementation.h"
#include "grid-content/contentServer/http/client/ClientImplementation.h"
#include "grid-content/contentServer/postgresql/PostgresqlImplementation.h"
#include "grid-content/contentServer/redis/RedisImplementation.h"
#include <macgyver/Exception.h>
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

    T::SessionId sessionId = toInt64(argv[1]);
    char* filename = argv[2];


    FILE *file = fopen(filename,"re");
    if (file == nullptr)
    {
      fprintf(stdout,"Cannot open file (%s) for reading!\n",filename);
      return -3;
    }


    ContentServer::ServiceInterface *service = nullptr;

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
      redis->init(argv[argc-3],toInt64(argv[argc-2]),argv[argc-1]);
      service = redis;
    }
    else
    if (strcmp(argv[argc-2],"-pg") == 0)
    {
      ContentServer::PostgresqlImplementation *pg = new ContentServer::PostgresqlImplementation();
      pg->init(argv[argc-1],"",false);
      service = pg;
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
      if (fgets(st,1000,file) != nullptr)
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

        if (c >= 12)
        {
          T::FileInfo fileInfo;
          fileInfo.mFileId = 0;
          fileInfo.mFileType = toInt64(field[0]);
          fileInfo.mProducerId = toInt64(field[1]);
          fileInfo.mGenerationId = toInt64(field[2]);
          fileInfo.mName = field[3];
          fileInfo.mFlags = toInt64(field[4]);

          T::ContentInfo *contentInfo = new T::ContentInfo();
          contentInfo->mFileId = 0;
          contentInfo->mFileType = fileInfo.mFileType;
          contentInfo->mProducerId = fileInfo.mProducerId;
          contentInfo->mGenerationId = fileInfo.mGenerationId;
          contentInfo->mMessageIndex = toInt64(field[5]);
          contentInfo->setForecastTime(field[6]);
          contentInfo->mFmiParameterId = toUInt32(field[7]);
          contentInfo->setFmiParameterName(field[8]);
          //contentInfo->mGribParameterId = toUInt32(field[9]);
          contentInfo->mFmiParameterLevelId = toInt64(field[9]);
          //contentInfo->mGrib1ParameterLevelId = toInt64(field[11]);
          // contentInfo->mGrib2ParameterLevelId = toInt64(field[12]);
          contentInfo->mParameterLevel = (T::ParamLevel)toInt64(field[10]);
          contentInfo->mFlags = toInt64(field[11]);


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
  catch (Fmi::Exception& e)
  {
    Fmi::Exception exception(BCP,"Service call failed!",nullptr);
    exception.printError();
    return -4;
  }
}

