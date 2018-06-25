#include "grid-files/common/Exception.h"
#include "grid-files/common/Log.h"
#include "grid-files/common/ShowFunction.h"
#include "grid-files/common/GeneralFunctions.h"
#include "grid-files/common/ShowFunction.h"
#include "grid-files/identification/GridDef.h"
#include "grid-content/contentServer/redis/RedisImplementation.h"
#include "grid-content/contentServer/corba/client/ClientImplementation.h"
#include "grid-content/contentServer/http/client/ClientImplementation.h"

#include <libpq-fe.h>
#include <stdlib.h>
#include <string.h>

#define FUNCTION_TRACE FUNCTION_TRACE_OFF


using namespace SmartMet;


// Some temporary storage structures:

struct ForecastRec
{
  uint        producerId = 0;
  std::string producerName;
  std::string analysisTime;
  std::string generationName;
  std::string forecastTime;
  std::string forecastPeriod;
  std::string tableName;
  uint        geometryId = 0;
  int         forecastTypeId = 0;
  int         forecastTypeValue = 0;
  std::string lastUpdated;
  std::string csv;
  bool        checked = false;
};


struct FileRec
{
  std::string fileName;
  std::string paramId;
  int         levelId;
  int         levelValue;
};



// Global variables:

ConfigurationFile         mConfigurationFile;
std::string               mGridConfigFile;
uint                      mSourceId = 100;
std::string               mProducerFile;
std::string               mPreloadFile;
uint                      mMaxMessageSize = 5000;
std::string               mRadonConnectionString;
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
Log*                      mDebugLogPtr = NULL;
T::SessionId              mSessionId = 0;
uint                      mGlobalFileId = 0;
std::string               mLastUpdateTime = "19000101T000000";

std::set<std::string>     mProducerList;
std::set<std::string>     mPreloadList;
T::ProducerInfoList       mSourceProducerList;
T::GenerationInfoList     mSourceGenerationList;
std::vector<ForecastRec>  mSourceForacastList;
T::ProducerInfoList       mTargetProducerList;
T::GenerationInfoList     mTargetGenerationList;
T::FileInfoList           mTargetFileList;
T::ContentInfoList        mTargetContentList;

uint                      mWaitTime = 300;
uint                      mTimeOutTime = 900;
bool                      mTimeOut = false;




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
      "smartmet.tools.grid.radon2smartmet.content-source.preloadFile",
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
      "smartmet.tools.grid.radon2smartmet.debug-log.truncateSize",
      NULL
    };


    mConfigurationFile.readFile(configFile);
    //mConfigurationFile.print(std::cout,0,0);

    uint t=0;
    while (configAttribute[t] != NULL)
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
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.maxMessageSize",mMaxMessageSize);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.content-source.source-id",mSourceId);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.content-source.producerFile",mProducerFile);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.content-source.preloadFile",mPreloadFile);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.content-source.radon.connection-string",mRadonConnectionString);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.content-storage.type",mStorageType);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.content-storage.redis.address",mRedisAddress);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.content-storage.redis.port",mRedisPort);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.content-storage.redis.tablePrefix",mRedisTablePrefix);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.content-storage.corba.ior",mContentServerIor);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.content-storage.http.url",mContentServerUrl);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.debug-log.enabled", mDebugLogEnabled);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.debug-log.file", mDebugLogFile);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.debug-log.maxSize", mDebugLogMaxSize);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.radon2smartmet.debug-log.truncateSize", mDebugLogTruncateSize);

  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Constructor failed!", NULL);
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
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,NULL);
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
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,NULL);
  }
}





void readProducerList(const char *filename)
{
  try
  {
    FILE *file = fopen(filename,"r");
    if (file == NULL)
    {
      PRINT_DATA(mDebugLogPtr,"Producer file not available. Accepting all produces");
      return;
    }

    mProducerList.clear();

    char st[1000];
    while (!feof(file))
    {
      if (fgets(st,1000,file) != NULL)
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
    throw SmartMet::Spine::Exception(BCP, exception_operation_failed, NULL);
  }
}





void readPreloadList(const char *filename)
{
  try
  {
    FILE *file = fopen(filename,"r");
    if (file == NULL)
    {
      PRINT_DATA(mDebugLogPtr,"Preload file not available.");
      return;
    }

    mPreloadList.clear();

    char st[1000];
    while (!feof(file))
    {
      if (fgets(st,1000,file) != NULL)
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
    throw SmartMet::Spine::Exception(BCP, exception_operation_failed, NULL);
  }
}





uint readFiles(PGconn *conn,const char *tableName,uint producerId,uint geometryId,int forecastTypeId,int forecastTypeValue,const char *analysisTime,const char *forecastPeriod,std::vector<FileRec>& fileRecList)
{
  FUNCTION_TRACE
  try
  {
    char sql[2000];
    char *p = sql;

     p+= sprintf(p,"SELECT\n");
     p+= sprintf(p,"  file_location,\n");
     p+= sprintf(p,"  param_id,\n");
     p+= sprintf(p,"  level_id,\n");
     p+= sprintf(p,"  level_value::int,\n");
     p+= sprintf(p,"  to_char((analysis_time+forecast_period) at time zone 'utc','yyyymmddThh24MISS'),\n");
     p+= sprintf(p,"  forecast_type_id,\n");
     p+= sprintf(p,"  forecast_type_value::int,\n");
     p+= sprintf(p,"  geometry_id\n");
     p+= sprintf(p,"FROM\n");
     p+= sprintf(p,"  %s\n",tableName);
     p+= sprintf(p,"WHERE\n");
     p+= sprintf(p,"  to_char(analysis_time, 'yyyymmddThh24MISS') = '%s' AND forecast_period = '%s' AND geometry_id = %u AND forecast_type_id = %d AND forecast_type_value = %d AND producer_id = %u",analysisTime,forecastPeriod,geometryId,forecastTypeId,forecastTypeValue,producerId);


    //printf("%s\n",sql);
    PGresult *res = PQexec(conn,sql);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
      SmartMet::Spine::Exception exception(BCP,"Postgresql error!");
      exception.addParameter("ErrorMessage",PQerrorMessage(conn));
      throw exception;
    }

    uint fileCount = 0;
    //int fieldCount = PQnfields(res);
    int rowCount = PQntuples(res);

    for (int i = 0; i < rowCount; i++)
    {
      if (strstr(PQgetvalue(res, i, 0),"masala") != NULL)
      {
        FileRec rec;

        rec.fileName = PQgetvalue(res, i, 0);
        rec.paramId = PQgetvalue(res, i, 1);
        rec.levelId = atoi(PQgetvalue(res, i, 2));
        rec.levelValue = atoi(PQgetvalue(res, i, 3));

        if (rec.levelId == 2)
          rec.levelValue = rec.levelValue * 100;

        fileRecList.push_back(rec);
#if 0
        printf("%u;%u;%s;%u;%u;%u;%u;%u\n",
               mGlobalFileId,
               0,   // Type
               PQgetvalue(res, i, 0),  // fileName
               producerId,
               0, //generationId,
               0,   // GroupFlas
               1,   // Flags (1 = content predefined)
               mSourceId
              );

        //readContent(conn,producerId,generationId,mGlobalFileId,0,PQgetvalue(res, i, 7),PQgetvalue(res, i, 4),PQgetvalue(res, i, 4),PQgetvalue(res, i, 1),PQgetvalue(res, i, 2),PQgetvalue(res, i, 3),PQgetvalue(res, i, 5),PQgetvalue(res, i, 6));
        mGlobalFileId++;
#endif
        fileCount++;
      }
    }
    PQclear(res);
    return fileCount;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, exception_operation_failed, NULL);
  }
}





void readSourceForecastTimes(PGconn *conn)
{
  FUNCTION_TRACE
  try
  {
    mSourceForacastList.clear();

    char sql[2000];
    char *p = sql;
    p+= sprintf(p,"SELECT DISTINCT\n");
    p+= sprintf(p,"  fmi_producer.id,\n");
    p+= sprintf(p,"  fmi_producer.name,\n");
    p+= sprintf(p,"  to_char(ss_state.analysis_time at time zone 'utc', 'yyyymmddThh24MISS'),\n");
    p+= sprintf(p,"  to_char((ss_state.analysis_time+ss_state.forecast_period) at time zone 'utc', 'yyyymmddThh24MISS'),\n");
    p+= sprintf(p,"  ss_state.forecast_period,\n");
    p+= sprintf(p,"  ss_state.table_name,\n");
    p+= sprintf(p,"  ss_state.geometry_id,\n");
    p+= sprintf(p,"  ss_state.forecast_type_id,\n");
    p+= sprintf(p,"  ss_state.forecast_type_value,\n");
    p+= sprintf(p,"  to_char(ss_state.last_updated at time zone 'utc', 'yyyymmddThh24MISS')\n");
    p+= sprintf(p,"FROM\n");
    p+= sprintf(p,"  ss_state LEFT OUTER JOIN fmi_producer ON fmi_producer.id = ss_state.producer_id\n");
    p+= sprintf(p,"ORDER BY\n");
    p+= sprintf(p,"  to_char(ss_state.analysis_time at time zone 'utc', 'yyyymmddThh24MISS') desc,\n");
    p+= sprintf(p,"  to_char((ss_state.analysis_time+ss_state.forecast_period) at time zone 'utc', 'yyyymmddThh24MISS');\n");

    PGresult *res = PQexec(conn, sql);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
      SmartMet::Spine::Exception exception(BCP,"Postgresql error!");
      exception.addParameter("ErrorMessage",PQerrorMessage(conn));
      throw exception;
    }

    //int fieldCount = PQnfields(res);
    int rowCount = PQntuples(res);

    for (int i = 0; i < rowCount; i++)
    {
      uint producerId = atoi(PQgetvalue(res, i, 0));
      T::ProducerInfo *producer = mSourceProducerList.getProducerInfoById(producerId);
      if (producer != NULL)
      {
        ForecastRec forecastRec;
        forecastRec.producerId = atoi(PQgetvalue(res, i, 0));
        forecastRec.producerName = PQgetvalue(res, i, 1);
        forecastRec.analysisTime = PQgetvalue(res, i, 2);
        forecastRec.generationName = forecastRec.producerName + ":" + forecastRec.analysisTime;
        forecastRec.forecastTime = PQgetvalue(res, i, 3);
        forecastRec.forecastPeriod = PQgetvalue(res, i, 4);
        forecastRec.tableName = PQgetvalue(res, i, 5);
        forecastRec.geometryId  = atoi(PQgetvalue(res, i, 6));
        forecastRec.forecastTypeId = atoi(PQgetvalue(res, i, 7));
        forecastRec.forecastTypeValue = atoi(PQgetvalue(res, i, 8));
        forecastRec.lastUpdated = PQgetvalue(res, i, 9);

        T::GenerationInfo *generation = mTargetGenerationList.getGenerationInfoByName(forecastRec.generationName.c_str());
        if (generation != NULL)
        {
          char st[200];
          sprintf(st,"%u;%u;%u;%d;%d;%s;%s;",
              mSourceId,
              generation->mGenerationId,
              forecastRec.geometryId,
              forecastRec.forecastTypeId,
              forecastRec.forecastTypeValue,
              forecastRec.forecastTime.c_str(),
              forecastRec.lastUpdated.c_str());

          forecastRec.csv = st;

        }
/*
        printf("%u;%s;%s;%s;%s;%s;%s;%u;%d;%d;%s;%s;\n",
            forecastRec.producerId,
            forecastRec.producerName.c_str(),
            forecastRec.analysisTime.c_str(),
            forecastRec.generationName.c_str(),
            forecastRec.forecastTime.c_str(),
            forecastRec.forecastPeriod.c_str(),
            forecastRec.tableName.c_str(),
            forecastRec.geometryId,
            forecastRec.forecastTypeId,
            forecastRec.forecastTypeValue,
            forecastRec.lastUpdated.c_str(),
            forecastRec.csv.c_str());
*/
        mSourceForacastList.push_back(forecastRec);
      }
    }
    PQclear(res);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, exception_operation_failed, NULL);
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
    p += sprintf(p,"SELECT DISTINCT\n");
    p += sprintf(p,"  fmi_producer.id,\n");
    p += sprintf(p,"  fmi_producer.name,\n");
    p += sprintf(p,"  to_char(ss_state.analysis_time at time zone 'utc', 'yyyymmddThh24MISS')\n");
    p += sprintf(p,"FROM\n");
    p += sprintf(p,"  ss_state LEFT OUTER JOIN fmi_producer ON fmi_producer.id = ss_state.producer_id\n");
    p += sprintf(p,"ORDER BY");
    p += sprintf(p,"  fmi_producer.id,to_char(ss_state.analysis_time at time zone 'utc', 'yyyymmddThh24MISS') desc;");

    PGresult *res = PQexec(conn, sql);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
      SmartMet::Spine::Exception exception(BCP,"Postgresql error!");
      exception.addParameter("ErrorMessage",PQerrorMessage(conn));
      throw exception;
    }

    //int fieldCount = PQnfields(res);
    int rowCount = PQntuples(res);

    for (int i = 0; i < rowCount; i++)
    {
      char st[1000];

      uint producerId = atoi(PQgetvalue(res, i, 0));
      T::ProducerInfo *producer = mSourceProducerList.getProducerInfoById(producerId);
      if (producer != NULL)
      {
        T::GenerationInfo *generation = new T::GenerationInfo();
        generation->mGenerationId = i + 1;
        generation->mGenerationType = 0;
        generation->mProducerId = producerId;
        sprintf(st,"%s:%s",producer->mName.c_str(),PQgetvalue(res, i, 2));
        generation->mName = st;
        sprintf(st,"Producer %s generation %s",producer->mName.c_str(),PQgetvalue(res, i, 2));
        generation->mDescription = st;
        generation->mAnalysisTime = PQgetvalue(res, i, 2);
        generation->mStatus = T::GenerationStatus::STATUS_READY;
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
    throw SmartMet::Spine::Exception(BCP, exception_operation_failed, NULL);
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
    p += sprintf(p,"SELECT DISTINCT\n");
    p += sprintf(p,"  fmi_producer.id,\n");
    p += sprintf(p,"  fmi_producer.name,\n");
    p += sprintf(p,"  fmi_producer.description\n");
    p += sprintf(p,"FROM\n");
    p += sprintf(p,"  ss_state LEFT OUTER JOIN fmi_producer ON fmi_producer.id = ss_state.producer_id");

    PGresult *res = PQexec(conn,sql);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
      SmartMet::Spine::Exception exception(BCP,"Postgresql error!");
      exception.addParameter("ErrorMessage",PQerrorMessage(conn));
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
        producer->mProducerId = atoi(PQgetvalue(res, i, 0));
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
    throw SmartMet::Spine::Exception(BCP, exception_operation_failed, NULL);
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
        if (sourceProducer == NULL)
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
        if (targetProducer == NULL)
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
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,NULL);
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
        if (sourceGeneration == NULL)
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
        T::GenerationInfo *targetGeneration = mTargetGenerationList.getGenerationInfoByName(sourceGeneration->mName);
        if (targetGeneration == NULL)
        {
          // The generation information is not available in the target data storage. So, we should add it.

          // Finding producer name:
          T::ProducerInfo *sourceProducer = mSourceProducerList.getProducerInfoById(sourceGeneration->mProducerId);
          if (sourceProducer != NULL)
          {
            // Finding producer id from the target data storage.
            T::ProducerInfo targetProducer;
            int result = targetInterface->getProducerInfoByName(mSessionId,sourceProducer->mName,targetProducer);
            if (result != 0)
            {
              SmartMet::Spine::Exception exception(BCP,"The producer information not found from the target data storage!");
              exception.addParameter("ProducerName",sourceProducer->mName);
              exception.addParameter("Result",ContentServer::getResultString(result));
              throw exception;
            }

            T::GenerationInfo generationInfo(*sourceGeneration);
            generationInfo.mProducerId = targetProducer.mProducerId;
            generationInfo.mSourceId = mSourceId;

            PRINT_DATA(mDebugLogPtr,"  -- Add generation : %s\n",generationInfo.mName.c_str());

            result = targetInterface->addGenerationInfo(mSessionId,generationInfo);
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
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,NULL);
  }
}





bool isForecastValid(std::string forecast)
{
  try
  {
    for (auto it=mSourceForacastList.begin(); it!=mSourceForacastList.end(); ++it)
    {
      if (it->csv == forecast)
      {
        it->checked = true;
        return true;
      }
    }

    return false;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,NULL);
  }
}





void deleteForecast(ContentServer::ServiceInterface *targetInterface,std::string forecast)
{
  try
  {
    char *field[100];
    char st[1000];
    strcpy(st,forecast.c_str());

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

    if (c >= 6)
    {
      uint sourceId = (uint)atoll(field[0]);
      uint generationId = (uint)atoll(field[1]);
      uint geometryId = (uint)atoll(field[2]);
      short forecastType = (short)atoll(field[3]);
      short forecastNumber = (short)atoll(field[4]);
      std::string forecastTime = field[5];
      std::string modificationTime = field[6];

      if (sourceId == mSourceId)
        targetInterface->deleteFileInfoListByGenerationIdAndForecastTime(mSessionId,generationId,geometryId,forecastType,forecastNumber,forecastTime.c_str());
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,NULL);
  }
}





uint addForecast(ContentServer::ServiceInterface *targetInterface,PGconn *conn,ForecastRec& forecast,std::vector<T::FileAndContent>& fileAndContentList)
{
  FUNCTION_TRACE
  try
  {
    T::GenerationInfo *generation = mTargetGenerationList.getGenerationInfoByName(forecast.generationName);
    if (generation == NULL)
    {
      PRINT_DATA(mDebugLogPtr,"**** Unknown generation : %s\n",forecast.generationName.c_str());
      return 0;
    }

    std::vector<FileRec> fileRecList;
    uint cnt = readFiles(conn,forecast.tableName.c_str(),forecast.producerId,forecast.geometryId,forecast.forecastTypeId,forecast.forecastTypeValue,forecast.analysisTime.c_str(),forecast.forecastPeriod.c_str(),fileRecList);
    if (cnt == 0)
      return 0;

    for (auto it=fileRecList.begin(); it != fileRecList.end(); ++it)
    {
      T::FileAndContent fc;

      fc.mFileInfo.mGroupFlags = 0;
      fc.mFileInfo.mProducerId = generation->mProducerId;
      fc.mFileInfo.mGenerationId = generation->mGenerationId;
      fc.mFileInfo.mFileId = 0;
      fc.mFileInfo.mFileType = T::FileType::Unknown;
      fc.mFileInfo.mName = it->fileName;
      fc.mFileInfo.mFlags = T::FileInfo::Flags::PredefinedContent;
      fc.mFileInfo.mSourceId = mSourceId;


      T::ContentInfo *contentInfo = new T::ContentInfo();

      contentInfo->mFileId = 0;
      contentInfo->mFileType = T::FileType::Unknown;
      contentInfo->mMessageIndex = 0;
      contentInfo->mProducerId = generation->mProducerId;
      contentInfo->mGenerationId = generation->mGenerationId;
      contentInfo->mGroupFlags = 0;
      contentInfo->mForecastTime = forecast.forecastTime;
      contentInfo->mFmiParameterId = it->paramId;
      contentInfo->mFmiParameterLevelId = it->levelId;
      contentInfo->mParameterLevel = it->levelValue;
      contentInfo->mForecastType = forecast.forecastTypeId;
      contentInfo->mForecastNumber = forecast.forecastTypeValue;
      contentInfo->mServerFlags = 0;
      contentInfo->mFlags = 0;
      contentInfo->mSourceId = mSourceId;
      contentInfo->mGeometryId = forecast.geometryId;
      contentInfo->mModificationTime = forecast.lastUpdated;

      Identification::FmiParameterDef fmiDef;
      if (Identification::gridDef.getFmiParameterDefById(it->paramId,fmiDef))
      {
        contentInfo->mFmiParameterName = fmiDef.mParameterName;
        contentInfo->mFmiParameterUnits = fmiDef.mParameterUnits;

        Identification::NewbaseParameterDef newbaseDef;
        if (Identification::gridDef.getNewbaseParameterDefByFmiId(it->paramId,newbaseDef))
        {
          contentInfo->mNewbaseParameterId = newbaseDef.mNewbaseParameterId;
          contentInfo->mNewbaseParameterName = newbaseDef.mParameterName;
        }
/*
        Identification::FmiParameterId_grib1 g1Def;
        if (Identification::gridDef.mMessageIdentifier_fmi.getGrib1ParameterDef(it->paramId,g1Def))
          contentInfo->mGrib1ParameterLevelId = (T::ParamLevelId)(*g1Def.mGribParameterLevelId);

        Identification::FmiParameterId_grib2 g2Def;
        if (Identification::gridDef.mMessageIdentifier_fmi.getGrib2ParameterDef(it->paramId,g2Def))
          contentInfo->mGrib2ParameterLevelId = (T::ParamLevelId)(*g2Def.mGribParameterLevelId);
*/
      }

      char st[200];
      sprintf(st,"%s;%s;%d;%d;%05d;%d;%d;1;",
          forecast.producerName.c_str(),
          contentInfo->mFmiParameterName.c_str(),
          (int)T::ParamLevelIdType::FMI,
          (int)contentInfo->mFmiParameterLevelId,
          (int)contentInfo->mParameterLevel,
          (int)contentInfo->mForecastType,
          (int)contentInfo->mForecastNumber);

      if (mPreloadList.find(toLowerString(std::string(st))) != mPreloadList.end())
        contentInfo->mFlags = T::ContentInfo::Flags::PreloadRequired;


#if 0
      fprintf(contentFile,"%u;%u;%u;%s;%u;%u;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%u;%u;%u;%s;\n",
             fileId,
             0, // messageIndex
             fileType,
             producerId,
             generationId,
             0, // groupFlags
             startTime,
             endTime,
             fmiParameterId,
             fmiParameterName.c_str(),
             gribParameterId.c_str(),
             "", // cdmParameterId
             "", // cdmParameterName
             newbaseParameterId.c_str(),
             newbaseParameterName.c_str(),
             fmiLevelId,
             grib1ParameterLevelId.c_str(),
             grib2ParameterLevelId.c_str(),
             parameterLevel,
             fmiParameterUnits.c_str(),
             gribParameterId.c_str(),
             forecastType,
             pertubationNumber,
             0, // serverFlags,
             0, // flags,
             mSourceId,
             geometryId
          );
#endif

      fc.mContentInfoList.addContentInfo(contentInfo);
      //targetInterface->addFileInfoWithContentList(mSessionId,fileInfo,contentInfoList);

      fileAndContentList.push_back(fc);

      PRINT_DATA(mDebugLogPtr,"      * Add file :  %s\n",it->fileName.c_str());

      if (fileAndContentList.size() >= mMaxMessageSize)
      {
        int result = targetInterface->addFileInfoListWithContent(mSessionId,fileAndContentList);
        if (result != 0)
        {
          fprintf(stdout,"ERROR (%d) : %s\n",result,ContentServer::getResultString(result).c_str());
        }
        fileAndContentList.clear();
      }
    }

    return cnt;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,NULL);
  }
}





void updateForecastTimes(ContentServer::ServiceInterface *targetInterface,PGconn *conn)
{
  FUNCTION_TRACE
  try
  {
    std::string lastUpdated = mLastUpdateTime;
    char ss[20];
    sprintf(ss,"%u;",mSourceId);
    int ssLen = (int)strlen(ss);

    std::set<std::string> forecastList;
    int result = targetInterface->getGenerationIdGeometryIdAndForecastTimeList(mSessionId,forecastList);
    if (result != 0)
    {
      SmartMet::Spine::Exception exception(BCP,"Cannot get forecast time list from the target data storage!");
      throw exception;
    }

    std::vector<T::ForecastTime> forecastTimeList;
    for (auto forecast=forecastList.begin(); forecast!=forecastList.end(); ++forecast)
    {
      if (strncasecmp(forecast->c_str(),ss,ssLen) == 0  &&  !isForecastValid(*forecast))
      {
        PRINT_DATA(mDebugLogPtr,"  -- Delete forecast : %s\n",forecast->c_str());
        forecastTimeList.push_back(T::ForecastTime(forecast->c_str()));
        //deleteForecast(targetInterface,*forecast);
      }
    }

    if (forecastTimeList.size() > 0)
    {
      targetInterface->deleteFileInfoListByForecastTimeList(mSessionId,forecastTimeList);
    }


    time_t startTime = time(0);
    mTimeOut = false;

    std::vector<T::FileAndContent> fileAndContentList;

    for (auto it=mSourceForacastList.begin(); it!=mSourceForacastList.end()  &&  !mTimeOut; ++it)
    {
      if (!it->checked  &&  it->lastUpdated > mLastUpdateTime)
      {
        PRINT_DATA(mDebugLogPtr,"  -- Add forecast : %u;%s;%s;%s;%s;%u;%d;%d;%s;\n",
            it->producerId,
            it->producerName.c_str(),
            it->analysisTime.c_str(),
            it->generationName.c_str(),
            it->forecastTime.c_str(),
            it->geometryId,
            it->forecastTypeId,
            it->forecastTypeValue,
            it->lastUpdated.c_str());

        addForecast(targetInterface,conn,*it,fileAndContentList);

        if (fileAndContentList.size() >= mMaxMessageSize)
        {
          int result = targetInterface->addFileInfoListWithContent(mSessionId,fileAndContentList);
          if (result != 0)
          {
            fprintf(stdout,"ERROR (%d) : %s\n",result,ContentServer::getResultString(result).c_str());
          }
          fileAndContentList.clear();
        }

        if (it->lastUpdated > lastUpdated)
          lastUpdated = it->lastUpdated;
      }

      if (mWaitTime > 0  &&  (time(0) - startTime) >= mTimeOutTime)
      {
        // Additions have take so much time that we should interrupt the loop
        // and restart the update process from the begin. The point is that
        // there can be a lot of content removals waiting and we should process
        // them also in a reasonable time frame.

        mTimeOut = true;
      }
    }

    if (fileAndContentList.size() > 0)
    {
      int result = targetInterface->addFileInfoListWithContent(mSessionId,fileAndContentList);
      if (result != 0)
      {
        fprintf(stdout,"ERROR (%d) : %s\n",result,ContentServer::getResultString(result).c_str());
      }
      fileAndContentList.clear();
    }

    if (lastUpdated > mLastUpdateTime)
      mLastUpdateTime = lastUpdated;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,NULL);
  }
}





int main(int argc, char *argv[])
{
  FUNCTION_TRACE
  try
  {
    if (argc != 3)
    {
      fprintf(stderr,"USAGE: radon2smartmet <configFile> <loopWaitTime>\n");
      return -1;
    }

    readConfigFile(argv[1]);

    // Initializing the global structures. These are needed when
    // extracting information from GRIB files.

    Identification::gridDef.init(mGridConfigFile.c_str());

    ContentServer::ServiceInterface *targetInterface = NULL;

    mWaitTime = atoi(argv[2]);

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

    if (targetInterface == NULL)
    {
      fprintf(stderr,"No target data source defined!\n");
      return -3;
    }

    PGconn *conn = PQconnectdb(mRadonConnectionString.c_str());
    if (PQstatus(conn) != CONNECTION_OK)
    {
      fprintf(stderr,"Postgresql error: %s\n",PQerrorMessage(conn));
      return -4;
    }

    bool ind = true;
    while (ind)
    {
      PRINT_DATA(mDebugLogPtr,"\n");
      PRINT_DATA(mDebugLogPtr,"********************************************************************\n");
      PRINT_DATA(mDebugLogPtr,"****************************** UPDATE ******************************\n");
      PRINT_DATA(mDebugLogPtr,"********************************************************************\n");

      PRINT_DATA(mDebugLogPtr,"* Reading producer names that belongs to this update\n");
      readProducerList(mProducerFile.c_str());

      PRINT_DATA(mDebugLogPtr,"* Reading preload parameters\n");
      readPreloadList(mPreloadFile.c_str());

      PRINT_DATA(mDebugLogPtr,"* Reading producer information from the target data storage\n");
      readTargetProducers(targetInterface);

      PRINT_DATA(mDebugLogPtr,"* Reading producer information from the source data storage\n");
      readSourceProducers(conn);

      PRINT_DATA(mDebugLogPtr,"* Updating producer information into the target data storage\n");
      updateProducers(targetInterface);

      PRINT_DATA(mDebugLogPtr,"* Reading updated producer information from the target data storage\n");
      readTargetProducers(targetInterface);

      PRINT_DATA(mDebugLogPtr,"* Reading generation information from the target data storage\n");
      readTargetGenerations(targetInterface);

      PRINT_DATA(mDebugLogPtr,"* Reading generation information from the source data storage\n");
      readSourceGenerations(conn);

      PRINT_DATA(mDebugLogPtr,"* Updating generation information into the target data storage\n");
      updateGenerations(targetInterface);

      PRINT_DATA(mDebugLogPtr,"* Reading updated generation information from the target data storage\n");
      readTargetGenerations(targetInterface);

      PRINT_DATA(mDebugLogPtr,"* Reading forecast time information from the source data storage\n");
      readSourceForecastTimes(conn);

      PRINT_DATA(mDebugLogPtr,"* Updating forecast time information into the target data storage\n");
      updateForecastTimes(targetInterface,conn);

      PRINT_DATA(mDebugLogPtr,"********************************************************************\n\n");

      if (mWaitTime > 0)
      {
        if (!mTimeOut)     // If we interrupted the addition process then we should not sleep.
          sleep(mWaitTime);
      }
      else
      {
        ind = false;
      }
      mTimeOut = false;
    }

    PQfinish(conn);

    return 0;
  }
  catch (SmartMet::Spine::Exception& e)
  {
    SmartMet::Spine::Exception exception(BCP,"Service call failed!",NULL);
    exception.printError();
    return -3;
  }
}

