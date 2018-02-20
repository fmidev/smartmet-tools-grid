#include "contentServer/corba/client/ClientImplementation.h"
#include "contentServer/http/client/ClientImplementation.h"
#include "contentServer/redis/RedisImplementation.h"
#include "grid-files/grid/PhysicalGridFile.h"
#include "grid-files/grid/PrintOptions.h"
#include "grid-files/identification/GridDef.h"
#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"
#include "grid-files/common/GeneralDefinitions.h"
#include "grid-files/common/ImageFunctions.h"
#include "grid-files/grid/Typedefs.h"
#include "grid-files/grid/ValueCache.h"

#include <iostream>
#include <stdexcept>
#include <string.h>
#include <vector>
#include <sys/time.h>


using namespace SmartMet;


ContentServer::ServiceInterface *service = NULL;

char *producerNamePtr = NULL;
char *generationNamePtr = NULL;
char *analysisTimePtr = NULL;



void init()
{
  try
  {
    char *configFile = getenv(SMARTMET_GRID_CONFIG_FILE);
    if (configFile == NULL)
    {
      printf("%s not defined!\n",SMARTMET_GRID_CONFIG_FILE);
      exit(-1);
    }

    // Initializing the global structures. These are needed when
    // extracting information from GRIB files.

    Identification::gridDef.init(configFile);
  }
  catch (...)
  {
    throw Spine::Exception(BCP,exception_operation_failed,NULL);
  }
}





void addMessage(GRID::GridFile& gridFile,const GRID::Message& message)
{
  try
  {
    uint producerId = gridFile.getProducerId();
    uint generationId = gridFile.getGenerationId();
    uint fileId = gridFile.getFileId();
    uint messageIndex = message.getMessageIndex();

    std::string producerName = message.getFmiProducerName();
    if (producerNamePtr != NULL)
      producerName = producerNamePtr;

    std::string generationName;
    std::string filename = gridFile.getFileName();
    T::TimeString referenceTime = message.getReferenceTime();
    T::TimeString forecastTime = message.getForecastTime();


    if (producerId == 0)
    {
      Identification::FmiProducerId_grib producerDef;

      if (producerNamePtr == NULL)
      {
        if (!Identification::gridDef.getFmiProducerByName(producerName,producerDef))
        {
          printf("Producer '%s' not found!\n",producerName.c_str());
          return;
        }
      }

      T::ProducerInfo producerInfo;
      int result = service->getProducerInfoByName(0,producerName,producerInfo);
      if (result == ContentServer::Result::OK)
      {
        producerId = producerInfo.mProducerId;
      }
      else
      if (result == ContentServer::Result::DATA_NOT_FOUND)
      {
        producerInfo.mProducerId = producerDef.mProducerId;
        producerInfo.mName = producerName;
        producerInfo.mTitle = producerName;
        producerInfo.mDescription = producerDef.mProducerDescription;
        producerInfo.mFlags = 0;
        producerInfo.mSourceId = 0;

        int res = service->addProducerInfo(0,producerInfo);
        if (res != 0)
        {
          printf("Producer '%s' addition failed (%d)!\n",producerInfo.mName.c_str(),res);
          return;
        }

        producerId = producerInfo.mProducerId;
      }
      else
      {
        printf("Producer '%s' search failed (%d)!\n",producerName.c_str(),result);
        return;
      }
    }

    gridFile.setProducerId(producerId);

    if (analysisTimePtr != NULL)
      generationName = producerName + ":" + analysisTimePtr;
    else
      generationName = producerName + ":" + referenceTime;

    if (generationNamePtr != NULL)
      generationName = generationNamePtr;

    if (generationId == 0)
    {
      T::GenerationInfo generationInfo;
      int result = service->getGenerationInfoByName(0,generationName,generationInfo);
      if (result == ContentServer::Result::OK)
      {
        generationId = generationInfo.mGenerationId;
      }
      else
      if (result == ContentServer::Result::DATA_NOT_FOUND)
      {
        generationInfo.mGenerationId = 0;
        generationInfo.mGenerationType = 0;
        generationInfo.mProducerId = producerId;
        generationInfo.mName = generationName;
        generationInfo.mDescription = "";
        if (analysisTimePtr != NULL)
          generationInfo.mAnalysisTime = analysisTimePtr;
        generationInfo.mStatus = T::GenerationStatus::STATUS_READY;
        generationInfo.mFlags = 0;
        generationInfo.mSourceId = 0;

        int res = service->addGenerationInfo(0,generationInfo);
        if (res != 0)
        {
          printf("Generation '%s' addition failed (%d)!\n",generationInfo.mName.c_str(),res);
          return;
        }

        generationId = generationInfo.mGenerationId;
      }
    }

    gridFile.setGenerationId(generationId);


    if (fileId == 0)
    {
      T::FileInfo fileInfo;

      int result = service->getFileInfoByName(0,filename,fileInfo);
      if (result == ContentServer::Result::OK)
      {
        fileId = fileInfo.mFileId;
      }
      else
      if (result == ContentServer::Result::DATA_NOT_FOUND)
      {
        fileInfo.mProducerId = producerId;
        fileInfo.mGenerationId = generationId;
        fileInfo.mFileId = 0;
        fileInfo.mFileType = gridFile.getFileType();
        fileInfo.mName = filename;
        fileInfo.mGroupFlags = 0;
        fileInfo.mFlags = (uint)T::FileInfoFlags::CONTENT_PREDEFINED;
        fileInfo.mSourceId = 0;

        int res = service->addFileInfo(0,fileInfo);
        if (res != 0)
        {
          printf("File '%s' addition failed (%d)!\n",fileInfo.mName.c_str(),res);
          return;
        }

        fileId = fileInfo.mFileId;
      }
    }

    gridFile.setFileId(fileId);

    T::ContentInfo contentInfo;
    int result = service->getContentInfo(0,fileId,messageIndex,contentInfo);
    if (result == ContentServer::Result::DATA_NOT_FOUND)
    {
      contentInfo.mFileId = fileId;
      contentInfo.mFileType = gridFile.getFileType();
      contentInfo.mMessageIndex = messageIndex;
      contentInfo.mProducerId = producerId;
      contentInfo.mGenerationId = generationId;
      contentInfo.mGroupFlags = 0;
      contentInfo.mForecastTime = forecastTime;
      contentInfo.mFmiParameterId = message.getFmiParameterId();
      contentInfo.mFmiParameterName = message.getFmiParameterName();
      contentInfo.mGribParameterId = message.getGribParameterId();
      contentInfo.mCdmParameterId = message.getCdmParameterId();
      contentInfo.mCdmParameterName = message.getCdmParameterName();
      contentInfo.mNewbaseParameterId = message.getNewbaseParameterId();
      contentInfo.mNewbaseParameterName = message.getNewbaseParameterName();
      contentInfo.mFmiParameterLevelId = message.getFmiParameterLevelId();
      contentInfo.mGrib1ParameterLevelId = message.getGrib1ParameterLevelId();
      contentInfo.mGrib2ParameterLevelId = message.getGrib2ParameterLevelId();
      contentInfo.mParameterLevel = message.getGridParameterLevel();
      contentInfo.mFmiParameterUnits = message.getFmiParameterUnits();
      contentInfo.mGribParameterUnits = message.getGribParameterUnits();
      contentInfo.mForecastType = message.getForecastType();
      contentInfo.mForecastNumber = message.getForecastNumber();
      contentInfo.mServerFlags = 0;
      contentInfo.mFlags = 0;
      contentInfo.mSourceId = 0;
      contentInfo.mGeometryId = message.getGridGeometryId();
      //contentInfo.mModificationTime;

      int  res = service->addContentInfo(0,contentInfo);
      if (res != 0)
      {
        printf("Content '%s' [%u:%u]' addition failed (%d)!\n",filename.c_str(),fileId,messageIndex,res);
        return;
      }
    }



    printf("PRODUCER-ID        : %u\n",producerId);
    printf("PRODUCER-NAME      : %s\n",producerName.c_str());
    printf("GENERATION-ID      : %u\n",generationId);
    printf("GENERATION-NAME    : %s\n",generationName.c_str());
    printf("FILE-ID            : %u\n",fileId);
    printf("FILE-NAME          : %s\n",filename.c_str());
    printf("MESSAGE-INDEX      : %u\n",messageIndex);
    printf("REFERENCE-TIME     : %s\n",referenceTime.c_str());
    printf("FORECAST-TIME      : %s\n",forecastTime.c_str());
    printf("FMI-PARAMETER-NAME : %s\n",contentInfo.mFmiParameterName.c_str());

    //message.print(std);
    printf("\n");
  }
  catch (...)
  {
    throw Spine::Exception(BCP,exception_operation_failed,NULL);
  }
}





void addFile(GRID::GridFile& gridFile)
{
  try
  {
    //gridFile.print(std::cout,0,0);
    //printf("\n");


    const GRID::Message *prevMessage = NULL;
    const GRID::Message *nextMessage = NULL;
    std::size_t messageCount = gridFile.getNumberOfMessages();
    for (std::size_t m=0; m<messageCount; m++)
    {
      const GRID::Message *message = gridFile.getMessageByIndex(m);
      if ((m+1) < messageCount)
        nextMessage = gridFile.getMessageByIndex(m+1);

      if (prevMessage != NULL  && message->getForecastTime() == prevMessage->getForecastTime()  &&  message->getFmiParameterId() == prevMessage->getFmiParameterId())
      {
        prevMessage = message;
        message = NULL;
      }
      else
      if (nextMessage != NULL  && message->getForecastTime() == nextMessage->getForecastTime()  &&  message->getFmiParameterId() == nextMessage->getFmiParameterId())
      {
        prevMessage = NULL;
        message = NULL;
      }

      if (message != NULL)
      {
        addMessage(gridFile,*message);
        prevMessage = message;
      }
    }
  }
  catch (...)
  {
    throw Spine::Exception(BCP,exception_operation_failed,NULL);
  }
}





void print_usage()
{
  printf("-------------------------------------------------------------------------------- \n");
  printf("PROGRAM :\n");
  printf("  The program adds the grid file information into the ContentServer.\n");
  printf("\n");
  printf("USAGE :\n");
  printf("  grid_add <file> [[-http <url>]|[-redis <address> <port> <tablePrefix>]]\n");
  printf("\n");
  printf("PARAMETERS :\n");
  printf("  <file>   -- The full name of the grid file (with the directory path)\n");
  printf("\n");
  printf("EXAMPLES :\n");
  printf("  grid_add /data/test/mygribfile.grib2\n");
  printf("  grid_add /data/test/mygribfile.grib2 -redis 127.0.0.1 6379 \"\"\n");
  printf("-------------------------------------------------------------------------------- \n");
  printf("\n");
}


int run(int argc, char **argv)
{
  try
  {
    uint optionFlags = 0;

    if (argc < 2)
    {
      print_usage();
      return -1;
    }

    init();
    std::string filename = argv[1];

    for (int t=2; t<argc; t++)
    {
      if (strcmp(argv[t],"-http") == 0  &&  (t+1) < argc)
      {
        ContentServer::HTTP::ClientImplementation *client = new ContentServer::HTTP::ClientImplementation();
        client->init(argv[t+1]);
        service = client;
      }

      if (strcmp(argv[t],"-redis") == 0  &&  (t+3) < argc)
      {
        ContentServer::RedisImplementation *redis = new ContentServer::RedisImplementation();
        redis->init(argv[t+1],atoi(argv[t+2]),argv[t+3]);
        service = redis;
      }

      if (strcmp(argv[t],"-producer") == 0  &&  (t+1) < argc)
        producerNamePtr = argv[t+1];

      if (strcmp(argv[t],"-generation") == 0  &&  (t+1) < argc)
        generationNamePtr = argv[t+1];

      if (strcmp(argv[t],"-analysisTime") == 0  &&  (t+1) < argc)
        analysisTimePtr = argv[t+1];
    }

    if (service == NULL)
    {
      char *serviceIor = getenv("SMARTMET_CS_IOR");

      if (strcmp(argv[argc-2],"-ior") == 0)
        serviceIor = argv[argc-1];

      if (serviceIor == NULL)
      {
        fprintf(stdout,"Service IOR not defined!\n");
        return -2;
      }

      ContentServer::Corba::ClientImplementation *client = new ContentServer::Corba::ClientImplementation();
      client->init(serviceIor);
      service = client;
    }

    GRID::PhysicalGridFile gridFile;
    gridFile.read(filename);
    addFile(gridFile);

    //delete service;
    return 0;
  }
  catch (...)
  {
    throw Spine::Exception(BCP,exception_operation_failed,NULL);
  }
}


// ----------------------------------------------------------------------
/*
 * \brief Main program apart from exception handling
 */
// ----------------------------------------------------------------------

int main(int argc, char **argv) try
{
  return run(argc, argv);
}
catch (...)
{
  Spine::Exception exception(BCP,exception_operation_failed,NULL);
  exception.printError();
}
