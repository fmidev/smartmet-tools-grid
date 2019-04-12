#include "grid-files/common/Exception.h"
#include "grid-files/common/Log.h"
#include "grid-files/common/ShowFunction.h"
#include "grid-files/common/GeneralFunctions.h"
#include "grid-files/common/ShowFunction.h"
#include "grid-files/identification/GridDef.h"
#include "grid-files/grid/PhysicalGridFile.h"
#include "grid-content/contentServer/redis/RedisImplementation.h"
#include "grid-content/contentServer/corba/client/ClientImplementation.h"
#include "grid-content/contentServer/http/client/ClientImplementation.h"
#include "grid-content/lua/LuaFile.h"

#include <libpq-fe.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#define FUNCTION_TRACE FUNCTION_TRACE_OFF


using namespace SmartMet;


// Global variables:

ConfigurationFile         mConfigurationFile;
std::string               mGridConfigFile;
uint                      mSourceId = 100;
std::string               mProducerDefFile;
std::string               mPreloadFile;
uint                      mMaxMessageSize = 5000;
std::string               mStorageType;
std::string               mRedisAddress;
int                       mRedisPort = 6379;
std::string               mRedisTablePrefix;
std::string               mContentServerIor;
std::string               mContentServerUrl;
bool                      mDebugLogEnabled = false;
std::string               mDebugLogFile;
int                       mDebugLogMaxSize = 100000000;
int                       mDebugLogTruncateSize = 20000000;
Log                       mDebugLog;
Log*                      mDebugLogPtr = nullptr;
T::SessionId              mSessionId = 0;
uint                      mGlobalFileId = 0;

std::set<std::string>     mPreloadList;
T::ProducerInfoList       mSourceProducerList;
T::GenerationInfoList     mSourceGenerationList;
T::FileInfoList           mSourceFileList;
T::ContentInfoList        mSourceContentList;

T::ProducerInfoList       mTargetProducerList;
T::GenerationInfoList     mTargetGenerationList;
T::FileInfoList           mTargetFileList;
T::ContentInfoList        mTargetContentList;

std::vector<std::string>  mContentDirectories;
std::vector<std::string>  mContentPatterns;
std::map<std::string,std::string> mProducerAbbrList;

Lua::LuaFile              mLuaFile;
std::string               mLuaFilename;
std::string               mLuaFunction;





void sig_handler(int signum)
{
  {
    try
    {
      sprintf(nullptr,"Crashing the system for the core dump");
    }
    catch (...)
    {
      SmartMet::Spine::Exception exception(BCP,exception_operation_failed,nullptr);
      exception.printError();
      exit(-1);
    }
  }
}




void readConfigFile(const char* configFile)
{
  try
  {
    const char *configAttribute[] =
    {
      "smartmet.library.grid-files.configFile",
      "smartmet.tools.grid.filesys2smartmet.maxMessageSize",
      "smartmet.tools.grid.filesys2smartmet.content-source.source-id",
      "smartmet.tools.grid.filesys2smartmet.content-source.producerDefFile",
      "smartmet.tools.grid.filesys2smartmet.content-source.preloadFile",
      "smartmet.tools.grid.filesys2smartmet.content-source.directories",
      "smartmet.tools.grid.filesys2smartmet.content-source.patterns",
      "smartmet.tools.grid.filesys2smartmet.content-source.filenameFixer.luaFilename",
      "smartmet.tools.grid.filesys2smartmet.content-source.filenameFixer.luaFunction",
      "smartmet.tools.grid.filesys2smartmet.content-storage.type",
      "smartmet.tools.grid.filesys2smartmet.content-storage.redis.address",
      "smartmet.tools.grid.filesys2smartmet.content-storage.redis.port",
      "smartmet.tools.grid.filesys2smartmet.content-storage.redis.tablePrefix",
      "smartmet.tools.grid.filesys2smartmet.content-storage.corba.ior",
      "smartmet.tools.grid.filesys2smartmet.content-storage.http.url",
      "smartmet.tools.grid.filesys2smartmet.debug-log.enabled",
      "smartmet.tools.grid.filesys2smartmet.debug-log.file",
      "smartmet.tools.grid.filesys2smartmet.debug-log.maxSize",
      "smartmet.tools.grid.filesys2smartmet.debug-log.truncateSize",
      nullptr
    };


    mConfigurationFile.readFile(configFile);
    //mConfigurationFile.print(std::cout,0,0);

    uint t=0;
    while (configAttribute[t] != nullptr)
    {
      if (!mConfigurationFile.findAttribute(configAttribute[t]))
      {
        SmartMet::Spine::Exception exception(BCP, "Missing configuration attribute!");
        exception.addParameter("File",configFile);
        exception.addParameter("Attribute",configAttribute[t]);
        throw exception;
      }
      t++;
    }

    mConfigurationFile.getAttributeValue("smartmet.library.grid-files.configFile", mGridConfigFile);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.filesys2smartmet.maxMessageSize",mMaxMessageSize);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.filesys2smartmet.content-source.source-id",mSourceId);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.filesys2smartmet.content-source.producerDefFile",mProducerDefFile);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.filesys2smartmet.content-source.preloadFile",mPreloadFile);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.filesys2smartmet.content-source.directories",mContentDirectories);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.filesys2smartmet.content-source.patterns",mContentPatterns);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.filesys2smartmet.content-source.filenameFixer.luaFilename",mLuaFilename);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.filesys2smartmet.content-source.filenameFixer.luaFunction",mLuaFunction);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.filesys2smartmet.content-storage.type",mStorageType);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.filesys2smartmet.content-storage.redis.address",mRedisAddress);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.filesys2smartmet.content-storage.redis.port",mRedisPort);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.filesys2smartmet.content-storage.redis.tablePrefix",mRedisTablePrefix);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.filesys2smartmet.content-storage.corba.ior",mContentServerIor);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.filesys2smartmet.content-storage.http.url",mContentServerUrl);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.filesys2smartmet.debug-log.enabled", mDebugLogEnabled);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.filesys2smartmet.debug-log.file", mDebugLogFile);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.filesys2smartmet.debug-log.maxSize", mDebugLogMaxSize);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.filesys2smartmet.debug-log.truncateSize", mDebugLogTruncateSize);

    if (mLuaFilename > " ")
     mLuaFile.init(mLuaFilename);

  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Constructor failed!", nullptr);
  }
}





void readTargetProducers(ContentServer::ServiceInterface *targetInterface)
{
  FUNCTION_TRACE
  try
  {
    mTargetProducerList.clear();
    int result = targetInterface->getProducerInfoList(mSessionId,mTargetProducerList);
    if (result != 0)
    {
      SmartMet::Spine::Exception exception(BCP,"Cannot read the producer list from the target data storage!");
      exception.addParameter("Result",ContentServer::getResultString(result));
      throw exception;
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,nullptr);
  }
}





void readTargetGenerations(ContentServer::ServiceInterface *targetInterface)
{
  FUNCTION_TRACE
  try
  {
    mTargetGenerationList.clear();
    int result = targetInterface->getGenerationInfoList(mSessionId,mTargetGenerationList);
    if (result != 0)
    {
      SmartMet::Spine::Exception exception(BCP,"Cannot read the generation list from the target data storage!");
      exception.addParameter("Result",ContentServer::getResultString(result));
      throw exception;
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,nullptr);
  }
}





void readTargetFiles(ContentServer::ServiceInterface *targetInterface)
{
  FUNCTION_TRACE
  try
  {
    mTargetFileList.clear();
    uint len = 1;
    uint startFileId = 0;
    uint maxRecords = 10000;

    while (len > 0)
    {
      T::FileInfoList fileList;
      int result = targetInterface->getFileInfoListBySourceId(mSessionId,mSourceId,startFileId,maxRecords,fileList);
      if (result != 0)
      {
        SmartMet::Spine::Exception exception(BCP,"Cannot read the file list from the target data storage!");
        exception.addParameter("Result",ContentServer::getResultString(result));
        throw exception;
      }

      len = fileList.getLength();
      for (uint t=0; t<len; t++)
      {
        T::FileInfo *fileInfo = fileList.getFileInfoByIndex(t);
        if (fileInfo->mSourceId == mSourceId)
        {
          mTargetFileList.addFileInfo(fileInfo->duplicate());
        }

        if (fileInfo->mFileId >= startFileId)
          startFileId = fileInfo->mFileId + 1;
      }
    }

    mTargetFileList.sort(T::FileInfo::ComparisonMethod::fileName);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,nullptr);
  }
}





void readPreloadList(const char *filename)
{
  try
  {
    FILE *file = fopen(filename,"re");
    if (file == nullptr)
    {
      PRINT_DATA(mDebugLogPtr,"Preload file not available.");
      return;
    }

    mPreloadList.clear();

    char st[1000];
    while (!feof(file))
    {
      if (fgets(st,1000,file) != nullptr)
      {
        if (st[0] != '#')
        {
          char *p = st;
          while (*p != '\0')
          {
            if (*p <= ' ')
              *p = '\0';
            else
              p++;
          }

          std::vector<std::string> partList;
          splitString(st,';',partList);
          if (partList.size() >= 8)
          {
            if (partList[7] == "1")
              mPreloadList.insert(toLowerString(std::string(st)));
          }
        }
      }
    }
    fclose(file);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, exception_operation_failed, nullptr);
  }
}





void readSourceFiles(std::vector<std::pair<std::string,std::string>>& fileList)
{
  FUNCTION_TRACE
  try
  {
    mSourceFileList.clear();
    mSourceContentList.clear();

    for (auto it = fileList.begin(); it != fileList.end(); ++it)
    {
      std::string fn = it->second.c_str();
      if (mLuaFunction > " ")
      {
        std::vector<std::string> params;
        params.push_back(fn);
        fn = mLuaFile.executeFunctionCall6(mLuaFunction,params);
      }
      //printf("FILE : %s\n",fn.c_str());

      std::vector<std::string> partList;
      splitString(fn,'_',partList);

      if (partList.size() > 2)
      {
        std::string producerAbbr = partList[0];
        std::string generationTime = partList[1];

        auto pp = mProducerAbbrList.find(producerAbbr);
        if (pp ==  mProducerAbbrList.end())
        {
          //std::cout << "Unknown producer abbr : " << producerAbbr << "\n";
        }
        else
        {
          T::ProducerInfo *producer = mTargetProducerList.getProducerInfoByName(pp->second.c_str());
          if (producer == nullptr)
          {
            //std::cout << "Producer not found : " << pp->second << "\n";
          }
          else
          {
            char st[1000];
            sprintf(st,"%s:%s",producer->mName.c_str(),generationTime.c_str());
            T::GenerationInfo *generation =  mTargetGenerationList.getGenerationInfoByName(std::string(st));
            if (generation != nullptr)
            {
              char filename[1000];
              sprintf(filename,"%s/%s",it->first.c_str(),it->second.c_str());

              time_t modificationTime = getFileModificationTime(filename);
              if (modificationTime > 0)
              {
                T::FileInfo *fileInfo = new T::FileInfo();
                fileInfo->mGroupFlags = 0;
                fileInfo->mProducerId = producer->mProducerId;
                fileInfo->mGenerationId = generation->mGenerationId;
                fileInfo->mFileId = mSourceFileList.getLength() + 1;
                //fileInfo->mFileType;
                fileInfo->mName = filename;
                fileInfo->mFlags = T::FileInfo::Flags::PredefinedContent;
                fileInfo->mSourceId = mSourceId;
                fileInfo->mModificationTime = localTimeFromTimeT(modificationTime,"UTC");

                mSourceFileList.addFileInfo(fileInfo);
                fileInfo->print(std::cout,0,0);
              }
            }
            else
            {
              //printf("*** Generation not found : %s\n",st);
            }
          }
        }
      }
    }
    mSourceFileList.sort(T::FileInfo::ComparisonMethod::fileName);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, exception_operation_failed, nullptr);
  }
}





void readSourceGenerations(std::vector<std::pair<std::string,std::string>>& fileList)
{
  FUNCTION_TRACE
  try
  {
    std::set<std::string> processedGenerations;

    mSourceGenerationList.clear();

    for (auto it = fileList.begin(); it != fileList.end(); ++it)
    {
      std::string fn = it->second.c_str();
      if (mLuaFunction > " ")
      {
        std::vector<std::string> params;
        params.push_back(fn);
        fn = mLuaFile.executeFunctionCall6(mLuaFunction,params);
      }

      std::vector<std::string> partList;
      splitString(fn,'_',partList);

      if (partList.size() > 2)
      {
        std::string producerAbbr = partList[0];
        std::string generationTime = partList[1];

        auto pp = mProducerAbbrList.find(producerAbbr);
        if (pp ==  mProducerAbbrList.end())
        {
          //std::cout << "Unknown producer abbr : " << producerAbbr << "\n";
        }
        else
        {
          //std::cout << "Found [" << producerAbbr << "] : " << pp->second << "\n";

          T::ProducerInfo *producer = mTargetProducerList.getProducerInfoByName(pp->second.c_str());
          if (producer == nullptr)
          {
            //std::cout << "Producer not found : " << pp->second << "\n";
          }
          else
          {
            char st[1000];
            sprintf(st,"%s:%s",producer->mName.c_str(),generationTime.c_str());

            if (processedGenerations.find(st) == processedGenerations.end())
            {
              processedGenerations.insert(st);

              T::GenerationInfo *generation =  mSourceGenerationList.getGenerationInfoByName(std::string(st));
              if (generation == nullptr)
              {
                generation = new T::GenerationInfo();
                generation->mGenerationId = mSourceGenerationList.getLength() + 1;
                generation->mGenerationType = 0;
                generation->mProducerId = producer->mProducerId;
                generation->mName = st;
                sprintf(st,"Producer %s generation %s",producer->mName.c_str(),generationTime.c_str());
                generation->mDescription = st;
                generation->mAnalysisTime = generationTime;
                generation->mStatus = T::GenerationInfo::Status::Ready;
                generation->mFlags = 0;
                generation->mSourceId = mSourceId;
                mSourceGenerationList.addGenerationInfo(generation);

                //generation->print(std::cout,0,0);
              }
              else
              {
                //std::cout << " -- generation exists\n";
              }
            }
          }
        }
      }
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, exception_operation_failed, nullptr);
  }
}





void setMessageContent(SmartMet::GRID::GridFile& gridFile,SmartMet::GRID::Message& message,T::ContentInfo& contentInfo)
{
  try
  {
    contentInfo.mFileType = gridFile.getFileType();
    contentInfo.mGroupFlags = 0;
    contentInfo.mForecastTime = message.getForecastTime();
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
    contentInfo.mSourceId = mSourceId;
    contentInfo.mGeometryId = message.getGridGeometryId();
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, exception_operation_failed, nullptr);
  }
}





void readSourceContent(uint producerId,uint generationId,const char *filename,T::ContentInfoList& contentList)
{
  try
  {
    contentList.clear();

    T::ProducerInfo *producer = nullptr;
    if (mPreloadList.size() > 0)
      producer = mTargetProducerList.getProducerInfoById(producerId);

    SmartMet::GRID::PhysicalGridFile gridFile;
    gridFile.read(filename);

    uint messageCount = gridFile.getNumberOfMessages();
    for (uint t=0; t<messageCount; t++)
    {
      SmartMet::GRID::Message *message = gridFile.getMessageByIndex(t);

      T::ContentInfo *contentInfo = new T::ContentInfo();
      contentInfo->mFileId = 0;
      contentInfo->mMessageIndex = t;
      contentInfo->mProducerId = producerId;
      contentInfo->mGenerationId = generationId;

      setMessageContent(gridFile,*message,*contentInfo);

      if (producer != nullptr)
      {
        char st[200];
        sprintf(st,"%s;%s;%d;%d;%05d;%d;%d;1;",
            producer->mName.c_str(),
            contentInfo->mFmiParameterName.c_str(),
            (int)T::ParamLevelIdTypeValue::FMI,
            (int)contentInfo->mFmiParameterLevelId,
            (int)contentInfo->mParameterLevel,
            (int)contentInfo->mForecastType,
            (int)contentInfo->mForecastNumber);

          if (mPreloadList.find(toLowerString(std::string(st))) != mPreloadList.end())
            contentInfo->mFlags = T::ContentInfo::Flags::PreloadRequired;
      }

      contentList.addContentInfo(contentInfo);
    }
    //contentList.print(std::cout,0,0);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, exception_operation_failed, nullptr);
  }
}





void readSourceProducers()
{
  FUNCTION_TRACE
  try
  {
    mSourceProducerList.clear();

    FILE *file = fopen(mProducerDefFile.c_str(),"re");
    if (file == nullptr)
    {
      SmartMet::Spine::Exception exception(BCP,"Cannot open file!");
      exception.addParameter("Filename",mProducerDefFile);
      throw exception;
    }

    char st[1000];

    while (!feof(file))
    {
      if (fgets(st,1000,file) != nullptr  &&  st[0] != '#')
      {
        bool ind = false;
        char *field[100];
        uint c = 1;
        field[0] = st;
        char *p = st;
        while (*p != '\0'  &&  c < 100)
        {
          if (*p == '"')
            ind = !ind;

          if ((*p == ';'  || *p == '\n') && !ind)
          {
            *p = '\0';
            p++;
            field[c] = p;
            c++;
          }
          else
          {
            p++;
          }
        }

        if (c > 3)
        {
          T::ProducerInfo *producer = new T::ProducerInfo();
          producer->mProducerId = mSourceProducerList.getLength() + 1;
          producer->mName = field[1];
          producer->mTitle = field[2];
          producer->mDescription = field[3];
          producer->mFlags = 0;
          producer->mSourceId = mSourceId;

          mSourceProducerList.addProducerInfo(producer);

          mProducerAbbrList.insert(std::pair<std::string,std::string>(field[0],field[1]));
        }
      }

    }
    fclose(file);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, exception_operation_failed, nullptr);
  }
}





void updateProducers(ContentServer::ServiceInterface *targetInterface)
{
  FUNCTION_TRACE
  try
  {
    uint len = mTargetProducerList.getLength();
    for (uint t=0; t<len; t++)
    {
      T::ProducerInfo *targetProducer = mTargetProducerList.getProducerInfoByIndex(t);

      // Checking if we have created the current producers - if so, then we can also remove it.
      if (targetProducer->mSourceId == mSourceId)
      {
        T::ProducerInfo *sourceProducer = mSourceProducerList.getProducerInfoByName(targetProducer->mName);
        if (sourceProducer == nullptr)
        {
          // The producer information is not available in the source data storage. So, we should remove
          // it also from the target data storage.

          PRINT_DATA(mDebugLogPtr,"  -- Remove producer : %s\n",targetProducer->mName.c_str());
          int result = targetInterface->deleteProducerInfoById(mSessionId,targetProducer->mProducerId);
          if (result != 0)
          {
            SmartMet::Spine::Exception exception(BCP,"Cannot delete the producer information from the target data storage!");
            exception.addParameter("ProducerId",std::to_string(targetProducer->mProducerId));
            exception.addParameter("ProducerName",targetProducer->mName);
            exception.addParameter("Result",ContentServer::getResultString(result));
            throw exception;
          }
        }
      }
    }


    len = mSourceProducerList.getLength();
    for (uint t=0; t<len; t++)
    {
      T::ProducerInfo *sourceProducer = mSourceProducerList.getProducerInfoByIndex(t);
      if (sourceProducer->mSourceId == mSourceId)
      {
        T::ProducerInfo *targetProducer = mTargetProducerList.getProducerInfoByName(sourceProducer->mName);
        if (targetProducer == nullptr)
        {
          // The producer information is not available in the target data storage. So, we should add it.

          T::ProducerInfo producer(*sourceProducer);
          producer.mProducerId = 0;
          producer.mSourceId = mSourceId;

          PRINT_DATA(mDebugLogPtr,"  -- Add producer : %s\n",producer.mName.c_str());
          int result = targetInterface->addProducerInfo(mSessionId,producer);
          if (result != 0)
          {
            SmartMet::Spine::Exception exception(BCP,"Cannot add the producer information into the target data storage!");
            exception.addParameter("ProducerId",std::to_string(sourceProducer->mProducerId));
            exception.addParameter("ProducerName",sourceProducer->mName);
            exception.addParameter("Result",ContentServer::getResultString(result));
            throw exception;
          }
        }
      }
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,nullptr);
  }
}





void updateGenerations(ContentServer::ServiceInterface *targetInterface)
{
  FUNCTION_TRACE
  try
  {
    std::set<uint> generationIdList;
    uint len = mTargetGenerationList.getLength();
    for (uint t=0; t<len; t++)
    {
      T::GenerationInfo *targetGeneration = mTargetGenerationList.getGenerationInfoByIndex(t);
      if (targetGeneration->mSourceId == mSourceId)
      {
        T::GenerationInfo *sourceGeneration = mSourceGenerationList.getGenerationInfoByName(targetGeneration->mName);
        if (sourceGeneration == nullptr)
        {
          // The generation information is not available in the source data storage. So, we should remove
          // it also from the target data storage.

          PRINT_DATA(mDebugLogPtr,"  -- Remove generation : %s\n",targetGeneration->mName.c_str());

          generationIdList.insert(targetGeneration->mGenerationId);
        }
      }
    }

    if (generationIdList.size() > 0)
    {
      int result = targetInterface->deleteGenerationInfoListByIdList(mSessionId,generationIdList);
      if (result != 0)
      {
        SmartMet::Spine::Exception exception(BCP,"Cannot delete the generation information from the target data storage!");
        exception.addParameter("Result",ContentServer::getResultString(result));
        throw exception;
      }
    }


    len = mSourceGenerationList.getLength();
    for (uint t=0; t<len; t++)
    {
      T::GenerationInfo *sourceGeneration = mSourceGenerationList.getGenerationInfoByIndex(t);
      if (sourceGeneration->mSourceId == mSourceId)
      {
        T::FileInfoList sourceFiles;
        mSourceFileList.getFileInfoListByGenerationId(sourceGeneration->mGenerationId,sourceFiles);

        T::GenerationInfo *targetGeneration = mTargetGenerationList.getGenerationInfoByName(sourceGeneration->mName);
        if (targetGeneration == nullptr)
        {
          // The generation information is not available in the target data storage. So, we should add it.


          T::GenerationInfo generationInfo(*sourceGeneration);
          generationInfo.mSourceId = mSourceId;
          generationInfo.mGenerationId = 0;

          PRINT_DATA(mDebugLogPtr,"  -- Add generation : %s\n",generationInfo.mName.c_str());

          int result = targetInterface->addGenerationInfo(mSessionId,generationInfo);
          if (result != 0)
          {
            SmartMet::Spine::Exception exception(BCP,"Cannot add the generation information into the target data storage!");
            exception.addParameter("GenerationName",generationInfo.mName);
            exception.addParameter("Result",ContentServer::getResultString(result));
            throw exception;
          }
        }
      }
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,nullptr);
  }
}





void updateFiles(ContentServer::ServiceInterface *targetInterface)
{
  FUNCTION_TRACE
  try
  {
    std::set<uint> fileIdList;
    uint len = mTargetFileList.getLength();
    for (uint t=0; t<len; t++)
    {
      T::FileInfo *targetFile = mTargetFileList.getFileInfoByIndex(t);
      if (targetFile->mSourceId == mSourceId)
      {
        T::FileInfo *sourceFile = mSourceFileList.getFileInfoByName(targetFile->mName);
        if (sourceFile == nullptr)
        {
          // The file information is not available in the source data storage. So, we should remove
          // it also from the target data storage.

          PRINT_DATA(mDebugLogPtr,"  -- Remove file : %s\n",targetFile->mName.c_str());

          fileIdList.insert(targetFile->mFileId);
        }
      }
    }

    if (fileIdList.size() > 0)
    {
      int result = targetInterface->deleteFileInfoListByFileIdList(mSessionId,fileIdList);
      if (result != 0)
      {
        SmartMet::Spine::Exception exception(BCP,"Cannot delete the file information from the target data storage!");
        exception.addParameter("Result",ContentServer::getResultString(result));
        throw exception;
      }
    }


    len = mSourceFileList.getLength();
    for (uint t=0; t<len; t++)
    {
      T::FileInfo *sourceFile = mSourceFileList.getFileInfoByIndex(t);
      if (sourceFile->mSourceId == mSourceId)
      {
        T::FileInfo *targetFile = mTargetFileList.getFileInfoByName(sourceFile->mName);
        if (targetFile != nullptr  &&  targetFile->mModificationTime != sourceFile->mModificationTime)
        {
          // The file exists in the target data storage, but its modification time is different. So, we should remove it.

          PRINT_DATA(mDebugLogPtr,"  -- Remove file: %s\n",targetFile->mName.c_str());
          int result = targetInterface->deleteFileInfoById(mSessionId,targetFile->mFileId);
          if (result != 0)
          {
            SmartMet::Spine::Exception exception(BCP,"Cannot delete the file information from the target data storage!");
            exception.addParameter("Result",ContentServer::getResultString(result));
            throw exception;
          }
          targetFile = nullptr;
        }

        if (targetFile == nullptr)
        {
          // The file information is not available in the target data storage. So, we should add it.

          T::FileInfo fileInfo(*sourceFile);
          fileInfo.mFileId = 0;

          PRINT_DATA(mDebugLogPtr,"  -- Add file: %s\n",fileInfo.mName.c_str());

          T::ContentInfoList contentList;
          readSourceContent(fileInfo.mProducerId,fileInfo.mGenerationId,fileInfo.mName.c_str(),contentList);
          if (contentList.getLength() > 0)
          {
            int result = targetInterface->addFileInfoWithContentList(mSessionId,fileInfo,contentList);
            if (result != 0)
            {
              SmartMet::Spine::Exception exception(BCP,"Cannot add the file information into the target data storage!");
              exception.addParameter("FileName",fileInfo.mName);
              exception.addParameter("Result",ContentServer::getResultString(result));
              throw exception;
            }
          }
        }
      }
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,nullptr);
  }
}





int main(int argc, char *argv[])
{
  FUNCTION_TRACE
  try
  {
    if (argc != 3)
    {
      fprintf(stderr,"USAGE: filesys2smartmet <configFile> <loopWaitTime>\n");
      return -1;
    }

    readConfigFile(argv[1]);

    signal(SIGUSR1, sig_handler);

    // Initializing the global structures. These are needed when
    // extracting information from GRIB files.

    Identification::gridDef.init(mGridConfigFile.c_str());

    ContentServer::ServiceInterface *targetInterface = nullptr;

    uint waitTime = toInt64(argv[2]);

    if (mStorageType =="redis")
    {
      ContentServer::RedisImplementation *redisImplementation = new ContentServer::RedisImplementation();
      redisImplementation->init(mRedisAddress.c_str(),mRedisPort,mRedisTablePrefix.c_str());
      targetInterface = redisImplementation;
    }

    if (mStorageType =="corba")
    {
      ContentServer::Corba::ClientImplementation *client = new ContentServer::Corba::ClientImplementation();
      client->init(mContentServerIor.c_str());
      targetInterface = client;
    }

    if (mStorageType =="http")
    {
      ContentServer::HTTP::ClientImplementation *httpClient = new ContentServer::HTTP::ClientImplementation();
      httpClient->init(mContentServerUrl.c_str());
      targetInterface = httpClient;
    }

    if (mDebugLogEnabled)
    {
      mDebugLog.init(true,mDebugLogFile.c_str(),mDebugLogMaxSize,mDebugLogTruncateSize);
      mDebugLogPtr = &mDebugLog;
    }

    if (targetInterface == nullptr)
    {
      fprintf(stderr,"No target data source defined!\n");
      return -3;
    }


    bool ind = true;
    while (ind)
    {
      PRINT_DATA(mDebugLogPtr,"\n");
      PRINT_DATA(mDebugLogPtr,"********************************************************************\n");
      PRINT_DATA(mDebugLogPtr,"****************************** UPDATE ******************************\n");
      PRINT_DATA(mDebugLogPtr,"********************************************************************\n");

      std::set<std::string> dirList;
      std::vector<std::pair<std::string,std::string>> fileList;

      for (auto dir = mContentDirectories.begin(); dir != mContentDirectories.end(); ++dir)
      {
        getFileList(dir->c_str(),mContentPatterns,true,dirList,fileList);
        //std::cout << "GET : " << dir->c_str() << " : " << fileList.size() << "\n";
      }


      PRINT_DATA(mDebugLogPtr,"* Reading preload parameters\n");
      readPreloadList(mPreloadFile.c_str());

      PRINT_DATA(mDebugLogPtr,"* Reading producer information from the target data storage\n");
      readTargetProducers(targetInterface);

      PRINT_DATA(mDebugLogPtr,"* Reading producer information from the source data storage\n");
      readSourceProducers();

      PRINT_DATA(mDebugLogPtr,"* Updating producer information into the target data storage\n");
      updateProducers(targetInterface);

      PRINT_DATA(mDebugLogPtr,"* Reading updated producer information from the target data storage\n");
      readTargetProducers(targetInterface);

      PRINT_DATA(mDebugLogPtr,"* Reading generation information from the target data storage\n");
      readTargetGenerations(targetInterface);

      PRINT_DATA(mDebugLogPtr,"* Reading generation information from the source data storage\n");
      readSourceGenerations(fileList);

      PRINT_DATA(mDebugLogPtr,"* Updating generation information into the target data storage\n");
      updateGenerations(targetInterface);

      PRINT_DATA(mDebugLogPtr,"* Reading updated generation information from the target data storage\n");
      readTargetGenerations(targetInterface);

      PRINT_DATA(mDebugLogPtr,"* Reading file information from the target data storage\n");
      readTargetFiles(targetInterface);

      PRINT_DATA(mDebugLogPtr,"* Reading file information from the source data storage\n");
      readSourceFiles(fileList);

      PRINT_DATA(mDebugLogPtr,"* Updating file information into the target data storage\n");
      updateFiles(targetInterface);

      PRINT_DATA(mDebugLogPtr,"********************************************************************\n\n");

      if (waitTime > 0)
        sleep(waitTime);
      else
        ind = false;
    }

    return 0;
  }
  catch (SmartMet::Spine::Exception& e)
  {
    SmartMet::Spine::Exception exception(BCP,"Service call failed!",nullptr);
    exception.printError();
    return -3;
  }
}

