/*! \file
 *  \brief Daemon that continuously synchronises meteorological forecast file metadata from the FMI Radon database into a SmartMet Content Server.
 */

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
#include "grid-content/contentServer/postgresql/PostgresqlImplementation.h"
#include <boost/bimap.hpp>

#include <csignal>
#include <cstring>
#include <libpq-fe.h>
#include <cstdlib>
#include <chrono>
#include <thread>


#define FUNCTION_TRACE FUNCTION_TRACE_OFF

using namespace SmartMet;

// Some temporary storage structures:

struct ForecastRec
{
    T::ProducerId producerId = 0;
    uint geometryId = 0;
    int forecastTypeId = 0;
    int forecastTypeValue = 0;
    time_t deletionTime = 0;
    time_t lastUpdated = 0;
    std::string producerName;
    std::string analysisTime;
    std::string generationName;
    std::string forecastTime;
    std::string forecastPeriod;
    std::string tableName;
};

struct FileRec
{
    T::ProducerId producerId = 0;
    T::FileId fileId = 0;
    T::MessageIndex messageIndex = 0;
    uint paramId = 0;
    UInt64 filePosition = 0;
    uint messageSize = 0;
    T::GeometryId geometryId = 0;
    time_t analysisTime = 0;
    time_t forecastTime = 0;
    int levelValue = 0;
    short levelId = 0;
    T::ForecastType forecastType = 0;
    T::ForecastNumber forecastNumber = 0;
    uchar serverType = 0;
    uchar format = 0;
    int aggregation_id = 0;
    int aggregation_period = 0;
    int processing_type_id = 0;
    float processing_type_value = 0.0F;
    float processing_type_value2 = 0.0F;
    time_t lastUpdated = 0;
    std::string producerName;
    std::string server;
    bool cachingRequested = false;
};

struct FileRecVec
{
    time_t updateTime = 0;
    uint loadCounter = 0;
    std::string info;
    std::vector<FileRec> files;
};

struct GeomRec
{
    T::ProducerId producerId = 0;
    T::GeometryId geometryId = 0;
    T::ParamLevelId levelId = 0;
    std::string producerName;
    std::string analysisTime;
};

using FileRec_map = std::unordered_map<std::string, FileRecVec>;


// Global variables:

namespace
{
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
std::string mRedisPassword;
std::string mRedisTablePrefix;
std::string mContentServerIor;
std::string mContentServerUrl;
std::string mPostgresqlConnectionString;

bool mProcessingLogEnabled = false;
std::string mProcessingLogFile;
int mProcessingLogMaxSize = 100000000;
int mProcessingLogTruncateSize = 20000000;
Log mProcessingLog;
Log* mProcessingLogPtr = nullptr;

bool mDebugLogEnabled = false;
std::string mDebugLogFile;
int mDebugLogMaxSize = 100000000;
int mDebugLogTruncateSize = 20000000;
Log mDebugLog;
Log* mDebugLogPtr = nullptr;

T::SessionId mSessionId = 0;

std::set<std::string> mProducerList;
std::unordered_map<std::string,std::set<int>> mProducerLevelTypeCacheList;

std::set<std::string> mProducerFullList;
std::set<uint> mProducerIdList;
std::unordered_map<std::string,GeomRec> mSourceGeometryList;
std::unordered_map<std::string,std::set<int>> mProducerGeometryList;

std::unordered_map<std::string, std::vector<std::string>> mProducerDependensies;
std::unordered_map<std::string, std::set<std::string>> mAcceptedParameters;
std::unordered_map<std::string, std::set<std::string>> mIgnoredParameters;

std::set<std::string> mUpdateProducerList;
T::ProducerInfoList mSourceProducerList;
T::GenerationInfoList mSourceGenerationList;
//std::vector<ForecastRec>  mSourceForacastList;
std::set<int> mSourceFilenames;
int mFileIdCounter = 0;
}  // namespace

using string_bimap = boost::bimap< std::string, int >;
using bimap_rec = string_bimap::value_type;

namespace
{
string_bimap mFilenameMap;

T::ProducerInfoList mTargetProducerList;
T::GenerationInfoList mTargetGenerationList;
T::GeometryInfoList mTargetGeometryList;
std::set<UInt64> mTargetContentList;
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




/*! \brief Sig handler. */

void sig_handler(int /*signum*/)
{
  printf("##### SHUTDOWN REQUESTED #####\n");
  shutdownRequested = true;
}



/*! \brief Read config file. */

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
        "smartmet.tools.grid.radon2smartmet.processing-log.enabled",
        "smartmet.tools.grid.radon2smartmet.processing-log.file",
        "smartmet.tools.grid.radon2smartmet.processing-log.maxSize",
        "smartmet.tools.grid.radon2smartmet.processing-log.truncateSize",
        "smartmet.tools.grid.radon2smartmet.debug-log.enabled",
        "smartmet.tools.grid.radon2smartmet.debug-log.file",
        "smartmet.tools.grid.radon2smartmet.debug-log.maxSize",
        "smartmet.tools.grid.radon2smartmet.debug-log.truncateSize",
        nullptr
    };

    const char *configAttribute_redis[] =
    {
        "smartmet.tools.grid.radon2smartmet.content-storage.redis.address",
        "smartmet.tools.grid.radon2smartmet.content-storage.redis.port",
        "smartmet.tools.grid.radon2smartmet.content-storage.redis.tablePrefix",
        nullptr
    };

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

    if (mStorageType == "redis")
    {
      uint t = 0;
      while (configAttribute_redis[t] != nullptr)
      {
        if (!mConfigurationFile.findAttribute(configAttribute_redis[t]))
        {
          Fmi::Exception exception(BCP, "Missing configuration attribute!");
          exception.addParameter("File", configFile);
          exception.addParameter("Attribute", configAttribute_redis[t]);
          throw exception;
        }
        t++;
      }

      mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.content-storage.redis.address", mRedisAddress);
      mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.content-storage.redis.port", mRedisPort);
      mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.content-storage.redis.tablePrefix", mRedisTablePrefix);
      mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.content-storage.redis.lockEnabled", mRedisLockEnabled);
      mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.content-storage.redis.password", mRedisPassword);
    }
    else
    {
      mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.content-storage.corba.ior", mContentServerIor);
      mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.content-storage.http.url", mContentServerUrl);
      mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.content-storage.postgresql.connection-string", mPostgresqlConnectionString);
    }

    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.processing-log.enabled", mProcessingLogEnabled);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.processing-log.file", mProcessingLogFile);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.processing-log.maxSize", mProcessingLogMaxSize);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.processing-log.truncateSize", mProcessingLogTruncateSize);
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





/*! \brief Get file id. */

int getFileId(const std::string& filename,bool create)
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




/*! \brief Get filename. */

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





/*! \brief Check if the producer should be processed during the current loop. */

bool isProducerScheduled(int f,int i)
{
  return (mWaitTime == 0 || mLoopCounter == f || (mLoopCounter > f  &&  ((mLoopCounter-f) % i) == 0));
}




/*! \brief Add the producer and its settings. */

void addProducer(const std::string& pname,int e,const std::vector<std::string>& pList,const std::set<int>& geometries,const std::set<int>& levelTypes,const std::set<std::string>& acceptedParameters,const std::set<std::string>& ignoredParameters)
{
  mProducerList.insert(pname);

  if (!geometries.empty())
    mProducerGeometryList.insert(std::pair<std::string, std::set<int>>(pname,geometries));

  if (!levelTypes.empty())
    mProducerLevelTypeCacheList.insert(std::pair<std::string, std::set<int>>(pname,levelTypes));

  if (!acceptedParameters.empty())
    mAcceptedParameters.insert(std::pair<std::string, std::set<std::string>>(pname,acceptedParameters));

  if (!ignoredParameters.empty())
    mIgnoredParameters.insert(std::pair<std::string, std::set<std::string>>(pname,ignoredParameters));

  if (e == 0)
    mGenerationStatusCheckIgnore.insert(pname);

  if (pList.size() > 1)
    mProducerDependensies.insert(std::pair<std::string, std::vector<std::string>>(pname,pList));
}




/*! \brief Read a producer line from the producer file. */

void readProducerLine(char *st)
{
  int f = 1;
  int i = 1;
  int e = 0;
  char *p = strstr(st,"\n");
  if (p)
    *p = '\0';

  std::vector <std::string> fields;
  splitString(st,';',fields);
  std::set<int> geometries;
  std::set<int> levelTypes;

  if (!fields.empty())
  {
    if (fields.size() > 1)
      e = toInt32(fields[1]);

    if (fields.size() > 2)
      f = toInt32(fields[2]);

    if (fields.size() > 3)
      i = toInt32(fields[3]);

    if (fields.size() > 4)
      splitString(fields[4],',',geometries);

    std::set<std::string> acceptedParameters;
    if (fields.size() > 5)
      splitString(fields[5],',',acceptedParameters);

    std::set<std::string> ignoredParameters;
    if (fields.size() > 6)
      splitString(fields[6],',',ignoredParameters);

    if (fields.size() > 7)
      splitString(fields[7],',',levelTypes);

    std::vector<std::string> pList;
    splitString(fields[0],',',pList);

    for (auto it = pList.begin(); it != pList.end(); ++it)
    {
      std::string pname = toUpperString(*it);
      mProducerFullList.insert(pname);

      if (isProducerScheduled(f,i))
        addProducer(pname,e,pList,geometries,levelTypes,acceptedParameters,ignoredParameters);
    }
  }
}




/*! \brief Read producer list. */

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
    mProducerFullList.clear();
    mProducerDependensies.clear();
    mAcceptedParameters.clear();
    mIgnoredParameters.clear();

    char st[1000];
    while (fgets(st,1000,file) != nullptr)
    {
      if (st[0] != '#')
        readProducerLine(st);
    }
    (void)fclose(file);
    mLoopCounter++;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}





/*! \brief Read target producers. */

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




/*! \brief Read target generations. */

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




/*! \brief Read target geometries. */

void readTargetGeometries(T::GeometryInfoList& targetGeometryList)
{
  FUNCTION_TRACE
  try
  {
    targetGeometryList.clear();
    targetGeometryList.setComparisonMethod(T::GeometryInfo::ComparisonMethod::none);
    int result = mTargetInterface->getGeometryInfoList(mSessionId, targetGeometryList);
    if (result != 0)
    {
      Fmi::Exception exception(BCP, "Cannot read the geometry list from the target data storage!");
      exception.addParameter("Result", ContentServer::getResultString(result));
      throw exception;
    }
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}







/*! \brief Read target file list. */

void readTargetFileList(T::ProducerId producerId, T::FileInfoList& targetFileList)
{
  FUNCTION_TRACE
  try
  {
    targetFileList.clear();
    targetFileList.setComparisonMethod(T::FileInfo::ComparisonMethod::none);
    T::FileId startFileId = 0;
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











/*! \brief Read target content list. */

void readTargetContentList(std::set<UInt64>& targetContentList)
{
  FUNCTION_TRACE
  try
  {
    targetContentList.clear();

    T::FileId startFileId = 0;
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

            UInt64 id = contentInfo->mFileId;
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









/*! \brief Get table info. */

std::string getTableInfo(PGconn *conn, const char *tableName, T::ProducerId producerId, uint geometryId, const char *analysisTime,time_t& lastUpdate,uint& count)
{
  FUNCTION_TRACE
  try
  {
    if (shutdownRequested)
      return "";

    std::string tbl = std::string(tableName) + ":" + std::to_string(producerId) + ":" + std::to_string(geometryId) + ":" + std::string(analysisTime);
    auto pos = mTableUpdates.find(tbl);
    if (pos != mTableUpdates.end())
      return pos->second;

    char sql[2000];
    char *p = sql;

    PRINT_DATA(mDebugLogPtr, "      ---- Read table information %s:%u:%u:%s\n", tableName, producerId, geometryId, analysisTime);

    p += sprintf(p, "SELECT\n");
    p += sprintf(p, "  count(file_location),\n");
    p += sprintf(p, "  max(to_char(last_updated, 'yyyymmddThh24MISS'))\n");
    p += sprintf(p, "FROM\n");
    p += sprintf(p, "  %s\n", tableName);
    p += sprintf(p, "WHERE\n");
    sprintf(p, "  producer_id=%u AND geometry_id=%u AND analysis_time = to_timestamp('%s', 'yyyymmddThh24MISS')\n", producerId, geometryId, analysisTime);

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
      count = toInt32(PQgetvalue(res, 0, 0));
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




/*! \brief Read a file record from the query result row. */

void readFileRec(PGresult *res,int i,FileRec& rec,std::string& forecastPeriod)
{
  rec.fileId = getFileId(PQgetvalue(res, i, 0),true);
  rec.messageIndex = toInt64(PQgetvalue(res, i, 1));
  rec.filePosition = toInt64(PQgetvalue(res, i, 2));
  rec.messageSize = toInt64(PQgetvalue(res, i, 3));
  rec.paramId = toUInt32(PQgetvalue(res, i, 4));
  rec.levelId = toInt64(PQgetvalue(res, i, 5));
  rec.levelValue = toInt64(PQgetvalue(res, i, 6));
  rec.analysisTime = utcTimeToTimeT(PQgetvalue(res, i, 7));
  rec.forecastTime = utcTimeToTimeT(PQgetvalue(res, i, 8));
  forecastPeriod = PQgetvalue(res, i, 9);
  rec.forecastType = toInt64(PQgetvalue(res, i, 10));
  rec.forecastNumber = toInt64(PQgetvalue(res, i, 11));
  rec.geometryId = toInt64(PQgetvalue(res, i, 12));
  rec.producerId = toInt64(PQgetvalue(res, i, 13));

  rec.format = toInt32(PQgetvalue(res, i, 14));
  rec.serverType = toInt32(PQgetvalue(res, i, 15));
  rec.server = PQgetvalue(res, i, 16);
/*
  rec.aggregation_id = 0;
  rec.aggregation_period = 0;
  rec.processing_type_id = 0;
  rec.processing_type_value = 0;
  rec.processing_type_value2 = 0;
*/
  rec.aggregation_id = toInt32(PQgetvalue(res, i, 17));
  rec.aggregation_period = toInt64(PQgetvalue(res, i, 18)) / 60;
  rec.processing_type_id = toInt32(PQgetvalue(res, i, 19));

  rec.processing_type_value = ParamValueMissing;
  if (!PQgetisnull(res, i, 20))
    rec.processing_type_value = toFloat(PQgetvalue(res, i, 20));

  rec.processing_type_value2 = ParamValueMissing;
  if (!PQgetisnull(res, i, 21))
    rec.processing_type_value2 = toFloat(PQgetvalue(res, i, 21));
}




/*! \brief Check if the parameter is accepted and not ignored for the producer. */

bool isParameterAccepted(const std::string& producerName,uint paramId)
{
  bool accepted = true;

  Identification::FmiParameterDef fmiDef;
  if (Identification::gridDef.getFmiParameterDefById(paramId, fmiDef))
  {
    auto arec = mAcceptedParameters.find(producerName);
    if (arec != mAcceptedParameters.end()  &&  !arec->second.empty())
    {
      if (arec->second.find(fmiDef.mParameterName) == arec->second.end())
        accepted = false;
    }

    if (accepted)
    {
      auto irec = mIgnoredParameters.find(producerName);
      if (irec != mIgnoredParameters.end()  &&  !irec->second.empty())
      {
        if (irec->second.find(fmiDef.mParameterName) != irec->second.end())
          accepted = false;
      }
    }
  }

  return accepted;
}




/*! \brief Add the file record into the file record map. */

void addFileRec(const char *tableName,FileRec& rec,const std::string& forecastPeriod,time_t updateTime,std::string& info)
{
  char key[200];
  sprintf(key, "%s;%u;%d;%d;%d;%ld;%s", tableName, rec.producerId, rec.geometryId, rec.forecastType, rec.forecastNumber, rec.analysisTime, forecastPeriod.c_str());

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




/*! \brief Read table records. */

void readTableRecords(PGconn *conn, const char *tableName, T::ProducerId producerId, std::string& producerName,uint geometryId,const char *analysisTime, time_t updateTime, std::string& info)
{
  FUNCTION_TRACE
  try
  {
    if (shutdownRequested)
      return;

    auto lt = mProducerLevelTypeCacheList.find(producerName);


    std::string tbl = std::string(tableName) + ":" + std::to_string(producerId) + ":" + std::to_string(geometryId) + ":" + std::string(analysisTime);
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
    p += sprintf(p, "  file_format_id,\n");
    p += sprintf(p, "  file_protocol_id,\n");
    p += sprintf(p, "  file_server,\n");

    p += sprintf(p, "  aggregation_id,\n");
    p += sprintf(p, "  date_part('epoch',aggregation_period::interval),\n");
    p += sprintf(p, "  processing_type_id,\n");
    p += sprintf(p, "  processing_type_value,\n");
    p += sprintf(p, "  processing_type_value2\n");

    p += sprintf(p, "FROM\n");
    p += sprintf(p, "  %s\n", tableName);
    p += sprintf(p, "WHERE\n");
    sprintf(p, "  producer_id=%u AND geometry_id = %u AND analysis_time = to_timestamp('%s', 'yyyymmddThh24MISS') AND file_format_id IN (1,2,3,4)\n", producerId, geometryId,analysisTime);

    //printf("%s\n",sql);
    PGresult *res = PQexec(conn, sql);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
      Fmi::Exception exception(BCP, "Postgresql error!");
      exception.addParameter("ErrorMessage", PQerrorMessage(conn));
      printf("%s\n",sql);
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
      std::string forecastPeriod;
      readFileRec(res,i,rec,forecastPeriod);

      rec.producerName = producerName;
      rec.lastUpdated = updateTime;

      if (rec.levelId == 2)
        rec.levelValue = rec.levelValue * 100;

      rec.cachingRequested = lt != mProducerLevelTypeCacheList.end()  &&  lt->second.find(rec.levelId) != lt->second.end();

      bool accepted = isParameterAccepted(producerName,rec.paramId);

      if (accepted)
        addFileRec(tableName,rec,forecastPeriod,updateTime,info);
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




/*! \brief Read source files and content. */

std::vector<FileRec>& readSourceFilesAndContent(
    PGconn *conn,
    const char *tableName,
    T::ProducerId producerId,
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
    sprintf(key, "%s;%u;%u;%d;%d;%ld;%s", tableName, producerId, geometryId, forecastTypeId, forecastTypeValue, utcTimeToTimeT(analysisTime), forecastPeriod);

    uint count = 0;
    time_t lastUpdate = 0;
    std::string info = getTableInfo(conn, tableName, producerId, geometryId, analysisTime,lastUpdate,count);
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
      std::string tbl = std::string(tableName) + ":" + std::to_string(producerId) + ":" + std::to_string(geometryId) + ":" + std::string(analysisTime);
      auto pos = mFileTables.find(tbl);
      if (pos != mFileTables.end())
      {
        mFileTables.erase(pos);
      }
    }

    readTableRecords(conn, tableName, producerId, producerName, geometryId, analysisTime, updateTime, info);
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




/*! \brief Read source forecast times. */

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
    sprintf(p, "  to_char((ss_state_v.analysis_time+ss_state_v.forecast_period) at time zone 'utc', 'yyyymmddThh24MISS');\n");

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

      T::ProducerId producerId = toInt64(PQgetvalue(res, i, 0));
      std::string analysisTime = PQgetvalue(res, i, 2);
      int geometryId = toInt64(PQgetvalue(res, i, 6));


      T::ProducerInfo *producer = mSourceProducerList.getProducerInfoById(producerId);
      if (producer != nullptr)
      {
        auto geom = mProducerGeometryList.find(toUpperString(producer->mName));
        if (geom == mProducerGeometryList.end() ||  geom->second.find(geometryId) != geom->second.end())
        {
          ForecastRec forecastRec;
          forecastRec.producerId = producerId;
          forecastRec.producerName = PQgetvalue(res, i, 1);
          forecastRec.analysisTime = analysisTime;
          forecastRec.generationName = forecastRec.producerName + ":" + forecastRec.analysisTime;
          forecastRec.forecastTime = PQgetvalue(res, i, 3);
          forecastRec.forecastPeriod = PQgetvalue(res, i, 4);
          forecastRec.tableName = PQgetvalue(res, i, 5);
          forecastRec.geometryId = geometryId;
          forecastRec.forecastTypeId = toInt64(PQgetvalue(res, i, 7));
          forecastRec.forecastTypeValue = toInt64(PQgetvalue(res, i, 8));
          std::string dtime = PQgetvalue(res, i, 9);
          forecastRec.deletionTime = utcTimeToTimeT(dtime);
          std::string modtime = PQgetvalue(res, i, 10);
          forecastRec.lastUpdated = utcTimeToTimeT(modtime);
          sourceForecastList.push_back(forecastRec);
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




/*! \brief Read source generations. */

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
    sprintf(p, "  to_char(ss_state_v.last_updated at time zone 'utc', 'yyyymmddThh24MISS') desc;");

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

      T::ProducerId producerId = toInt64(PQgetvalue(res, i, 0));
      std::string analysisTime = PQgetvalue(res, i, 2);

      T::ProducerInfo *producer = mSourceProducerList.getProducerInfoById(producerId);
      if (producer != nullptr)
      {
        if (producerId != prevProducerId || analysisTime != prevAnalysisTime)
        {
          auto *generation = new T::GenerationInfo();
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





/*! \brief Read source geometries. */

void readSourceGeometries(PGconn *conn,std::unordered_map<std::string,GeomRec>& geometries)
{
  FUNCTION_TRACE
  try
  {
    if (shutdownRequested)
      return;

    geometries.clear();

    char sql[1000];
    char *p = sql;
    p += sprintf(p, "SELECT DISTINCT\n");
    p += sprintf(p, "  producer_id,\n");
    p += sprintf(p, "  geometry_id,\n");
    p += sprintf(p, "  to_char(analysis_time at time zone 'utc', 'yyyymmddThh24MISS')\n");
    p += sprintf(p, "FROM\n");
    p += sprintf(p, "  ss_state\n");
    p += sprintf(p, "ORDER BY\n");
    p += sprintf(p, "  producer_id,to_char(analysis_time at time zone 'utc', 'yyyymmddThh24MISS'),");
    sprintf(p, "  geometry_id;");

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

      char st[1000];

      GeomRec rec;
      rec.producerId = toInt64(PQgetvalue(res, i, 0));
      rec.geometryId = toInt64(PQgetvalue(res, i, 1));
      rec.analysisTime = PQgetvalue(res, i, 2);
      rec.levelId = 0;

      T::ProducerInfo *producer = mSourceProducerList.getProducerInfoById(rec.producerId);
      if (producer != nullptr)
      {
        auto geom = mProducerGeometryList.find(toUpperString(producer->mName));
        if (geom == mProducerGeometryList.end() ||  geom->second.find(rec.geometryId) != geom->second.end())
        {
          rec.producerName = producer->mName;
          sprintf(st, "%s:%s:%d:%d",rec.producerName.c_str(), rec.analysisTime.c_str(),rec.geometryId,rec.levelId);
          geometries.insert(std::pair<std::string,GeomRec>(toUpperString(st),rec));
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





/*! \brief Read ready generations. */

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
    p += sprintf(p, "  geometry_id,\n");
    p += sprintf(p, "  0,\n");
    p += sprintf(p, "  to_char(ss_forecast_status.analysis_time at time zone 'utc', 'yyyymmddThh24MISS'),\n");
    p += sprintf(p, "  to_char(ss_forecast_status.status_time at time zone 'utc', 'yyyymmddThh24MISS')\n");
    p += sprintf(p, "FROM\n");
    p += sprintf(p, "  ss_forecast_status LEFT OUTER JOIN fmi_producer ON fmi_producer.id = ss_forecast_status.producer_id\n");
    p += sprintf(p, "WHERE\n");
    p += sprintf(p, " upper(status)='READY'\n");
    //p += sprintf(p, " ss_forecast_status.geometry_id IS NULL AND upper(status)='READY'\n");
    p += sprintf(p, "ORDER BY\n");
    p += sprintf(p, "  fmi_producer.id,to_char(ss_forecast_status.analysis_time at time zone 'utc', 'yyyymmddThh24MISS') desc,");
    sprintf(p, "  to_char(ss_forecast_status.status_time at time zone 'utc', 'yyyymmddThh24MISS') desc;");

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
    int prevGeometryId = -100;
    int prevLevelId = -100;

    for (int i = 0; i < rowCount; i++)
    {
      if (shutdownRequested)
        return;

      char st[1000];

      T::ProducerId producerId = toInt64(PQgetvalue(res, i, 0));
      int geometryId = toInt64(PQgetvalue(res, i, 2));
      int levelId = toInt64(PQgetvalue(res, i, 3));
      std::string analysisTime = PQgetvalue(res, i, 4);

      T::ProducerInfo *producer = mSourceProducerList.getProducerInfoById(producerId);
      if (producer != nullptr)
      {
        if (producerId != prevProducerId || analysisTime != prevAnalysisTime  || prevGeometryId != geometryId || prevLevelId != levelId)
        {
          std::string mtime = PQgetvalue(res, i, 5);
          time_t modTime = utcTimeToTimeT(mtime);

          sprintf(st, "%s:%s", producer->mName.c_str(), analysisTime.c_str());
          std::string gName = toUpperString(st);
          T::GenerationInfo *sourceGeneration = mSourceGenerationList.getGenerationInfoByName(gName);
          if (sourceGeneration != nullptr  &&  sourceGeneration->mModificationTime > modTime)
            modTime = sourceGeneration->mModificationTime;

          sprintf(st, "%s:%s:%d:%d", producer->mName.c_str(), analysisTime.c_str(),geometryId,levelId);
          std::string key = toUpperString(st);
          readyGenerations.insert(std::pair<std::string,time_t>(key,modTime));
          prevProducerId = producerId;
          prevGeometryId = geometryId;
          prevLevelId = levelId;
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





/*! \brief Read source producers. */

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
    sprintf(p, "  ss_state_v LEFT OUTER JOIN fmi_producer ON fmi_producer.id = ss_state_v.producer_id");

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
      if (mProducerList.empty() || mProducerList.find(searchStr) != mProducerList.end())
      {
        auto *producer = new T::ProducerInfo();
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




/*! \brief Remove the producer information from the target data storage. */

void deleteTargetProducer(T::ProducerInfo *targetProducer)
{
  PRINT_DATA(mDebugLogPtr, "  -- Remove producer : %s\n", targetProducer->mName.c_str());

  int result = mTargetInterface->deleteProducerInfoById(mSessionId, targetProducer->mProducerId);
  if (result == 0)
  {
    PRINT_EVENT_LINE(mProcessingLogPtr,"PRODUCER-DELETE;OK;%s;",targetProducer->mName.c_str());
  }
  else
  {
    PRINT_EVENT_LINE(mProcessingLogPtr,"PRODUCER-DELETE;FAIL;%s;",targetProducer->mName.c_str());
  }

  if (result != 0)
  {
    Fmi::Exception exception(BCP, "Cannot delete the producer information from the target data storage!");
    exception.addParameter("ProducerId", std::to_string(targetProducer->mProducerId));
    exception.addParameter("ProducerName", targetProducer->mName);
    exception.addParameter("Result", ContentServer::getResultString(result));
    exception.printError();
  }
}




/*! \brief Add the producer information into the target data storage. */

void addTargetProducer(T::ProducerInfo *sourceProducer)
{
  // The producer information is not available in the target data storage. So, we should add it.

  T::ProducerInfo producer(*sourceProducer);
  producer.mProducerId = 0;
  producer.mSourceId = mSourceId;

  PRINT_DATA(mDebugLogPtr, "  -- Add producer : %s\n", producer.mName.c_str());

  int result = mTargetInterface->addProducerInfo(mSessionId, producer);
  if (result == 0)
  {
    PRINT_EVENT_LINE(mProcessingLogPtr,"PRODUCER-ADD;OK;%s;",sourceProducer->mName.c_str());
  }
  else
  {
    PRINT_EVENT_LINE(mProcessingLogPtr,"PRODUCER-ADD;FAIL;%s;",sourceProducer->mName.c_str());
  }

  if (result != 0)
  {
    Fmi::Exception exception(BCP, "Cannot add the producer information into the target data storage!");
    exception.addParameter("ProducerId", std::to_string(sourceProducer->mProducerId));
    exception.addParameter("ProducerName", sourceProducer->mName);
    exception.addParameter("Result", ContentServer::getResultString(result));
    throw exception;
  }
}




/*! \brief Update producers. */

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
        bool producerFound = false;
        if (mProducerFullList.empty() || mProducerList.find(searchStr) != mProducerList.end())
          producerFound = true;

        T::ProducerInfo *sourceProducer = nullptr;
        if (producerFound)
          sourceProducer = mSourceProducerList.getProducerInfoByName(targetProducer->mName);

        if (sourceProducer == nullptr)
        {
          if (mProducerFullList.find(searchStr) == mProducerFullList.end())
          {
            // The producer information is not wanted or it is not available in the source data
            // storage. So, we should remove it also from the target data storage.

            deleteTargetProducer(targetProducer);
          }
        }
        else
        {
          mProducerIdList.insert(targetProducer->mProducerId);
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
          addTargetProducer(sourceProducer);
        }
      }
    }
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}





/*! \brief Update loop start. */

void updateLoopStart()
{
  FUNCTION_TRACE
  try
  {
    T::EventInfo event;
    event.mType = SmartMet::ContentServer::EventType::UPDATE_LOOP_START;
    event.mId1 = mSourceId;

    int result = mTargetInterface->addEventInfo(mSessionId, event);
    if (result == 0)
    {
      PRINT_EVENT_LINE(mProcessingLogPtr,"UPDATE-LOOP-START;OK;%u;",mSourceId);
    }
    else
    {
      PRINT_EVENT_LINE(mProcessingLogPtr,"UPDATE-LOOP-START;FAIL;%u;%d",mSourceId,result);
    }

    if (result != 0)
    {
      Fmi::Exception exception(BCP, "Cannot add the event information into the target data storage!");
      exception.addParameter("Result", ContentServer::getResultString(result));
      throw exception;
    }
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}





/*! \brief Update loop end. */

void updateLoopEnd()
{
  FUNCTION_TRACE
  try
  {
    T::EventInfo event;
    event.mType = ContentServer::EventType::UPDATE_LOOP_END;
    event.mId1 = mSourceId;

    int result = mTargetInterface->addEventInfo(mSessionId, event);
    if (result == 0)
    {
      PRINT_EVENT_LINE(mProcessingLogPtr,"UPDATE-LOOP-END;OK;%u;",mSourceId);
    }
    else
    {
      PRINT_EVENT_LINE(mProcessingLogPtr,"UPDATE-LOOP-END;FAIL;%u;%d",mSourceId,result);
    }

    if (result != 0)
    {
      Fmi::Exception exception(BCP, "Cannot add the event information into the target data storage!");
      exception.addParameter("Result", ContentServer::getResultString(result));
      throw exception;
    }
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}





/*! \brief Mark the target generation to be removed if it is not available in the source data storage or if it is modified. */

void checkTargetGeneration(T::GenerationInfo *targetGeneration,std::set <T::GenerationId>& generationIdList)
{
  T::GenerationInfo *sourceGeneration = mSourceGenerationList.getGenerationInfoByName(targetGeneration->mName);
  if (sourceGeneration == nullptr)
  {
    // The generation information is not available in the source data storage.
    // So, we should remove the generation from the target data storage.

    PRINT_EVENT_LINE(mProcessingLogPtr,"GENERATION-DELETE;OK;%s;",targetGeneration->mName.c_str());
    PRINT_DATA(mDebugLogPtr, "  -- Remove generation : %s\n", targetGeneration->mName.c_str());

    generationIdList.insert(targetGeneration->mGenerationId);
  }
  else if (sourceGeneration->mModificationTime > targetGeneration->mModificationTime)
  {
    std::string key = toUpperString(targetGeneration->mName + ":0:0");
    auto mTime = mReadyGenerations.find(key);
    if (mTime != mReadyGenerations.end())
    {
      // The target generation is marked to be "ready" but it is older than the source
      // generation. So, we should remove the generation from the target data storage and reload
      // its information.

      PRINT_EVENT_LINE(mProcessingLogPtr,"GENERATION-DELETE;OK;%s;",targetGeneration->mName.c_str());
      PRINT_DATA(mDebugLogPtr, "  -- Remove modified generation : %s\n", targetGeneration->mName.c_str());

      generationIdList.insert(targetGeneration->mGenerationId);
    }
  }
}




/*! \brief Add the generation information into the target data storage. */

void addTargetGeneration(T::GenerationInfo *sourceGeneration)
{
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
    if (result == 0)
    {
      PRINT_EVENT_LINE(mProcessingLogPtr,"GENERATION-ADD;OK;%s;",generationInfo.mName.c_str());
    }
    else
    {
      PRINT_EVENT_LINE(mProcessingLogPtr,"GENERATION-ADD;FAIL;%s;",generationInfo.mName.c_str());
    }

    if (result != 0)
    {

      Fmi::Exception exception(BCP, "Cannot add the generation information into the target data storage!");
      exception.addParameter("GenerationName", generationInfo.mName);
      exception.addParameter("Result", ContentServer::getResultString(result));
      throw exception;
    }
  }
}




/*! \brief Update generations. */

void updateGenerations()
{
  FUNCTION_TRACE
  try
  {
    std::set <T::GenerationId> generationIdList;
    uint len = mTargetGenerationList.getLength();
    for (uint t = 0; t < len; t++)
    {
      T::GenerationInfo *targetGeneration = mTargetGenerationList.getGenerationInfoByIndex(t);
      if (targetGeneration->mSourceId == mSourceId  &&  mProducerIdList.find(targetGeneration->mProducerId) != mProducerIdList.end())
      {
        checkTargetGeneration(targetGeneration,generationIdList);
      }
    }

    if (!generationIdList.empty())
    {
      int result = mTargetInterface->deleteGenerationInfoListByIdList(mSessionId, generationIdList);
      if (result != 0)
      {
        Fmi::Exception exception(BCP, "Cannot delete the generation information from the target data storage!");
        exception.addParameter("Result", ContentServer::getResultString(result));
        exception.printError();
      }

      for (unsigned long it : generationIdList)
        mTargetGenerationList.deleteGenerationInfoById(it);
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

          addTargetGeneration(sourceGeneration);
        }
      }
    }
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}




/*! \brief Remove the geometry information from the target data storage and set the generation status to running. */

void deleteTargetGeometry(T::GeometryInfo *targetGeometry,T::GenerationInfo *targetGeneration,const std::string& key)
{
  PRINT_EVENT_LINE(mProcessingLogPtr,"GEOMETRY-DELETE;OK;%lu;%d;%d",targetGeometry->mGenerationId,targetGeometry->mGeometryId,targetGeometry->mLevelId);
  PRINT_DATA(mDebugLogPtr, "  -- Remove geometry : %s\n",key.c_str());

  int result = mTargetInterface->deleteGeometryInfoById(mSessionId,targetGeometry->mGenerationId,targetGeometry->mGeometryId,targetGeometry->mLevelId);
  if (result != 0)
  {
    Fmi::Exception exception(BCP, "Cannot delete the geometry information from the target data storage!");
    exception.addParameter("Result", ContentServer::getResultString(result));
    exception.printError();
  }

  PRINT_DATA(mDebugLogPtr, "  -- Update generation status to 'running': %s\n", targetGeneration->mName.c_str());
  targetGeneration->mStatus = T::GenerationInfo::Status::Running;
  result = mTargetInterface->setGenerationInfo(mSessionId,*targetGeneration);
  if (result == 0)
  {
    PRINT_EVENT_LINE(mProcessingLogPtr,"+ GENERATION-STATUS-UPDATE-TO-RUNNING;OK;%s;",targetGeneration->mName.c_str());
  }
  else
  {
    PRINT_EVENT_LINE(mProcessingLogPtr,"GENERATION-STATUS-UPDATE-TO-RUNNING;FAIL;%s;",targetGeneration->mName.c_str());
  }
}




/*! \brief Add the geometry information into the target data storage and set the generation status to running. */

void addTargetGeometry(const std::string& gname,T::GenerationInfo *targetGeneration,const GeomRec& geom)
{
  // The geometry information is not available in the target data storage. So, we should add it.

  T::GeometryInfo geomInfo;
  geomInfo.mProducerId = targetGeneration->mProducerId;
  geomInfo.mGenerationId = targetGeneration->mGenerationId;
  geomInfo.mGeometryId = geom.geometryId;
  geomInfo.mLevelId = geom.levelId;
  geomInfo.mSourceId = mSourceId;
  geomInfo.mStatus = T::GenerationInfo::Status::Running;
  geomInfo.mModificationTime = targetGeneration->mModificationTime;
  geomInfo.mDeletionTime = targetGeneration->mDeletionTime;

  PRINT_DATA(mDebugLogPtr, "  -- Add geometry : %s:%d\n", gname.c_str(),geomInfo.mGeometryId);

  int result = mTargetInterface->addGeometryInfo(mSessionId,geomInfo);
  if (result == 0)
  {
    PRINT_EVENT_LINE(mProcessingLogPtr,"GEOMETRY-ADD;OK;%lu;%d;%d;",geomInfo.mGenerationId,geomInfo.mGeometryId,geomInfo.mLevelId);
  }
  else
  {
    PRINT_EVENT_LINE(mProcessingLogPtr,"GENERATION-ADD;FAIL;%lu;%d;%d",geomInfo.mGenerationId,geomInfo.mGeometryId,geomInfo.mLevelId);
  }
  if (result != 0)
  {
    Fmi::Exception exception(BCP, "Cannot add the geometry information into the target data storage!");
    exception.addParameter("Generation", gname);
    exception.addParameter("GeometryId", std::to_string(geomInfo.mGeometryId));
    exception.addParameter("LevelId", std::to_string(geomInfo.mLevelId));
    exception.addParameter("Result", ContentServer::getResultString(result));
    throw exception;
  }

  PRINT_DATA(mDebugLogPtr, "  -- Update generation status to 'running': %s\n", targetGeneration->mName.c_str());
  targetGeneration->mStatus = T::GenerationInfo::Status::Running;
  result = mTargetInterface->setGenerationInfo(mSessionId,*targetGeneration);
  if (result == 0)
  {
    PRINT_EVENT_LINE(mProcessingLogPtr,"* GENERATION-STATUS-UPDATE-TO-RUNNING;OK;%s;",targetGeneration->mName.c_str());
  }
  else
  {
    PRINT_EVENT_LINE(mProcessingLogPtr,"GENERATION-STATUS-UPDATE-TO-RUNNING;FAIL;%s;",targetGeneration->mName.c_str());
  }
}




/*! \brief Update geometries. */

void updateGeometries()
{
  FUNCTION_TRACE
  try
  {
    uint len = mTargetGeometryList.getLength();
    for (uint t = 0; t < len; t++)
    {
      T::GeometryInfo *targetGeometry = mTargetGeometryList.getGeometryInfoByIndex(t);
      if (targetGeometry->mSourceId == mSourceId  &&  mProducerIdList.find(targetGeometry->mProducerId) != mProducerIdList.end())
      {
        T::GenerationInfo *targetGeneration = mTargetGenerationList.getGenerationInfoById(targetGeometry->mGenerationId);
        if (targetGeneration != nullptr)
        {
          char st[200];
          sprintf(st, "%s:%d:%d",targetGeneration->mName.c_str(),targetGeometry->mGeometryId,targetGeometry->mLevelId);

          std::string key = toUpperString(st);
          if (mSourceGeometryList.find(key) == mSourceGeometryList.end())
          {
            deleteTargetGeometry(targetGeometry,targetGeneration,key);
          }
        }
      }
    }

    for (auto &g : mSourceGeometryList)
    {
      std::string gname = toUpperString(g.second.producerName + ":" + g.second.analysisTime);
      T::GenerationInfo *targetGeneration = mTargetGenerationList.getGenerationInfoByName(gname);
      if (targetGeneration != nullptr)
      {
        T::GeometryInfo *targetGeom = mTargetGeometryList.getGeometryInfoById(targetGeneration->mGenerationId,g.second.geometryId,g.second.levelId);

        if (targetGeom == nullptr)
        {
          addTargetGeometry(gname,targetGeneration,g.second);
        }
      }
    }
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}




/*! \brief Set the geometry status to ready in the target data storage. */

void setGeometryReady(T::GenerationInfo *targetGeneration,T::GeometryInfo *geomInfo)
{
  geomInfo->mStatus = T::GeometryInfo::Status::Ready;
  int result = mTargetInterface->setGeometryInfo(mSessionId,*geomInfo);

  if (result == 0)
  {
    PRINT_EVENT_LINE(mProcessingLogPtr,"GEOMETRY-STATUS-UPDATE-TO-READY;OK;%s:%d:%d;",targetGeneration->mName.c_str(),geomInfo->mGeometryId,geomInfo->mLevelId);
  }
  else
  {
    PRINT_EVENT_LINE(mProcessingLogPtr,"GEOMETRY-STATUS-UPDATE-TO-READY;FAIL;%s:%d:%d;",targetGeneration->mName.c_str(),geomInfo->mGeometryId,geomInfo->mLevelId);
  }
}




/*! \brief Set all the geometries of a ready generation to ready. */

void setReadyGenerationGeometries(T::GenerationInfo *targetGeneration,std::unordered_map<std::string,time_t>::iterator mTime)
{
  uint gLen = mTargetGeometryList.getLength();
  for (uint g=0; g<gLen; g++)
  {
    auto *geomInfo = (T::GeometryInfo*)mTargetGeometryList.getGeometryInfoByIndex(g);
    if (geomInfo->mGenerationId == targetGeneration->mGenerationId &&  geomInfo->mStatus != T::GeometryInfo::Status::Ready)
    {
      std::string gkey = toUpperString(targetGeneration->mName + ":" + std::to_string(geomInfo->mGeometryId) + ":0");
      auto gTime = mReadyGenerations.find(gkey);

      if (gTime != mReadyGenerations.end())
      {
        geomInfo->mModificationTime = gTime->second;
      }
      else
      {
        if (mTime != mReadyGenerations.end())
          geomInfo->mModificationTime = mTime->second;
      }

      setGeometryReady(targetGeneration,geomInfo);
    }
  }
}




/*! \brief Set the generation status to ready in the target data storage. */

void setTargetGenerationReady(T::GenerationInfo *targetGeneration,std::unordered_map<std::string,time_t>::iterator mTime)
{
  PRINT_DATA(mDebugLogPtr, "  -- Update generation status : %s\n", targetGeneration->mName.c_str());
  targetGeneration->mStatus = T::GenerationInfo::Status::Ready;
  if (mTime != mReadyGenerations.end())
    targetGeneration->mModificationTime = mTime->second;

  int result = mTargetInterface->setGenerationInfo(mSessionId,*targetGeneration);

  if (result == 0)
  {
    PRINT_EVENT_LINE(mProcessingLogPtr,"GENERATION-STATUS-UPDATE-TO-READY;OK;%s;",targetGeneration->mName.c_str());
  }
  else
  {
    PRINT_EVENT_LINE(mProcessingLogPtr,"GENERATION-STATUS-UPDATE-TO-READY;FAIL;%s;",targetGeneration->mName.c_str());
  }

  if (result != 0)
  {
    Fmi::Exception exception(BCP, "Cannot update the generation status into the target data storage!");
    exception.addParameter("GenerationName", targetGeneration->mName);
    exception.addParameter("Result", ContentServer::getResultString(result));
    throw exception;
  }
}




/*! \brief Set the ready geometries of a generation which is not ready to ready. */

void setReadyGeometries(T::GenerationInfo *targetGeneration)
{
  uint gLen = mTargetGeometryList.getLength();
  for (uint g=0; g<gLen; g++)
  {
    auto *geomInfo = (T::GeometryInfo*)mTargetGeometryList.getGeometryInfoByIndex(g);
    if (geomInfo->mGenerationId == targetGeneration->mGenerationId  &&  geomInfo->mStatus != T::GeometryInfo::Status::Ready)
    {
      std::string key = toUpperString(targetGeneration->mName + ":" + std::to_string(geomInfo->mGeometryId) + ":0");
      auto mTime = mReadyGenerations.find(key);
      if (mTime == mReadyGenerations.end())
      {
        key = toUpperString(targetGeneration->mName + ":" + std::to_string(geomInfo->mGeometryId) + ":" + std::to_string(geomInfo->mLevelId));
        mTime = mReadyGenerations.find(key);
      }

      if (mTime != mReadyGenerations.end())
      {
        geomInfo->mModificationTime = mTime->second;
        setGeometryReady(targetGeneration,geomInfo);
      }
    }
  }
}




/*! \brief Update generation status. */

void updateGenerationStatus(T::ProducerInfo& targetProducer)
{
  FUNCTION_TRACE
  try
  {

    std::string pname = toUpperString(targetProducer.mName);
    bool ignore = false;
    if (mProducerList.empty()  ||  mGenerationStatusCheckIgnore.find(pname) != mGenerationStatusCheckIgnore.end())
    {
      PRINT_DATA(mDebugLogPtr, "  -- Ignore generation status update : %s\n", targetProducer.mName.c_str());
      ignore = true;
    }

    if (!ignore)
    {
      auto rec = mProducerDependensies.find(pname);
      if (rec != mProducerDependensies.end())
      {
        PRINT_DATA(mDebugLogPtr, "  -- Postpone generation status update : %s\n", targetProducer.mName.c_str());
        return; // The producer update has dependensies. We update these kinds of producers later.
      }
    }

    uint len = mTargetGenerationList.getLength();
    for (uint t = 0; t < len; t++)
    {
      T::GenerationInfo *targetGeneration = mTargetGenerationList.getGenerationInfoByIndex(t);
      if (targetGeneration->mSourceId == mSourceId  &&  targetGeneration->mProducerId == targetProducer.mProducerId  &&  targetGeneration->mStatus != T::GenerationInfo::Status::Ready)
      {
        std::string key = toUpperString(targetGeneration->mName + ":0:0");
        auto mTime = mReadyGenerations.find(key);
        if (ignore || mTime != mReadyGenerations.end())
        {
          setReadyGenerationGeometries(targetGeneration,mTime);
          setTargetGenerationReady(targetGeneration,mTime);
        }
        else
        {
          setReadyGeometries(targetGeneration);
        }
      }
    }
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}




/*! \brief Check if the generations of the producers the producer depends on are ready. */

bool dependenciesReady(const std::string& pname,const std::string& analysisTime,const std::string& suffix,bool geometry)
{
  bool ready = true;
  auto rec = mProducerDependensies.find(pname);
  if (rec != mProducerDependensies.end())
  {
    for (auto &it : rec->second)
    {
      std::string n = toUpperString(it + ":" + analysisTime + suffix);
      auto gen = mReadyGenerations.find(n);
      if (gen == mReadyGenerations.end())
      {
        if (geometry)
        {
          PRINT_DATA(mDebugLogPtr, "  -- Geometry dependency not ready: %s\n", n.c_str());
        }
        else
        {
          PRINT_DATA(mDebugLogPtr, "  -- Generation dependency not ready : %s\n", n.c_str());
        }
        ready = false;
      }
    }
  }
  return ready;
}




/*! \brief Check if the generation and the generations it depends on are ready. */

bool isGenerationReady(T::GenerationInfo *targetGeneration,const std::string& pname,const std::vector<std::string>& parts)
{
  std::string key = toUpperString(targetGeneration->mName + ":0:0");
  auto gen = mReadyGenerations.find(key);
  if (gen == mReadyGenerations.end())
  {
    PRINT_DATA(mDebugLogPtr, "  -- Generation not ready : %s\n", key.c_str());
    return false;
  }

  targetGeneration->mModificationTime = gen->second;
  return dependenciesReady(pname,parts[1],":0:0",false);
}




/*! \brief Set the geometries of a ready generation to ready and the generation status to ready. */

int setGenerationReady(T::GenerationInfo *targetGeneration)
{
  uint gLen = mTargetGeometryList.getLength();
  for (uint g=0; g<gLen; g++)
  {
    auto *geomInfo = (T::GeometryInfo*)mTargetGeometryList.getGeometryInfoByIndex(g);
    if (geomInfo->mGenerationId == targetGeneration->mGenerationId &&  geomInfo->mStatus != T::GeometryInfo::Status::Ready)
    {
      std::string key = toUpperString(targetGeneration->mName + ":" + std::to_string(geomInfo->mGeometryId) + ":0");
      auto gTime = mReadyGenerations.find(key);
      if (gTime != mReadyGenerations.end())
        geomInfo->mModificationTime = gTime->second;
      else
        geomInfo->mModificationTime = targetGeneration->mModificationTime;

      setGeometryReady(targetGeneration,geomInfo);
    }
  }

  PRINT_DATA(mDebugLogPtr, "  -- Update generation status to 'ready': %s\n", targetGeneration->mName.c_str());
  targetGeneration->mStatus = T::GenerationInfo::Status::Ready;
  targetGeneration->mModificationTime++;
  int result = mTargetInterface->setGenerationInfo(mSessionId,*targetGeneration);

  if (result == 0)
  {
    PRINT_EVENT_LINE(mProcessingLogPtr,"GENERATION-STATUS-UPDATE-TO-READY;OK;%s;",targetGeneration->mName.c_str());
  }
  else
  {
    PRINT_EVENT_LINE(mProcessingLogPtr,"GENERATION-STATUS-UPDATE-TO-READY;FAIL;%s;",targetGeneration->mName.c_str());
  }
  return result;
}




/*! \brief Check if the geometry and the geometries it depends on are ready. */

bool isGeometryReady(T::GenerationInfo *targetGeneration,T::GeometryInfo *geomInfo,const std::string& pname,const std::vector<std::string>& parts)
{
  std::string key = toUpperString(targetGeneration->mName + ":" + std::to_string(geomInfo->mGeometryId) + ":" + std::to_string(geomInfo->mLevelId));
  auto gTime = mReadyGenerations.find(key);
  if (gTime == mReadyGenerations.end())
  {
    PRINT_DATA(mDebugLogPtr, "  -- Geometry not ready: %s\n", key.c_str());
    return false;
  }

  geomInfo->mModificationTime = gTime->second;
  return dependenciesReady(pname,parts[1],":" + std::to_string(geomInfo->mGeometryId) + ":" + std::to_string(geomInfo->mLevelId),true);
}




/*! \brief Check if the generation status of the producer should be checked. */

bool isProducerCheckEnabled(T::GenerationInfo *targetGeneration,const std::string& pname)
{
  if (mProducerList.empty() || mProducerList.find(pname) != mProducerList.end())
  {
    PRINT_DATA(mDebugLogPtr, "  -- Checking generation status : %s (status=%u)\n", targetGeneration->mName.c_str(),targetGeneration->mStatus);
    return true;
  }
  return false;
}




/*! \brief Set the ready geometries of a generation which is no longer ready to ready and the generation status to running. */

int setGenerationRunning(T::GenerationInfo *targetGeneration,const std::string& pname,const std::vector<std::string>& parts)
{
  uint gLen = mTargetGeometryList.getLength();
  for (uint g=0; g<gLen; g++)
  {
    auto *geomInfo = (T::GeometryInfo*)mTargetGeometryList.getGeometryInfoByIndex(g);
    if (geomInfo->mGenerationId == targetGeneration->mGenerationId &&  geomInfo->mStatus != T::GeometryInfo::Status::Ready)
    {
      if (isGeometryReady(targetGeneration,geomInfo,pname,parts))
        setGeometryReady(targetGeneration,geomInfo);
    }
  }

  PRINT_DATA(mDebugLogPtr, "  -- Update generation status to 'running': %s\n", targetGeneration->mName.c_str());
  targetGeneration->mStatus = T::GenerationInfo::Status::Running;
  targetGeneration->mModificationTime++;
  int result = mTargetInterface->setGenerationInfo(mSessionId,*targetGeneration);
  if (result == 0)
  {
    PRINT_EVENT_LINE(mProcessingLogPtr,"# GENERATION-STATUS-UPDATE-TO-RUNNING;OK;%s;",targetGeneration->mName.c_str());
  }
  else
  {
    PRINT_EVENT_LINE(mProcessingLogPtr,"GENERATION-STATUS-UPDATE-TO-RUNNING;FAIL;%s;",targetGeneration->mName.c_str());
  }
  return result;
}




/*! \brief Update generation status. */

void updateGenerationStatus()
{
  FUNCTION_TRACE
  try
  {
    uint len = mTargetGenerationList.getLength();
    for (uint t = 0; t < len; t++)
    {
      T::GenerationInfo *targetGeneration = mTargetGenerationList.getGenerationInfoByIndex(t);
      //PRINT_EVENT_LINE(mProcessingLogPtr,"TARGET GENERATION %s:%d;",targetGeneration->mName.c_str(),targetGeneration->mGenerationId);

      if (targetGeneration->mSourceId == mSourceId)
      {
        std::vector<std::string> parts;
        splitString(targetGeneration->mName,':',parts);
        std::string pname = toUpperString(parts[0]);
        bool ready = true;
        bool producerCheckEnabled = isProducerCheckEnabled(targetGeneration,pname);

        if (producerCheckEnabled  &&  !mProducerList.empty()  &&  mGenerationStatusCheckIgnore.find(pname) == mGenerationStatusCheckIgnore.end())
          ready = isGenerationReady(targetGeneration,pname,parts);

        int result = 0;
        if (producerCheckEnabled &&  ready  &&  targetGeneration->mStatus != T::GenerationInfo::Status::Ready)
          result = setGenerationReady(targetGeneration);
        else if (producerCheckEnabled &&  !ready  &&  targetGeneration->mStatus == T::GenerationInfo::Status::Ready)
          result = setGenerationRunning(targetGeneration,pname,parts);

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







/*! \brief Delete target files. */

void deleteTargetFiles(T::ProducerId producerId,T::FileInfoList& targetFileList)
{
  FUNCTION_TRACE
  try
  {
    PRINT_DATA(mDebugLogPtr, "* Deleting old files from the target data storage\n");

    std::set <T::FileId> deleteList;
    uint len = targetFileList.getLength();
    for (uint t = 0; t < len; t++)
    {
      T::FileInfo *fileInfo = targetFileList.getFileInfoByIndex(t);
      if (fileInfo != nullptr && fileInfo->mProducerId == producerId)
      {
        if (fileInfo->mSourceId == mSourceId && mIgnoreGeneration.find(fileInfo->mGenerationId) == mIgnoreGeneration.end()  &&  mSourceFilenames.find(getFileId(fileInfo->mName,false)) == mSourceFilenames.end())
        {
          deleteList.insert(fileInfo->mFileId);
        }
      }
    }
    PRINT_DATA(mDebugLogPtr, "  -- Delete files : %lu\n", deleteList.size());
    if (!deleteList.empty())
      mTargetInterface->deleteFileInfoListByFileIdList(mSessionId, deleteList);
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}




/*! \brief Delete old file records. */

void deleteOldFileRecords(uint loadCounter)
{
  FUNCTION_TRACE
  try
  {
    std::set<std::string> entriesToRemove;

    for (auto &it : mFileRecMap)
    {
      if (it.second.loadCounter != loadCounter)
      {
        entriesToRemove.insert(it.first);
      }
    }

    for (const auto &it : entriesToRemove)
    {
      auto pos = mFileRecMap.find(it);
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




/*! \brief Count file records. */

uint countFileRecords()
{
  FUNCTION_TRACE
  try
  {
    uint cnt = 0;
    for (auto &it : mFileRecMap)
    {
      cnt = cnt + it.second.files.size();
    }

    return cnt;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}




/*! \brief Add the information of a new file which the target data storage does not have. */

void addNewFile(T::GenerationInfo *generation,const FileRec& it,const std::string& filename,ForecastRec& forecast,std::vector<T::FileAndContent>& fileAndContentList)
{
  PRINT_DATA(mDebugLogPtr, "**** New file : %s\n", filename.c_str());


  T::FileAndContent fc;
  fc.mFileInfo.mProducerId = generation->mProducerId;
  fc.mFileInfo.mGenerationId = generation->mGenerationId;
  fc.mFileInfo.mFileId = 0;
  fc.mFileInfo.mServerType = it.serverType;
  fc.mFileInfo.mServer = it.server;

  if (strncasecmp(it.server.c_str(),"http://",7) == 0)
  {
    fc.mFileInfo.mProtocol = 1;
    fc.mFileInfo.mServer = it.server.substr(7);
  }

  if (strncasecmp(it.server.c_str(),"https://",8) == 0)
  {
    fc.mFileInfo.mProtocol = 2;
    fc.mFileInfo.mServer = it.server.substr(8);
  }

  if (it.serverType > 1  &&  fc.mFileInfo.mProtocol == 0)
    fc.mFileInfo.mProtocol = 2;

  fc.mFileInfo.mFileType = it.format;
  fc.mFileInfo.mName = filename;

  if (it.cachingRequested)
    fc.mFileInfo.mFlags = T::FileInfo::Flags::LocalCacheRecommended;
  else
    fc.mFileInfo.mFlags = 0;

  fc.mFileInfo.mSourceId = mSourceId;
  fc.mFileInfo.mDeletionTime = forecast.deletionTime;

  fileAndContentList.push_back(fc);
}




/*! \brief Update the modification time of the generation. */

void updateGenerationModificationTime(T::GenerationInfo *generation,ForecastRec& forecast)
{
  if (generation->mModificationTime <= forecast.lastUpdated)
  {
    generation->mModificationTime = forecast.lastUpdated;
    time_t diff = (time(nullptr) - forecast.lastUpdated);
    if (diff > 300)
      generation->mModificationTime = forecast.lastUpdated + 1;
  }
}




/*! \brief Read source files by forecast time. */

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

    if (generation->mStatus != T::GenerationInfo::Status::Running && generation->mModificationTime > forecast.lastUpdated)
    {
      mIgnoreGeneration.insert(generation->mGenerationId);
      return;
    }

    //printf("%s:%s:%s\n",forecast.generationName.c_str(),utcTimeFromTimeT(generation->mModificationTime).c_str(),utcTimeFromTimeT(forecast.lastUpdated).c_str());

    generation->mStatus = T::GenerationInfo::Status::Running;

    std::vector<FileRec> recList = readSourceFilesAndContent(conn, forecast.tableName.c_str(), forecast.producerId, producerName, forecast.geometryId, forecast.forecastTypeId,
        forecast.forecastTypeValue, forecast.analysisTime.c_str(), forecast.forecastPeriod.c_str(), forecast.lastUpdated, loadCounter);

    mContentReadCount += recList.size();
    PRINT_DATA(mDebugLogPtr, "  -- Read files by forecast time %s:%u:%d:%u:%d : %lu (contentCount = %u)\n", forecast.forecastTime.c_str(), forecast.producerId, forecast.geometryId,
        forecast.forecastTypeId, forecast.forecastTypeValue, recList.size(), mContentReadCount);

    if (recList.empty())
      return;

    if (fileReadCount == 0  &&  targetFileList.getLength() == 0)
    {
      readTargetFileList(targetProducerId,targetFileList);
      targetFileList.sort(T::FileInfo::ComparisonMethod::fileName);
      fileReadCount++;
    }

    for (auto &it : recList)
    {
      if (shutdownRequested)
        return;

      fileRecList.push_back(it);
      T::FileInfo fileInfo;
      std::string filename = getFilename(it.fileId);
      if (mSourceFilenames.find(it.fileId) == mSourceFilenames.end() && targetFileList.getFileInfoByName(filename) == nullptr)
      {
        // The target data storage does not have the current file information. It should be added.

        addNewFile(generation,it,filename,forecast,fileAndContentList);
      }

      if (mSourceFilenames.find(it.fileId) == mSourceFilenames.end())
        mSourceFilenames.insert(it.fileId);
    }
    updateGenerationModificationTime(generation,forecast);
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}




/*! \brief Create the content information of a file record. */

T::ContentInfo* newContentInfo(T::FileInfo *fileInfo,const FileRec& it)
{
  auto *contentInfo = new T::ContentInfo();

  contentInfo->mFileId = fileInfo->mFileId;
  contentInfo->mFileType = fileInfo->mFileType;
  contentInfo->mMessageIndex = it.messageIndex;
  contentInfo->mFilePosition = it.filePosition;
  contentInfo->mMessageSize = it.messageSize;
  contentInfo->mProducerId = fileInfo->mProducerId;
  contentInfo->mGenerationId = fileInfo->mGenerationId;
  contentInfo->setForecastTime(utcTimeFromTimeT(it.forecastTime));
  contentInfo->mFmiParameterId = it.paramId;
  contentInfo->mFmiParameterLevelId = it.levelId;
  contentInfo->mParameterLevel = it.levelValue;
  contentInfo->mForecastType = it.forecastType;
  contentInfo->mForecastNumber = it.forecastNumber;
  contentInfo->mAggregationId = it.aggregation_id;
  contentInfo->mAggregationPeriod = it.aggregation_period;
  contentInfo->mProcessingTypeId = it.processing_type_id;
  contentInfo->mProcessingTypeValue1 = it.processing_type_value;
  contentInfo->mProcessingTypeValue2 = it.processing_type_value2;
  contentInfo->mFlags = 0;
  contentInfo->mSourceId = mSourceId;
  contentInfo->mGeometryId = it.geometryId;
  contentInfo->mModificationTime = it.lastUpdated;
  contentInfo->mDeletionTime = fileInfo->mDeletionTime;

  Identification::FmiParameterDef fmiDef;
  if (Identification::gridDef.getFmiParameterDefById(it.paramId, fmiDef))
  {
    contentInfo->setFmiParameterName(fmiDef.mParameterName);
  }

  return contentInfo;
}




/*! \brief Add the content of a file record to the content list if it does not exist in the target data storage. */

void addTargetContent(T::FileInfoList& targetFileList,FileRec& it,T::ContentInfoList& contentList)
{
  std::string filename = getFilename(it.fileId);
  T::FileInfo *fileInfo = targetFileList.getFileInfoByName(filename);
  if (fileInfo != nullptr)
  {
    UInt64 id = fileInfo->mFileId;
    id = (id << 32) + it.messageIndex;
    auto pos = mTargetContentList.find(id);
    if (pos == mTargetContentList.end())
    {
      // The content does not exists. It should be added.

      auto *contentInfo = newContentInfo(fileInfo,it);

      contentList.addContentInfo(contentInfo);

      if (contentList.getLength() >= mMaxMessageSize)
      {
        PRINT_EVENT_LINE(mProcessingLogPtr,"CONTENT-ADD;OK;%u (%u/%u);",contentList.getLength(), mContentAddCount, mContentReadCount);
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
}




/*! \brief Save target content. */

void saveTargetContent(T::ProducerId producerId,std::vector<FileRec>& fileRecList)
{
  try
  {
    if (fileRecList.empty())
      return;

    T::FileInfoList targetFileList;
    readTargetFileList(producerId,targetFileList);
    targetFileList.sort(T::FileInfo::ComparisonMethod::fileName);

    //std::set<UInt64> targetContentList;
    //if (fileRecList.size() > 0)
    //  readTargetContentList(producerId,targetContentList);

    T::ContentInfoList contentList;

    for (auto &it : fileRecList)
    {
      addTargetContent(targetFileList,it,contentList);
      mContentAddCount++;
    }

    PRINT_EVENT_LINE(mProcessingLogPtr,"CONTENT-ADD;OK;%u (%u/%u);",contentList.getLength(), mContentAddCount, mContentReadCount);
    PRINT_DATA(mDebugLogPtr, "  -- Add content information : %u (%u/%u)\n", contentList.getLength(), mContentAddCount, mContentReadCount);
    if (contentList.getLength() > 0)
      mTargetInterface->addContentList(mSessionId, contentList);

    //std::set<UInt64> tcList;
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







/*! \brief Add the file and content information into the target data storage. */

void addFileInfoList(std::vector<T::FileAndContent>& fileAndContentList)
{
  PRINT_DATA(mDebugLogPtr, "  -- Add file information : %lu\n", fileAndContentList.size());
  int result = mTargetInterface->addFileInfoListWithContent(mSessionId, 0, fileAndContentList);
  if (result != 0)
  {
    fprintf(stdout, "ERROR (%d) : %s\n", result, ContentServer::getResultString(result).c_str());
  }
}




/*! \brief Add the file and content information into the target data storage in parts of the maximum message size. */

void addFileAndContentList(std::vector<T::FileAndContent>& fileAndContentList)
{
  if (fileAndContentList.size() <= mMaxMessageSize)
  {
    addFileInfoList(fileAndContentList);
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
        addFileInfoList(fcList);
        fcList.clear();
      }
    }

    if (!fcList.empty())
    {
      addFileInfoList(fcList);
      fcList.clear();
    }
  }
}




/*! \brief Update the files of the producer into the target data storage. Returns false if shutdown is requested. */

bool updateTargetProducerFiles(PGconn *conn,T::ProducerInfo *sourceProducer,T::ProducerInfo *targetProducer)
{
  PRINT_EVENT_LINE(mProcessingLogPtr,"PRODUCER-UPDATE;START;%s;",targetProducer->mName.c_str());

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
    return false;

  addFileAndContentList(fileAndContentList);

  fileAndContentList.clear();

  PRINT_DATA(mDebugLogPtr,"* Updating content information into the target data storage\n");
  saveTargetContent(targetProducer->mProducerId,fileRecList);
  //printf("-- save target content %ld\n",fileRecList.size());

  PRINT_DATA(mDebugLogPtr, "* Updating generation status information into the target data storage\n");
  updateGenerationStatus(*targetProducer);

  time_t endTime = time(nullptr);
  PRINT_EVENT_LINE(mProcessingLogPtr,"PRODUCER-UPDATE;END;%s updated in %ld seconds;",targetProducer->mName.c_str(),(endTime-startTime));
  PRINT_DATA(mDebugLogPtr, "### PRODUCER UPDATED IN %ld SECONDS (%s)\n",(endTime-startTime),targetProducer->mName.c_str());
  // printf("### PRODUCER UPDATED IN %3ld SECONDS (%s)\n",(endTime-startTime),targetProducer->mName.c_str());

  return true;
}




/*! \brief Update target files. */

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
          if (!updateTargetProducerFiles(conn,sourceProducer,targetProducer))
            return;
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
}  // namespace




/*! \brief Create the target data storage interface. Returns the Redis implementation if it is used. */

ContentServer::RedisImplementation* createTargetInterface()
{
  ContentServer::RedisImplementation *redisImplementation = nullptr;

  if (mStorageType == "redis")
  {
    redisImplementation = new ContentServer::RedisImplementation();
    redisImplementation->init(mRedisAddress.c_str(), mRedisPort, mRedisTablePrefix.c_str(),mRedisLockEnabled,mRedisPassword.c_str());
    mTargetInterface = redisImplementation;
  }

  if (mStorageType == "postgresql")
  {
    auto *client = new ContentServer::PostgresqlImplementation();
    client->init(mPostgresqlConnectionString.c_str(),"",true);
    mTargetInterface = client;
  }

  if (mStorageType == "http")
  {
    auto *httpClient = new ContentServer::HTTP::ClientImplementation();
    httpClient->init(mContentServerUrl);
    mTargetInterface = httpClient;
  }

  if (mStorageType == "corba")
  {
    auto *corbaClient = new ContentServer::Corba::ClientImplementation();
    corbaClient->init(mContentServerIor);
    mTargetInterface = corbaClient;
  }

  return redisImplementation;
}




/*! \brief Initialize the logs. */

void initLogs()
{
  if (mProcessingLogEnabled)
  {
    mProcessingLog.init(true, mProcessingLogFile.c_str(), mProcessingLogMaxSize, mProcessingLogTruncateSize);
    mProcessingLogPtr = &mProcessingLog;
  }

  if (mDebugLogEnabled)
  {
    mDebugLog.init(true, mDebugLogFile.c_str(), mDebugLogMaxSize, mDebugLogTruncateSize);
    mDebugLogPtr = &mDebugLog;
  }
}




/*! \brief Update the producer and generation information. */

void updateProducerAndGenerationInfo(PGconn *conn)
{
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
}




/*! \brief Update the geometry, file and content information. */

void updateGeometryAndContentInfo(PGconn *conn,ContentServer::RedisImplementation *redisImplementation)
{
  if (!shutdownRequested)
  {
    PRINT_DATA(mDebugLogPtr, "* Reading geometry information from the target data storage\n");
    readTargetGeometries(mTargetGeometryList);
    mTargetGeometryList.sort(T::GeometryInfo::ComparisonMethod::generationId);
  }

  if (!shutdownRequested)
  {
    PRINT_DATA(mDebugLogPtr, "* Reading geometry information from the source data storage\n");
    readSourceGeometries(conn,mSourceGeometryList);
  }

  if (!shutdownRequested)
  {
    PRINT_DATA(mDebugLogPtr, "* Updating geometry information into the target data storage\n");
    updateGeometries();
    readTargetGeometries(mTargetGeometryList);
    mTargetGeometryList.sort(T::GeometryInfo::ComparisonMethod::generationId);
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




/*! \brief Print the sizes of the data structures. */

void printDataStructures(time_t loopStart)
{
  PRINT_DATA(mDebugLogPtr, "* Data structures\n");
  PRINT_DATA(mDebugLogPtr, "   - mSourceProducerList   = %u\n",mSourceProducerList.getLength());
  PRINT_DATA(mDebugLogPtr, "   - mSourceGenerationList = %u\n",mSourceGenerationList.getLength());
  PRINT_DATA(mDebugLogPtr, "   - mSourceFilenames      = %lu\n",mSourceFilenames.size());
  PRINT_DATA(mDebugLogPtr, "   - mFilenameMap          = %lu\n",mFilenameMap.size());
  PRINT_DATA(mDebugLogPtr, "   - mFileRecMap           = %lu\n",mFileRecMap.size());
  PRINT_DATA(mDebugLogPtr, "   - fileRecords           = %u\n",countFileRecords());
  PRINT_DATA(mDebugLogPtr, "   - loopCounter           = %d\n",mLoopCounter);
  PRINT_DATA(mDebugLogPtr, "   - loopTime              = %ld sec\n",time(nullptr)-loopStart);

  PRINT_DATA(mDebugLogPtr, "********************************************************************\n\n");
}




/*! \brief Clear the data of the update loop, force full database read every 4th hour. */

void clearLoopData(time_t& lastDatabaseRead,PGconn *conn)
{
  mProducerList.clear();
  mSourceProducerList.clear();
  mSourceGenerationList.clear();

  std::set<int> sourceFilenames;
  mSourceFilenames.clear();
  mSourceFilenames.swap(sourceFilenames);

  std::set<UInt64> tcList;
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




/*! \brief Connect to the Radon database. Returns false if the connection fails. */

bool connectToRadon(PGconn*& conn)
{
  conn = PQconnectdb(mRadonConnectionString.c_str());
  if (PQstatus(conn) != CONNECTION_OK)
  {
    fprintf(stderr, "Postgresql error: %s\n", PQerrorMessage(conn));
    return false;
  }

  reconnectionRequired = false;
  return true;
}




/*! \brief Run one update of the target data storage. */

void runUpdate(PGconn *conn,ContentServer::RedisImplementation *redisImplementation)
{
  PRINT_DATA(mDebugLogPtr, "\n");
  PRINT_DATA(mDebugLogPtr, "********************************************************************\n");
  PRINT_DATA(mDebugLogPtr, "****************************** UPDATE ******************************\n");
  PRINT_DATA(mDebugLogPtr, "********************************************************************\n");

  PRINT_EVENT_LINE(mProcessingLogPtr,"UPDATE;START;;");

  try
  {
    updateLoopStart();

    if (!shutdownRequested)
    {
      PRINT_DATA(mDebugLogPtr, "* Reading producer names that belong to this update\n");
      readProducerList(mProducerFile.c_str());
    }

    if (!mProducerList.empty() ||  mProducerFullList.empty())
    {
      updateProducerAndGenerationInfo(conn);

      updateGeometryAndContentInfo(conn,redisImplementation);
    }

    updateLoopEnd();
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Processing exception!", nullptr);
    exception.printError();
  }
}




/*! \brief Wait for the next update loop. Returns false if there is no next loop. */

bool waitNextLoop()
{
  if (mWaitTime > 0 && !shutdownRequested)
  {
    if (mLoopCounter > 10)
      std::this_thread::sleep_for(std::chrono::seconds(mWaitTime));
  }
  else
  {
    return false;
  }
  return true;
}




/*! \brief Close the database connection if the connection should not be kept open. */

void closeConnectionIfNeeded(PGconn*& conn)
{
  if (mWaitTime >= 1800 || reconnectionRequired)
  {
    if (conn != nullptr)
      PQfinish(conn);

    conn = nullptr;
  }
}




/*! \brief Program entry point. */

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

    (void)signal(SIGINT, sig_handler);
    (void)signal(SIGTERM, sig_handler);
    (void)signal(SIGHUP, sig_handler);


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

    ContentServer::RedisImplementation *redisImplementation = createTargetInterface();

    initLogs();

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
      if ((reconnectionRequired || conn == nullptr) && !connectToRadon(conn))
        return -4;

      runUpdate(conn,redisImplementation);


      if (!shutdownRequested)
      {
        printDataStructures(loopStart);
        clearLoopData(lastDatabaseRead,conn);
      }

      PRINT_EVENT_LINE(mProcessingLogPtr,"UPDATE;END;Loop time %ld seconds;",time(nullptr)-loopStart);

      ind = waitNextLoop();

      closeConnectionIfNeeded(conn);
    }

    PQfinish(conn);

    return 0;
  }
  catch (Fmi::Exception&)
  {
    Fmi::Exception exception(BCP, "Service call failed!", nullptr);
    exception.printError();
    return -3;
  }
}

