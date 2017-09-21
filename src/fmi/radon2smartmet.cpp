#include "grid-files/common/Exception.h"
#include "grid-files/common/ShowFunction.h"
#include "grid-files/common/GeneralFunctions.h"
#include "grid-files/common/ShowFunction.h"
#include "grid-files/identification/GribDef.h"
#include "grid-content/contentServer/redis/RedisImplementation.h"

#include <libpq-fe.h>
#include <stdlib.h>
#include <string.h>


#define FUNCTION_TRACE FUNCTION_TRACE_OFF



using namespace SmartMet;



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


T::SessionId mSessionId = 0;
uint sourceId = 200;
uint globalFileId = 0;
std::string lastUpdateTime("19000101T000000");

T::ProducerInfoList      mSourceProducerList;
T::GenerationInfoList    mSourceGenerationList;
std::vector<ForecastRec> mSourceForacastList;

T::ProducerInfoList      mTargetProducerList;
T::GenerationInfoList    mTargetGenerationList;
T::FileInfoList          mTargetFileList;
T::ContentInfoList       mTargetContentList;





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




#if 0
void readTargetFiles(ContentServer::ServiceInterface *targetInterface)
{
  FUNCTION_TRACE
  try
  {
    printf("Reading files from the target data storage\n");
    mTargetFileList.clear();
    uint len = 1;
    uint startFileId = 0;
    uint maxRecords = 10000;

    while (len > 0)
    {
      T::FileInfoList fileList;
      int result = targetInterface->getFileInfoList(mSessionId,startFileId,maxRecords,fileList);
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
        mTargetFileList.addFileInfo(fileInfo->duplicate());
        if (fileInfo->mFileId >= startFileId)
          startFileId = fileInfo->mFileId + 1;

      }
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,NULL);
  }
}





void readTargetContent(ContentServer::ServiceInterface *targetInterface)
{
  FUNCTION_TRACE
  try
  {
    printf("Reading content from the target data storage\n");
    mTargetContentList.clear();
    uint len = 1;
    uint startFileId = 0;
    uint startMessageIndex = 0;
    uint maxRecords = 10000;

    while (len > 0)
    {
      T::ContentInfoList contentList;
      int result = targetInterface->getContentListBySourceId(mSessionId,mTargetId,startFileId,startMessageIndex,maxRecords,contentList);
      if (result != 0)
      {
        SmartMet::Spine::Exception exception(BCP,"Cannot read the content list from the target data storage!");
        exception.addParameter("Result",ContentServer::getResultString(result));
        throw exception;
      }

      len = contentList.getLength();
      for (uint t=0; t<len; t++)
      {
        T::ContentInfo *contentInfo = contentList.getContentInfoByIndex(t);
        mTargetContentList.addContentInfo(contentInfo->duplicate());
        if (contentInfo->mFileId > startFileId  || (contentInfo->mFileId == startFileId  &&  contentInfo->mMessageIndex >= startMessageIndex))
        {
          startFileId = contentInfo->mFileId;
          startMessageIndex = contentInfo->mMessageIndex + 1;
        }
      }
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,NULL);
  }
}
#endif





#if 0
void readContent(PGconn *conn,char *producerId,uint generationId,uint fileId,uint fileType,char *geometryId,char *startTime,char *endTime,char *fmiParameterId,char *fmiLevelId,char *parameterLevel,char *forecastType,char *pertubationNumber)
{
  FUNCTION_TRACE
  try
  {

    std::string fmiParameterName;
    std::string fmiParameterUnits;
    std::string newbaseParameterId;
    std::string newbaseParameterName;
    std::string gribParameterId;
    std::string gribParameterUnits;

    std::string grib1ParameterLevelId;
    std::string grib2ParameterLevelId;


    const Identification::ParameterDefinition_fmi_cptr fmiDef = Identification::gribDef.mMessageIdentifier_fmi.getParameterDefById(fmiParameterId);

    if (fmiDef != NULL)
    {
      fmiParameterName = fmiDef->mParameterName;
      fmiParameterUnits = fmiDef->mParameterUnits;
      newbaseParameterId = fmiDef->mNewbaseId;

      const Identification::Parameter_newbase_cptr  nbDef = Identification::gribDef.mMessageIdentifier_fmi.getParameter_newbaseId(fmiDef->mNewbaseId);
      if (nbDef != NULL)
        newbaseParameterName = nbDef->mParameterName;

      const Identification::Parameter_grib1_fmi_cptr g1Def = Identification::gribDef.mMessageIdentifier_fmi.getParameter_grib1(fmiParameterId);
      if (g1Def != NULL)
        grib1ParameterLevelId = toString(g1Def->mGribParameterLevelId);

      const Identification::Parameter_grib2_fmi_cptr g2Def = Identification::gribDef.mMessageIdentifier_fmi.getParameter_grib2(fmiParameterId);
      if (g2Def != NULL)
        grib2ParameterLevelId = toString(g2Def->mGribParameterLevelId);

    }


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
           sourceId,
           geometryId
        );
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, exception_operation_failed, NULL);
  }
}
#endif




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

        fileRecList.push_back(rec);
#if 0
        printf("%u;%u;%s;%u;%u;%u;%u;%u\n",
               globalFileId,
               0,   // Type
               PQgetvalue(res, i, 0),  // fileName
               producerId,
               0, //generationId,
               0,   // GroupFlas
               1,   // Flags (1 = content predefined)
               sourceId
              );

        //readContent(conn,producerId,generationId,globalFileId,0,PQgetvalue(res, i, 7),PQgetvalue(res, i, 4),PQgetvalue(res, i, 4),PQgetvalue(res, i, 1),PQgetvalue(res, i, 2),PQgetvalue(res, i, 3),PQgetvalue(res, i, 5),PQgetvalue(res, i, 6));
        globalFileId++;
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
    p+= sprintf(p,"  ss_state LEFT OUTER JOIN fmi_producer ON fmi_producer.id = ss_state.producer_id;\n");

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
          sprintf(st,"%u;%u;%d;%d;%s;%s;",
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
    p += sprintf(p,"  ss_state LEFT OUTER JOIN fmi_producer ON fmi_producer.id = ss_state.producer_id;");

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
        generation->mStatus = T::GenerationStatus::STATUS_READY;
        generation->mFlags = 0;
        generation->mSourceId = sourceId;
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
      T::ProducerInfo *producer = new T::ProducerInfo();
      producer->mProducerId = atoi(PQgetvalue(res, i, 0));
      producer->mName = PQgetvalue(res, i, 1);
      producer->mTitle = PQgetvalue(res, i, 1);
      producer->mDescription = PQgetvalue(res, i, 2);
      producer->mFlags = 0;
      producer->mSourceId = sourceId;

      mSourceProducerList.addProducerInfo(producer);
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
      if (targetProducer->mSourceId == sourceId)
      {
        T::ProducerInfo *sourceProducer = mSourceProducerList.getProducerInfoByName(targetProducer->mName);
        if (sourceProducer == NULL)
        {
          // The producer information is not available in the source data storage. So, we should remove
          // it also from the target data storage.

          printf("  -- Remove producer : %s\n",targetProducer->mName.c_str());

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
      if (sourceProducer->mSourceId == sourceId)
      {
        T::ProducerInfo *targetProducer = mTargetProducerList.getProducerInfoByName(sourceProducer->mName);
        if (targetProducer == NULL)
        {
          // The producer information is not available in the target data storage. So, we should add it.

          T::ProducerInfo producer(*sourceProducer);
          producer.mProducerId = 0;
          producer.mSourceId = sourceId;

          printf("  -- Add producer : %s\n",producer.mName.c_str());

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
    uint len = mTargetGenerationList.getLength();
    for (uint t=0; t<len; t++)
    {
      T::GenerationInfo *targetGeneration = mTargetGenerationList.getGenerationInfoByIndex(t);
      if (targetGeneration->mSourceId == sourceId)
      {
        T::GenerationInfo *sourceGeneration = mSourceGenerationList.getGenerationInfoByName(targetGeneration->mName);
        if (sourceGeneration == NULL)
        {
          // The generation information is not available in the source data storage. So, we should remove
          // it also from the target data storage.

          printf("  -- Remove generation : %s\n",targetGeneration->mName.c_str());

          int result = targetInterface->deleteGenerationInfoById(mSessionId,targetGeneration->mGenerationId);
          if (result != 0)
          {
            SmartMet::Spine::Exception exception(BCP,"Cannot delete the generation information from the target data storage!");
            exception.addParameter("GenerationId",std::to_string(targetGeneration->mGenerationId));
            exception.addParameter("GenerationName",targetGeneration->mName);
            exception.addParameter("Result",ContentServer::getResultString(result));
            throw exception;
          }
        }
      }
    }

    len = mSourceGenerationList.getLength();
    for (uint t=0; t<len; t++)
    {
      T::GenerationInfo *sourceGeneration = mSourceGenerationList.getGenerationInfoByIndex(t);
      if (sourceGeneration->mSourceId == sourceId)
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
            generationInfo.mSourceId = sourceId;

            printf("  -- Add generation : %s\n",generationInfo.mName.c_str());
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

    if (c >= 5)
    {
      uint generationId = (uint)atoll(field[0]);
      uint geometryId = (uint)atoll(field[1]);
      short forecastType = (short)atoll(field[2]);
      short forecastNumber = (short)atoll(field[3]);
      std::string forecastTime = field[4];
      std::string modificationTime = field[5];

      /*int result = */targetInterface->deleteFileInfoListByGenerationIdAndForecastTime(mSessionId,generationId,geometryId,forecastType,forecastNumber,forecastTime.c_str());
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,NULL);
  }
}





uint addForecast(ContentServer::ServiceInterface *targetInterface,PGconn *conn,ForecastRec& forecast)
{
  FUNCTION_TRACE
  try
  {
    T::GenerationInfo *generation = mTargetGenerationList.getGenerationInfoByName(forecast.generationName);
    if (generation == NULL)
    {
      printf("**** Unknown generation : %s\n",forecast.generationName.c_str());
      return 0;
    }

    std::vector<FileRec> fileRecList;
    uint cnt = readFiles(conn,forecast.tableName.c_str(),forecast.producerId,forecast.geometryId,forecast.forecastTypeId,forecast.forecastTypeValue,forecast.analysisTime.c_str(),forecast.forecastPeriod.c_str(),fileRecList);
    if (cnt == 0)
      return 0;

    for (auto it=fileRecList.begin(); it != fileRecList.end(); ++it)
    {

      T::FileInfo fileInfo;
      T::ContentInfoList contentInfoList;

      fileInfo.mGroupFlags = 0;
      fileInfo.mProducerId = generation->mProducerId;
      fileInfo.mGenerationId = generation->mGenerationId;
      fileInfo.mFileId = 0;
      fileInfo.mFileType = T::FileType::Unknown;
      fileInfo.mName = it->fileName;
      fileInfo.mFlags = (uint)T::FileInfoFlags::CONTENT_PREDEFINED;
      fileInfo.mSourceId = sourceId;


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
      contentInfo->mSourceId = sourceId;
      contentInfo->mGeometryId = forecast.geometryId;
      contentInfo->mModificationTime = forecast.lastUpdated;

      const Identification::ParameterDefinition_fmi_cptr fmiDef = Identification::gribDef.mMessageIdentifier_fmi.getParameterDefById(it->paramId);

      if (fmiDef != NULL)
      {
        contentInfo->mFmiParameterName = fmiDef->mParameterName;
        contentInfo->mFmiParameterUnits = fmiDef->mParameterUnits;
        contentInfo->mNewbaseParameterId = fmiDef->mNewbaseId;

        const Identification::Parameter_newbase_cptr  nbDef = Identification::gribDef.mMessageIdentifier_fmi.getParameter_newbaseId(fmiDef->mNewbaseId);
        if (nbDef != NULL)
          contentInfo->mNewbaseParameterName = nbDef->mParameterName;

        const Identification::Parameter_grib1_fmi_cptr g1Def = Identification::gribDef.mMessageIdentifier_fmi.getParameter_grib1(it->paramId);
        if (g1Def != NULL)
          contentInfo->mGrib1ParameterLevelId = (T::ParamLevelId)(*g1Def->mGribParameterLevelId);

        const Identification::Parameter_grib2_fmi_cptr g2Def = Identification::gribDef.mMessageIdentifier_fmi.getParameter_grib2(it->paramId);
        if (g2Def != NULL)
          contentInfo->mGrib2ParameterLevelId = (T::ParamLevelId)(*g2Def->mGribParameterLevelId);

      }
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
             sourceId,
             geometryId
          );
#endif

      contentInfoList.addContentInfo(contentInfo);
      targetInterface->addFileInfoWithContentList(mSessionId,fileInfo,contentInfoList);

      printf("      * Add file :  %s\n",it->fileName.c_str());
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
    std::string lastUpdated = lastUpdateTime;

    std::set<std::string> forecastList;
    int result = targetInterface->getGenerationIdGeometryIdAndForecastTimeList(mSessionId,forecastList);
    if (result != 0)
    {
      SmartMet::Spine::Exception exception(BCP,"Cannot get forecast time list from the target data storage!");
      throw exception;
    }

    for (auto forecast=forecastList.begin(); forecast!=forecastList.end(); ++forecast)
    {
      if (!isForecastValid(*forecast))
      {
        printf("  -- Delete forecast : %s\n",forecast->c_str());
        deleteForecast(targetInterface,*forecast);
      }
    }

    for (auto it=mSourceForacastList.begin(); it!=mSourceForacastList.end(); ++it)
    {
      if (!it->checked  &&  it->lastUpdated > lastUpdateTime)
      {
        printf("  -- Add forecast : %u;%s;%s;%s;%s;%u;%d;%d;%s;\n",
            it->producerId,
            it->producerName.c_str(),
            it->analysisTime.c_str(),
            it->generationName.c_str(),
            it->forecastTime.c_str(),
            it->geometryId,
            it->forecastTypeId,
            it->forecastTypeValue,
            it->lastUpdated.c_str());

        addForecast(targetInterface,conn,*it);

        if (it->lastUpdated > lastUpdated)
          lastUpdated = it->lastUpdated;
      }
    }

    if (lastUpdated > lastUpdateTime)
      lastUpdateTime = lastUpdated;
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
    if (argc < 5)
    {
      fprintf(stderr,"USAGE: radon2smartmet <sourceId> <dbConnectionString> [-redis <redisAddress> <redisPort> <tablePrefix>]\n");
      return -1;
    }

    char *configDir = getenv("SMARTMET_GRID_CONFIG_DIR");
    if (configDir == NULL)
    {
      fprintf(stderr,"SMARTMET_GRID_CONFIG_DIR not defined!\n");
      return -2;
    }

    SmartMet::Identification::gribDef.init(configDir);

    ContentServer::ServiceInterface *targetInterface = NULL;

    sourceId = (uint)atoi(argv[1]);
    char *connectionString = argv[2];
    char *connectionType = argv[3];

    if (strcasecmp(connectionType,"-redis") == 0)
    {
      char *redisAddress = (char*)argv[4];
      int redisPort = atoi(argv[5]);
      char *tablePrefix = (char*)argv[6];
      ContentServer::RedisImplementation *redisImplementation = new ContentServer::RedisImplementation();
      redisImplementation->init(redisAddress,redisPort,tablePrefix);
      targetInterface = redisImplementation;
    }

    if (targetInterface == NULL)
    {
      fprintf(stderr,"No target data source defined!\n");
      return -3;
    }

    PGconn *conn = PQconnectdb(connectionString);
    if (PQstatus(conn) != CONNECTION_OK)
    {
      fprintf(stderr,"Postgresql error: %s\n",PQerrorMessage(conn));
      return -4;
    }

    while (true)
    {
      printf("\n");
      printf("********************************************************************\n");
      printf("****************************** UPDATE ******************************\n");
      printf("********************************************************************\n");

      printf("* Reading producer information from the target data storage\n");
      readTargetProducers(targetInterface);
      printf("* Reading producer information from the source data storage\n");
      readSourceProducers(conn);
      printf("* Updating producer information into the target data storage\n");
      updateProducers(targetInterface);
      printf("* Reading updated producer information from the target data storage\n");
      readTargetProducers(targetInterface);

      printf("* Reading generation information from the target data storage\n");
      readTargetGenerations(targetInterface);
      printf("* Reading generation information from the source data storage\n");
      readSourceGenerations(conn);
      printf("* Updating generation information into the target data storage\n");
      updateGenerations(targetInterface);
      readTargetGenerations(targetInterface);
      printf("* Reading updated generation information from the target data storage\n");

      printf("* Reading forecast time information from the source data storage\n");
      readSourceForecastTimes(conn);

      printf("* Updating forecast time information into the target data storage\n");
      updateForecastTimes(targetInterface,conn);

      printf("********************************************************************\n\n");

      sleep(300);
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

