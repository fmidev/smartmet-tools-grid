/*! \file
 *  \brief Content Server client: reads a file list with embedded content records from a local file and registers all file and content entries in the Content Server.
 */

#include "grid-content/contentServer/corba/client/ClientImplementation.h"
#include "grid-content/contentServer/http/client/ClientImplementation.h"
#include "grid-content/contentServer/postgresql/PostgresqlImplementation.h"
#include "grid-content/contentServer/redis/RedisImplementation.h"
#include <macgyver/Exception.h>
#include "grid-files/common/GeneralFunctions.h"

using namespace SmartMet;


/*! \brief Splits the line in place into fields separated by ';' or newline. Returns the
    number of separators found (at most 100). */

uint splitFields(char *st,char *field[100])
{
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
  return c;
}



/*! \brief Parses the file information and returns the content information of a record. */

T::ContentInfo* parseRecord(char *field[],T::FileInfo& fileInfo)
{
  fileInfo.mFileId = 0;
  fileInfo.mFileType = toInt64(field[0]);
  fileInfo.mProducerId = toInt64(field[1]);
  fileInfo.mGenerationId = toUInt64(field[2]);
  fileInfo.mName = field[3];
  fileInfo.mFlags = toInt64(field[4]);

  auto *contentInfo = new T::ContentInfo();
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
  return contentInfo;
}



/*! \brief Creates the content server client selected by the command line options. Returns
    nullptr if the CORBA service IOR is not defined. */

ContentServer::ServiceInterface* createService(int argc,char *argv[])
{
  if (argc == 5  &&  strcmp(argv[3],"-http") == 0)
  {
    auto *httpService = new ContentServer::HTTP::ClientImplementation();
    httpService->init(argv[4]);
    return httpService;
  }
  else if (argc > 5  &&  strcmp(argv[argc-5],"-redis") == 0)
  {
    auto *redis = new ContentServer::RedisImplementation();
    redis->init(argv[argc-4],toInt64(argv[argc-3]),argv[argc-2],argv[argc-1]);
    return redis;
  }
  else if (strcmp(argv[argc-2],"-pg") == 0)
  {
    auto *pg = new ContentServer::PostgresqlImplementation();
    pg->init(argv[argc-1],"",false);
    return pg;
  }
  else
  {
    char *serviceIor = getenv("SMARTMET_CS_IOR");

    if (strcmp(argv[argc-2],"-ior") == 0)
      serviceIor = argv[argc-1];

    if (serviceIor == nullptr)
    {
      fprintf(stdout,"Service IOR not defined!\n");
      return nullptr;
    }

    auto *corbaService = new ContentServer::Corba::ClientImplementation();
    corbaService->init(serviceIor);
    return corbaService;
  }
}



/*! \brief Program entry point. */

int main(int argc, char *argv[])
{
  try
  {
    if (argc < 3)
    {
      fprintf(stdout,"USAGE: cs_addFileAndContentListFromFile <sessionId> <filename> [[-http <url>]|[-redis <address> <port> <tablePrefix> <password>]]\n");
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


    ContentServer::ServiceInterface *service = createService(argc,argv);
    if (service == nullptr)
    {
      (void)fclose(file);
      return -2;
    }


    T::FileInfo prevFileInfo;
    T::ContentInfoList contentList;
    uint count = 0;

    char st[1000];
    while (fgets(st,1000,file) != nullptr)
    {
      count++;
        if ((count % 1000) == 0)
          printf("%u\n",count);

        char *field[100];
        uint c = splitFields(st,field);

        if (c >= 12)
        {
          T::FileInfo fileInfo;
          auto *contentInfo = parseRecord(field,fileInfo);


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

    (void)fclose(file);

    if (contentList.getLength() > 0)
    {
      /*int result = */service->addFileInfoWithContentList(sessionId,prevFileInfo,contentList);
    }

    delete service;

    return 0;
  }
  catch (Fmi::Exception&)
  {
    Fmi::Exception exception(BCP,"Service call failed!",nullptr);
    exception.printError();
    return -4;
  }
}

