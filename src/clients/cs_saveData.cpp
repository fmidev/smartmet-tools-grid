#include "contentServer/corba/client/ClientImplementation.h"
#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"

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


    if (argc != 3)
    {
      fprintf(stdout,"USAGE: cs_saveData <sessionId> <directory>\n");
      return -1;
    }


    // ### Session:
    T::SessionId sessionId = (SmartMet::T::SessionId)atoll(argv[1]);


    // ### Service:
    ContentServer::Corba::ClientImplementation service;
    service.init(serviceIor);

    std::string dir = argv[2];

    unsigned long long startTime = getTime();

    T::ServerInfoList serverInfoList;
    int result = service.getDataServerInfoList(sessionId,serverInfoList);
    if (result != 0)
    {
      fprintf(stdout,"ERROR (%d) : %s\n",result,ContentServer::getResultString(result).c_str());
      return -3;
    }
    serverInfoList.writeToFile(dir + "/dataServers.csv");



    T::ProducerInfoList producerInfoList;
    result = service.getProducerInfoList(sessionId,producerInfoList);
    if (result != 0)
    {
      fprintf(stdout,"ERROR (%d) : %s\n",result,ContentServer::getResultString(result).c_str());
      return -3;
    }
    producerInfoList.writeToFile(dir + "/producers.csv");

    T::GenerationInfoList generationInfoList;
    result = service.getGenerationInfoList(sessionId,generationInfoList);
    if (result != 0)
    {
      fprintf(stdout,"ERROR (%d) : %s\n",result,ContentServer::getResultString(result).c_str());
      return -3;
    }
    generationInfoList.writeToFile(dir + "/generations.csv");


    bool ind = true;
    uint startFileId = 0;
    while (ind)
    {
      T::FileInfoList fileInfoList;
      result = service.getFileInfoList(sessionId,startFileId,10000,fileInfoList);
      if (result != 0)
      {
        fprintf(stdout,"ERROR (%d) : %s\n",result,ContentServer::getResultString(result).c_str());
        return -3;
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



    ind = true;
    startFileId = 0;
    uint startMessageIndex = 0;
    while (ind)
    {
      T::ContentInfoList contentInfoList;
      result = service.getContentList(sessionId,startFileId,startMessageIndex,10000,contentInfoList);
      if (result != 0)
      {
        fprintf(stdout,"ERROR (%d) : %s\n",result,ContentServer::getResultString(result).c_str());
        return -3;
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
    SmartMet::Spine::Exception exception(BCP,"Service call failed!",NULL);
    exception.printError();
    return -4;
  }
}

