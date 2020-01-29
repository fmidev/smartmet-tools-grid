#include "grid-files/common/Exception.h"
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

#define FUNCTION_TRACE FUNCTION_TRACE_OFF

using namespace SmartMet;

// Some temporary storage structures:

struct ForecastRec
{
    uint producerId = 0;
    std::string producerName;
    std::string analysisTime;
    std::string deletionTime;
    std::string generationName;
    std::string forecastTime;
    std::string forecastPeriod;
    std::string tableName;
    uint geometryId = 0;
    int forecastTypeId = 0;
    int forecastTypeValue = 0;
    std::string lastUpdated;
};

struct FileRec
{
    int fileId;
    uint messageIndex;
    ulonglong filePosition;
    uint messageSize;
    std::string paramId;
    short levelId;
    int levelValue;
    time_t analysisTime;
    time_t forecastTime;
    short forecastType;
    short forecastNumber;
    int geometryId;
    int producerId;
};

struct FileRecVec
{
    std::string updateTime;
    uint loadCounter;
    std::vector<FileRec> files;
};

typedef std::map<std::string, FileRecVec> FileRec_map;


// Global variables:

std::vector<FileRec> emptyRecList;
ConfigurationFile mConfigurationFile;
std::string mGridConfigFile;
uint mSourceId = 100;
std::string mProducerFile;
std::string mPreloadFile;
uint mMaxMessageSize = 50000;
std::string mRadonConnectionString;
std::string mStorageType;
std::string mRedisAddress;
int mRedisPort = 6379;
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
std::set<std::string> mPreloadList;
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

FileRec_map mFileRecMap;
std::set<std::string> mFileTables;
std::map<std::string, std::string> mTableUpdates;
uint mWaitTime = 300;
uint mContentReadCount = 0;
uint mContentAddCount = 0;
uint mFileLoadCounter = 0;

ContentServer::ServiceInterface *mTargetInterface = nullptr;




void readConfigFile(const char* configFile)
{
  try
  {
    const char *configAttribute[] =
    { "smartmet.library.grid-files.configFile", "smartmet.tools.grid.radon2smartmet.maxMessageSize", "smartmet.tools.grid.radon2smartmet.content-source.source-id",
        "smartmet.tools.grid.radon2smartmet.content-source.producerFile", "smartmet.tools.grid.radon2smartmet.content-source.preloadFile",
        "smartmet.tools.grid.radon2smartmet.content-source.radon.connection-string", "smartmet.tools.grid.radon2smartmet.content-storage.type",
        "smartmet.tools.grid.radon2smartmet.content-storage.redis.address", "smartmet.tools.grid.radon2smartmet.content-storage.redis.port",
        "smartmet.tools.grid.radon2smartmet.content-storage.redis.tablePrefix", "smartmet.tools.grid.radon2smartmet.content-storage.corba.ior",
        "smartmet.tools.grid.radon2smartmet.content-storage.http.url", "smartmet.tools.grid.radon2smartmet.debug-log.enabled", "smartmet.tools.grid.radon2smartmet.debug-log.file",
        "smartmet.tools.grid.radon2smartmet.debug-log.maxSize", "smartmet.tools.grid.radon2smartmet.debug-log.truncateSize", nullptr };

    mConfigurationFile.readFile(configFile);
    //mConfigurationFile.print(std::cout,0,0);

    uint t = 0;
    while (configAttribute[t] != nullptr)
    {
      if (!mConfigurationFile.findAttribute(configAttribute[t]))
      {
        SmartMet::Spine::Exception exception(BCP, "Missing configuration attribute!");
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
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.content-source.preloadFile", mPreloadFile);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.content-source.radon.connection-string", mRadonConnectionString);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.content-storage.type", mStorageType);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.content-storage.redis.address", mRedisAddress);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.content-storage.redis.port", mRedisPort);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.content-storage.redis.tablePrefix", mRedisTablePrefix);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.content-storage.corba.ior", mContentServerIor);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.content-storage.http.url", mContentServerUrl);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.debug-log.enabled", mDebugLogEnabled);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.debug-log.file", mDebugLogFile);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.debug-log.maxSize", mDebugLogMaxSize);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.debug-log.truncateSize", mDebugLogTruncateSize);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Constructor failed!", nullptr);
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
    throw SmartMet::Spine::Exception(BCP, "Constructor failed!", nullptr);
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
    throw SmartMet::Spine::Exception(BCP, "Constructor failed!", nullptr);
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

    char st[1000];
    while (!feof(file))
    {
      if (fgets(st, 1000, file) != nullptr)
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
          //printf("Add producer [%s]\n",st);
          mProducerList.insert(toLowerString(std::string(st)));
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




void readPreloadList(const char *filename)
{
  FUNCTION_TRACE
  try
  {
    FILE *file = fopen(filename, "re");
    if (file == nullptr)
    {
      PRINT_DATA(mDebugLogPtr, "  -- Preload file not available.\n");
      return;
    }

    mPreloadList.clear();

    char st[1000];
    while (!feof(file))
    {
      if (fgets(st, 1000, file) != nullptr)
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

          std::vector < std::string > partList;
          splitString(st, ';', partList);
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




void readTargetProducers(T::ProducerInfoList& targetProducerList)
{
  FUNCTION_TRACE
  try
  {
    targetProducerList.clear();
    int result = mTargetInterface->getProducerInfoList(mSessionId, targetProducerList);
    if (result != 0)
    {
      SmartMet::Spine::Exception exception(BCP, "Cannot read the producer list from the target data storage!");
      exception.addParameter("Result", ContentServer::getResultString(result));
      throw exception;
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, exception_operation_failed, nullptr);
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
      SmartMet::Spine::Exception exception(BCP, "Cannot read the generation list from the target data storage!");
      exception.addParameter("Result", ContentServer::getResultString(result));
      throw exception;
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, exception_operation_failed, nullptr);
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
    throw SmartMet::Spine::Exception(BCP, exception_operation_failed, nullptr);
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

          fileInfo->mModificationTime = "";
          fileInfo->mDeletionTime = "";

          targetFileList.addFileInfo(fileInfo->duplicate());

          if (fileInfo->mFileId >= startFileId)
            startFileId = fileInfo->mFileId + 1;
        }
      }
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, exception_operation_failed, nullptr);
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
    throw SmartMet::Spine::Exception(BCP, exception_operation_failed, nullptr);
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
    throw SmartMet::Spine::Exception(BCP, exception_operation_failed, nullptr);
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
    throw SmartMet::Spine::Exception(BCP, exception_operation_failed, nullptr);
  }
}





std::string getTableInfo(PGconn *conn, const char *tableName, uint producerId, const char *analysisTime)
{
  FUNCTION_TRACE
  try
  {
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
    p += sprintf(p, "  producer_id=%u AND to_char(analysis_time, 'yyyymmddThh24MISS')='%s'\n", producerId, analysisTime);

    //printf("%s\n",sql);
    PGresult *res = PQexec(conn, sql);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
      SmartMet::Spine::Exception exception(BCP, "Postgresql error!");
      exception.addParameter("ErrorMessage", PQerrorMessage(conn));
      throw exception;
    }

    //int fieldCount = PQnfields(res);
    int rowCount = PQntuples(res);
    std::string result;

    if (rowCount == 1)
    {
      std::string count = PQgetvalue(res, 0, 0);
      std::string lastUpdated = PQgetvalue(res, 0, 1);
      result = lastUpdated + ":" + count;

      mTableUpdates.insert(std::pair<std::string, std::string>(tbl, result));
    }
    PQclear(res);

    return result;
  }
  catch (...)
  {
    SmartMet::Spine::Exception exception(BCP, exception_operation_failed, nullptr);
    exception.printError();
    std::string tmp;
    return tmp;
    // throw exception;
  }
}




void readTableRecords(PGconn *conn, const char *tableName, uint producerId, const char *analysisTime, std::string& info)
{
  FUNCTION_TRACE
  try
  {
    std::string tbl = std::string(tableName) + ":" + std::to_string(producerId) + ":" + std::string(analysisTime);
    if (mFileTables.find(tbl) != mFileTables.end())
      return;

    mFileTables.insert(tbl);

    char sql[2000];
    char *p = sql;

    p += sprintf(p, "SELECT\n");
    p += sprintf(p, "  file_location,\n");
    p += sprintf(p, "  message_no,\n");
    p += sprintf(p, "  byte_offset,\n");
    p += sprintf(p, "  byte_length,\n");
    p += sprintf(p, "  param_id,\n");
    p += sprintf(p, "  level_id,\n");
    p += sprintf(p, "  level_value::int,\n");
    p += sprintf(p, "  to_char(analysis_time, 'yyyymmddThh24MISS'),\n");
    p += sprintf(p, "  to_char((analysis_time+forecast_period) at time zone 'utc','yyyymmddThh24MISS'),\n");
    p += sprintf(p, "  forecast_period,\n");
    p += sprintf(p, "  forecast_type_id,\n");
    p += sprintf(p, "  forecast_type_value::int,\n");
    p += sprintf(p, "  geometry_id,\n");
    p += sprintf(p, "  producer_id\n");
    p += sprintf(p, "FROM\n");
    p += sprintf(p, "  %s\n", tableName);
    p += sprintf(p, "WHERE\n");
    p += sprintf(p, "  producer_id=%u AND to_char(analysis_time, 'yyyymmddThh24MISS')='%s'\n", producerId, analysisTime);

    //printf("%s\n",sql);
    PGresult *res = PQexec(conn, sql);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
      SmartMet::Spine::Exception exception(BCP, "Postgresql error!");
      exception.addParameter("ErrorMessage", PQerrorMessage(conn));
      throw exception;
    }

    //int fieldCount = PQnfields(res);
    int rowCount = PQntuples(res);

    PRINT_DATA(mDebugLogPtr, "      ---- Read table records %s:%u:%s : %d\n", tableName, producerId, analysisTime, rowCount);

    for (int i = 0; i < rowCount; i++)
    {
      if (strstr(PQgetvalue(res, i, 0), "masala") != nullptr)
      {
        FileRec rec;
        rec.fileId = getFileId(PQgetvalue(res, i, 0),true);
        rec.messageIndex = toInt64(PQgetvalue(res, i, 1));
        rec.filePosition = toInt64(PQgetvalue(res, i, 2));
        rec.messageSize = toInt64(PQgetvalue(res, i, 3));
        rec.paramId = PQgetvalue(res, i, 4);
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

        if (rec.levelId == 2)
          rec.levelValue = rec.levelValue * 100;

        char key[200];
        sprintf(key, "%s;%u;%u;%d;%d;%lu;%s", tableName, rec.producerId, rec.geometryId, rec.forecastType, rec.forecastNumber, rec.analysisTime, forecastPeriod.c_str());

        auto pos = mFileRecMap.find(key);
        if (pos == mFileRecMap.end())
        {
          FileRecVec vec;
          vec.updateTime = info;
          vec.files.push_back(rec);
          mFileRecMap.insert(std::pair<std::string, FileRecVec>(key, vec));
        }
        else
        {
          if (pos->second.updateTime != info)
            pos->second.files.clear();

          pos->second.updateTime = info;
          pos->second.files.push_back(rec);
        }
      }
    }
    PQclear(res);
  }
  catch (...)
  {
    SmartMet::Spine::Exception exception(BCP, exception_operation_failed, nullptr);
    exception.printError();
    // throw exception;
  }
}




std::vector<FileRec>& readSourceFilesAndContent(
    PGconn *conn,
    const char *tableName,
    uint producerId,
    uint geometryId,
    int forecastTypeId,
    int forecastTypeValue,
    const char *analysisTime,
    const char *forecastPeriod,
    std::string& updateTime,
    uint loadCounter)
{
  FUNCTION_TRACE
  try
  {
    char key[200];
    sprintf(key, "%s;%u;%u;%d;%d;%lu;%s", tableName, producerId, geometryId, forecastTypeId, forecastTypeValue, utcTimeToTimeT(analysisTime), forecastPeriod);

    std::string info = getTableInfo(conn, tableName, producerId, analysisTime);

    auto pos = mFileRecMap.find(key);
    if (pos != mFileRecMap.end())
    {
      if (pos->second.updateTime == info)
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

    readTableRecords(conn, tableName, producerId, analysisTime, info);
    pos = mFileRecMap.find(key);
    if (pos == mFileRecMap.end())
      return emptyRecList;

    pos->second.loadCounter = loadCounter;
    pos->second.updateTime = info;
    return pos->second.files;
  }
  catch (...)
  {
    SmartMet::Spine::Exception exception(BCP, exception_operation_failed, nullptr);
    exception.printError();
    // throw exception;
    return emptyRecList;
  }
}




void readSourceForecastTimes(PGconn *conn, uint fmiProducerId, std::vector<ForecastRec>& sourceForacastList)
{
  FUNCTION_TRACE
  try
  {
    char sql[2000];
    char *p = sql;
    p += sprintf(p, "SELECT DISTINCT\n");
    p += sprintf(p, "  fmi_producer.id,\n");
    p += sprintf(p, "  fmi_producer.name,\n");
    p += sprintf(p, "  to_char(ss_state_v.analysis_time at time zone 'utc', 'yyyymmddThh24MISS'),\n");
    p += sprintf(p, "  to_char((ss_state_v.analysis_time+ss_state_v.forecast_period) at time zone 'utc', 'yyyymmddThh24MISS'),\n");
    p += sprintf(p, "  ss_state_v.forecast_period,\n");
    p += sprintf(p, "  ss_state_v.table_name,\n");
    p += sprintf(p, "  ss_state_v.geometry_id,\n");
    p += sprintf(p, "  ss_state_v.forecast_type_id,\n");
    p += sprintf(p, "  ss_state_v.forecast_type_value,\n");
    p += sprintf(p, "  to_char(ss_state_v.delete_time at time zone 'utc', 'yyyymmddThh24MISS'),\n");
    p += sprintf(p, "  to_char(ss_state_v.last_updated at time zone 'utc', 'yyyymmddThh24MISS')\n");
    p += sprintf(p, "FROM\n");
    p += sprintf(p, "  ss_state_v LEFT OUTER JOIN fmi_producer ON fmi_producer.id = ss_state_v.producer_id\n");
    p += sprintf(p, "WHERE\n");
    p += sprintf(p, "  fmi_producer.id=%u\n", fmiProducerId);
    p += sprintf(p, "ORDER BY\n");
    p += sprintf(p, "  fmi_producer.id,to_char(ss_state_v.analysis_time at time zone 'utc', 'yyyymmddThh24MISS') desc,\n");
    p += sprintf(p, "  to_char((ss_state_v.analysis_time+ss_state_v.forecast_period) at time zone 'utc', 'yyyymmddThh24MISS');\n");

    PGresult *res = PQexec(conn, sql);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
      SmartMet::Spine::Exception exception(BCP, "Postgresql error!");
      exception.addParameter("ErrorMessage", PQerrorMessage(conn));
      throw exception;
    }

    //int fieldCount = PQnfields(res);
    int rowCount = PQntuples(res);

    for (int i = 0; i < rowCount; i++)
    {
      uint producerId = toInt64(PQgetvalue(res, i, 0));
      T::ProducerInfo *producer = mSourceProducerList.getProducerInfoById(producerId);
      if (producer != nullptr)
      {
        ForecastRec forecastRec;
        forecastRec.producerId = toInt64(PQgetvalue(res, i, 0));
        forecastRec.producerName = PQgetvalue(res, i, 1);
        forecastRec.analysisTime = PQgetvalue(res, i, 2);
        forecastRec.generationName = forecastRec.producerName + ":" + forecastRec.analysisTime;
        forecastRec.forecastTime = PQgetvalue(res, i, 3);
        forecastRec.forecastPeriod = PQgetvalue(res, i, 4);
        forecastRec.tableName = PQgetvalue(res, i, 5);
        forecastRec.geometryId = toInt64(PQgetvalue(res, i, 6));
        forecastRec.forecastTypeId = toInt64(PQgetvalue(res, i, 7));
        forecastRec.forecastTypeValue = toInt64(PQgetvalue(res, i, 8));
        forecastRec.deletionTime = PQgetvalue(res, i, 9);
        forecastRec.lastUpdated = PQgetvalue(res, i, 10);

        sourceForacastList.push_back(forecastRec);
      }
    }
    PQclear(res);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, exception_operation_failed, nullptr);
  }
}




void readSourceGenerations(PGconn *conn)
{
  FUNCTION_TRACE
  try
  {
    mSourceGenerationList.clear();

    char sql[1000];
    char *p = sql;
    p += sprintf(p, "SELECT DISTINCT\n");
    p += sprintf(p, "  fmi_producer.id,\n");
    p += sprintf(p, "  fmi_producer.name,\n");
    p += sprintf(p, "  to_char(ss_state_v.analysis_time at time zone 'utc', 'yyyymmddThh24MISS')\n");
    p += sprintf(p, "FROM\n");
    p += sprintf(p, "  ss_state_v LEFT OUTER JOIN fmi_producer ON fmi_producer.id = ss_state_v.producer_id\n");
    p += sprintf(p, "ORDER BY");
    p += sprintf(p, "  fmi_producer.id,to_char(ss_state_v.analysis_time at time zone 'utc', 'yyyymmddThh24MISS') desc;");

    PGresult *res = PQexec(conn, sql);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
      SmartMet::Spine::Exception exception(BCP, "Postgresql error!");
      exception.addParameter("ErrorMessage", PQerrorMessage(conn));
      throw exception;
    }

    //int fieldCount = PQnfields(res);
    int rowCount = PQntuples(res);

    for (int i = 0; i < rowCount; i++)
    {
      char st[1000];

      uint producerId = toInt64(PQgetvalue(res, i, 0));
      T::ProducerInfo *producer = mSourceProducerList.getProducerInfoById(producerId);
      if (producer != nullptr)
      {
        T::GenerationInfo *generation = new T::GenerationInfo();
        generation->mGenerationId = i + 1;
        generation->mGenerationType = 0;
        generation->mProducerId = producerId;
        sprintf(st, "%s:%s", producer->mName.c_str(), PQgetvalue(res, i, 2));
        generation->mName = st;
        sprintf(st, "Producer %s generation %s", producer->mName.c_str(), PQgetvalue(res, i, 2));
        generation->mDescription = st;
        generation->mAnalysisTime = PQgetvalue(res, i, 2);
        generation->mStatus = T::GenerationInfo::Status::Ready;
        generation->mFlags = 0;
        generation->mSourceId = mSourceId;
        mSourceGenerationList.addGenerationInfo(generation);
      }
    }
    PQclear(res);

    //mSourceGenerationList.print(std::cout,0,0);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, exception_operation_failed, nullptr);
  }
}




void readSourceProducers(PGconn *conn)
{
  FUNCTION_TRACE
  try
  {
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
      SmartMet::Spine::Exception exception(BCP, "Postgresql error!");
      exception.addParameter("ErrorMessage", PQerrorMessage(conn));
      throw exception;
    }

    //int fieldCount = PQnfields(res);
    int rowCount = PQntuples(res);

    for (int i = 0; i < rowCount; i++)
    {
      std::string searchStr = toLowerString(std::string(PQgetvalue(res, i, 1)));
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
    throw SmartMet::Spine::Exception(BCP, exception_operation_failed, nullptr);
  }
}




void updateProducers()
{
  FUNCTION_TRACE
  try
  {
    uint len = mTargetProducerList.getLength();
    for (uint t = 0; t < len; t++)
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

          PRINT_DATA(mDebugLogPtr, "  -- Remove producer : %s\n", targetProducer->mName.c_str());

          int result = mTargetInterface->deleteProducerInfoById(mSessionId, targetProducer->mProducerId);
          if (result != 0)
          {
            SmartMet::Spine::Exception exception(BCP, "Cannot delete the producer information from the target data storage!");
            exception.addParameter("ProducerId", std::to_string(targetProducer->mProducerId));
            exception.addParameter("ProducerName", targetProducer->mName);
            exception.addParameter("Result", ContentServer::getResultString(result));
            exception.printError();
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
            SmartMet::Spine::Exception exception(BCP, "Cannot add the producer information into the target data storage!");
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
    throw SmartMet::Spine::Exception(BCP, exception_operation_failed, nullptr);
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
      if (targetGeneration->mSourceId == mSourceId)
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
        SmartMet::Spine::Exception exception(BCP, "Cannot delete the generation information from the target data storage!");
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
              SmartMet::Spine::Exception exception(BCP, "The producer information not found from the target data storage!");
              exception.addParameter("ProducerName", sourceProducer->mName);
              throw exception;
            }

            T::GenerationInfo generationInfo(*sourceGeneration);
            generationInfo.mProducerId = targetProducer->mProducerId;
            generationInfo.mSourceId = mSourceId;

            PRINT_DATA(mDebugLogPtr, "  -- Add generation : %s\n", generationInfo.mName.c_str());

            int result = mTargetInterface->addGenerationInfo(mSessionId, generationInfo);
            if (result != 0)
            {
              SmartMet::Spine::Exception exception(BCP, "Cannot add the generation information into the target data storage!");
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
    throw SmartMet::Spine::Exception(BCP, exception_operation_failed, nullptr);
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
      if (fileInfo != nullptr)
      {
        if (fileInfo->mSourceId == mSourceId && mSourceFilenames.find(getFileId(fileInfo->mName,false)) == mSourceFilenames.end())
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
    throw SmartMet::Spine::Exception(BCP, exception_operation_failed, nullptr);
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
    throw SmartMet::Spine::Exception(BCP, exception_operation_failed, nullptr);
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
    throw SmartMet::Spine::Exception(BCP, exception_operation_failed, nullptr);
  }
}




void readSourceFilesByForecastTime(PGconn *conn, ForecastRec& forecast, uint loadCounter, T::FileInfoList& targetFileList, std::vector<T::FileAndContent>& fileAndContentList, std::vector<FileRec>& fileRecList)
{
  FUNCTION_TRACE
  try
  {
    T::GenerationInfo generation;
    if (mTargetInterface->getGenerationInfoByName(mSessionId, forecast.generationName, generation) != 0)
    {
      PRINT_DATA(mDebugLogPtr, "**** Unknown generation : %s\n", forecast.generationName.c_str());
      return;
    }

    std::vector<FileRec> recList = readSourceFilesAndContent(conn, forecast.tableName.c_str(), forecast.producerId, forecast.geometryId, forecast.forecastTypeId,
        forecast.forecastTypeValue, forecast.analysisTime.c_str(), forecast.forecastPeriod.c_str(), forecast.lastUpdated, loadCounter);

    mContentReadCount += recList.size();
    PRINT_DATA(mDebugLogPtr, "  -- Read files by foracast time %s:%u:%d:%d:%d : %lu (contentCount = %u)\n", forecast.forecastTime.c_str(), forecast.producerId, forecast.geometryId,
        forecast.forecastTypeId, forecast.forecastTypeValue, recList.size(), mContentReadCount);
    if (recList.size() == 0)
      return;

    for (auto it = recList.begin(); it != recList.end(); ++it)
    {
      fileRecList.push_back(*it);
      T::FileInfo fileInfo;
      std::string filename = getFilename(it->fileId);
      if (mSourceFilenames.find(it->fileId) == mSourceFilenames.end() && targetFileList.getFileInfoByName(filename) == nullptr)
      {
        // File does not exists. It should be added.

        T::FileAndContent fc;

        fc.mFileInfo.mGroupFlags = 0;
        fc.mFileInfo.mProducerId = generation.mProducerId;
        fc.mFileInfo.mGenerationId = generation.mGenerationId;
        fc.mFileInfo.mFileId = 0;
        fc.mFileInfo.mFileType = T::FileTypeValue::Unknown;
        fc.mFileInfo.mName = filename;
        fc.mFileInfo.mFlags = T::FileInfo::Flags::PredefinedContent;
        fc.mFileInfo.mSourceId = mSourceId;
        fc.mFileInfo.mDeletionTime = forecast.deletionTime;

        fileAndContentList.push_back(fc);

        if (fileAndContentList.size() >= mMaxMessageSize)
        {
          PRINT_DATA(mDebugLogPtr, "  -- Add file information : %lu\n", fileAndContentList.size());
          int result = mTargetInterface->addFileInfoListWithContent(mSessionId, 0, fileAndContentList);
          if (result != 0)
          {
            fprintf(stdout, "ERROR (%d) : %s\n", result, ContentServer::getResultString(result).c_str());
          }
          fileAndContentList.clear();
        }
      }

      if (mSourceFilenames.find(it->fileId) == mSourceFilenames.end())
        mSourceFilenames.insert(it->fileId);
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, exception_operation_failed, nullptr);
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
//    T::ContentInfoList targetContentList;
    readTargetContentList(targetContentList);
    //targetContentList.sort(T::ContentInfo::ComparisonMethod::file_message);

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
          contentInfo->mGroupFlags = 0;
          contentInfo->mForecastTime = utcTimeFromTimeT(it->forecastTime);
          contentInfo->mFmiParameterId = it->paramId;
          contentInfo->mFmiParameterLevelId = it->levelId;
          contentInfo->mParameterLevel = it->levelValue;
          contentInfo->mForecastType = it->forecastType;
          contentInfo->mForecastNumber = it->forecastNumber;
          contentInfo->mServerFlags = 0;
          contentInfo->mFlags = 0;
          contentInfo->mSourceId = mSourceId;
          contentInfo->mGeometryId = it->geometryId;
          //contentInfo->mModificationTime = it->lastUpdated;

          Identification::FmiParameterDef fmiDef;
          if (Identification::gridDef.getFmiParameterDefById(it->paramId, fmiDef))
          {
            contentInfo->mFmiParameterName = fmiDef.mParameterName;
            contentInfo->mFmiParameterUnits = fmiDef.mParameterUnits;

            Identification::NewbaseParameterDef newbaseDef;
            if (Identification::gridDef.getNewbaseParameterDefByFmiId(it->paramId, newbaseDef))
            {
              contentInfo->mNewbaseParameterId = newbaseDef.mNewbaseParameterId;
              contentInfo->mNewbaseParameterName = newbaseDef.mParameterName;
            }
          }
          /*
           char st[200];
           sprintf(st,"%s;%s;%d;%d;%05d;%d;%d;1;",
           forecast.producerName.c_str(),
           contentInfo->mFmiParameterName.c_str(),
           (int)T::ParamLevelIdTypeValue::FMI,
           (int)contentInfo->mFmiParameterLevelId,
           (int)contentInfo->mParameterLevel,
           (int)contentInfo->mForecastType,
           (int)contentInfo->mForecastNumber);

           if (mPreloadList.find(toLowerString(std::string(st))) != mPreloadList.end())
           contentInfo->mFlags = T::ContentInfo::Flags::PreloadRequired;
           */
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
        printf("#### File info not found : %s\n", filename.c_str());
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
    throw SmartMet::Spine::Exception(BCP, exception_operation_failed, nullptr);
  }
}




void updateTargetFiles(PGconn *conn)
{
  FUNCTION_TRACE
  try
  {
    mFileLoadCounter++;


    PRINT_DATA(mDebugLogPtr, "* Reading file information from the target data storage\n");

    T::FileInfoList targetFileList;
    readTargetFileList(targetFileList);
    targetFileList.sort(T::FileInfo::ComparisonMethod::fileName);

    mContentReadCount = 0;
    mContentAddCount = 0;

    std::vector<FileRec> fileRecList;

    uint len = mSourceProducerList.getLength();
    for (uint t = 0; t < len; t++)
    {
      T::ProducerInfo *sourceProducer = mSourceProducerList.getProducerInfoByIndex(t);
      if (sourceProducer->mSourceId == mSourceId)
      {
        T::ProducerInfo *targetProducer = mTargetProducerList.getProducerInfoByName(sourceProducer->mName);
        if (targetProducer != nullptr)
        {
          PRINT_DATA(mDebugLogPtr, "  ** Add file information : %s\n", targetProducer->mName.c_str());

          std::vector<ForecastRec> sourceForacastList;
          readSourceForecastTimes(conn, sourceProducer->mProducerId, sourceForacastList);

          std::vector < T::FileAndContent > fileAndContentList;
          //std::vector<FileRec> fileRecList;

          for (auto it = sourceForacastList.begin(); it != sourceForacastList.end(); ++it)
          {
            readSourceFilesByForecastTime(conn, *it, mFileLoadCounter,targetFileList,fileAndContentList, fileRecList);
          }

          if (fileAndContentList.size() > 0)
          {
            PRINT_DATA(mDebugLogPtr, "  -- Add file information : %lu\n", fileAndContentList.size());
            int result = mTargetInterface->addFileInfoListWithContent(mSessionId, 0, fileAndContentList);
            if (result != 0)
            {
              fprintf(stdout, "ERROR (%d) : %s\n", result, ContentServer::getResultString(result).c_str());
            }
            fileAndContentList.clear();
          }

          //PRINT_DATA(mDebugLogPtr,"* Updating content information into the target data storage\n");
          //saveTargetContent(targetProducer->mProducerId,fileRecList);
        }
      }
    }

    std::map<std::string, std::string> tableUpdates;
    mTableUpdates.clear();
    mTableUpdates.swap(tableUpdates);

    std::set<std::string> fileTables;
    mFileTables.clear();
    mFileTables.swap(fileTables);

    targetFileList.clear();

    PRINT_DATA(mDebugLogPtr, "* Updating content information into the target data storage\n");
    saveTargetContent(fileRecList);

    deleteOldFileRecords(mFileLoadCounter);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, exception_operation_failed, nullptr);
  }
}




int main(int argc, char *argv[])
{
  FUNCTION_TRACE
  try
  {
    if (argc < 3)
    {
      fprintf(stderr, "USAGE: radon2smartmet <configFile> <loopWaitTime>\n");
      return -1;
    }

    readConfigFile(argv[1]);

    // Initializing the global structures. These are needed when
    // extracting information from GRIB files.

    Identification::gridDef.init(mGridConfigFile.c_str());

    mWaitTime = toInt64(argv[2]);

    if (mStorageType == "redis")
    {
      ContentServer::RedisImplementation *redisImplementation = new ContentServer::RedisImplementation();
      redisImplementation->init(mRedisAddress.c_str(), mRedisPort, mRedisTablePrefix.c_str());
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

    PGconn *conn = PQconnectdb(mRadonConnectionString.c_str());
    if (PQstatus(conn) != CONNECTION_OK)
    {
      fprintf(stderr, "Postgresql error: %s\n", PQerrorMessage(conn));
      return -4;
    }

    time_t lastDatabaseRead = time(nullptr);
    bool ind = true;
    while (ind)
    {
      PRINT_DATA(mDebugLogPtr, "\n");
      PRINT_DATA(mDebugLogPtr, "********************************************************************\n");
      PRINT_DATA(mDebugLogPtr, "****************************** UPDATE ******************************\n");
      PRINT_DATA(mDebugLogPtr, "********************************************************************\n");

      PRINT_DATA(mDebugLogPtr, "* Reading producer names that belongs to this update\n");
      readProducerList(mProducerFile.c_str());

      PRINT_DATA(mDebugLogPtr, "* Reading preload parameters\n");
      readPreloadList(mPreloadFile.c_str());

      PRINT_DATA(mDebugLogPtr, "* Reading producer information from the target data storage\n");
      readTargetProducers(mTargetProducerList);

      PRINT_DATA(mDebugLogPtr, "* Reading producer information from the source data storage\n");
      readSourceProducers(conn);

      PRINT_DATA(mDebugLogPtr, "* Updating producer information into the target data storage\n");
      updateProducers();
      readTargetProducers(mTargetProducerList);

      PRINT_DATA(mDebugLogPtr, "* Reading generation information from the target data storage\n");
      readTargetGenerations(mTargetGenerationList);
      mTargetGenerationList.sort(T::GenerationInfo::ComparisonMethod::generationName);

      PRINT_DATA(mDebugLogPtr, "* Reading generation information from the source data storage\n");
      readSourceGenerations(conn);

      PRINT_DATA(mDebugLogPtr, "* Updating generation information into the target data storage\n");
      updateGenerations();
      readTargetGenerations(mTargetGenerationList);
      mTargetGenerationList.sort(T::GenerationInfo::ComparisonMethod::generationName);

      PRINT_DATA(mDebugLogPtr, "* Updating file and content information into the target data storage\n");
      updateTargetFiles(conn);

      PRINT_DATA(mDebugLogPtr, " * Data structures\n");
      PRINT_DATA(mDebugLogPtr, "   - mSourceProducerList   = %u\n",mSourceProducerList.getLength());
      PRINT_DATA(mDebugLogPtr, "   - mSourceGenerationList = %u\n",mSourceGenerationList.getLength());
      PRINT_DATA(mDebugLogPtr, "   - mSourceFilenames      = %lu\n",mSourceFilenames.size());
      PRINT_DATA(mDebugLogPtr, "   - mFilenameMap          = %lu\n",mFilenameMap.size());
      PRINT_DATA(mDebugLogPtr, "   - mFileRecMap           = %lu\n",mFileRecMap.size());
      PRINT_DATA(mDebugLogPtr, "   - fileRecords           = %u\n",countFileRecords());

      PRINT_DATA(mDebugLogPtr, "********************************************************************\n\n");

      mProducerList.clear();
      mPreloadList.clear();
      mSourceProducerList.clear();
      mSourceGenerationList.clear();

      std::set<int> sourceFilenames;
      mSourceFilenames.clear();
      mSourceFilenames.swap(sourceFilenames);

      if ((lastDatabaseRead + 14400) < time(nullptr))
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

      if (mWaitTime > 0)
      {
        sleep(mWaitTime);
      }
      else
      {
        ind = false;
      }
    }

    PQfinish(conn);

    return 0;
  }
  catch (SmartMet::Spine::Exception& e)
  {
    SmartMet::Spine::Exception exception(BCP, "Service call failed!", nullptr);
    exception.printError();
    return -3;
  }
}

