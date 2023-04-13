#include <macgyver/Exception.h>
#include "grid-files/common/Log.h"
#include "grid-files/common/MemoryMapper.h"
#include "grid-files/common/ShowFunction.h"
#include "grid-files/common/GeneralFunctions.h"
#include "grid-files/common/ShowFunction.h"
#include "grid-files/identification/GridDef.h"
#include "grid-files/grid/PhysicalGridFile.h"
#include "grid-files/common/DataFetcher_network.h"
#include "grid-files/common/DataFetcher_filesys.h"
#include "grid-content/contentServer/redis/RedisImplementation.h"
#include "grid-content/contentServer/postgresql/PostgresqlImplementation.h"
#include "grid-content/contentServer/corba/client/ClientImplementation.h"
#include "grid-content/contentServer/http/client/ClientImplementation.h"
#include "grid-content/lua/LuaFile.h"

#include <libpq-fe.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#define FUNCTION_TRACE FUNCTION_TRACE_OFF


using namespace SmartMet;



class Location
{
  public:
    std::string type;
    std::string url;
    uint  authenticationMethod = 0;
    std::string username;
    std::string password;
    std::vector<std::string> patterns;
};



// Global variables:

ConfigurationFile         mConfigurationFile;
std::string               mGridConfigFile;
uint                      mSourceId = 100;
std::string               mProducerDefFile;
uint                      mMaxMessageSize = 5000;
std::string               mStorageType;
std::string               mRedisAddress;
int                       mRedisPort = 6379;
std::string               mRedisTablePrefix;
bool                      mRedisLockEnabled = false;
std::string               mContentServerIor;
std::string               mContentServerUrl;
std::string               mPostgresqlConnectionString;

bool                      mProcessingLogEnabled = false;
std::string               mProcessingLogFile;
int                       mProcessingLogMaxSize = 100000000;
int                       mProcessingLogTruncateSize = 20000000;
Log                       mProcessingLog;
Log*                      mProcessingLogPtr = nullptr;

bool                      mDebugLogEnabled = false;
std::string               mDebugLogFile;
int                       mDebugLogMaxSize = 100000000;
int                       mDebugLogTruncateSize = 20000000;
Log                       mDebugLog;
Log*                      mDebugLogPtr = nullptr;

T::SessionId              mSessionId = 0;
uint                      mGlobalFileId = 0;

T::ProducerInfoList       mSourceProducerList;
T::GenerationInfoList     mSourceGenerationList;
T::FileInfoList           mSourceFileList;
T::ContentInfoList        mSourceContentList;

T::ProducerInfoList       mTargetProducerList;
T::GenerationInfoList     mTargetGenerationList;
T::FileInfoList           mTargetFileList;
T::ContentInfoList        mTargetContentList;

std::map<std::string,std::string> mProducerAbbrList;
std::vector<Location>     mLocations;

Lua::LuaFile              mLuaFile;
std::string               mLuaFilename;
std::string               mLuaFunction;
std::string               mCacheDir = "/tmp";

bool                      mMemoryMapperEnabled = false;


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
      "smartmet.tools.grid.filesys2smartmet.content-source.locations",
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
      "smartmet.tools.grid.filesys2smartmet.processing-log.enabled",
      "smartmet.tools.grid.filesys2smartmet.processing-log.file",
      "smartmet.tools.grid.filesys2smartmet.processing-log.maxSize",
      "smartmet.tools.grid.filesys2smartmet.processing-log.truncateSize",
      nullptr
    };


    mConfigurationFile.readFile(configFile);
    //mConfigurationFile.print(std::cout,0,0);

    uint t=0;
    while (configAttribute[t] != nullptr)
    {
      if (!mConfigurationFile.findAttribute(configAttribute[t]))
      {
        Fmi::Exception exception(BCP, "Missing configuration attribute!");
        exception.addParameter("File",configFile);
        exception.addParameter("Attribute",configAttribute[t]);
        throw exception;
      }
      t++;
    }

    mConfigurationFile.getAttributeValue("smartmet.library.grid-files.configFile", mGridConfigFile);
    mConfigurationFile.getAttributeValue("smartmet.library.grid-files.memoryMapper.enabled", mMemoryMapperEnabled);

    mConfigurationFile.getAttributeValue("smartmet.tools.grid.filesys2smartmet.maxMessageSize",mMaxMessageSize);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.filesys2smartmet.content-source.source-id",mSourceId);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.filesys2smartmet.content-source.producerDefFile",mProducerDefFile);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.filesys2smartmet.content-source.cacheDir",mCacheDir);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.filesys2smartmet.content-source.filenameFixer.luaFilename",mLuaFilename);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.filesys2smartmet.content-source.filenameFixer.luaFunction",mLuaFunction);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.filesys2smartmet.content-storage.type",mStorageType);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.filesys2smartmet.content-storage.redis.address",mRedisAddress);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.filesys2smartmet.content-storage.redis.port",mRedisPort);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.filesys2smartmet.content-storage.redis.tablePrefix",mRedisTablePrefix);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.filesys2smartmet.content-storage.redis.lockEnabled",mRedisLockEnabled);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.filesys2smartmet.content-storage.postgresql.connection-string", mPostgresqlConnectionString);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.filesys2smartmet.content-storage.corba.ior",mContentServerIor);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.filesys2smartmet.content-storage.http.url",mContentServerUrl);

    mConfigurationFile.getAttributeValue("smartmet.tools.grid.filesys2smartmet.processing-log.enabled", mProcessingLogEnabled);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.filesys2smartmet.processing-log.file", mProcessingLogFile);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.filesys2smartmet.processing-log.maxSize", mProcessingLogMaxSize);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.filesys2smartmet.processing-log.truncateSize", mProcessingLogTruncateSize);

    mConfigurationFile.getAttributeValue("smartmet.tools.grid.filesys2smartmet.debug-log.enabled", mDebugLogEnabled);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.filesys2smartmet.debug-log.file", mDebugLogFile);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.filesys2smartmet.debug-log.maxSize", mDebugLogMaxSize);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.filesys2smartmet.debug-log.truncateSize", mDebugLogTruncateSize);

    bool ind = true;
    uint c = 0;
    char tmp[100];
    while (ind)
    {
      Location rec;
      sprintf(tmp,"smartmet.tools.grid.filesys2smartmet.content-source.locations.location.%u.type",c);
      if (mConfigurationFile.getAttributeValue(tmp,rec.type))
      {
        sprintf(tmp,"smartmet.tools.grid.filesys2smartmet.content-source.locations.location.%u.url",c);
        mConfigurationFile.getAttributeValue(tmp,rec.url);

        sprintf(tmp,"smartmet.tools.grid.filesys2smartmet.content-source.locations.location.%u.authentication.method",c);
        mConfigurationFile.getAttributeValue(tmp,rec.authenticationMethod);

        sprintf(tmp,"smartmet.tools.grid.filesys2smartmet.content-source.locations.location.%u.authentication.username",c);
        mConfigurationFile.getAttributeValue(tmp,rec.username);

        sprintf(tmp,"smartmet.tools.grid.filesys2smartmet.content-source.locations.location.%u.authentication.password",c);
        mConfigurationFile.getAttributeValue(tmp,rec.password);

        sprintf(tmp,"smartmet.tools.grid.filesys2smartmet.content-source.locations.location.%u.patterns",c);
        mConfigurationFile.getAttributeValue(tmp,rec.patterns);

        mLocations.push_back(rec);

        c++;
      }
      else
      {
        ind = false;
      }
    }

    if (mLocations.size() == 0)
    {
      Fmi::Exception exception(BCP, "No location defined!");
      exception.addParameter("File",configFile);
      exception.addParameter("Attribute","smartmet.tools.grid.filesys2smartmet.content-source.locations.location[0].type");
      throw exception;
    }


    if (mLuaFilename > " ")
     mLuaFile.init(mLuaFilename);
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Constructor failed!", nullptr);
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
      Fmi::Exception exception(BCP,"Cannot read the producer list from the target data storage!");
      exception.addParameter("Result",ContentServer::getResultString(result));
      throw exception;
    }
  }
  catch (...)
  {
    throw Fmi::Exception(BCP,"Operation failed!",nullptr);
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
      Fmi::Exception exception(BCP,"Cannot read the generation list from the target data storage!");
      exception.addParameter("Result",ContentServer::getResultString(result));
      throw exception;
    }
  }
  catch (...)
  {
    throw Fmi::Exception(BCP,"Operation failed!",nullptr);
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
        Fmi::Exception exception(BCP,"Cannot read the file list from the target data storage!");
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
    throw Fmi::Exception(BCP,"Operation failed!",nullptr);
  }
}



void splitFilename(std::string& fullName,std::string& path,std::string& filename)
{
  char buf[1000];
  strcpy(buf,fullName.c_str());
  char *p = buf;
  char *pr = buf;
  while (p != nullptr)
  {
    p = strstr(p,"/");
    if (p)
    {
      pr = p;
      p++;
    }
  }

  if (*pr == '/')
  {
    *pr = '\0';
    pr++;
    path = buf;
    filename = pr;
    return;
  }

  path = "";
  filename = fullName;
}




void readSourceFiles(std::vector<DataFetcher::FileRec>& fileList)
{
  FUNCTION_TRACE
  try
  {
    mSourceFileList.clear();
    mSourceContentList.clear();

    for (auto it = fileList.begin(); it != fileList.end(); ++it)
    {
      std::string fn;
      std::string path;
      splitFilename(it->filename,path,fn);
      if (mLuaFunction > " ")
      {
        std::vector<std::string> params;
        params.push_back(fn);
        params.push_back(path);
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
          T::ProducerInfo *producer = mTargetProducerList.getProducerInfoByName(pp->second.c_str());
          if (producer == nullptr)
          {
            //std::cout << "Producer not found : " << pp->second << "\n";
          }
          else
          {
            char st[1000];
            sprintf(st,"%s:%s",producer->mName.c_str(),generationTime.c_str());
            std::string str = st;
            T::GenerationInfo *generation =  mTargetGenerationList.getGenerationInfoByName(str);
            if (generation != nullptr)
            {
              T::FileInfo *fileInfo = new T::FileInfo();
              fileInfo->mProducerId = producer->mProducerId;
              fileInfo->mGenerationId = generation->mGenerationId;
              fileInfo->mFileId = mSourceFileList.getLength() + 1;
              //fileInfo->mFileType;
              fileInfo->mName = it->filename;
              fileInfo->mFlags = 0;
              fileInfo->mSourceId = mSourceId;
              fileInfo->mModificationTime = it->lastModified;
              fileInfo->mServer = it->server;
              fileInfo->mServerType = it->serverType;
              fileInfo->mProtocol = it->protocol;
              fileInfo->mSize = it->size;


              mSourceFileList.addFileInfo(fileInfo);
              //fileInfo->print(std::cout,0,0);
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
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}





void readSourceGenerations(std::vector<DataFetcher::FileRec>& fileList)
{
  FUNCTION_TRACE
  try
  {
    std::set<std::string> processedGenerations;

    mSourceGenerationList.clear();

    for (auto it = fileList.begin(); it != fileList.end(); ++it)
    {
      std::string fn;
      std::string path;
      splitFilename(it->filename,path,fn);
      //std::string fn = it->second.c_str();
      if (mLuaFunction > " ")
      {
        std::vector<std::string> params;
        params.push_back(fn);
        params.push_back(path);
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

              std::string str = st;
              T::GenerationInfo *generation =  mSourceGenerationList.getGenerationInfoByName(str);
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
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}





void setMessageContent(SmartMet::GRID::GridFile& gridFile,SmartMet::GRID::Message& message,T::ContentInfo& contentInfo)
{
  try
  {
    contentInfo.mFileType = message.getMessageType();
    contentInfo.mFilePosition = message.getFilePosition();
    contentInfo.mMessageSize = message.getMessageSize();
    contentInfo.setForecastTime(message.getForecastTime());
    contentInfo.mFmiParameterId = message.getFmiParameterId();
    contentInfo.setFmiParameterName(message.getFmiParameterName());
    contentInfo.mFmiParameterLevelId = message.getFmiParameterLevelId();
    contentInfo.mParameterLevel = message.getGridParameterLevel();
    contentInfo.mForecastType = message.getForecastType();
    contentInfo.mForecastNumber = message.getForecastNumber();
    contentInfo.mFlags = 0;
    contentInfo.mSourceId = mSourceId;
    contentInfo.mGeometryId = message.getGridGeometryId();
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}





void getCacheFileName(const char *filename,char *cacheFilename)
{
  try
  {
    std::size_t hash = 0;
    boost::hash_combine(hash,std::string(filename));
    sprintf(cacheFilename,"%s/F2S_%lu",mCacheDir.c_str(),hash);
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}





bool readSourceContentCache(const char *cacheFilename,time_t modificationTime,T::ContentInfoList& contentList)
{
  try
  {
    FILE *file = fopen(cacheFilename,"r");
    if (file == NULL)
      return false;

    char buf[1000];
    if (fgets(buf,1000,file) != 0  &&  buf[0] == '#')
    {
      char *p = strstr(buf,"\n");
      if (p)
        *p = '\0';

      time_t modTime = atoll(buf+1);
      if (modTime != modificationTime)
      {
        fclose(file);
        remove(cacheFilename);
        return false;
      }
    }

    while (!feof(file))
    {
      if (fgets(buf,1000,file)  &&  buf[0] != '#')
      {
        T::ContentInfo *contentInfo = new T::ContentInfo();
        contentInfo->setCsv(buf);
        if (contentInfo->mModificationTime == modificationTime)
        {
          contentList.addContentInfo(contentInfo);
        }
        else
        {
          delete contentInfo;
        }
      }
    }
    fclose(file);
    return true;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}




void readSourceContent(T::FileInfo& fileInfo,T::ContentInfoList& contentList)
{
  try
  {
    contentList.clear();

    char fname[1000];
    sprintf(fname,"%s:%s",fileInfo.mServer.c_str(),fileInfo.mName.c_str());

    char cacheFilename[1000];
    getCacheFileName(fname,cacheFilename);

    if (readSourceContentCache(cacheFilename,fileInfo.mModificationTime,contentList))
      return;

    FILE *file = fopen(cacheFilename,"w");
    if (file == NULL)
    {
      Fmi::Exception exception(BCP, "Cannot create cached index file!");
      exception.addParameter("CacheFilename",cacheFilename);
      throw exception;
    }

    fprintf(file,"#%lu\n",fileInfo.mModificationTime);
    fprintf(file,"#%s\n",fname);

    SmartMet::GRID::PhysicalGridFile gridFile;
    gridFile.setServer(fileInfo.mServer);
    gridFile.setProtocol(fileInfo.mProtocol);
    gridFile.setServerType(fileInfo.mServerType);
    gridFile.setSize(fileInfo.mSize);
    gridFile.read(fileInfo.mName);

    uint messageCount = gridFile.getNumberOfMessages();
    for (uint t=0; t<messageCount; t++)
    {
      SmartMet::GRID::Message *message = gridFile.getMessageByIndex(t);
      if (message != nullptr)
      {
        T::ContentInfo *contentInfo = new T::ContentInfo();
        contentInfo->mFileId = 0;
        contentInfo->mMessageIndex = t;
        contentInfo->mProducerId = fileInfo.mProducerId;
        contentInfo->mGenerationId = fileInfo.mGenerationId;
        contentInfo->mModificationTime = fileInfo.mModificationTime;

        setMessageContent(gridFile,*message,*contentInfo);
        //contentInfo->print(std::cout,0,0);
        contentList.addContentInfo(contentInfo);
        std::string s = contentInfo->getCsv();
        fprintf(file,"%s\n",s.c_str());
      }
    }
    fclose(file);
    //contentList.print(std::cout,0,0);
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.printError();

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
      Fmi::Exception exception(BCP,"Cannot open file!");
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
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
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
            PRINT_EVENT_LINE(mProcessingLogPtr,"PRODUCER-DELETE;FAIL;%s;",targetProducer->mName.c_str());

            Fmi::Exception exception(BCP,"Cannot delete the producer information from the target data storage!");
            exception.addParameter("ProducerId",std::to_string(targetProducer->mProducerId));
            exception.addParameter("ProducerName",targetProducer->mName);
            exception.addParameter("Result",ContentServer::getResultString(result));
            throw exception;
          }
          else
          {
            PRINT_EVENT_LINE(mProcessingLogPtr,"PRODUCER-DELETE;OK;%s;",targetProducer->mName.c_str());
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
            PRINT_EVENT_LINE(mProcessingLogPtr,"PRODUCER-ADD;FAIL;%s;",producer.mName.c_str());

            Fmi::Exception exception(BCP,"Cannot add the producer information into the target data storage!");
            exception.addParameter("ProducerId",std::to_string(sourceProducer->mProducerId));
            exception.addParameter("ProducerName",sourceProducer->mName);
            exception.addParameter("Result",ContentServer::getResultString(result));
            throw exception;
          }
          else
          {
            PRINT_EVENT_LINE(mProcessingLogPtr,"PRODUCER-ADD;OK;%s;",producer.mName.c_str());
          }
        }
      }
    }
  }
  catch (...)
  {
    throw Fmi::Exception(BCP,"Operation failed!",nullptr);
  }
}





void updateGenerations(ContentServer::ServiceInterface *targetInterface)
{
  FUNCTION_TRACE
  try
  {
    std::set<uint> generationIdList;
    std::vector<std::string> generationNameList;
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
          generationNameList.push_back(targetGeneration->mName);
        }
      }
    }

    if (generationIdList.size() > 0)
    {
      int result = targetInterface->deleteGenerationInfoListByIdList(mSessionId,generationIdList);
      if (result != 0)
      {
        for (auto it = generationNameList.begin(); it != generationNameList.end(); ++it)
          PRINT_EVENT_LINE(mProcessingLogPtr,"GENERATION-DELETE;FAIL;%s;",it->c_str());

        Fmi::Exception exception(BCP,"Cannot delete the generation information from the target data storage!");
        exception.addParameter("Result",ContentServer::getResultString(result));
        throw exception;
      }
      else
      {
        for (auto it = generationNameList.begin(); it != generationNameList.end(); ++it)
          PRINT_EVENT_LINE(mProcessingLogPtr,"GENERATION-DELETE;OK;%s;",it->c_str());
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
            PRINT_EVENT_LINE(mProcessingLogPtr,"GENERATION-ADD;FAIL;%s;",generationInfo.mName.c_str());

            Fmi::Exception exception(BCP,"Cannot add the generation information into the target data storage!");
            exception.addParameter("GenerationName",generationInfo.mName);
            exception.addParameter("Result",ContentServer::getResultString(result));
            throw exception;
          }
          else
          {
            PRINT_EVENT_LINE(mProcessingLogPtr,"GENERATION-ADD;OK;%s;",generationInfo.mName.c_str());
          }
        }
      }
    }
  }
  catch (...)
  {
    throw Fmi::Exception(BCP,"Operation failed!",nullptr);
  }
}





void updateFiles(ContentServer::ServiceInterface *targetInterface)
{
  FUNCTION_TRACE
  try
  {
    std::set<uint> fileIdList;
    std::vector<std::string> fileNameList;
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

          fileNameList.push_back(targetFile->mName);
          fileIdList.insert(targetFile->mFileId);
        }
      }
    }

    if (fileIdList.size() > 0)
    {
      int result = targetInterface->deleteFileInfoListByFileIdList(mSessionId,fileIdList);
      if (result != 0)
      {
        for (auto it = fileNameList.begin(); it != fileNameList.end(); ++it)
          PRINT_EVENT_LINE(mProcessingLogPtr,"FILE-DELETE;FAIL;%s;",it->c_str());


        Fmi::Exception exception(BCP,"Cannot delete the file information from the target data storage!");
        exception.addParameter("Result",ContentServer::getResultString(result));
        throw exception;
      }
      else
      {
        for (auto it = fileNameList.begin(); it != fileNameList.end(); ++it)
          PRINT_EVENT_LINE(mProcessingLogPtr,"FILE-DELETE;OK;%s;",it->c_str());
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
            PRINT_EVENT_LINE(mProcessingLogPtr,"FILE-DELETE;FAIL;%s;",targetFile->mName.c_str());

            Fmi::Exception exception(BCP,"Cannot delete the file information from the target data storage!");
            exception.addParameter("Result",ContentServer::getResultString(result));
            throw exception;
          }
          else
          {
            PRINT_EVENT_LINE(mProcessingLogPtr,"FILE-DELETE;OK;%s;",targetFile->mName.c_str());
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
          try
          {
            readSourceContent(fileInfo,contentList);
          }
          catch (...)
          {
            Fmi::Exception exception(BCP,"File read failed!",nullptr);
            exception.printError();
            std::string st = exception.getStackTrace();
            PRINT_DATA(mDebugLogPtr,"%s",st.c_str());
          }

          if (contentList.getLength() > 0)
          {
            int result = targetInterface->addFileInfoWithContentList(mSessionId,fileInfo,contentList);
            if (result != 0)
            {
              PRINT_EVENT_LINE(mProcessingLogPtr,"FILE-ADD;FAIL;%s;",fileInfo.mName.c_str());

              Fmi::Exception exception(BCP,"Cannot add the file information into the target data storage!");
              exception.addParameter("FileName",fileInfo.mName);
              exception.addParameter("Result",ContentServer::getResultString(result));
              throw exception;
            }
            else
            {
              PRINT_EVENT_LINE(mProcessingLogPtr,"FILE-ADD;OK;%s;",fileInfo.mName.c_str());
            }
          }
          else
          {
            PRINT_EVENT_LINE(mProcessingLogPtr,"FILE-READ;FAIL;%s;",fileInfo.mName.c_str());
          }
        }
      }
    }
  }
  catch (...)
  {
    throw Fmi::Exception(BCP,"Operation failed!",nullptr);
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

    // Initializing the global structures. These are needed when
    // extracting information from GRIB files.

    Identification::gridDef.init(mGridConfigFile.c_str());

    ContentServer::ServiceInterface *targetInterface = nullptr;
    ContentServer::RedisImplementation *redisImplementation = nullptr;

    uint waitTime = toInt64(argv[2]);

    if (mStorageType =="redis")
    {
      redisImplementation = new ContentServer::RedisImplementation();
      redisImplementation->init(mRedisAddress.c_str(),mRedisPort,mRedisTablePrefix.c_str());
      targetInterface = redisImplementation;
    }

    if (mStorageType == "postgresql")
    {
      ContentServer::PostgresqlImplementation *client = new ContentServer::PostgresqlImplementation();
      client->init(mPostgresqlConnectionString.c_str(),"",true);
      targetInterface = client;
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

    if (mProcessingLogEnabled)
    {
      mProcessingLog.init(true,mProcessingLogFile.c_str(),mProcessingLogMaxSize,mProcessingLogTruncateSize);
      mProcessingLogPtr = &mProcessingLog;
    }

    if (targetInterface == nullptr)
    {
      fprintf(stderr,"No target data source defined!\n");
      return -3;
    }


    memoryMapper.setEnabled(mMemoryMapperEnabled);

    bool ind = true;
    while (ind)
    {
      PRINT_EVENT_LINE(mProcessingLogPtr,"UPDATE-LOOP-START");

      PRINT_DATA(mDebugLogPtr,"\n");
      PRINT_DATA(mDebugLogPtr,"********************************************************************\n");
      PRINT_DATA(mDebugLogPtr,"****************************** UPDATE ******************************\n");
      PRINT_DATA(mDebugLogPtr,"********************************************************************\n");

      //std::set<std::string> dirList;
      //std::vector<std::pair<std::string,std::string>> fileList;

      uint serverType = 0;
      uint protocol = 0;
      std::string server;
      std::string dirName;

      DataFetcher_filesys df_filesys;
      DataFetcher_network df_network;
      std::vector<DataFetcher::FileRec> fileList;

      for (auto location = mLocations.begin(); location != mLocations.end(); ++location)
      {
        //printf("LOCATION %s\n",location->url.c_str());
        if (location->type.empty() ||  strcmp(location->type.c_str(),"FS") == 0)
        {
          serverType = DataFetcher::ServerType::Filesys;
          dirName = location->url;
          df_filesys.getFileList(serverType,protocol,server.c_str(),dirName.c_str(),location->patterns,fileList);
        }
        else
        if (mMemoryMapperEnabled  &&  (location->type == "S3" ||  location->type == "s3"))
        {
          serverType = DataFetcher::ServerType::S3;
          DataFetcher::splitUrl(location->url.c_str(),protocol,server,dirName);

          if (location->authenticationMethod)
          {
            memoryMapper.addAccessInfo(server.c_str(),location->authenticationMethod,location->username.c_str(),location->password.c_str());
            df_network.addAccessInfo(server.c_str(),location->authenticationMethod,location->username.c_str(),location->password.c_str());
          }

          df_network.getFileList(serverType,protocol,server.c_str(),dirName.c_str(),location->patterns,fileList);
          //std::cout << "GET : " << dir->c_str() << " : " << server << " : " <<  protocol << " : " << dirName << " : " << fileList.size() << "\n";
        }
      }

      //for (auto it = fileList.begin(); it != fileList.end(); ++it)
      //  std::cout << it->filename << ";" << it->size << ";" << utcTimeFromTimeT(it->lastModified) << "\n";

      // Defining index cache filenames
      std::set<std::string> cacheFiles;
      char filename[1000];
      for (auto it = fileList.begin();it != fileList.end();++it)
      {
        sprintf(filename,"%s:%s",it->server.c_str(),it->filename.c_str());
        char cacheFilename[1000];
        getCacheFileName(filename,cacheFilename);
        cacheFiles.insert(std::string(cacheFilename));
      }

      // Finding current index cache filenames
      std::vector<std::string> cachePatterns;
      cachePatterns.push_back(std::string("F2S_*"));
      std::set<std::string> cacheDirList;
      std::vector<std::pair<std::string,std::string>> cacheFileList;
      getFileList(mCacheDir.c_str(),cachePatterns,false,cacheDirList,cacheFileList);

      // Removing index cache files that do not have corresponding grid files
      for (auto it = cacheFileList.begin();it != cacheFileList.end();++it)
      {
        sprintf(filename,"%s:%s",it->first.c_str(),it->second.c_str());
        if (cacheFiles.find(filename) == cacheFiles.end())
          remove(filename);
      }

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

      if (redisImplementation)
      {
        PRINT_DATA(mDebugLogPtr, "* Checking filenames \n");
        redisImplementation->syncFilenames();
      }


      PRINT_DATA(mDebugLogPtr,"********************************************************************\n\n");

      PRINT_EVENT_LINE(mProcessingLogPtr,"UPDATE-LOOP-END");

      if (waitTime > 0)
        sleep(waitTime);
      else
        ind = false;
    }

    return 0;
  }
  catch (Fmi::Exception& e)
  {
    Fmi::Exception exception(BCP,"Service call failed!",nullptr);
    exception.printError();
    return -3;
  }
}

