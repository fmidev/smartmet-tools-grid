#include "grid-content/contentServer/corba/client/ClientImplementation.h"
#include "grid-content/contentServer/http/client/ClientImplementation.h"
#include "grid-content/contentServer/redis/RedisImplementation.h"
#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"

using namespace SmartMet;


int main(int argc, char *argv[])
{
  try
  {
    char *serviceIor = getenv("SMARTMET_CS_IOR");
    if (serviceIor == nullptr)
    {
      fprintf(stdout,"SMARTMET_CS_IOR not defined!\n");
      return -2;
    }


    if (argc < 3)
    {
      fprintf(stdout,"USAGE: cs_saveData <sessionId> <directory>\n");
      fprintf(stdout,"  [[-http <url>]|[-redis <address> <port> <tablePrefix>]|[-ior <ior>]]\n");
      return -1;
    }

    // ### Session:
    T::SessionId sessionId = (SmartMet::T::SessionId)atoll(argv[1]);
    std::string dir = argv[2];

    ContentServer::ServiceInterface *service = nullptr;
    ContentServer::HTTP::ClientImplementation httpClient;
    ContentServer::RedisImplementation redisClient;
    ContentServer::Corba::ClientImplementation corbaClient;

    if (strcmp(argv[argc-2],"-http") == 0)
    {
      httpClient.init(argv[argc-1]);
      service = &httpClient;
    }
    else
    if (argc > 4  &&  strcmp(argv[argc-4],"-redis") == 0)
    {
      redisClient.init(argv[argc-3],atoi(argv[argc-2]),argv[argc-1]);
      service = &redisClient;
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

      corbaClient.init(serviceIor);
      service = &corbaClient;
    }

    if (service == nullptr)
    {
      fprintf(stdout,"Service not defined!\n");
      return -2;
    }


    unsigned long long startTime = getTime();


    // Reading data server information

    T::ServerInfoList serverInfoList;
    int result = service->getDataServerInfoList(sessionId,serverInfoList);
    if (result != 0)
    {
      fprintf(stdout,"ERROR (%d) : %s\n",result,ContentServer::getResultString(result).c_str());
      return -4;
    }
    serverInfoList.writeToFile(dir + "/dataServers.csv");


    // Reading producer information

    T::ProducerInfoList producerInfoList;
    result = service->getProducerInfoList(sessionId,producerInfoList);
    if (result != 0)
    {
      fprintf(stdout,"ERROR (%d) : %s\n",result,ContentServer::getResultString(result).c_str());
      return -5;
    }
    producerInfoList.writeToFile(dir + "/producers.csv");


    // Reading generation information

    T::GenerationInfoList generationInfoList;
    result = service->getGenerationInfoList(sessionId,generationInfoList);
    if (result != 0)
    {
      fprintf(stdout,"ERROR (%d) : %s\n",result,ContentServer::getResultString(result).c_str());
      return -6;
    }
    generationInfoList.writeToFile(dir + "/generations.csv");


    // Reading file information

    bool ind = true;
    uint startFileId = 0;
    while (ind)
    {
      T::FileInfoList fileInfoList;
      result = service->getFileInfoList(sessionId,startFileId,10000,fileInfoList);
      if (result != 0)
      {
        fprintf(stdout,"ERROR (%d) : %s\n",result,ContentServer::getResultString(result).c_str());
        return -7;
      }

      if (startFileId == 0)
        fileInfoList.writeToFile(dir + "/files.csv","w");
      else
        fileInfoList.writeToFile(dir + "/files.csv","a");

      uint len = fileInfoList.getLength();
      if (len == 0)
      {
        ind = false;
      }
      else
      {
        T::FileInfo *fileInfo = (T::FileInfo*)fileInfoList.getFileInfoByIndex(len-1);
        startFileId = fileInfo->mFileId + 1;
      }
    }


    // Reading content information

    ind = true;
    startFileId = 0;
    uint startMessageIndex = 0;
    while (ind)
    {
      T::ContentInfoList contentInfoList;
      result = service->getContentList(sessionId,startFileId,startMessageIndex,10000,contentInfoList);
      if (result != 0)
      {
        fprintf(stdout,"ERROR (%d) : %s\n",result,ContentServer::getResultString(result).c_str());
        return -8;
      }

      if (startFileId == 0)
        contentInfoList.writeToFile(dir + "/content.csv","w");
      else
        contentInfoList.writeToFile(dir + "/content.csv","a");

      uint len = contentInfoList.getLength();
      if (len == 0)
      {
        ind = false;
      }
      else
      {
        T::ContentInfo *contentInfo = (T::ContentInfo*)contentInfoList.getContentInfoByIndex(len-1);
        startFileId = contentInfo->mFileId;
        startMessageIndex = contentInfo->mMessageIndex + 1;
      }
    }

    unsigned long long endTime = getTime();

    printf("\nTIME : %f sec\n\n",(float)(endTime-startTime)/1000000);

    return 0;
  }
  catch (SmartMet::Spine::Exception& e)
  {
    SmartMet::Spine::Exception exception(BCP,"Service call failed!",nullptr);
    exception.printError();
    return -10;
  }
}

