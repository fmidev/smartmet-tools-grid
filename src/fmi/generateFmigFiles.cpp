#include "grid-content/contentServer/corba/client/ClientImplementation.h"
#include "grid-content/contentServer/redis/RedisImplementation.h"
#include "grid-content/contentServer/cache/CacheImplementation.h"
#include "grid-content/dataServer/corba/client/ClientImplementation.h"
#include "grid-content/dataServer/cache/CacheImplementation.h"
#include "grid-content/dataServer/implementation/ServiceImplementation.h"
#include "grid-content/dataServer/implementation/VirtualContentFactory_type1.h"
#include "grid-files/common/Exception.h"
#include "grid-files/identification/GridDef.h"
#include "grid-files/common/Typedefs.h"
#include "grid-files/common/GeneralFunctions.h"
#include "grid-files/common/ShowFunction.h"
#include "grid-files/common/FileWriter.h"

#include <iostream>
#include <stdexcept>
#include <string.h>
#include <vector>
#include <pthread.h>
#include <sys/time.h>
#include <signal.h>

#define FUNCTION_TRACE FUNCTION_TRACE_OFF

using namespace SmartMet;

struct ParamRec
{
    std::string producerName;
    std::string parameter;
    T::GeometryId geometryId;
    T::ParamLevelId levelId;
    T::ParamLevel level;
    T::ForecastType forecastType;
    T::ForecastNumber forecastNumber;
};

std::vector<ParamRec> mParamList;
std::set<std::string> mProducerList;

ContentServer::Corba::ClientImplementation *contentServer = nullptr;
DataServer::Corba::ClientImplementation *dataServer = nullptr;

bool mShutdownRequested = false;

ConfigurationFile mConfigurationFile;
std::string mGridConfigFile;
uint mWaitTime = 0;

std::string mContentServerIor;
bool mContentServerProcessingLogEnabled;
std::string mContentServerProcessingLogFile;
int mContentServerProcessingLogMaxSize = 100000000;
int mContentServerProcessingLogTruncateSize = 20000000;
Log mContentServerProcessingLog;
bool mContentServerDebugLogEnabled = false;
std::string mContentServerDebugLogFile;
int mContentServerDebugLogMaxSize = 100000000;
int mContentServerDebugLogTruncateSize = 20000000;
Log mContentServerDebugLog;

std::string mDataServerIor;
bool mDataServerProcessingLogEnabled = false;
std::string mDataServerProcessingLogFile;
int mDataServerProcessingLogMaxSize = 100000000;
int mDataServerProcessingLogTruncateSize = 20000000;
Log mDataServerProcessingLog;
bool mDataServerDebugLogEnabled = false;
std::string mDataServerDebugLogFile;
int mDataServerDebugLogMaxSize = 100000000;
int mDataServerDebugLogTruncateSize = 20000000;
Log mDataServerDebugLog;

std::string mTargetDir;
std::string mParameterFile;
std::string mProducerPrefix;
bool mContentStorageEnabled = false;
std::string mContentStorageType;
std::string mContentStorageRedisAddress = "127.0.0.1";
uint mContentStorageRedisPort = 6379;
std::string mContentStorageRedisTablePrefix = "a.";
std::string mContentStorageCorbaIor;
std::string mContentStorageHttpUrl;
bool mDebugLogEnabled = false;
std::string mDebugLogFile = "/devb/stdout";
uint mDebugLogMaxSize = 100000000;
uint mDebugLogTruncateSize = 50000000;

Log mDebugLog;
Log* mDebugLogPtr = nullptr;




uint getTimesteps(const char *filename)
{
  FUNCTION_TRACE
  try
  {
    FILE *file = fopen(filename,"r");
    if (file == nullptr)
      return 0;

    fseek(file,6,SEEK_SET);

    uchar v[4];
    fread(v,4,1,file);
    fclose(file);

    uint a = v[0];
    uint b = v[1];
    uint c = v[2];
    uint d = v[3];
    uint len =  static_cast<unsigned int>((a << 24 | b << 16 | c << 8 | d));

    return len;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,nullptr);
  }
}




void readConfigFile(const char* configFile)
{
  try
  {
    const char *configAttribute[] =
    {
        "smartmet.library.grid-files.configFile",
        "smartmet.tools.grid.content-server.ior",
        "smartmet.tools.grid.content-server.processing-log.enabled",
        "smartmet.tools.grid.content-server.processing-log.file",
        "smartmet.tools.grid.content-server.processing-log.maxSize",
        "smartmet.tools.grid.content-server.processing-log.truncateSize",
        "smartmet.tools.grid.content-server.debug-log.enabled",
        "smartmet.tools.grid.content-server.debug-log.file",
        "smartmet.tools.grid.content-server.debug-log.maxSize",
        "smartmet.tools.grid.content-server.debug-log.truncateSize",
        "smartmet.tools.grid.data-server.ior",
        "smartmet.tools.grid.data-server.processing-log.enabled",
        "smartmet.tools.grid.data-server.processing-log.file",
        "smartmet.tools.grid.data-server.processing-log.maxSize",
        "smartmet.tools.grid.data-server.processing-log.truncateSize",
        "smartmet.tools.grid.data-server.debug-log.enabled",
        "smartmet.tools.grid.data-server.debug-log.file",
        "smartmet.tools.grid.data-server.debug-log.maxSize",
        "smartmet.tools.grid.data-server.debug-log.truncateSize",
        "smartmet.tools.grid.generateFmigFiles.parameterFile",
        "smartmet.tools.grid.generateFmigFiles.targetDir",
        "smartmet.tools.grid.generateFmigFiles.producerPrefix",
        "smartmet.tools.grid.generateFmigFiles.content-storage.enabled",
        "smartmet.tools.grid.generateFmigFiles.content-storage.type",
        "smartmet.tools.grid.generateFmigFiles.content-storage.redis.address",
        "smartmet.tools.grid.generateFmigFiles.content-storage.redis.port",
        "smartmet.tools.grid.generateFmigFiles.content-storage.redis.tablePrefix",
        "smartmet.tools.grid.generateFmigFiles.content-storage.corba.ior",
        "smartmet.tools.grid.generateFmigFiles.content-storage.http.url",
        "smartmet.tools.grid.generateFmigFiles.debug-log.enabled",
        "smartmet.tools.grid.generateFmigFiles.debug-log.file",
        "smartmet.tools.grid.generateFmigFiles.debug-log.maxSize",
        "smartmet.tools.grid.generateFmigFiles.debug-log.truncateSize", nullptr };

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

    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.ior", mContentServerIor);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.processing-log.enabled", mContentServerProcessingLogEnabled);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.processing-log.file", mContentServerProcessingLogFile);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.processing-log.maxSize", mContentServerProcessingLogMaxSize);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.processing-log.truncateSize", mContentServerProcessingLogTruncateSize);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.debug-log.enabled", mContentServerDebugLogEnabled);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.debug-log.file", mContentServerDebugLogFile);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.debug-log.maxSize", mContentServerDebugLogMaxSize);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.debug-log.truncateSize", mContentServerDebugLogTruncateSize);

    mConfigurationFile.getAttributeValue("smartmet.tools.grid.data-server.ior", mDataServerIor);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.data-server.processing-log.enabled", mDataServerProcessingLogEnabled);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.data-server.processing-log.file", mDataServerProcessingLogFile);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.data-server.processing-log.maxSize", mDataServerProcessingLogMaxSize);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.data-server.processing-log.truncateSize", mDataServerProcessingLogTruncateSize);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.data-server.debug-log.enabled", mDataServerDebugLogEnabled);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.data-server.debug-log.file", mDataServerDebugLogFile);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.data-server.debug-log.maxSize", mDataServerDebugLogMaxSize);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.data-server.debug-log.truncateSize", mDataServerDebugLogTruncateSize);

    mConfigurationFile.getAttributeValue("smartmet.tools.grid.generateFmigFiles.parameterFile", mParameterFile);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.generateFmigFiles.targetDir", mTargetDir);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.generateFmigFiles.producerPrefix", mProducerPrefix);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.generateFmigFiles.content-storage.enabled", mContentStorageEnabled);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.generateFmigFiles.content-storage.type", mContentStorageType);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.generateFmigFiles.content-storage.redis.address", mContentStorageRedisAddress);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.generateFmigFiles.content-storage.redis.port", mContentStorageRedisPort);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.generateFmigFiles.content-storage.redis.tablePrefix", mContentStorageRedisTablePrefix);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.generateFmigFiles.content-storage.corba.ior", mContentStorageCorbaIor);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.generateFmigFiles.content-storage.http.url", mContentStorageHttpUrl);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.generateFmigFiles.debug-log.enabled", mDebugLogEnabled);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.generateFmigFiles.debug-log.file", mDebugLogFile);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.generateFmigFiles.debug-log.maxSize", mDebugLogMaxSize);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.generateFmigFiles.debug-log.truncateSize", mDebugLogTruncateSize);

    // Initializing information that is needed for identifying the content of the grid files.

    SmartMet::Identification::gridDef.init(mGridConfigFile.c_str());
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Constructor failed!", nullptr);
  }
}





void readParameterFile(const char *filename)
{
  FUNCTION_TRACE
  try
  {
    FILE *file = fopen(filename, "re");
    if (file == nullptr)
    {
      SmartMet::Spine::Exception exception(BCP, "Cannot open the parameter file!");
      exception.addParameter("Filename", std::string(filename));
      throw exception;
    }

    mParamList.clear();
    mProducerList.clear();

    char st[1000];

    while (!feof(file))
    {
      if (fgets(st, 1000, file) != nullptr && st[0] != '#')
      {
        bool ind = false;
        char *field[100];
        uint c = 1;
        field[0] = st;
        char *p = st;
        while (*p != '\0' && c < 100)
        {
          if (*p == '"')
            ind = !ind;

          if ((*p == ':' || *p == '\n') && !ind)
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

        if (c > 6)
        {
          ParamRec rec;

          if (field[0][0] != '\0')
            rec.parameter = field[0];

          if (field[1][0] != '\0')
          {
            rec.producerName = toUpperString(std::string(field[1]));
            if (mProducerList.find(rec.producerName) == mProducerList.end())
              mProducerList.insert(rec.producerName);
          }

          if (field[2][0] != '\0')
            rec.geometryId = toInt64(field[2]);
          else
            rec.geometryId = -1;

          if (field[3][0] != '\0')
            rec.levelId = toInt64(field[3]);
          else
            rec.levelId = 0;

          if (field[4][0] != '\0')
            rec.level = toInt64(field[4]);
          else
            rec.level = 0;

          if (field[5][0] != '\0')
            rec.forecastType = toInt64(field[5]);
          else
            rec.forecastType = -1;

          if (field[6][0] != '\0')
            rec.forecastNumber = toInt64(field[6]);
          else
            rec.forecastNumber = -1;

          mParamList.push_back(rec);
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





void processParameters()
{

  try
  {
    // ### Session:
    T::SessionId sessionId = 0;

    T::ParamKeyType paramKeyType = T::ParamKeyTypeValue::FMI_NAME;
    T::ParamLevelIdType parameterLevelIdType = T::ParamLevelIdTypeValue::FMI;
    std::string start = "15000101T000000";
    std::string end = "30000101T000000";
    uint requestFlags = 0;

    std::set<std::string> filenameList;

    for (auto producer = mProducerList.begin(); producer != mProducerList.end(); ++producer)
    {
      PRINT_DATA(mDebugLogPtr, "PRODUCER : %s\n",producer->c_str());

      T::ProducerInfo producerInfo;

      int result = contentServer->getProducerInfoByName(sessionId, *producer, producerInfo);
      if (result == 0)
      {
        T::GenerationInfoList generationInfoList;

        result = contentServer->getGenerationInfoListByProducerId(sessionId, producerInfo.mProducerId, generationInfoList);
        if (result == 0  &&  generationInfoList.getLength() > 0)
        {
          T::GenerationInfo *gLastInfo = generationInfoList.getLastGenerationInfoByProducerIdAndStatus(producerInfo.mProducerId,T::GenerationInfo::Status::Ready);
          uint gLen = generationInfoList.getLength();
          for (uint g = 0; g < gLen; g++)
          {
            T::GenerationInfo *gInfo = generationInfoList.getGenerationInfoByIndex(g);

            PRINT_DATA(mDebugLogPtr, "  - Generation : %s\n",gInfo->mName.c_str());

            for (auto param = mParamList.begin(); param != mParamList.end(); ++param)
            {
              if (param->producerName == *producer)
              {
                PRINT_DATA(mDebugLogPtr, "     * Parameter : %s:%s:%d:%d:%d:%d:%d\n",param->parameter.c_str(),param->producerName.c_str(),param->geometryId,param->levelId, param->level, param->forecastType, param->forecastNumber);

                T::ContentInfoList contentInfoList;
                result = contentServer->getContentListByParameterAndGenerationId(sessionId, gInfo->mGenerationId, paramKeyType, param->parameter, parameterLevelIdType,
                    param->levelId, param->level, param->level, param->forecastType, param->forecastNumber, param->geometryId, start, end, requestFlags, contentInfoList);

                PRINT_DATA(mDebugLogPtr,"        -- Timesteps : %u\n",contentInfoList.getLength());
                if (result == 0)
                {
                  uint len = contentInfoList.getLength();
                  if (len > 0)
                  {
                    T::GridData data[len];

                    T::ContentInfo *cInfo = contentInfoList.getContentInfoByIndex(0);

                    char filename[300];
                    sprintf(filename, "%s%s_%s_%s_%d_%d_%d_%d_%d.fmig", mProducerPrefix.c_str(),producerInfo.mName.c_str(), gInfo->mAnalysisTime.c_str(), cInfo->mFmiParameterName.c_str(),
                        cInfo->mGeometryId, cInfo->mFmiParameterLevelId, cInfo->mParameterLevel, cInfo->mForecastType, cInfo->mForecastNumber);
                    filenameList.insert(std::string(filename));

                    sprintf(filename, "%s/%s%s_%s_%s_%d_%d_%d_%d_%d.fmig", mTargetDir.c_str(), mProducerPrefix.c_str(),producerInfo.mName.c_str(), gInfo->mAnalysisTime.c_str(), cInfo->mFmiParameterName.c_str(),
                        cInfo->mGeometryId, cInfo->mFmiParameterLevelId, cInfo->mParameterLevel, cInfo->mForecastType, cInfo->mForecastNumber);

                    //if (gInfo == gLastInfo)
                    {
                      if (getFileSize(filename) >= 0)
                      {
                        uint timeSteps = getTimesteps(filename);
                        PRINT_DATA(mDebugLogPtr,"        -- Timesteps currently available %u / %u\n",timeSteps,len);
                        if (timeSteps < len)
                          remove(filename);
                      }

                      if (getFileSize(filename) <= 0)
                      {
                        for (uint t = 0; t < len; t++)
                        {
                          cInfo = contentInfoList.getContentInfoByIndex(t);
                          PRINT_DATA(mDebugLogPtr,"           * %u : ForecastTime : %s\n",t,cInfo->mForecastTime.c_str());
                          dataServer->getGridData(sessionId, cInfo->mFileId, cInfo->mMessageIndex, data[t]);
                          data[t].mForecastTime = cInfo->mForecastTime;
                        }

                        char buf[10000];

                        FileWriter fileWriter;
                        fileWriter.setLittleEndian(false);

                        fileWriter.createFile(filename);

                        const char *ftype = "FMIG";
                        uchar version = 1;
                        uchar format = 1;

                        fileWriter.write_data((void*) ftype, 4);
                        fileWriter << version;
                        fileWriter << format;
                        fileWriter << len;
                        fileWriter << data[0].mColumns;
                        fileWriter << data[0].mRows;

                        char *p = buf;
                        p += sprintf(p, "producer.id\t%u\t", cInfo->mProducerId);
                        p += sprintf(p, "producer.name\t%s\t", producerInfo.mName.c_str());
                        p += sprintf(p, "geometry.id\t%d\t", cInfo->mGeometryId);
                        p += sprintf(p, "generation.id\t%u\t", cInfo->mGenerationId);
                        p += sprintf(p, "generation.name\t%s\t", gInfo->mName.c_str());
                        p += sprintf(p, "param.fmi.id\t%s\t", cInfo->mFmiParameterId.c_str());
                        p += sprintf(p, "param.fmi.name\t%s\t", cInfo->mFmiParameterName.c_str());
                        p += sprintf(p, "param.grib.id\t%s\t", cInfo->mGribParameterId.c_str());
                        p += sprintf(p, "param.cdm.id\t%s\t", cInfo->mCdmParameterId.c_str());
                        p += sprintf(p, "param.cdm.name\t%s\t", cInfo->mCdmParameterName.c_str());
                        p += sprintf(p, "param.newbase.id\t%s\t", cInfo->mNewbaseParameterId.c_str());
                        p += sprintf(p, "param.newbase.name\t%s\t", cInfo->mNewbaseParameterName.c_str());
                        p += sprintf(p, "param.level.fmi.id\t%u\t", cInfo->mFmiParameterLevelId);
                        p += sprintf(p, "param.level.grib1.id\t%u\t", cInfo->mGrib1ParameterLevelId);
                        p += sprintf(p, "param.level.grib2.id\t%u\t", cInfo->mGrib2ParameterLevelId);
                        p += sprintf(p, "param.level.value\t%d\t", cInfo->mParameterLevel);
                        p += sprintf(p, "param.units\t%s\t", cInfo->mFmiParameterUnits.c_str());
                        p += sprintf(p, "param.forecast.type\t%d\t", cInfo->mForecastType);
                        p += sprintf(p, "param.forecast.number\t%d\t", cInfo->mForecastNumber);

                        uint headerLen = (uint)(p - buf);
                        fileWriter << headerLen;
                        fileWriter.write_data(buf, headerLen);

                        for (uint t = 0; t < len; t++)
                        {
                          short year = 0;
                          uchar month = 0, day = 0, hour = 0, minute = 0, second = 0;
                          splitTimeString(data[t].mForecastTime, year, month, day, hour, minute, second);

                          fileWriter << year;
                          fileWriter << month;
                          fileWriter << day;
                          fileWriter << hour;
                          fileWriter << minute;
                          fileWriter << second;
                        }

                        uint sz = data[0].mValues.size();
                        for (uint s = 0; s < sz; s++)
                        {
                          float min = ParamValueMissing;
                          float max = ParamValueMissing;
                          for (uint t = 0; t < len; t++)
                          {
                            float v = ParamValueMissing;

                            if (data[t].mValues.size() > s)
                              v = data[t].mValues[s];

                            if (v != ParamValueMissing)
                            {
                              if (min == ParamValueMissing || v < min)
                                min = v;

                              if (max == ParamValueMissing || v > max)
                                max = v;
                            }
                          }

                          fileWriter << min;
                          fileWriter << max;

                          for (uint t = 0; t < len; t++)
                          {
                            //printf("%u/%u/%lu/%u\n",s,data[t].mValues.size(),t);
                            float d = (max - min) / 65530;
                            float v = ParamValueMissing;

                            if (data[t].mValues.size() > s)
                              v = data[t].mValues[s];

                            ushort vv = 0xFFFF;
                            if (v != ParamValueMissing)
                              vv = (ushort)((v - min) / d);

                            fileWriter << vv;
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }

    std::vector<std::string> filePatterns;
    filePatterns.push_back(std::string("*.fmig"));
    std::set<std::string> dirList;
    std::vector<std::pair<std::string,std::string>> fileList;

    getFileList(mTargetDir.c_str(),filePatterns,false,dirList,fileList);


    for (auto it = fileList.begin(); it != fileList.end(); ++it)
    {
      std::string fname = it->first + "/" + it->second;
      if (filenameList.find(it->second) == filenameList.end())
      {
        std::cout << "REMOVE : " << fname << "\n";
        remove(fname.c_str());
      }
    }

  }
  catch (SmartMet::Spine::Exception& e)
  {
    SmartMet::Spine::Exception exception(BCP,"Service call failed!",nullptr);
    throw exception;
  }
}





int main(int argc, char *argv[])
{
  try
  {
    if (argc != 3)
    {
      fprintf(stderr,"USAGE: generateFmigFiles <configFile> <loopWaitTime>\n");
      return -1;
    }

    //signal(SIGINT, sig_handler);

    readConfigFile(argv[1]);
    mWaitTime = toInt64(argv[2]);

    contentServer = new ContentServer::Corba::ClientImplementation();
    contentServer->init(mContentServerIor);

    dataServer = new DataServer::Corba::ClientImplementation();
    dataServer->init(mDataServerIor);

    if (mContentServerProcessingLogEnabled && mContentServerProcessingLogFile.length() > 0)
    {
      mContentServerProcessingLog.init(true,mContentServerProcessingLogFile.c_str(),mContentServerProcessingLogMaxSize,mContentServerProcessingLogTruncateSize);
      contentServer->setProcessingLog(&mContentServerProcessingLog);
    }

    if (mContentServerDebugLogEnabled && mContentServerDebugLogFile.length() > 0)
    {
      mContentServerDebugLog.init(true,mContentServerDebugLogFile.c_str(),mContentServerDebugLogMaxSize,mContentServerDebugLogTruncateSize);
      contentServer->setDebugLog(&mContentServerDebugLog);
    }

    if (mDataServerProcessingLogEnabled && mDataServerProcessingLogFile.length() > 0)
    {
      mDataServerProcessingLog.init(true,mDataServerProcessingLogFile.c_str(),mDataServerProcessingLogMaxSize,mDataServerProcessingLogTruncateSize);
      dataServer->setProcessingLog(&mDataServerProcessingLog);
    }

    if (mDataServerDebugLogEnabled && mDataServerDebugLogFile.length() > 0)
    {
      mDataServerDebugLog.init(true,mDataServerDebugLogFile.c_str(),mDataServerDebugLogMaxSize,mDataServerDebugLogTruncateSize);
      dataServer->setDebugLog(&mDataServerDebugLog);
    }

    if (mDebugLogEnabled)
    {
      mDebugLog.init(true,mDebugLogFile.c_str(),mDebugLogMaxSize,mDebugLogTruncateSize);
      mDebugLogPtr = &mDebugLog;
    }

    bool ind = true;
    while (ind)
    {
      PRINT_DATA(mDebugLogPtr, "\n");
      PRINT_DATA(mDebugLogPtr, "********************************************************************\n");

      readParameterFile(mParameterFile.c_str());
      processParameters();

      if (mWaitTime > 0)
      {
        sleep(mWaitTime);
      }
      else
      {
        ind = false;
      }
    }

    delete contentServer;
    delete dataServer;
    return 0;
  }
  catch (...)
  {
    SmartMet::Spine::Exception exception(BCP,exception_operation_failed,nullptr);
    exception.printError();
    return -1;
  }
}
