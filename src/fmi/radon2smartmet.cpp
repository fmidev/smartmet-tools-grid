#include <macgyver/Exception.h>
#include "grid-files/common/Log.h"
#include "grid-files/common/ShowFunction.h"
#include "grid-files/common/GeneralFunctions.h"
#include "grid-files/common/ShowFunction.h"
#include "grid-files/identification/GridDef.h"
#include "grid-content/contentServer/redis/RedisImplementation.h"
#include "grid-content/contentServer/corba/client/ClientImplementation.h"
#include "grid-content/contentServer/http/client/ClientImplementation.h"
#include <grid-content/contentServer/cache/CacheImplementation.h>
#include <boost/bimap.hpp>

#include <libpq-fe.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>


#define FUNCTION_TRACE FUNCTION_TRACE_OFF

using namespace SmartMet;

// Some temporary storage structures:

struct ForecastRec
{
    uint producerId = 0;
    std::string producerName;
    std::string analysisTime;
    time_t deletionTime = 0;
    std::string generationName;
    std::string forecastTime;
    std::string forecastPeriod;
    std::string tableName;
    uint geometryId = 0;
    int forecastTypeId = 0;
    int forecastTypeValue = 0;
    time_t lastUpdated;
};

struct FileRec
{
    int fileId;
    uchar format;
    uint messageIndex;
    ulonglong filePosition;
    uint messageSize;
    uint paramId;
    short levelId;
    int levelValue;
    time_t analysisTime;
    time_t forecastTime;
    short forecastType;
    short forecastNumber;
    int geometryId;
    int producerId;
    std::string producerName;
    time_t lastUpdated;
};

struct FileRecVec
{
    time_t updateTime;
    std::string info;
    uint loadCounter;
    std::vector<FileRec> files;
};

typedef std::unordered_map<std::string, FileRecVec> FileRec_map;


// Global variables:

std::vector<FileRec> emptyRecList;
ConfigurationFile mConfigurationFile;
std::string mGridConfigFile;
uint mSourceId = 100;
std::string mProducerFile;
uint mMaxMessageSize = 50000;
std::string mRadonConnectionString;
std::string mStorageType;
std::string mRedisAddress;
int mRedisPort = 6379;
bool mRedisLockEnabled = false;
std::string mRedisTablePrefix;
std::string mContentServerIor;
std::string mContentServerUrl;
bool mDebugLogEnabled = false;
std::string mDebugLogFile;
int mDebugLogMaxSize = 100000000;
int mDebugLogTruncateSize = 20000000;
Log mDebugLog;
Log* mDebugLogPtr = nullptr;
T::SessionId mSessionId = 0;

std::set<std::string> mProducerList;
std::set<uint> mProducerIdList;
std::unordered_map<std::string, std::vector<std::string>> mProducerDependensies;


std::set<std::string> mUpdateProducerList;
T::ProducerInfoList mSourceProducerList;
T::GenerationInfoList mSourceGenerationList;
//std::vector<ForecastRec>  mSourceForacastList;
std::set<int> mSourceFilenames;
int mFileIdCounter = 0;

typedef boost::bimap< std::string, int > string_bimap;
typedef string_bimap::value_type bimap_rec;

string_bimap mFilenameMap;

T::ProducerInfoList mTargetProducerList;
T::GenerationInfoList mTargetGenerationList;
std::set<unsigned long long> mTargetContentList;
uint fileReadCount = 0;

FileRec_map mFileRecMap;
std::set<std::string> mFileTables;
std::unordered_map<std::string, std::string> mTableUpdates;
uint mWaitTime = 300;
uint mContentReadCount = 0;
uint mContentAddCount = 0;
uint mFileLoadCounter = 0;
int mLoopCounter = 1;
std::set<uint> mIgnoreGeneration;
bool reconnectionRequired = true;
bool shutdownRequested = false;
std::set<std::string> mGenerationStatusCheckIgnore;


ContentServer::ServiceInterface *mTargetInterface = nullptr;
std::unordered_map<std::string,time_t> mReadyGenerations;




void sig_handler(int signum)
{
  printf("##### SHUTDOWN REQUESTED #####\n");
  shutdownRequested = true;
}



void readConfigFile(const char* configFile)
{
  try
  {
    const char *configAttribute[] =
    {
        "smartmet.library.grid-files.configFile",
        "smartmet.tools.grid.radon2smartmet.maxMessageSize",
        "smartmet.tools.grid.radon2smartmet.content-source.source-id",
        "smartmet.tools.grid.radon2smartmet.content-source.producerFile",
        "smartmet.tools.grid.radon2smartmet.content-source.radon.connection-string",
        "smartmet.tools.grid.radon2smartmet.content-storage.type",
        "smartmet.tools.grid.radon2smartmet.content-storage.redis.address",
        "smartmet.tools.grid.radon2smartmet.content-storage.redis.port",
        "smartmet.tools.grid.radon2smartmet.content-storage.redis.tablePrefix",
        "smartmet.tools.grid.radon2smartmet.content-storage.corba.ior",
        "smartmet.tools.grid.radon2smartmet.content-storage.http.url",
        "smartmet.tools.grid.radon2smartmet.debug-log.enabled",
        "smartmet.tools.grid.radon2smartmet.debug-log.file",
        "smartmet.tools.grid.radon2smartmet.debug-log.maxSize",
        "smartmet.tools.grid.radon2smartmet.debug-log.truncateSize", nullptr };

    mConfigurationFile.readFile(configFile);
    //mConfigurationFile.print(std::cout,0,0);

    uint t = 0;
    while (configAttribute[t] != nullptr)
    {
      if (!mConfigurationFile.findAttribute(configAttribute[t]))
      {
        Fmi::Exception exception(BCP, "Missing configuration attribute!");
        exception.addParameter("File", configFile);
        exception.addParameter("Attribute", configAttribute[t]);
        throw exception;
      }
      t++;
    }

    mConfigurationFile.getAttributeValue("smartmet.library.grid-files.configFile", mGridConfigFile);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.maxMessageSize", mMaxMessageSize);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.content-source.source-id", mSourceId);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.content-source.producerFile", mProducerFile);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.content-source.radon.connection-string", mRadonConnectionString);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.content-storage.type", mStorageType);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.content-storage.redis.address", mRedisAddress);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.content-storage.redis.port", mRedisPort);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.content-storage.redis.tablePrefix", mRedisTablePrefix);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.content-storage.redis.lockEnabled", mRedisLockEnabled);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.content-storage.corba.ior", mContentServerIor);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.content-storage.http.url", mContentServerUrl);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.debug-log.enabled", mDebugLogEnabled);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.debug-log.file", mDebugLogFile);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.debug-log.maxSize", mDebugLogMaxSize);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.debug-log.truncateSize", mDebugLogTruncateSize);
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Constructor failed!", nullptr);
  }
}





int getFileId(std::string filename,bool create)
{
  try
  {
    auto pos = mFilenameMap.left.find(filename);
    if (pos != mFilenameMap.left.end())
      return pos->second;

    if (!create)
      return -1;

    int id = mFileIdCounter++;

    mFilenameMap.insert(bimap_rec(filename,id));
    return id;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Constructor failed!", nullptr);
  }
}




std::string getFilename(int fileId)
{
  try
  {
    auto pos = mFilenameMap.right.find(fileId);
    if (pos != mFilenameMap.right.end())
      return pos->second;

    return "";
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Constructor failed!", nullptr);
  }
}





void readProducerList(const char *filename)
{
  FUNCTION_TRACE
  try
  {
    FILE *file = fopen(filename, "re");
    if (file == nullptr)
    {
      PRINT_DATA(mDebugLogPtr, "  -- Producer file not available. Accepting all produces\n");
      return;
    }

    mProducerList.clear();
    mProducerDependensies.clear();

    char st[1000];
    while (!feof(file))
    {
      if (fgets(st,1000,file) != nullptr  &&  st[0] != '#')
      {
        int f = 1;
        int i = 1;
        int e = 0;
        char *p = strstr(st,"\n");
        if (p)
          *p = '\0';

        std::vector <std::string> fields;
        splitString(st,';',fields);

        if (fields.size() > 1)
          e = toInt32(fields[1]);

        if (fields.size() > 2)
          f = toInt32(fields[2]);

        if (fields.size() > 3)
          i = toInt32(fields[3]);

        if (mWaitTime == 0 || mLoopCounter == f || (mLoopCounter > f  &&  ((mLoopCounter-f) % i) == 0))
        {
          std::vector<std::string> pList;
          splitString(fields[0],',',pList);

          for (auto it = pList.begin(); it != pList.end(); ++it)
          {
            std::string pname = toUpperString(*it);
            mProducerList.insert(pname);

            if (e == 0)
              mGenerationStatusCheckIgnore.insert(pname);

            if (pList.size() > 1)
              mProducerDependensies.insert(std::pair<std::string, std::vector<std::string>>(pname,pList));
          }
        }
      }
    }
    fclose(file);
    mLoopCounter++;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}





void readTargetProducers(T::ProducerInfoList& targetProducerList)
{
  FUNCTION_TRACE
  try
  {
    targetProducerList.clear();
    int result = mTargetInterface->getProducerInfoList(mSessionId, targetProducerList);
    if (result != 0)
    {
      Fmi::Exception exception(BCP, "Cannot read the producer list from the target data storage!");
      exception.addParameter("Result", ContentServer::getResultString(result));
      throw exception;
    }
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}




void readTargetGenerations(T::GenerationInfoList& targetGenerationList)
{
  FUNCTION_TRACE
  try
  {
    targetGenerationList.clear();
    targetGenerationList.setComparisonMethod(T::GenerationInfo::ComparisonMethod::none);
    int result = mTargetInterface->getGenerationInfoList(mSessionId, targetGenerationList);
    if (result != 0)
    {
      Fmi::Exception exception(BCP, "Cannot read the generation list from the target data storage!");
      exception.addParameter("Result", ContentServer::getResultString(result));
      throw exception;
    }
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}




void readTargetFileList(T::FileInfoList& targetFileList)
{
  FUNCTION_TRACE
  try
  {
    targetFileList.clear();
    targetFileList.setComparisonMethod(T::FileInfo::ComparisonMethod::none);
    uint startFileId = 0;
    uint len = 50000;
    while (len > 0)
    {
      T::FileInfoList fileInfoList;
      mTargetInterface->getFileInfoList(mSessionId, startFileId, 50000, fileInfoList);

      len = fileInfoList.getLength();
      for (uint t = 0; t < len; t++)
      {
        T::FileInfo *fileInfo = fileInfoList.getFileInfoByIndex(t);
        if (fileInfo != nullptr)
        {
          targetFileList.addFileInfo(fileInfo->duplicate());

          if (fileInfo->mFileId >= startFileId)
            startFileId = fileInfo->mFileId + 1;
        }
      }
    }
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}




void readTargetFileList(uint producerId, T::FileInfoList& targetFileList)
{
  FUNCTION_TRACE
  try
  {
    targetFileList.clear();
    targetFileList.setComparisonMethod(T::FileInfo::ComparisonMethod::none);
    uint startFileId = 0;
    uint len = 50000;
    while (len > 0)
    {
      T::FileInfoList fileInfoList;
      mTargetInterface->getFileInfoListByProducerId(mSessionId, producerId, startFileId, 50000, fileInfoList);

      len = fileInfoList.getLength();
      for (uint t = 0; t < len; t++)
      {
        T::FileInfo *fileInfo = fileInfoList.getFileInfoByIndex(t);
        if (fileInfo != nullptr)
        {
          // Clearing information that we do not need (=> saving memory)

          targetFileList.addFileInfo(fileInfo->duplicate());

          if (fileInfo->mFileId >= startFileId)
            startFileId = fileInfo->mFileId + 1;
        }
      }
    }
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}




void readTargetContentList(uint producerId, T::ContentInfoList& targetContentList)
{
  FUNCTION_TRACE
  try
  {
    targetContentList.clear();

    uint startFileId = 0;
    uint startMessageIndex = 0;
    uint len = 50000;
    while (len > 0)
    {
      T::ContentInfoList contentInfoList;
      int result = mTargetInterface->getContentListByProducerId(mSessionId, producerId, startFileId, startMessageIndex, 50000, contentInfoList);

      if (result == 0)
      {
        len = contentInfoList.getLength();
        for (uint t = 0; t < len; t++)
        {
          T::ContentInfo *contentInfo = contentInfoList.getContentInfoByIndex(t);
          if (contentInfo != nullptr)
          {
            startFileId = contentInfo->mFileId;
            startMessageIndex = contentInfo->mMessageIndex + 1;

            targetContentList.addContentInfo(contentInfo->duplicate());
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




void readTargetContentList(T::ContentInfoList& targetContentList)
{
  FUNCTION_TRACE
  try
  {
    targetContentList.clear();

    uint startFileId = 0;
    uint startMessageIndex = 0;
    uint len = 50000;
    while (len > 0)
    {
      T::ContentInfoList contentInfoList;
      int result = mTargetInterface->getContentList(mSessionId, startFileId, startMessageIndex, 50000, contentInfoList);

      if (result == 0)
      {
        len = contentInfoList.getLength();
        for (uint t = 0; t < len; t++)
        {
          T::ContentInfo *contentInfo = contentInfoList.getContentInfoByIndex(t);
          if (contentInfo != nullptr)
          {
            startFileId = contentInfo->mFileId;
            startMessageIndex = contentInfo->mMessageIndex + 1;

            targetContentList.addContentInfo(contentInfo->duplicate());
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





void readTargetContentList(std::set<unsigned long long>& targetContentList)
{
  FUNCTION_TRACE
  try
  {
    targetContentList.clear();

    uint startFileId = 0;
    uint startMessageIndex = 0;
    uint len = 50000;
    while (len > 0)
    {
      T::ContentInfoList contentInfoList;
      int result = mTargetInterface->getContentList(mSessionId, startFileId, startMessageIndex, 50000, contentInfoList);

      if (result == 0)
      {
        len = contentInfoList.getLength();
        for (uint t = 0; t < len; t++)
        {
          T::ContentInfo *contentInfo = contentInfoList.getContentInfoByIndex(t);
          if (contentInfo != nullptr)
          {
            startFileId = contentInfo->mFileId;
            startMessageIndex = contentInfo->mMessageIndex + 1;

            unsigned long long id = contentInfo->mFileId;
            id = (id << 32) + contentInfo->mMessageIndex;

            targetContentList.insert(id);
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





void readTargetContentList(uint producerId,std::set<unsigned long long>& targetContentList)
{
  FUNCTION_TRACE
  try
  {
    targetContentList.clear();

    uint startFileId = 0;
    uint startMessageIndex = 0;
    uint len = 50000;
    while (len > 0)
    {
      T::ContentInfoList contentInfoList;
      int result = mTargetInterface->getContentListByProducerId(mSessionId, producerId, startFileId, startMessageIndex, 50000, contentInfoList);

      if (result == 0)
      {
        len = contentInfoList.getLength();
        for (uint t = 0; t < len; t++)
        {
          T::ContentInfo *contentInfo = contentInfoList.getContentInfoByIndex(t);
          if (contentInfo != nullptr  &&  contentInfo->mProducerId == producerId)
          {
            startFileId = contentInfo->mFileId;
            startMessageIndex = contentInfo->mMessageIndex + 1;

            unsigned long long id = contentInfo->mFileId;
            id = (id << 32) + contentInfo->mMessageIndex;

            targetContentList.insert(id);
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





std::string getTableInfo(PGconn *conn, const char *tableName, uint producerId, const char *analysisTime,time_t& lastUpdate,uint& count)
{
  FUNCTION_TRACE
  try
  {
    if (shutdownRequested)
      return "";

    std::string tbl = std::string(tableName) + ":" + std::to_string(producerId) + ":" + std::string(analysisTime);
    auto pos = mTableUpdates.find(tbl);
    if (pos != mTableUpdates.end())
      return pos->second;

    char sql[2000];
    char *p = sql;

    PRINT_DATA(mDebugLogPtr, "      ---- Read table information %s:%u:%s\n", tableName, producerId, analysisTime);

    p += sprintf(p, "SELECT\n");
    p += sprintf(p, "  count(file_location),\n");
    p += sprintf(p, "  max(to_char(last_updated, 'yyyymmddThh24MISS'))\n");
    p += sprintf(p, "FROM\n");
    p += sprintf(p, "  %s\n", tableName);
    p += sprintf(p, "WHERE\n");
    p += sprintf(p, "  producer_id=%u AND analysis_time = to_timestamp('%s', 'yyyymmddThh24MISS')\n", producerId, analysisTime);

    //printf("%s\n",sql);
    PGresult *res = PQexec(conn, sql);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
      Fmi::Exception exception(BCP, "Postgresql error!");
      exception.addParameter("ErrorMessage", PQerrorMessage(conn));
      throw exception;
    }

    //int fieldCount = PQnfields(res);
    int rowCount = PQntuples(res);
    std::string result;

    if (rowCount == 1)
    {
      count = atoi(PQgetvalue(res, 0, 0));
      std::string tt = PQgetvalue(res, 0, 1);
      if (!tt.empty())
        lastUpdate =  utcTimeToTimeT(PQgetvalue(res, 0, 1));
      else
        lastUpdate = 0;

      result = std::to_string(lastUpdate) + ":" + std::to_string(count);

      mTableUpdates.insert(std::pair<std::string, std::string>(tbl, result));
    }
    PQclear(res);

    return result;
  }
  catch (...)
  {
    reconnectionRequired = true;
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.printError();
    std::string tmp;
    return tmp;
    // throw exception;
  }
}




void readTableRecords(PGconn *conn, const char *tableName, uint producerId, std::string& producerName,const char *analysisTime, time_t updateTime, std::string& info)
{
  FUNCTION_TRACE
  try
  {
    if (shutdownRequested)
      return;

    std::string tbl = std::string(tableName) + ":" + std::to_string(producerId) + ":" + std::string(analysisTime);
    if (mFileTables.find(tbl) != mFileTables.end())
      return;

    mFileTables.insert(tbl);

    char sql[2000];
    char *p = sql;

    p += sprintf(p, "SELECT\n");
    // When encountering files in s3 storage, prepend a slash so that the file location looks like an absolute path to some directory.
    // Then just expect that the bucket is mounted to correct location using s3fs or similar.
    // For example file in s3: 'my-bucket/my-file.grib' becomes '/my-bucket/my-file.grib'
    p += sprintf(p, "  CASE file_protocol_id WHEN 1 THEN file_location WHEN 2 THEN '/' || file_location END AS file_location,\n");
    p += sprintf(p, "  message_no,\n");
    p += sprintf(p, "  byte_offset,\n");
    p += sprintf(p, "  byte_length,\n");
    p += sprintf(p, "  param_id,\n");
    p += sprintf(p, "  level_id,\n");
    p += sprintf(p, "  level_value::int,\n");
    p += sprintf(p, "  to_char(analysis_time, 'yyyymmddThh24MISS'),\n");
    p += sprintf(p, "  to_char((analysis_time+forecast_period) at time zone 'utc','yyyymmddThh24MISS'),\n");
    p += sprintf(p, "  date_part('epoch',forecast_period::interval),\n");
    p += sprintf(p, "  forecast_type_id,\n");
    p += sprintf(p, "  forecast_type_value::int,\n");
    p += sprintf(p, "  geometry_id,\n");
    p += sprintf(p, "  producer_id,\n");
    p += sprintf(p, "  file_format_id\n");
    p += sprintf(p, "FROM\n");
    p += sprintf(p, "  %s\n", tableName);
    p += sprintf(p, "WHERE\n");
    p += sprintf(p, "  producer_id=%u AND analysis_time = to_timestamp('%s', 'yyyymmddThh24MISS') AND file_format_id IN (1,2,3,4)\n", producerId, analysisTime);

    //printf("%s\n",sql);
    PGresult *res = PQexec(conn, sql);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
      Fmi::Exception exception(BCP, "Postgresql error!");
      exception.addParameter("ErrorMessage", PQerrorMessage(conn));
      throw exception;
    }

    //int fieldCount = PQnfields(res);
    int rowCount = PQntuples(res);

    PRINT_DATA(mDebugLogPtr, "      ---- Read table records %s:%u:%s : %d\n", tableName, producerId, analysisTime, rowCount);

    for (int i = 0; i < rowCount; i++)
    {
      if (shutdownRequested)
        return;

      FileRec rec;
      rec.fileId = getFileId(PQgetvalue(res, i, 0),true);
      rec.messageIndex = toInt64(PQgetvalue(res, i, 1));
      rec.filePosition = toInt64(PQgetvalue(res, i, 2));
      rec.messageSize = toInt64(PQgetvalue(res, i, 3));
      rec.paramId = toUInt32(PQgetvalue(res, i, 4));
      rec.levelId = toInt64(PQgetvalue(res, i, 5));
      rec.levelValue = toInt64(PQgetvalue(res, i, 6));
      rec.analysisTime = utcTimeToTimeT(PQgetvalue(res, i, 7));
      rec.forecastTime = utcTimeToTimeT(PQgetvalue(res, i, 8));
      //rec.analysisTime = PQgetvalue(res, i, 7);
      //rec.forecastTime = PQgetvalue(res, i, 8);
      std::string forecastPeriod = PQgetvalue(res, i, 9);
      rec.forecastType = toInt64(PQgetvalue(res, i, 10));
      rec.forecastNumber = toInt64(PQgetvalue(res, i, 11));
      rec.geometryId = toInt64(PQgetvalue(res, i, 12));
      rec.producerId = toInt64(PQgetvalue(res, i, 13));

      rec.format = toInt32(PQgetvalue(res, i, 14));
      rec.producerName = producerName;
      rec.lastUpdated = updateTime;

      if (rec.levelId == 2)
        rec.levelValue = rec.levelValue * 100;

      char key[200];
      sprintf(key, "%s;%u;%u;%d;%d;%lu;%s", tableName, rec.producerId, rec.geometryId, rec.forecastType, rec.forecastNumber, rec.analysisTime, forecastPeriod.c_str());

      auto pos = mFileRecMap.find(key);
      if (pos == mFileRecMap.end())
      {
        FileRecVec vec;
        vec.updateTime = updateTime;
        vec.info = info;
        vec.files.push_back(rec);
        mFileRecMap.insert(std::pair<std::string, FileRecVec>(key, vec));
      }
      else
      {
        if (pos->second.info != info)
          pos->second.files.clear();

        pos->second.info = info;
        pos->second.files.push_back(rec);
      }
    }
    PQclear(res);
  }
  catch (...)
  {
    reconnectionRequired = true;
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.printError();
    // throw exception;
  }
}




std::vector<FileRec>& readSourceFilesAndContent(
    PGconn *conn,
    const char *tableName,
    uint producerId,
    std::string& producerName,
    uint geometryId,
    int forecastTypeId,
    int forecastTypeValue,
    const char *analysisTime,
    const char *forecastPeriod,
    time_t updateTime,
    uint loadCounter)
{
  FUNCTION_TRACE
  try
  {
    if (shutdownRequested)
      return emptyRecList;

    char key[200];
    sprintf(key, "%s;%u;%u;%d;%d;%lu;%s", tableName, producerId, geometryId, forecastTypeId, forecastTypeValue, utcTimeToTimeT(analysisTime), forecastPeriod);

    uint count = 0;
    time_t lastUpdate = 0;
    std::string info = getTableInfo(conn, tableName, producerId, analysisTime,lastUpdate,count);
    if (info.empty())
      return emptyRecList;

    auto pos = mFileRecMap.find(key);
    if (pos != mFileRecMap.end())
    {
      if (pos->second.info == info)
      {
        pos->second.loadCounter = loadCounter;
        return pos->second.files;
      }
      else
      {
        std::string tbl = std::string(tableName) + ":" + std::to_string(producerId) + ":" + std::string(analysisTime);
        auto pos = mFileTables.find(tbl);
        if (pos != mFileTables.end())
        {
          mFileTables.erase(pos);
        }
      }
    }

    readTableRecords(conn, tableName, producerId, producerName, analysisTime, updateTime, info);
    pos = mFileRecMap.find(key);
    if (pos == mFileRecMap.end())
      return emptyRecList;

    pos->second.loadCounter = loadCounter;
    pos->second.info = info;
    return pos->second.files;
  }
  catch (...)
  {
    reconnectionRequired = true;
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    exception.printError();
    // throw exception;
    return emptyRecList;
  }
}




void readSourceForecastTimes(PGconn *conn, uint fmiProducerId, std::vector<ForecastRec>& sourceForecastList)
{
  FUNCTION_TRACE
  try
  {
    if (shutdownRequested)
      return;

    char sql[2000];
    char *p = sql;
    p += sprintf(p, "SELECT DISTINCT\n");
    p += sprintf(p, "  fmi_producer.id,\n");
    p += sprintf(p, "  fmi_producer.name,\n");
    p += sprintf(p, "  to_char(ss_state_v.analysis_time at time zone 'utc', 'yyyymmddThh24MISS'),\n");
    p += sprintf(p, "  to_char((ss_state_v.analysis_time+ss_state_v.forecast_period) at time zone 'utc', 'yyyymmddThh24MISS'),\n");
    p += sprintf(p, "  date_part('epoch',ss_state_v.forecast_period::interval),\n");
    p += sprintf(p, "  ss_state_v.table_name,\n");
    p += sprintf(p, "  ss_state_v.geometry_id,\n");
    p += sprintf(p, "  ss_state_v.forecast_type_id,\n");
    p += sprintf(p, "  ss_state_v.forecast_type_value,\n");
    p += sprintf(p, "  to_char(ss_state_v.delete_time at time zone 'utc', 'yyyymmddThh24MISS'),\n");
    p += sprintf(p, "  to_char(ss_state_v.last_updated at time zone 'utc', 'yyyymmddThh24MISS'),\n");
    p += sprintf(p, "  to_char(ss_state_v.last_updated at time zone 'utc', 'yyyy,mm,dd,hh24,MI,SS,US')\n");
    p += sprintf(p, "FROM\n");
    p += sprintf(p, "  ss_state_v LEFT OUTER JOIN fmi_producer ON fmi_producer.id = ss_state_v.producer_id\n");
    p += sprintf(p, "WHERE\n");
    p += sprintf(p, "  fmi_producer.id=%u\n", fmiProducerId);
    p += sprintf(p, "ORDER BY\n");
    p += sprintf(p, "  fmi_producer.id,to_char(ss_state_v.analysis_time at time zone 'utc', 'yyyymmddThh24MISS') desc,\n");
    p += sprintf(p, "  to_char(ss_state_v.last_updated at time zone 'utc', 'yyyy,mm,dd,hh24,MI,SS,US') asc,\n");
    p += sprintf(p, "  to_char((ss_state_v.analysis_time+ss_state_v.forecast_period) at time zone 'utc', 'yyyymmddThh24MISS');\n");

    PGresult *res = PQexec(conn, sql);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
      Fmi::Exception exception(BCP, "Postgresql error!");
      exception.addParameter("ErrorMessage", PQerrorMessage(conn));
      reconnectionRequired = true;
      throw exception;
    }

    //int fieldCount = PQnfields(res);
    int rowCount = PQntuples(res);

    for (int i = 0; i < rowCount; i++)
    {
      if (shutdownRequested)
        return;

      uint producerId = toInt64(PQgetvalue(res, i, 0));
      std::string analysisTime = PQgetvalue(res, i, 2);

      T::ProducerInfo *producer = mSourceProducerList.getProducerInfoById(producerId);
      if (producer != nullptr)
      {
        ForecastRec forecastRec;
        forecastRec.producerId = producerId;
        forecastRec.producerName = PQgetvalue(res, i, 1);
        forecastRec.analysisTime = analysisTime;
        forecastRec.generationName = forecastRec.producerName + ":" + forecastRec.analysisTime;
        forecastRec.forecastTime = PQgetvalue(res, i, 3);
        forecastRec.forecastPeriod = PQgetvalue(res, i, 4);
        forecastRec.tableName = PQgetvalue(res, i, 5);
        forecastRec.geometryId = toInt64(PQgetvalue(res, i, 6));
        forecastRec.forecastTypeId = toInt64(PQgetvalue(res, i, 7));
        forecastRec.forecastTypeValue = toInt64(PQgetvalue(res, i, 8));
        std::string dtime = PQgetvalue(res, i, 9);
        forecastRec.deletionTime = utcTimeToTimeT(dtime);
        std::string modtime = PQgetvalue(res, i, 10);
        forecastRec.lastUpdated = utcTimeToTimeT(modtime);
        sourceForecastList.push_back(forecastRec);
      }
    }
    PQclear(res);
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}




void readSourceGenerations(PGconn *conn)
{
  FUNCTION_TRACE
  try
  {
    if (shutdownRequested)
      return;

    mSourceGenerationList.clear();

    char sql[1000];
    char *p = sql;
    p += sprintf(p, "SELECT DISTINCT\n");
    p += sprintf(p, "  fmi_producer.id,\n");
    p += sprintf(p, "  fmi_producer.name,\n");
    p += sprintf(p, "  to_char(ss_state_v.analysis_time at time zone 'utc', 'yyyymmddThh24MISS'),\n");
    p += sprintf(p, "  to_char(ss_state_v.delete_time at time zone 'utc', 'yyyymmddThh24MISS'),\n");
    p += sprintf(p, "  to_char(ss_state_v.last_updated at time zone 'utc', 'yyyymmddThh24MISS')\n");
    p += sprintf(p, "FROM\n");
    p += sprintf(p, "  ss_state_v LEFT OUTER JOIN fmi_producer ON fmi_producer.id = ss_state_v.producer_id\n");
    p += sprintf(p, "ORDER BY");
    p += sprintf(p, "  fmi_producer.id,to_char(ss_state_v.analysis_time at time zone 'utc', 'yyyymmddThh24MISS') desc,");
    p += sprintf(p, "  to_char(ss_state_v.last_updated at time zone 'utc', 'yyyymmddThh24MISS') desc;");

    PGresult *res = PQexec(conn, sql);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
      Fmi::Exception exception(BCP, "Postgresql error!");
      exception.addParameter("ErrorMessage", PQerrorMessage(conn));
      reconnectionRequired = true;
      throw exception;
    }

    //int fieldCount = PQnfields(res);
    int rowCount = PQntuples(res);

    uint prevProducerId = 0;
    std::string prevAnalysisTime;

    for (int i = 0; i < rowCount; i++)
    {
      if (shutdownRequested)
        return;

      char st[1000];

      uint producerId = toInt64(PQgetvalue(res, i, 0));
      std::string analysisTime = PQgetvalue(res, i, 2);

      T::ProducerInfo *producer = mSourceProducerList.getProducerInfoById(producerId);
      if (producer != nullptr)
      {
        if (producerId != prevProducerId || analysisTime != prevAnalysisTime)
        {
          T::GenerationInfo *generation = new T::GenerationInfo();
          generation->mGenerationId = i + 1;
          generation->mGenerationType = 0;
          generation->mProducerId = producerId;
          sprintf(st, "%s:%s", producer->mName.c_str(), PQgetvalue(res, i, 2));
          generation->mName = st;
          sprintf(st, "Producer %s generation %s", producer->mName.c_str(), PQgetvalue(res, i, 2));
          generation->mDescription = st;
          generation->mAnalysisTime = analysisTime;
          generation->mStatus = T::GenerationInfo::Status::Ready;
          generation->mFlags = 0;
          generation->mSourceId = mSourceId;
          std::string dtime = PQgetvalue(res, i, 3);
          generation->mDeletionTime = utcTimeToTimeT(dtime);
          std::string mtime = PQgetvalue(res, i, 4);
          generation->mModificationTime = utcTimeToTimeT(mtime);
          mSourceGenerationList.addGenerationInfo(generation);
          prevProducerId = producerId;
          prevAnalysisTime = analysisTime;
        }
      }
    }
    PQclear(res);
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}




void readReadyGenerations(PGconn *conn,std::unordered_map<std::string,time_t>& readyGenerations)
{
  FUNCTION_TRACE
  try
  {
    if (shutdownRequested)
      return;

    readyGenerations.clear();

    char sql[1000];
    char *p = sql;
    p += sprintf(p, "SELECT DISTINCT\n");
    p += sprintf(p, "  fmi_producer.id,\n");
    p += sprintf(p, "  fmi_producer.name,\n");
    p += sprintf(p, "  to_char(ss_forecast_status.analysis_time at time zone 'utc', 'yyyymmddThh24MISS'),\n");
    p += sprintf(p, "  to_char(ss_forecast_status.status_time at time zone 'utc', 'yyyymmddThh24MISS')\n");
    p += sprintf(p, "FROM\n");
    p += sprintf(p, "  ss_forecast_status LEFT OUTER JOIN fmi_producer ON fmi_producer.id = ss_forecast_status.producer_id\n");
    p += sprintf(p, "WHERE\n");
    p += sprintf(p, " ss_forecast_status.geometry_id IS NULL AND upper(status)='READY'\n");
    p += sprintf(p, "ORDER BY\n");
    p += sprintf(p, "  fmi_producer.id,to_char(ss_forecast_status.analysis_time at time zone 'utc', 'yyyymmddThh24MISS') desc,");
    p += sprintf(p, "  to_char(ss_forecast_status.status_time at time zone 'utc', 'yyyymmddThh24MISS') desc;");

    PGresult *res = PQexec(conn, sql);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
      Fmi::Exception exception(BCP, "Postgresql error!");
      exception.addParameter("ErrorMessage", PQerrorMessage(conn));
      reconnectionRequired = true;
      throw exception;
    }

    //int fieldCount = PQnfields(res);
    int rowCount = PQntuples(res);

    uint prevProducerId = 0;
    std::string prevAnalysisTime;

    for (int i = 0; i < rowCount; i++)
    {
      if (shutdownRequested)
        return;

      char st[1000];

      uint producerId = toInt64(PQgetvalue(res, i, 0));
      std::string analysisTime = PQgetvalue(res, i, 2);

      T::ProducerInfo *producer = mSourceProducerList.getProducerInfoById(producerId);
      if (producer != nullptr)
      {
        if (producerId != prevProducerId || analysisTime != prevAnalysisTime)
        {
          std::string mtime = PQgetvalue(res, i, 3);
          sprintf(st, "%s:%s", producer->mName.c_str(), analysisTime.c_str());

          readyGenerations.insert(std::pair<std::string,time_t>(st,utcTimeToTimeT(mtime)));
          prevProducerId = producerId;
          prevAnalysisTime = analysisTime;
        }
      }
    }
    PQclear(res);
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}





void readSourceProducers(PGconn *conn)
{
  FUNCTION_TRACE
  try
  {
    if (shutdownRequested)
      return;

    mSourceProducerList.clear();

    char sql[1000];
    char *p = sql;
    p += sprintf(p, "SELECT DISTINCT\n");
    p += sprintf(p, "  fmi_producer.id,\n");
    p += sprintf(p, "  fmi_producer.name,\n");
    p += sprintf(p, "  fmi_producer.description\n");
    p += sprintf(p, "FROM\n");
    p += sprintf(p, "  ss_state_v LEFT OUTER JOIN fmi_producer ON fmi_producer.id = ss_state_v.producer_id");

    PGresult *res = PQexec(conn, sql);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
      Fmi::Exception exception(BCP, "Postgresql error!");
      exception.addParameter("ErrorMessage", PQerrorMessage(conn));
      reconnectionRequired = true;
      throw exception;
    }

    //int fieldCount = PQnfields(res);
    int rowCount = PQntuples(res);

    for (int i = 0; i < rowCount; i++)
    {
      if (shutdownRequested)
        return;

      std::string searchStr = toUpperString(std::string(PQgetvalue(res, i, 1)));
      if (mProducerList.size() == 0 || mProducerList.find(searchStr) != mProducerList.end())
      {
        T::ProducerInfo *producer = new T::ProducerInfo();
        producer->mProducerId = toInt64(PQgetvalue(res, i, 0));
        producer->mName = PQgetvalue(res, i, 1);
        producer->mTitle = PQgetvalue(res, i, 1);
        producer->mDescription = PQgetvalue(res, i, 2);
        producer->mFlags = 0;
        producer->mSourceId = mSourceId;

        mSourceProducerList.addProducerInfo(producer);
      }
    }
    PQclear(res);
    //mSourceProducerList.print(std::cout,0,0);
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}




void updateProducers()
{
  FUNCTION_TRACE
  try
  {
    mProducerIdList.clear();
    uint len = mTargetProducerList.getLength();
    for (uint t = 0; t < len; t++)
    {
      T::ProducerInfo *targetProducer = mTargetProducerList.getProducerInfoByIndex(t);

      // Checking if we have created the current producers - if so, then we can also remove it.
      if (targetProducer->mSourceId == mSourceId)
      {
        std::string searchStr = toUpperString(targetProducer->mName);
        if (mProducerList.size() == 0 || mProducerList.find(searchStr) != mProducerList.end())
        {
          mProducerIdList.insert(targetProducer->mProducerId);
          T::ProducerInfo *sourceProducer = mSourceProducerList.getProducerInfoByName(targetProducer->mName);
          if (sourceProducer == nullptr)
          {
            // The producer information is not available in the source data storage. So, we should remove
            // it also from the target data storage.

            PRINT_DATA(mDebugLogPtr, "  -- Remove producer : %s\n", targetProducer->mName.c_str());

            int result = mTargetInterface->deleteProducerInfoById(mSessionId, targetProducer->mProducerId);
            if (result != 0)
            {
              Fmi::Exception exception(BCP, "Cannot delete the producer information from the target data storage!");
              exception.addParameter("ProducerId", std::to_string(targetProducer->mProducerId));
              exception.addParameter("ProducerName", targetProducer->mName);
              exception.addParameter("Result", ContentServer::getResultString(result));
              exception.printError();
            }
          }
        }
      }
    }

    len = mSourceProducerList.getLength();
    for (uint t = 0; t < len; t++)
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

          PRINT_DATA(mDebugLogPtr, "  -- Add producer : %s\n", producer.mName.c_str());

          int result = mTargetInterface->addProducerInfo(mSessionId, producer);
          if (result != 0)
          {
            Fmi::Exception exception(BCP, "Cannot add the producer information into the target data storage!");
            exception.addParameter("ProducerId", std::to_string(sourceProducer->mProducerId));
            exception.addParameter("ProducerName", sourceProducer->mName);
            exception.addParameter("Result", ContentServer::getResultString(result));
            throw exception;
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




void updateGenerations()
{
  FUNCTION_TRACE
  try
  {
    std::set < uint > generationIdList;
    uint len = mTargetGenerationList.getLength();
    for (uint t = 0; t < len; t++)
    {
      T::GenerationInfo *targetGeneration = mTargetGenerationList.getGenerationInfoByIndex(t);
      if (targetGeneration->mSourceId == mSourceId  &&  mProducerIdList.find(targetGeneration->mProducerId) != mProducerIdList.end())
      {
        T::GenerationInfo *sourceGeneration = mSourceGenerationList.getGenerationInfoByName(targetGeneration->mName);
        if (sourceGeneration == nullptr)
        {
          // The generation information is not available in the source data storage. So, we should remove
          // it also from the target data storage.

          PRINT_DATA(mDebugLogPtr, "  -- Remove generation : %s\n", targetGeneration->mName.c_str());

          generationIdList.insert(targetGeneration->mGenerationId);
        }
      }
    }

    if (generationIdList.size() > 0)
    {
      int result = mTargetInterface->deleteGenerationInfoListByIdList(mSessionId, generationIdList);
      if (result != 0)
      {
        Fmi::Exception exception(BCP, "Cannot delete the generation information from the target data storage!");
        exception.addParameter("Result", ContentServer::getResultString(result));
        exception.printError();
      }
    }

    len = mSourceGenerationList.getLength();
    for (uint t = 0; t < len; t++)
    {
      T::GenerationInfo *sourceGeneration = mSourceGenerationList.getGenerationInfoByIndex(t);
      if (sourceGeneration->mSourceId == mSourceId)
      {
        T::GenerationInfo *targetGeneration = mTargetGenerationList.getGenerationInfoByName(sourceGeneration->mName);
        if (targetGeneration == nullptr)
        {
          // The generation information is not available in the target data storage. So, we should add it.

          // Finding producer name:
          T::ProducerInfo *sourceProducer = mSourceProducerList.getProducerInfoById(sourceGeneration->mProducerId);
          if (sourceProducer != nullptr)
          {
            // Finding producer id from the target data storage.
            T::ProducerInfo *targetProducer = mTargetProducerList.getProducerInfoByName(sourceProducer->mName);
            if (targetProducer == nullptr)
            {
              Fmi::Exception exception(BCP, "The producer information not found from the target data storage!");
              exception.addParameter("ProducerName", sourceProducer->mName);
              throw exception;
            }

            T::GenerationInfo generationInfo(*sourceGeneration);
            generationInfo.mProducerId = targetProducer->mProducerId;
            generationInfo.mSourceId = mSourceId;
            generationInfo.mStatus = T::GenerationInfo::Status::Running;

            PRINT_DATA(mDebugLogPtr, "  -- Add generation : %s\n", generationInfo.mName.c_str());
            //sourceGeneration->print(std::cout,0,0);

            int result = mTargetInterface->addGenerationInfo(mSessionId, generationInfo);
            if (result != 0)
            {
              Fmi::Exception exception(BCP, "Cannot add the generation information into the target data storage!");
              exception.addParameter("GenerationName", generationInfo.mName);
              exception.addParameter("Result", ContentServer::getResultString(result));
              throw exception;
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




void updateGenerationStatus(T::ProducerInfo& targetProducer)
{
  FUNCTION_TRACE
  try
  {

    std::string pname = toUpperString(targetProducer.mName);
    bool ignore = false;
    if (mGenerationStatusCheckIgnore.find(pname) != mGenerationStatusCheckIgnore.end())
      ignore = true;

    if (!ignore)
    {
      auto rec = mProducerDependensies.find(pname);
      if (rec != mProducerDependensies.end())
        return; // The producer update has dependensies. We update these kinds of producers later.
    }

    uint len = mTargetGenerationList.getLength();
    for (uint t = 0; t < len; t++)
    {
      T::GenerationInfo *targetGeneration = mTargetGenerationList.getGenerationInfoByIndex(t);
      if (targetGeneration->mSourceId == mSourceId  &&  targetGeneration->mProducerId == targetProducer.mProducerId  &&  targetGeneration->mStatus != T::GenerationInfo::Status::Ready)
      {
        if (ignore || mReadyGenerations.find(targetGeneration->mName) != mReadyGenerations.end())
        {
          PRINT_DATA(mDebugLogPtr, "  -- Update generation status : %s\n", targetGeneration->mName.c_str());
          targetGeneration->mStatus = T::GenerationInfo::Status::Ready;

          int result = mTargetInterface->setGenerationInfo(mSessionId,*targetGeneration);
          if (result != 0)
          {
            Fmi::Exception exception(BCP, "Cannot update the generation status into the target data storage!");
            exception.addParameter("GenerationName", targetGeneration->mName);
            exception.addParameter("Result", ContentServer::getResultString(result));
            throw exception;
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




void updateGenerationStatus()
{
  FUNCTION_TRACE
  try
  {
    uint len = mTargetGenerationList.getLength();
    for (uint t = 0; t < len; t++)
    {
      T::GenerationInfo *targetGeneration = mTargetGenerationList.getGenerationInfoByIndex(t);

      if (targetGeneration->mSourceId == mSourceId)
      {
        std::vector<std::string> parts;
        splitString(targetGeneration->mName,':',parts);
        std::string pname = toUpperString(parts[0]);
        bool ready = true;

        if (mGenerationStatusCheckIgnore.find(pname) == mGenerationStatusCheckIgnore.end())
        {
          auto gen = mReadyGenerations.find(targetGeneration->mName);
          if (gen == mReadyGenerations.end())
          {
            ready = false;
          }
          else
          {
            auto rec = mProducerDependensies.find(pname);
            if (rec != mProducerDependensies.end())
            {
              for (auto it = rec->second.begin(); it != rec->second.end(); ++it)
              {
                std::string n = *it + ":" + parts[1];
                auto gen = mReadyGenerations.find(n);
                if (gen == mReadyGenerations.end())
                  ready = false;
              }
            }
          }
        }

        int result = 0;
        if (ready  &&  targetGeneration->mStatus != T::GenerationInfo::Status::Ready)
        {
          PRINT_DATA(mDebugLogPtr, "  -- Update generation status to 'ready': %s\n", targetGeneration->mName.c_str());
          targetGeneration->mStatus = T::GenerationInfo::Status::Ready;
          result = mTargetInterface->setGenerationInfo(mSessionId,*targetGeneration);
        }
        else
        if (!ready  &&  targetGeneration->mStatus == T::GenerationInfo::Status::Ready)
        {
          PRINT_DATA(mDebugLogPtr, "  -- Update generation status to 'running': %s\n", targetGeneration->mName.c_str());
          targetGeneration->mStatus = T::GenerationInfo::Status::Running;
          result = mTargetInterface->setGenerationInfo(mSessionId,*targetGeneration);
        }

        if (result != 0)
        {
          Fmi::Exception exception(BCP, "Cannot update the generation status into the target data storage!");
          exception.addParameter("GenerationName", targetGeneration->mName);
          exception.addParameter("Result", ContentServer::getResultString(result));
          throw exception;
        }
      }
    }
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}




void deleteTargetFiles(T::FileInfoList& targetFileList)
{
  FUNCTION_TRACE
  try
  {
    PRINT_DATA(mDebugLogPtr, "* Deleting old files from the target data storage\n");

    std::set < uint > deleteList;
    uint len = targetFileList.getLength();
    for (uint t = 0; t < len; t++)
    {
      T::FileInfo *fileInfo = targetFileList.getFileInfoByIndex(t);
      if (fileInfo != nullptr && (fileInfo->mFlags & T::FileInfo::Flags::VirtualContent) == 0)
      {
        if (fileInfo->mSourceId == mSourceId && mIgnoreGeneration.find(fileInfo->mGenerationId) == mIgnoreGeneration.end()  &&  mSourceFilenames.find(getFileId(fileInfo->mName,false)) == mSourceFilenames.end())
        {
          deleteList.insert(fileInfo->mFileId);
        }
      }
    }
    PRINT_DATA(mDebugLogPtr, "  -- Delete files : %lu\n", deleteList.size());
    if (deleteList.size() > 0)
      mTargetInterface->deleteFileInfoListByFileIdList(mSessionId, deleteList);
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}




void deleteTargetFiles(uint producerId,T::FileInfoList& targetFileList)
{
  FUNCTION_TRACE
  try
  {
    PRINT_DATA(mDebugLogPtr, "* Deleting old files from the target data storage\n");

    std::set < uint > deleteList;
    uint len = targetFileList.getLength();
    for (uint t = 0; t < len; t++)
    {
      T::FileInfo *fileInfo = targetFileList.getFileInfoByIndex(t);
      if (fileInfo != nullptr && fileInfo->mProducerId == producerId  &&  (fileInfo->mFlags & T::FileInfo::Flags::VirtualContent) == 0)
      {
        if (fileInfo->mSourceId == mSourceId && mIgnoreGeneration.find(fileInfo->mGenerationId) == mIgnoreGeneration.end()  &&  mSourceFilenames.find(getFileId(fileInfo->mName,false)) == mSourceFilenames.end())
        {
          deleteList.insert(fileInfo->mFileId);
        }
      }
    }
    PRINT_DATA(mDebugLogPtr, "  -- Delete files : %lu\n", deleteList.size());
    if (deleteList.size() > 0)
      mTargetInterface->deleteFileInfoListByFileIdList(mSessionId, deleteList);
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}




void deleteOldFileRecords(uint loadCounter)
{
  FUNCTION_TRACE
  try
  {
    std::set < std::string > entriesToRemove;

    for (auto it = mFileRecMap.begin(); it != mFileRecMap.end(); ++it)
    {
      if (it->second.loadCounter != loadCounter)
      {
        entriesToRemove.insert(it->first);
      }
    }

    for (auto it = entriesToRemove.begin(); it != entriesToRemove.end(); ++it)
    {
      auto pos = mFileRecMap.find(*it);
      if (pos != mFileRecMap.end())
      {
        pos->second.files.clear();
        pos->second.files.resize(0);
        pos->second.files.shrink_to_fit();
        mFileRecMap.erase(pos);
      }
    }
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}




uint countFileRecords()
{
  FUNCTION_TRACE
  try
  {
    uint cnt = 0;
    for (auto it = mFileRecMap.begin(); it != mFileRecMap.end(); ++it)
    {
      cnt = cnt + it->second.files.size();
    }

    return cnt;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}




void readSourceFilesByForecastTime(PGconn *conn, ForecastRec& forecast, uint loadCounter, uint targetProducerId,T::FileInfoList& targetFileList, std::string& producerName,std::vector<T::FileAndContent>& fileAndContentList, std::vector<FileRec>& fileRecList)
{
  FUNCTION_TRACE
  try
  {
    if (shutdownRequested)
      return;

    T::GenerationInfo *generation = mTargetGenerationList.getGenerationInfoByName(forecast.generationName);

    if (generation == nullptr)
    {
      PRINT_DATA(mDebugLogPtr, "**** Unknown generation : %s\n", forecast.generationName.c_str());
      return;
    }

    if (generation->mStatus == T::GenerationInfo::Status::Running || generation->mModificationTime <= forecast.lastUpdated)
    {
      //printf("%s:%s:%s\n",forecast.generationName.c_str(),utcTimeFromTimeT(generation->mModificationTime).c_str(),utcTimeFromTimeT(forecast.lastUpdated).c_str());

      generation->mStatus = T::GenerationInfo::Status::Running;

      std::vector<FileRec> recList = readSourceFilesAndContent(conn, forecast.tableName.c_str(), forecast.producerId, producerName, forecast.geometryId, forecast.forecastTypeId,
          forecast.forecastTypeValue, forecast.analysisTime.c_str(), forecast.forecastPeriod.c_str(), forecast.lastUpdated, loadCounter);

      mContentReadCount += recList.size();
      PRINT_DATA(mDebugLogPtr, "  -- Read files by forecast time %s:%u:%d:%d:%d : %lu (contentCount = %u)\n", forecast.forecastTime.c_str(), forecast.producerId, forecast.geometryId,
          forecast.forecastTypeId, forecast.forecastTypeValue, recList.size(), mContentReadCount);

      if (recList.size() == 0)
        return;

      if (fileReadCount == 0  &&  targetFileList.getLength() == 0)
      {
        readTargetFileList(targetProducerId,targetFileList);
        targetFileList.sort(T::FileInfo::ComparisonMethod::fileName);
        fileReadCount++;
      }

      for (auto it = recList.begin(); it != recList.end(); ++it)
      {
        if (shutdownRequested)
          return;

        fileRecList.push_back(*it);
        T::FileInfo fileInfo;
        std::string filename = getFilename(it->fileId);
        if (mSourceFilenames.find(it->fileId) == mSourceFilenames.end() && targetFileList.getFileInfoByName(filename) == nullptr)
        {
          // File does not exists. It should be added.

          T::FileAndContent fc;

          fc.mFileInfo.mProducerId = generation->mProducerId;
          fc.mFileInfo.mGenerationId = generation->mGenerationId;
          fc.mFileInfo.mFileId = 0;
          fc.mFileInfo.mFileType = it->format;
          fc.mFileInfo.mName = filename;
          fc.mFileInfo.mFlags = 0;
          fc.mFileInfo.mSourceId = mSourceId;
          fc.mFileInfo.mDeletionTime = forecast.deletionTime;

          fileAndContentList.push_back(fc);
        }

        if (mSourceFilenames.find(it->fileId) == mSourceFilenames.end())
          mSourceFilenames.insert(it->fileId);
      }
      if (generation->mModificationTime <= forecast.lastUpdated)
      {
        generation->mModificationTime = forecast.lastUpdated;
        time_t diff = (time(nullptr) - forecast.lastUpdated);
        // printf("TIME %ld\n",diff);
        if (diff > 300)
          generation->mModificationTime = forecast.lastUpdated + 1;
      }
    }
    else
    {
      mIgnoreGeneration.insert(generation->mGenerationId);
    }
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}




void saveTargetContent(uint producerId,std::vector<FileRec>& fileRecList)
{
  try
  {
    if (fileRecList.size() == 0)
      return;

    T::FileInfoList targetFileList;
    readTargetFileList(producerId,targetFileList);
    targetFileList.sort(T::FileInfo::ComparisonMethod::fileName);

    //std::set<unsigned long long> targetContentList;
    //if (fileRecList.size() > 0)
    //  readTargetContentList(producerId,targetContentList);

    T::ContentInfoList contentList;

    for (auto it = fileRecList.begin(); it != fileRecList.end(); ++it)
    {
      std::string filename = getFilename(it->fileId);
      T::FileInfo *fileInfo = targetFileList.getFileInfoByName(filename);
      if (fileInfo != nullptr)
      {
        unsigned long long id = fileInfo->mFileId;
        id = (id << 32) + it->messageIndex;
        auto pos = mTargetContentList.find(id);
        if (pos == mTargetContentList.end())
        {
          // The content does not exists. It should be added.

          T::ContentInfo *contentInfo = new T::ContentInfo();

          contentInfo->mFileId = fileInfo->mFileId;
          contentInfo->mFileType = fileInfo->mFileType;
          contentInfo->mMessageIndex = it->messageIndex;
          contentInfo->mFilePosition = it->filePosition;
          contentInfo->mMessageSize = it->messageSize;
          contentInfo->mProducerId = fileInfo->mProducerId;
          contentInfo->mGenerationId = fileInfo->mGenerationId;
          contentInfo->setForecastTime(utcTimeFromTimeT(it->forecastTime));
          contentInfo->mFmiParameterId = it->paramId;
          contentInfo->mFmiParameterLevelId = it->levelId;
          contentInfo->mParameterLevel = it->levelValue;
          contentInfo->mForecastType = it->forecastType;
          contentInfo->mForecastNumber = it->forecastNumber;
          contentInfo->mFlags = 0;
          contentInfo->mSourceId = mSourceId;
          contentInfo->mGeometryId = it->geometryId;
          contentInfo->mModificationTime = it->lastUpdated;
          contentInfo->mDeletionTime = fileInfo->mDeletionTime;

          Identification::FmiParameterDef fmiDef;
          if (Identification::gridDef.getFmiParameterDefById(it->paramId, fmiDef))
          {
            contentInfo->setFmiParameterName(fmiDef.mParameterName);
/*
            Identification::NewbaseParameterDef newbaseDef;
            if (Identification::gridDef.getNewbaseParameterDefByFmiId(it->paramId, newbaseDef))
            {
              contentInfo->mNewbaseParameterId = newbaseDef.mNewbaseParameterId;
              contentInfo->setNewbaseParameterName(newbaseDef.mParameterName);
            }

            Identification::NetCdfParameterDef netCdfDef;
            if (Identification::gridDef.getNetCdfParameterDefByFmiId(it->paramId, netCdfDef))
            {
              contentInfo->setNetCdfParameterName(netCdfDef.mParameterName);
            }
*/
          }

          contentList.addContentInfo(contentInfo);

          if (contentList.getLength() >= mMaxMessageSize)
          {
            PRINT_DATA(mDebugLogPtr, "  -- Add content information : %u (%u/%u)\n", contentList.getLength(), mContentAddCount, mContentReadCount);
            mTargetInterface->addContentList(mSessionId, contentList);
            contentList.clear();
          }
        }
      }
      else
      {
        PRINT_DATA(mDebugLogPtr,"#### File info not found : %s\n", filename.c_str());
      }
      mContentAddCount++;
    }

    PRINT_DATA(mDebugLogPtr, "  -- Add content information : %u (%u/%u)\n", contentList.getLength(), mContentAddCount, mContentReadCount);
    if (contentList.getLength() > 0)
      mTargetInterface->addContentList(mSessionId, contentList);

    //std::set<unsigned long long> tcList;
    //targetContentList.clear();
    //targetContentList.swap(tcList);

    deleteTargetFiles(producerId,targetFileList);
    targetFileList.clear();
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}




void saveTargetContent(std::vector<FileRec>& fileRecList)
{
  try
  {
    T::FileInfoList targetFileList;
    readTargetFileList(targetFileList);
    targetFileList.sort(T::FileInfo::ComparisonMethod::fileName);

    std::set<unsigned long long> targetContentList;
    readTargetContentList(targetContentList);

    T::ContentInfoList contentList;

    for (auto it = fileRecList.begin(); it != fileRecList.end(); ++it)
    {
      std::string filename = getFilename(it->fileId);
      T::FileInfo *fileInfo = targetFileList.getFileInfoByName(filename);
      if (fileInfo != nullptr)
      {
        unsigned long long id = fileInfo->mFileId;
        id = (id << 32) + it->messageIndex;
        auto pos = targetContentList.find(id);
        if (pos == targetContentList.end())
        {
          // The content does not exists. It should be added.

          T::ContentInfo *contentInfo = new T::ContentInfo();

          contentInfo->mFileId = fileInfo->mFileId;
          contentInfo->mFileType = T::FileTypeValue::Unknown;
          contentInfo->mMessageIndex = it->messageIndex;
          contentInfo->mFilePosition = it->filePosition;
          contentInfo->mMessageSize = it->messageSize;
          contentInfo->mProducerId = fileInfo->mProducerId;
          contentInfo->mGenerationId = fileInfo->mGenerationId;
          contentInfo->setForecastTime(utcTimeFromTimeT(it->forecastTime));
          contentInfo->mFmiParameterId = it->paramId;
          contentInfo->mFmiParameterLevelId = it->levelId;
          contentInfo->mParameterLevel = it->levelValue;
          contentInfo->mForecastType = it->forecastType;
          contentInfo->mForecastNumber = it->forecastNumber;
          contentInfo->mFlags = 0;
          contentInfo->mSourceId = mSourceId;
          contentInfo->mGeometryId = it->geometryId;
          contentInfo->mModificationTime = it->lastUpdated;
          contentInfo->mDeletionTime = fileInfo->mDeletionTime;

          Identification::FmiParameterDef fmiDef;
          if (Identification::gridDef.getFmiParameterDefById(it->paramId, fmiDef))
          {
            contentInfo->setFmiParameterName(fmiDef.mParameterName);
/*
            Identification::NewbaseParameterDef newbaseDef;
            if (Identification::gridDef.getNewbaseParameterDefByFmiId(it->paramId, newbaseDef))
            {
              contentInfo->mNewbaseParameterId = newbaseDef.mNewbaseParameterId;
              contentInfo->setNewbaseParameterName(newbaseDef.mParameterName);
            }

            Identification::NetCdfParameterDef netCdfDef;
            if (Identification::gridDef.getNetCdfParameterDefByFmiId(it->paramId, netCdfDef))
            {
              contentInfo->setNetCdfParameterName(netCdfDef.mParameterName);
            }
*/
          }

          contentList.addContentInfo(contentInfo);

          if (contentList.getLength() >= mMaxMessageSize)
          {
            PRINT_DATA(mDebugLogPtr, "  -- Add content information : %u (%u/%u)\n", contentList.getLength(), mContentAddCount, mContentReadCount);
            mTargetInterface->addContentList(mSessionId, contentList);
            contentList.clear();
          }
        }
      }
      else
      {
        PRINT_DATA(mDebugLogPtr,"#### File info not found : %s\n", filename.c_str());
      }
      mContentAddCount++;
    }

    PRINT_DATA(mDebugLogPtr, "  -- Add content information : %u (%u/%u)\n", contentList.getLength(), mContentAddCount, mContentReadCount);
    if (contentList.getLength() > 0)
      mTargetInterface->addContentList(mSessionId, contentList);

    std::set<unsigned long long> tcList;
    targetContentList.clear();
    targetContentList.swap(tcList);

    deleteTargetFiles(targetFileList);
    targetFileList.clear();
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}




void updateTargetFiles(PGconn *conn)
{
  FUNCTION_TRACE
  try
  {
    mIgnoreGeneration.clear();
    mFileLoadCounter++;

    mContentReadCount = 0;
    mContentAddCount = 0;

    readReadyGenerations(conn,mReadyGenerations);

    uint len = mSourceProducerList.getLength();
    for (uint t = 0; t < len; t++)
    {
      T::ProducerInfo *sourceProducer = mSourceProducerList.getProducerInfoByIndex(t);
      if (sourceProducer->mSourceId == mSourceId)
      {
        T::ProducerInfo *targetProducer = mTargetProducerList.getProducerInfoByName(sourceProducer->mName);
        if (targetProducer != nullptr)
        {
          time_t startTime = time(nullptr);
          PRINT_DATA(mDebugLogPtr, "* Reading file information from the target data storage\n");

          T::FileInfoList targetFileList;
          std::vector<FileRec> fileRecList;
          fileReadCount = 0;

          PRINT_DATA(mDebugLogPtr, "  ** Add file information : %s\n", targetProducer->mName.c_str());

          std::vector<ForecastRec> sourceForecastList;
          readSourceForecastTimes(conn, sourceProducer->mProducerId, sourceForecastList);
          std::vector < T::FileAndContent > fileAndContentList;

          for (auto it = sourceForecastList.begin(); it != sourceForecastList.end()  &&  !shutdownRequested; ++it)
          {
            readSourceFilesByForecastTime(conn, *it, mFileLoadCounter,targetProducer->mProducerId,targetFileList,targetProducer->mName,fileAndContentList, fileRecList);
          }

          if (shutdownRequested)
            return;

          if (fileAndContentList.size() <= mMaxMessageSize)
          {
            PRINT_DATA(mDebugLogPtr, "  -- Add file information : %lu\n", fileAndContentList.size());
            int result = mTargetInterface->addFileInfoListWithContent(mSessionId, 0, fileAndContentList);
            if (result != 0)
            {
              fprintf(stdout, "ERROR (%d) : %s\n", result, ContentServer::getResultString(result).c_str());
            }
          }
          else
          {
            std::vector < T::FileAndContent > fcList;
            uint sz = fileAndContentList.size();
            for (uint s=0; s<sz; s++)
            {
              fcList.push_back(fileAndContentList[s]);
              if (fcList.size() >= mMaxMessageSize)
              {
                PRINT_DATA(mDebugLogPtr, "  -- Add file information : %lu\n", fcList.size());
                int result = mTargetInterface->addFileInfoListWithContent(mSessionId, 0, fcList);
                if (result != 0)
                {
                  fprintf(stdout, "ERROR (%d) : %s\n", result, ContentServer::getResultString(result).c_str());
                }
                fcList.clear();
              }
            }

            if (fcList.size() > 0)
            {
              PRINT_DATA(mDebugLogPtr, "  -- Add file information : %lu\n", fcList.size());
              int result = mTargetInterface->addFileInfoListWithContent(mSessionId, 0, fcList);
              if (result != 0)
              {
                fprintf(stdout, "ERROR (%d) : %s\n", result, ContentServer::getResultString(result).c_str());
              }
              fcList.clear();
            }
          }

          fileAndContentList.clear();

          PRINT_DATA(mDebugLogPtr,"* Updating content information into the target data storage\n");
          saveTargetContent(targetProducer->mProducerId,fileRecList);
          //printf("-- save target content %ld\n",fileRecList.size());

          PRINT_DATA(mDebugLogPtr, "* Updating generation status information into the target data storage\n");
          updateGenerationStatus(*targetProducer);

          time_t endTime = time(nullptr);
          PRINT_DATA(mDebugLogPtr, "### PRODUCER UPDATED IN %ld SECONDS (%s)\n",(endTime-startTime),targetProducer->mName.c_str());
          // printf("### PRODUCER UPDATED IN %3ld SECONDS (%s)\n",(endTime-startTime),targetProducer->mName.c_str());
        }
      }
    }

    updateGenerationStatus();

    std::unordered_map<std::string, std::string> tableUpdates;
    mTableUpdates.clear();
    mTableUpdates.swap(tableUpdates);

    std::set<std::string> fileTables;
    mFileTables.clear();
    mFileTables.swap(fileTables);

    deleteOldFileRecords(mFileLoadCounter);
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}




int main(int argc, char *argv[])
{
  FUNCTION_TRACE
  try
  {
    if (argc < 2 || argc > 3)
    {
      fprintf(stderr, "USAGE: radon2smartmet <configFile> [loopWaitTime]\n");
      return -1;
    }

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);
    signal(SIGHUP, sig_handler);


    readConfigFile(argv[1]);

    // Initializing the global structures. These are needed when
    // extracting information from GRIB files.

    Identification::gridDef.init(mGridConfigFile.c_str());

    if (argc == 3)
    {
        mWaitTime = toInt64(argv[2]);
    }
    else
    {
        mWaitTime = 0; // disable looping
    }

    ContentServer::RedisImplementation *redisImplementation = nullptr;

    if (mStorageType == "redis")
    {
      redisImplementation = new ContentServer::RedisImplementation();
      redisImplementation->init(mRedisAddress.c_str(), mRedisPort, mRedisTablePrefix.c_str(),mRedisLockEnabled);
      mTargetInterface = redisImplementation;
    }

    if (mStorageType == "corba")
    {
      ContentServer::Corba::ClientImplementation *client = new ContentServer::Corba::ClientImplementation();
      client->init(mContentServerIor.c_str());
      mTargetInterface = client;
    }

    if (mStorageType == "http")
    {
      ContentServer::HTTP::ClientImplementation *httpClient = new ContentServer::HTTP::ClientImplementation();
      httpClient->init(mContentServerUrl.c_str());
      mTargetInterface = httpClient;
    }

    if (mDebugLogEnabled)
    {
      mDebugLog.init(true, mDebugLogFile.c_str(), mDebugLogMaxSize, mDebugLogTruncateSize);
      mDebugLogPtr = &mDebugLog;
    }

    if (mTargetInterface == nullptr)
    {
      fprintf(stderr, "No target data source defined!\n");
      return -3;
    }

    PGconn *conn = nullptr;
    time_t lastDatabaseRead = time(nullptr);
    bool ind = true;
    while (ind)
    {
      time_t loopStart = time(nullptr);
      if (reconnectionRequired || conn == nullptr)
      {
        conn = PQconnectdb(mRadonConnectionString.c_str());
        if (PQstatus(conn) != CONNECTION_OK)
        {
          fprintf(stderr, "Postgresql error: %s\n", PQerrorMessage(conn));
          return -4;
        }

        reconnectionRequired = false;
      }

      PRINT_DATA(mDebugLogPtr, "\n");
      PRINT_DATA(mDebugLogPtr, "********************************************************************\n");
      PRINT_DATA(mDebugLogPtr, "****************************** UPDATE ******************************\n");
      PRINT_DATA(mDebugLogPtr, "********************************************************************\n");

      try
      {
        if (!shutdownRequested)
        {
          PRINT_DATA(mDebugLogPtr, "* Reading producer names that belong to this update\n");
          readProducerList(mProducerFile.c_str());
        }

        if (!shutdownRequested)
        {
          PRINT_DATA(mDebugLogPtr, "* Reading producer information from the target data storage\n");
          readTargetProducers(mTargetProducerList);
        }

        if (!shutdownRequested)
        {
          PRINT_DATA(mDebugLogPtr, "* Reading producer information from the source data storage\n");
          readSourceProducers(conn);
        }

        if (!shutdownRequested)
        {
          PRINT_DATA(mDebugLogPtr, "* Updating producer information into the target data storage\n");
          updateProducers();
          readTargetProducers(mTargetProducerList);
        }

        if (!shutdownRequested)
        {
          PRINT_DATA(mDebugLogPtr, "* Reading generation information from the target data storage\n");
          readTargetGenerations(mTargetGenerationList);
          mTargetGenerationList.sort(T::GenerationInfo::ComparisonMethod::generationName);
        }

        if (!shutdownRequested)
        {
          PRINT_DATA(mDebugLogPtr, "* Reading generation information from the source data storage\n");
          readSourceGenerations(conn);
        }

        if (!shutdownRequested)
        {
          PRINT_DATA(mDebugLogPtr, "* Updating generation information into the target data storage\n");
          updateGenerations();
          readTargetGenerations(mTargetGenerationList);
          mTargetGenerationList.sort(T::GenerationInfo::ComparisonMethod::generationName);
        }

        if (!shutdownRequested)
        {
          PRINT_DATA(mDebugLogPtr, "* Reading content information from the target data storage\n");
          readTargetContentList(mTargetContentList);
          mTargetGenerationList.sort(T::GenerationInfo::ComparisonMethod::generationName);
        }

        if (!shutdownRequested)
        {
          PRINT_DATA(mDebugLogPtr, "* Updating file and content information into the target data storage\n");
          updateTargetFiles(conn);
        }

        if (!shutdownRequested && redisImplementation)
        {
          PRINT_DATA(mDebugLogPtr, "* Checking filenames \n");
          redisImplementation->syncFilenames();
        }
      }
      catch (...)
      {
        Fmi::Exception exception(BCP, "Processing exception!", nullptr);
        exception.printError();
      }


      if (!shutdownRequested)
      {
        PRINT_DATA(mDebugLogPtr, "* Data structures\n");
        PRINT_DATA(mDebugLogPtr, "   - mSourceProducerList   = %u\n",mSourceProducerList.getLength());
        PRINT_DATA(mDebugLogPtr, "   - mSourceGenerationList = %u\n",mSourceGenerationList.getLength());
        PRINT_DATA(mDebugLogPtr, "   - mSourceFilenames      = %lu\n",mSourceFilenames.size());
        PRINT_DATA(mDebugLogPtr, "   - mFilenameMap          = %lu\n",mFilenameMap.size());
        PRINT_DATA(mDebugLogPtr, "   - mFileRecMap           = %lu\n",mFileRecMap.size());
        PRINT_DATA(mDebugLogPtr, "   - fileRecords           = %u\n",countFileRecords());
        PRINT_DATA(mDebugLogPtr, "   - loopCounter           = %d\n",mLoopCounter);
        PRINT_DATA(mDebugLogPtr, "   - loopTime              = %ld sec\n",time(0)-loopStart);

        PRINT_DATA(mDebugLogPtr, "********************************************************************\n\n");

        mProducerList.clear();
        mSourceProducerList.clear();
        mSourceGenerationList.clear();

        std::set<int> sourceFilenames;
        mSourceFilenames.clear();
        mSourceFilenames.swap(sourceFilenames);

        std::set<unsigned long long> tcList;
        mTargetContentList.clear();
        mTargetContentList.swap(tcList);

        if ((lastDatabaseRead + 14400) < time(nullptr) || reconnectionRequired || conn == nullptr)
        {
          // ### Force full database read every 4th hour

          FileRec_map fileRecMap;
          string_bimap filenameMap;

          mFilenameMap.clear();
          mFilenameMap.swap(filenameMap);

          mFileRecMap.clear();
          mFileRecMap.swap(fileRecMap);

          lastDatabaseRead = time(nullptr);
        }
      }

      if (mWaitTime > 0 && !shutdownRequested)
      {
        if (mLoopCounter > 10)
          sleep(mWaitTime);
      }
      else
      {
        ind = false;
      }

      if (mWaitTime >= 1800 || reconnectionRequired)
      {
        if (conn != nullptr)
          PQfinish(conn);

        conn = nullptr;
      }
    }

    PQfinish(conn);

    return 0;
  }
  catch (Fmi::Exception& e)
  {
    Fmi::Exception exception(BCP, "Service call failed!", nullptr);
    exception.printError();
    return -3;
  }
}

