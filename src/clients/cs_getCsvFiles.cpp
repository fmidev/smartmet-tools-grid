#include "contentServer/corba/client/ClientImplementation.h"
#include "contentServer/http/client/ClientImplementation.h"
#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"

using namespace SmartMet;



void writeProducers(T::SessionId sessionId,ContentServer::ServiceInterface *serviceInterface,const char *dir)
{
  try
  {
    printf("Reading producer information from the data storage\n");

    char filename[300];
    sprintf(filename,"%s/producers.csv",dir);
    FILE *file = fopen(filename,"w");
    if (file == NULL)
    {
      SmartMet::Spine::Exception exception(BCP,"Cannot create file!");
      exception.addParameter("Filename",filename);
      throw exception;
    }

    T::ProducerInfoList producerList;
    int result = serviceInterface->getProducerInfoList(sessionId,producerList);
    if (result != 0)
    {
      SmartMet::Spine::Exception exception(BCP,"Cannot read the producer list from the data storage!");
      exception.addParameter("Result",ContentServer::getResultString(result));
      throw exception;
    }

    uint len = producerList.getLength();
    for (uint t=0; t<len; t++)
    {
      T::ProducerInfo *info = producerList.getProducerInfoByIndex(t);
      std::string st = info->getCsv();
      fprintf(file,"%s\n",st.c_str());
    }

    fclose(file);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,"Operation failed!",NULL);
  }
}





void writeGenerations(T::SessionId sessionId,ContentServer::ServiceInterface *serviceInterface,const char *dir)
{
  try
  {
    printf("Reading generation information from the data storage\n");

    char filename[300];
    sprintf(filename,"%s/generations.csv",dir);
    FILE *file = fopen(filename,"w");
    if (file == NULL)
    {
      SmartMet::Spine::Exception exception(BCP,"Cannot create file!");
      exception.addParameter("Filename",filename);
      throw exception;
    }

    T::GenerationInfoList generationList;
    int result = serviceInterface->getGenerationInfoList(sessionId,generationList);
    if (result != 0)
    {
      SmartMet::Spine::Exception exception(BCP,"Cannot read the generation list from the data storage!");
      exception.addParameter("Result",ContentServer::getResultString(result));
      throw exception;
    }

    uint len = generationList.getLength();
    for (uint t=0; t<len; t++)
    {
      T::GenerationInfo *info = generationList.getGenerationInfoByIndex(t);
      std::string st = info->getCsv();
      fprintf(file,"%s\n",st.c_str());
    }

    fclose(file);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,"Operation failed!",NULL);
  }
}





void writeFiles(T::SessionId sessionId,ContentServer::ServiceInterface *serviceInterface,const char *dir)
{
  try
  {
    printf("Reading file information from the data storage\n");

    char filename[300];
    sprintf(filename,"%s/files.csv",dir);
    FILE *file = fopen(filename,"w");
    if (file == NULL)
    {
      SmartMet::Spine::Exception exception(BCP,"Cannot create file!");
      exception.addParameter("Filename",filename);
      throw exception;
    }

    uint len = 1;
    uint startFileId = 0;
    uint maxRecords = 10000;
    while (len > 0)
    {
      T::FileInfoList fileList;
      int result = serviceInterface->getFileInfoList(sessionId,startFileId,maxRecords,fileList);
      if (result != 0)
      {
        SmartMet::Spine::Exception exception(BCP,"Cannot read the file list from the data storage!");
        exception.addParameter("Result",ContentServer::getResultString(result));
        throw exception;
      }

      len = fileList.getLength();
      for (uint t=0; t<len; t++)
      {
        T::FileInfo *info = fileList.getFileInfoByIndex(t);
        std::string st = info->getCsv();
        fprintf(file,"%s\n",st.c_str());
        if (info->mFileId >= startFileId)
          startFileId = info->mFileId + 1;
      }
    }
    fclose(file);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,"Operation failed!",NULL);
  }
}





void writeContent(T::SessionId sessionId,ContentServer::ServiceInterface *serviceInterface,const char *dir)
{
  try
  {
    printf("Reading content information from the data storage\n");

    char filename[300];
    sprintf(filename,"%s/content.csv",dir);
    FILE *file = fopen(filename,"w");
    if (file == NULL)
    {
      SmartMet::Spine::Exception exception(BCP,"Cannot create file!");
      exception.addParameter("Filename",filename);
      throw exception;
    }

    uint len = 1;
    uint startFileId = 0;
    uint startMessageIndex = 0;
    uint maxRecords = 10000;

    while (len > 0)
    {
      T::ContentInfoList contentList;
      int result = serviceInterface->getContentList(sessionId,startFileId,startMessageIndex,maxRecords,contentList);
      if (result != 0)
      {
        SmartMet::Spine::Exception exception(BCP,"Cannot read the content list from the data storage!");
        exception.addParameter("Result",ContentServer::getResultString(result));
        throw exception;
      }

      len = contentList.getLength();
      for (uint t=0; t<len; t++)
      {
        T::ContentInfo *info = contentList.getContentInfoByIndex(t);
        std::string st = info->getCsv();
        fprintf(file,"%s\n",st.c_str());
        if (info->mFileId > startFileId  || (info->mFileId == startFileId  &&  info->mMessageIndex >= startMessageIndex))
        {
          startFileId = info->mFileId;
          startMessageIndex = info->mMessageIndex + 1;
        }
      }
    }
    fclose(file);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,"Operation failed!",NULL);
  }
}







int main(int argc, char *argv[])
{
  try
  {
    if (argc < 3)
    {
      fprintf(stdout,"USAGE: cs_getCsvFiles <sessionId> <directory> [-http <url>]\n");
      return -1;
    }

    T::SessionId sessionId = (SmartMet::T::SessionId)atoll(argv[1]);
    char *dir = argv[2];

    unsigned long long startTime = 0;
    unsigned long long endTime = 0;

    if (argc == 5  &&  strcmp(argv[3],"-http") == 0)
    {
      ContentServer::HTTP::ClientImplementation service;
      service.init(argv[4]);

      startTime = getTime();

      writeProducers(sessionId,&service,dir);
      writeGenerations(sessionId,&service,dir);
      writeFiles(sessionId,&service,dir);
      writeContent(sessionId,&service,dir);

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

      writeProducers(sessionId,&service,dir);
      writeGenerations(sessionId,&service,dir);
      writeFiles(sessionId,&service,dir);
      writeContent(sessionId,&service,dir);

      endTime = getTime();
    }

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

