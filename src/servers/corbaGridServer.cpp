#include "grid-content/contentServer/corba/client/ClientImplementation.h"
#include "grid-content/contentServer/redis/RedisImplementation.h"
#include "grid-content/contentServer/cache/CacheImplementation.h"
#include "grid-content/contentServer/http/client/ClientImplementation.h"
#include "grid-content/contentServer/corba/server/Server.h"
#include "grid-content/contentServer/corba/server/ServerInterface.h"
#include "grid-content/dataServer/cache/CacheImplementation.h"
#include "grid-content/dataServer/implementation/ServiceImplementation.h"
#include "grid-content/dataServer/implementation/VirtualContentFactory_type1.h"
#include "grid-content/dataServer/implementation/ServiceImplementation.h"
#include "grid-content/dataServer/corba/server/ServerInterface.h"
#include "grid-content/dataServer/corba/server/Server.h"
#include "grid-content/queryServer/corba/server/ServerInterface.h"
#include "grid-content/queryServer/corba/server/Server.h"
#include "grid-content/queryServer/corba/server/GridServer.h"
#include "grid-content/queryServer/implementation/ServiceImplementation.h"
#include <macgyver/Exception.h>
#include "grid-files/identification/GridDef.h"
#include "grid-files/common/Typedefs.h"
#include "grid-files/common/GeneralFunctions.h"

#include <iostream>
#include <stdexcept>
#include <string.h>
#include <vector>
#include <pthread.h>
#include <sys/time.h>
#include <signal.h>


using namespace SmartMet;


ContentServer::Corba::ClientImplementation *corbaContentClient = nullptr;
ContentServer::HTTP::ClientImplementation *httpContentClient = nullptr;
ContentServer::RedisImplementation *redisImplementation = nullptr;
ContentServer::CacheImplementation *cacheImplementation = nullptr;


QueryServer::Corba::GridServer *corbaServer = nullptr;
ContentServer::ServiceInterface *contentService = nullptr;
DataServer::ServiceImplementation *dataService = nullptr;
QueryServer::ServiceImplementation *queryService = nullptr;

bool mShutdownRequested = false;

ConfigurationFile   mConfigurationFile;
Log                 mContentServerProcessingLog;
Log                 mContentServerDebugLog;
Log                 mDataServerProcessingLog;
Log                 mDataServerDebugLog;
Log                 mQueryServerProcessingLog;
Log                 mQueryServerDebugLog;
pthread_t           mThread;
time_t              mParameterMappingUpdateTime = 0;

QueryServer::AliasFileCollection mProducerAliasFileCollection;

std::string grid_files_configFile;
uint grid_files_cache_numOfGrids = 0;
uint grid_files_cache_maxSizeInMegaBytes;
bool grid_files_requestCounter_enabled = false;
std::string corba_server_address;
std::string corba_server_port;
std::string content_server_iorFile;
std::string content_server_content_source_type;
std::string content_server_content_source_redis_address;
int content_server_content_source_redis_port = 6379;
std::string content_server_content_source_redis_tablePrefix;
std::string content_server_content_source_corba_ior;
std::string content_server_content_source_http_url;
bool content_server_cache_enabled = false;
bool content_server_cache_requestForwardEnabled = false;
uint content_server_cache_eventListMaxSize = 10;
bool content_server_processing_log_enabled = false;
std::string content_server_processing_log_file;
uint content_server_processing_log_maxSize = 0;
uint content_server_processing_log_truncateSize = 0;
bool content_server_debug_log_enabled = false;
std::string content_server_debug_log_file;
uint content_server_debug_log_maxSize = 0;
uint content_server_debug_log_truncateSize = 0;
std::string data_server_iorFile;
std::string data_server_name;
uint data_server_id = 0;
std::string data_server_grid_storage_directory;
bool data_server_virtualFiles_enabled = false;
bool data_server_memoryMapCheck_enabled = false;
std::string data_server_virtualFiles_definitionFile;
std::vector<std::string> data_server_luaFiles;
bool data_server_processing_log_enabled = false;
std::string data_server_processing_log_file;
uint data_server_processing_log_maxSize = 0;
uint data_server_processing_log_truncateSize = 0;
bool data_server_debug_log_enabled = false;
std::string data_server_debug_log_file;
uint data_server_debug_log_maxSize = 0;
uint data_server_debug_log_truncateSize = 0;
bool data_server_methods_enabled = false;
std::string query_server_producerFile;
std::string query_server_iorFile;
bool query_server_checkGeometryStatus = false;
std::vector<std::string> query_server_producerAliasFiles;
uint query_server_mappingTargetKeyType = 0;
std::string query_server_mappingUpdateFile_fmi;
std::string query_server_mappingUpdateFile_newbase;
std::string query_server_mappingUpdateFile_netCdf;
std::vector<std::string> query_server_luaFiles;
std::vector<std::string> query_server_mappingFiles;
std::vector<std::string> query_server_aliasFiles;
bool query_server_processing_log_enabled = false;
std::string query_server_processing_log_file;
uint query_server_processing_log_maxSize = 0;
uint query_server_processing_log_truncateSize = 0;
bool query_server_debug_log_enabled = false;
std::string query_server_debug_log_file;
uint query_server_debug_log_maxSize = 0;
uint query_server_debug_log_truncateSize = 0;






void loadMappings(QueryServer::ParamMappingFile_vec& parameterMappings)
{
  try
  {
    for (auto it = query_server_mappingFiles.begin(); it != query_server_mappingFiles.end(); ++it)
    {
      QueryServer::ParameterMappingFile mapping(*it);
      parameterMappings.push_back(mapping);
    }

    for (auto it = parameterMappings.begin(); it != parameterMappings.end(); ++it)
    {
      it->init();
    }
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}





FILE* openMappingFile(std::string mappingFile)
{
  try
  {
    FILE *file = fopen(mappingFile.c_str(),"we");
    if (file == nullptr)
    {
      Fmi::Exception exception(BCP, "Cannot open a mapping file for writing!");
      exception.addParameter("Filaname",mappingFile);
      throw exception;
    }

    fprintf(file,"# This file is automatically generated by the grid engine. The file contains\n");
    fprintf(file,"# mappings for the parameters found from the content server, which do not have\n");
    fprintf(file,"# mappings already defined. The point is that the query server cannot find \n");
    fprintf(file,"# requested parameters without mappings. On the other hand, the order of the mappings\n");
    fprintf(file,"# is also the search order of the parameters that do not contain complete search \n");
    fprintf(file,"# information (parameterIdType,levelIdType,levelId,level,etc.)\n");
    fprintf(file,"# \n");
    fprintf(file,"# If you want to change some of the mappings or their order, then you should move\n");
    fprintf(file,"# them to a permanent mapping file (which is not automatically overridden.\n");
    fprintf(file,"# \n");
    fprintf(file,"# FIELDS:\n");
    fprintf(file,"#  1) Producer name\n");
    fprintf(file,"#  2) Mapping name\n");
    fprintf(file,"#  3) Parameter id type:\n");
    fprintf(file,"#         1 = FMI_ID\n");
    fprintf(file,"#         2 = FMI_NAME\n");
    fprintf(file,"#         3 = GRIB_ID\n");
    fprintf(file,"#         4 = NEWBASE_ID\n");
    fprintf(file,"#         5 = NEWBASE_NAME\n");
    fprintf(file,"#         6 = NETCDF_NAME\n");
    fprintf(file,"#  4) Parameter id / name\n");
    fprintf(file,"#  5) Geometry id\n");
    fprintf(file,"#  6) Parameter level id type:\n");
    fprintf(file,"#         1 = FMI\n");
    fprintf(file,"#         2 = GRIB1\n");
    fprintf(file,"#         3 = GRIB2\n");
    fprintf(file,"#  7) Level id\n");
    fprintf(file,"#         FMI level identifiers:\n");
    fprintf(file,"#            1 Gound or water surface\n");
    fprintf(file,"#            2 Pressure level\n");
    fprintf(file,"#            3 Hybrid level\n");
    fprintf(file,"#            4 Altitude\n");
    fprintf(file,"#            5 Top of atmosphere\n");
    fprintf(file,"#            6 Height above ground in meters\n");
    fprintf(file,"#            7 Mean sea level\n");
    fprintf(file,"#            8 Entire atmosphere\n");
    fprintf(file,"#            9 Depth below land surface\n");
    fprintf(file,"#            10 Depth below some surface\n");
    fprintf(file,"#            11 Level at specified pressure difference from ground to level\n");
    fprintf(file,"#            12 Max equivalent potential temperature level\n");
    fprintf(file,"#            13 Layer between two metric heights above ground\n");
    fprintf(file,"#            14 Layer between two depths below land surface\n");
    fprintf(file,"#            15 Isothermal level, temperature in 1/100 K\n");
    fprintf(file,"#  8) Level\n");
    fprintf(file,"#  9) Area interpolation method\n");
    fprintf(file,"#         0 = None\n");
    fprintf(file,"#         1 = Linear\n");
    fprintf(file,"#         2 = Nearest\n");
    fprintf(file,"#         3 = Min\n");
    fprintf(file,"#         4 = Max\n");
    fprintf(file,"#         9 = Landscape\n");
    fprintf(file,"#         10 = Forbidden\n");
    fprintf(file,"#         500..999 = List\n");
    fprintf(file,"#         1000..65535 = External (interpolated by an external function)\n");
    fprintf(file,"# 10) Time interpolation method\n");
    fprintf(file,"#         0 = None\n");
    fprintf(file,"#         1 = Linear\n");
    fprintf(file,"#         2 = Nearest\n");
    fprintf(file,"#         3 = Min\n");
    fprintf(file,"#         4 = Max\n");
    fprintf(file,"#         6 = Previous\n");
    fprintf(file,"#         7 = Next\n");
    fprintf(file,"#         1000..65535 = External (interpolated by an external function)\n");
    fprintf(file,"# 11) Level interpolation method\n");
    fprintf(file,"#         0 = None\n");
    fprintf(file,"#         1 = Linear\n");
    fprintf(file,"#         2 = Nearest\n");
    fprintf(file,"#         3 = Min\n");
    fprintf(file,"#         4 = Max\n");
    fprintf(file,"#         5 = Logarithmic\n");
    fprintf(file,"#         6 = Previous\n");
    fprintf(file,"#         7 = Next\n");
    fprintf(file,"#         1000..65535 = External (interpolated by an external function)\n");
    fprintf(file,"# 12) Group flags\n");
    fprintf(file,"#         bit 0 = Climatological parameter (=> ignore year when searching) \n");
    fprintf(file,"#         bit 1 = Global parameter (=> ignore timestamp when searching, for example LandSeaMask)\n");
    fprintf(file,"# 13) Search match (Can this mapping used when searching mappings for incomplete parameters)\n");
    fprintf(file,"#         E = Enabled\n");
    fprintf(file,"#         D = Disabled\n");
    fprintf(file,"#         I = Ignore\n");
    fprintf(file,"# 14) Mapping function (enables data conversions during the mapping)\n");
    fprintf(file,"# 15) Reverse mapping function\n");
    fprintf(file,"# 16) Default precision\n");
    fprintf(file,"# \n");

    return file;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}





void updateMappings(T::ParamKeyType sourceParameterKeyType,T::ParamKeyType targetParameterKeyType,std::string mappingFile,QueryServer::ParamMappingFile_vec& parameterMappings)
{
  try
  {
    T::SessionId sessionId = 0;
    uint numOfNewMappings = 0;
    std::set<std::string> mapList;
    std::set<std::string> searchList;

    T::ProducerInfoList producerInfoList;
    int result = contentService->getProducerInfoList(sessionId,producerInfoList);
    if (result != 0)
    {
      std::cerr << __FILE__ << ":" << __LINE__ << ": The 'contentServer.getProducerInfoList()' service call returns an error!  Result : " << result << " : " << ContentServer::getResultString(result).c_str() << "\n";
      return;
    }

    FILE *file = nullptr;

    uint plen = producerInfoList.getLength();
    for (uint t=0; t<plen; t++)
    {
      T::ProducerInfo *producerInfo = producerInfoList.getProducerInfoByIndex(t);
      std::set<std::string> infoList;

      int result = contentService->getProducerParameterListByProducerId(sessionId,producerInfo->mProducerId,sourceParameterKeyType,targetParameterKeyType,infoList);
      if (result == 0)
      {
        for (auto it=infoList.begin(); it != infoList.end(); ++it)
        {
          std::vector<std::string> pl;
          splitString(it->c_str(),';',pl);
          if (pl.size() >= 8)
          {
            QueryServer::ParameterMapping m;
            m.mProducerName = pl[0];
            m.mParameterName = pl[1];
            m.mParameterKeyType = toInt16(pl[2].c_str());
            m.mParameterKey = pl[3];
            m.mGeometryId = toInt32(pl[4].c_str());
            //m.mParameterLevelIdType = toInt16(pl[5].c_str());
            m.mParameterLevelId = toInt16(pl[6].c_str());
            m.mParameterLevel = toInt32(pl[7].c_str());

            char key[200];
            sprintf(key,"%s;%s;%s;%s;%s;%s;%s;%s;",pl[0].c_str(),pl[1].c_str(),pl[2].c_str(),pl[3].c_str(),pl[4].c_str(),pl[5].c_str(),pl[6].c_str(),pl[7].c_str());
            std::string searchKey = m.mProducerName + ":" + m.mParameterName + std::to_string(m.mGeometryId);

            if (mapList.find(std::string(key)) == mapList.end())
            {
              mapList.insert(std::string(key));

              bool found = false;
              bool searchEnabled = false;
              for (auto it = parameterMappings.begin(); it != parameterMappings.end() && !found; ++it)
              {
                if (it->getFilename() != mappingFile)
                {
                  if (it->getMapping(m) != nullptr)
                  {
                    found = true;
                  }
                  else
                  {
                    if (searchList.find(searchKey) != searchList.end())
                    {
                      searchEnabled = true;
                    }
                    else
                    {
                      QueryServer::ParameterMapping_vec vec;
                      it->getMappings(m.mProducerName,m.mParameterName,m.mGeometryId,true,vec);
                      if (vec.size() > 0)
                      {
                        searchEnabled = true;
                      }
                    }
                  }
                }
              }

              if (!found)
              {
                numOfNewMappings++;

                char s = 'D';
                if (!searchEnabled  ||  (m.mParameterLevelId == 6   &&  m.mParameterLevel <= 10) ||  (m.mParameterLevelId == 1  &&  m.mParameterLevel == 0))
                {
                  if (m.mParameterLevelId != 2  &&  m.mParameterLevelId != 3  &&  m.mParameterLevelId != 4)
                    s = 'E';
                }

                if (searchList.find(searchKey) == searchList.end())
                  searchList.insert(searchKey);

                if (file == nullptr)
                  file = openMappingFile(mappingFile);

                fprintf(file,"%s;%s;%s;%s;%s;%s;%s;%s;",pl[0].c_str(),pl[1].c_str(),pl[2].c_str(),pl[3].c_str(),pl[4].c_str(),pl[5].c_str(),pl[6].c_str(),pl[7].c_str());

                Identification::FmiParameterDef paramDef;

                bool found = false;
                if (targetParameterKeyType == T::ParamKeyTypeValue::FMI_NAME)
                  found = Identification::gridDef.getFmiParameterDefByName(pl[3],paramDef);
                else
                if (targetParameterKeyType == T::ParamKeyTypeValue::FMI_ID)
                  found = Identification::gridDef.getFmiParameterDefById(toUInt32(pl[3]),paramDef);
                else
                if (targetParameterKeyType == T::ParamKeyTypeValue::NEWBASE_ID)
                  found = Identification::gridDef.getFmiParameterDefByNewbaseId(toUInt32(pl[3]),paramDef);

                if (found)
                {
                  if (paramDef.mAreaInterpolationMethod >= 0)
                    fprintf(file,"%d;",paramDef.mAreaInterpolationMethod);
                  else
                    fprintf(file,";");

                  if (paramDef.mTimeInterpolationMethod >= 0)
                    fprintf(file,"%d;",paramDef.mTimeInterpolationMethod);
                  else
                    fprintf(file,";");

                  if (paramDef.mLevelInterpolationMethod >= 0)
                    fprintf(file,"%d;",paramDef.mLevelInterpolationMethod);
                  else
                    fprintf(file,";");

                  fprintf(file,"0;%c;",s);

                  if (sourceParameterKeyType == T::ParamKeyTypeValue::NEWBASE_ID || sourceParameterKeyType == T::ParamKeyTypeValue::NEWBASE_NAME)
                  {
                    Identification::FmiParameterId_newbase paramMapping;
                    if (Identification::gridDef.getNewbaseParameterMappingByFmiId(paramDef.mFmiParameterId,paramMapping))
                    {
                      fprintf(file,"%s;",paramMapping.mConversionFunction.c_str());
                      fprintf(file,"%s;",paramMapping.mReverseConversionFunction.c_str());
                    }
                    else
                    {
                      fprintf(file,";;");
                    }
                  }
                  else
                  {
                    fprintf(file,";;");
                  }

                  if (paramDef.mDefaultPrecision >= 0)
                    fprintf(file,"%d;",(int)paramDef.mDefaultPrecision);
                  else
                    fprintf(file,";");

                  fprintf(file,"\n");
                }
                else
                {
                  fprintf(file,"1;1;1;0;D;;;;\n");
                }
              }
            }
          }
        }
      }
    }

    if (file == nullptr  &&  numOfNewMappings == 0)
    {
      // We found all mappings from the other files. That's why we should remove them
      // from the update file.

      file = openMappingFile(mappingFile);
    }

    if (file != nullptr)
      fclose(file);

  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}





void updateMappings()
{
  try
  {
    if ((time(nullptr) - mParameterMappingUpdateTime) > 300)
    {
      mParameterMappingUpdateTime = time(nullptr);

      QueryServer::ParamMappingFile_vec parameterMappings;
      loadMappings(parameterMappings);

      if (mShutdownRequested)
        return;

      if (!query_server_mappingUpdateFile_fmi.empty())
      {
        updateMappings(T::ParamKeyTypeValue::FMI_NAME,query_server_mappingTargetKeyType,query_server_mappingUpdateFile_fmi,parameterMappings);
      }

      if (mShutdownRequested)
        return;

      if (!query_server_mappingUpdateFile_newbase.empty())
      {
        updateMappings(T::ParamKeyTypeValue::NEWBASE_NAME,query_server_mappingTargetKeyType,query_server_mappingUpdateFile_newbase,parameterMappings);
      }

      if (!query_server_mappingUpdateFile_netCdf.empty())
      {
        updateMappings(T::ParamKeyTypeValue::NETCDF_NAME,query_server_mappingTargetKeyType,query_server_mappingUpdateFile_netCdf,parameterMappings);
      }
    }
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}





void updateProcessing()
{
  try
  {
    while (!mShutdownRequested)
    {
      updateMappings();
      sleep(1);
    }
  }
  catch (...)
  {
    throw Fmi::Exception(BCP,"Operation failed!",nullptr);
  }
}





static void* corbaQueryServer_updateThread(void *arg)
{
  try
  {
    updateProcessing();
    pthread_exit(nullptr);
    return nullptr;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP,"Operation failed!",nullptr);
    exception.printError();
    pthread_exit(nullptr);
    exit(-1);
  }
}





void startUpdateProcessing()
{
  try
  {
    pthread_create(&mThread,nullptr,corbaQueryServer_updateThread,nullptr);
  }
  catch (...)
  {
    throw Fmi::Exception(BCP,"Operation failed!",nullptr);
  }
}





void readConfigFile(const char* configFile)
{
  try
  {
    const char *configAttribute[] =
    {
         "smartmet.library.grid-files.configFile",
         "smartmet.library.grid-files.cache.numOfGrids",
         "smartmet.library.grid-files.cache.maxSizeInMegaBytes",
         "smartmet.library.grid-files.requestCounter.enabled",
         "smartmet.tools.grid.corba-server.address",
         "smartmet.tools.grid.corba-server.port",
         "smartmet.tools.grid.content-server.iorFile",
         "smartmet.tools.grid.content-server.content-source.type",
         "smartmet.tools.grid.content-server.content-source.redis.address",
         "smartmet.tools.grid.content-server.content-source.redis.port",
         "smartmet.tools.grid.content-server.content-source.redis.tablePrefix",
         "smartmet.tools.grid.content-server.content-source.corba.ior",
         "smartmet.tools.grid.content-server.content-source.http.url",
         "smartmet.tools.grid.content-server.cache.enabled",
         "smartmet.tools.grid.content-server.cache.requestForwardEnabled",
         "smartmet.tools.grid.content-server.cache.eventListMaxSize",
         "smartmet.tools.grid.content-server.processing-log.enabled",
         "smartmet.tools.grid.content-server.processing-log.file",
         "smartmet.tools.grid.content-server.processing-log.maxSize",
         "smartmet.tools.grid.content-server.processing-log.truncateSize",
         "smartmet.tools.grid.content-server.debug-log.enabled",
         "smartmet.tools.grid.content-server.debug-log.file",
         "smartmet.tools.grid.content-server.debug-log.maxSize",
         "smartmet.tools.grid.content-server.debug-log.truncateSize",
         "smartmet.tools.grid.data-server.iorFile",
         "smartmet.tools.grid.data-server.grid-storage.directory",
         "smartmet.tools.grid.data-server.virtualFiles.enabled",
         "smartmet.tools.grid.data-server.virtualFiles.definitionFile",
         "smartmet.tools.grid.data-server.luaFiles",
         "smartmet.tools.grid.data-server.processing-log.enabled",
         "smartmet.tools.grid.data-server.processing-log.file",
         "smartmet.tools.grid.data-server.processing-log.maxSize",
         "smartmet.tools.grid.data-server.processing-log.truncateSize",
         "smartmet.tools.grid.data-server.debug-log.enabled",
         "smartmet.tools.grid.data-server.debug-log.file",
         "smartmet.tools.grid.data-server.debug-log.maxSize",
         "smartmet.tools.grid.data-server.debug-log.truncateSize",
         "smartmet.tools.grid.query-server.iorFile",
         "smartmet.tools.grid.query-server.producerFile",
         "smartmet.tools.grid.query-server.producerAliasFiles",
         "smartmet.tools.grid.query-server.mappingTargetKeyType",
         "smartmet.tools.grid.query-server.mappingUpdateFile.fmi",
         "smartmet.tools.grid.query-server.mappingUpdateFile.newbase",
         "smartmet.tools.grid.query-server.luaFiles",
         "smartmet.tools.grid.query-server.mappingFiles",
         "smartmet.tools.grid.query-server.aliasFiles",
         "smartmet.tools.grid.query-server.processing-log.enabled",
         "smartmet.tools.grid.query-server.processing-log.file",
         "smartmet.tools.grid.query-server.processing-log.maxSize",
         "smartmet.tools.grid.query-server.processing-log.truncateSize",
         "smartmet.tools.grid.query-server.debug-log.enabled",
         "smartmet.tools.grid.query-server.debug-log.file",
         "smartmet.tools.grid.query-server.debug-log.maxSize",
         "smartmet.tools.grid.query-server.debug-log.truncateSize",
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


    mConfigurationFile.getAttributeValue("smartmet.library.grid-files.configFile",grid_files_configFile);
    mConfigurationFile.getAttributeValue("smartmet.library.grid-files.cache.numOfGrids",grid_files_cache_numOfGrids);
    mConfigurationFile.getAttributeValue("smartmet.library.grid-files.cache.maxSizeInMegaBytes",grid_files_cache_maxSizeInMegaBytes);
    mConfigurationFile.getAttributeValue("smartmet.library.grid-files.requestCounter.enabled",grid_files_requestCounter_enabled);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.corba-server.address",corba_server_address);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.corba-server.port",corba_server_port);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.iorFile",content_server_iorFile);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.content-source.type",content_server_content_source_type);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.content-source.redis.address",content_server_content_source_redis_address);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.content-source.redis.port",content_server_content_source_redis_port);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.content-source.redis.tablePrefix",content_server_content_source_redis_tablePrefix);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.content-source.corba.ior",content_server_content_source_corba_ior);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.content-source.http.url",content_server_content_source_http_url);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.cache.enabled",content_server_cache_enabled);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.cache.requestForwardEnabled",content_server_cache_requestForwardEnabled);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.cache.eventListMaxSize",content_server_cache_eventListMaxSize);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.processing-log.enabled",content_server_processing_log_enabled);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.processing-log.file",content_server_processing_log_file);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.processing-log.maxSize",content_server_processing_log_maxSize);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.processing-log.truncateSize",content_server_processing_log_truncateSize);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.debug-log.enabled",content_server_debug_log_enabled);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.debug-log.file",content_server_debug_log_file);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.debug-log.maxSize",content_server_debug_log_maxSize);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.debug-log.truncateSize",content_server_debug_log_truncateSize);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.data-server.iorFile",data_server_iorFile);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.data-server.grid-storage.directory",data_server_grid_storage_directory);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.data-server.virtualFiles.enabled",data_server_virtualFiles_enabled);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.data-server.virtualFiles.definitionFile",data_server_virtualFiles_definitionFile);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.data-server.luaFiles",data_server_luaFiles);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.data-server.processing-log.enabled",data_server_processing_log_enabled);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.data-server.processing-log.file",data_server_processing_log_file);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.data-server.processing-log.maxSize",data_server_processing_log_maxSize);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.data-server.processing-log.truncateSize",data_server_processing_log_truncateSize);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.data-server.debug-log.enabled",data_server_debug_log_enabled);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.data-server.debug-log.file",data_server_debug_log_file);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.data-server.debug-log.maxSize",data_server_debug_log_maxSize);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.data-server.debug-log.truncateSize",data_server_debug_log_truncateSize);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.query-server.iorFile",query_server_iorFile);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.query-server.producerFile",query_server_producerFile);
    mConfigurationFile.getAttributeValue("smartmet.engine.grid.query-server.checkGeometryStatus",query_server_checkGeometryStatus);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.query-server.producerAliasFiles",query_server_producerAliasFiles);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.query-server.mappingTargetKeyType",query_server_mappingTargetKeyType);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.query-server.mappingUpdateFile.fmi",query_server_mappingUpdateFile_fmi);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.query-server.mappingUpdateFile.newbase",query_server_mappingUpdateFile_newbase);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.query-server.mappingUpdateFile.netCdf",query_server_mappingUpdateFile_netCdf);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.query-server.luaFiles",query_server_luaFiles);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.query-server.mappingFiles",query_server_mappingFiles);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.query-server.aliasFiles",query_server_aliasFiles);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.query-server.processing-log.enabled",query_server_processing_log_enabled);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.query-server.processing-log.file",query_server_processing_log_file);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.query-server.processing-log.maxSize",query_server_processing_log_maxSize);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.query-server.processing-log.truncateSize",query_server_processing_log_truncateSize);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.query-server.debug-log.enabled",query_server_debug_log_enabled);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.query-server.debug-log.file",query_server_debug_log_file);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.query-server.debug-log.maxSize",query_server_debug_log_maxSize);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.query-server.debug-log.truncateSize",query_server_debug_log_truncateSize);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.query-server.dataServerMethodsEnabled",data_server_methods_enabled);



    // Initializing information that is needed for identifying the content of the grid files.

    SmartMet::Identification::gridDef.init(grid_files_configFile.c_str());
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Constructor failed!", nullptr);
  }
}









int main(int argc, char *argv[])
{
  try
  {
    if (argc != 2)
    {
      printf("\n");
      printf("##################################################################################\n");
      printf("                             corbaGridServer\n");
      printf("##################################################################################\n");
      printf(" DESCRIPTION:\n");
      printf("\n");
      printf(" USAGE:\n");
      printf("   corbaGridServer <configFile>\n");
      printf("\n");
      printf("##################################################################################\n");
      printf("\n");
      return -1;
    }

    //signal(SIGINT, sig_handler);

    readConfigFile(argv[1]);



    if (strcasecmp(content_server_content_source_type.c_str(),"redis") == 0)
    {
      redisImplementation = new ContentServer::RedisImplementation();
      redisImplementation->init(content_server_content_source_redis_address.c_str(),content_server_content_source_redis_port,content_server_content_source_redis_tablePrefix.c_str());
      contentService = redisImplementation;
    }
    else
    if (strcasecmp(content_server_content_source_type.c_str(),"corba") == 0)
    {
      corbaContentClient = new ContentServer::Corba::ClientImplementation();
      corbaContentClient->init(content_server_content_source_corba_ior.c_str());
      contentService = corbaContentClient;
    }
    else
    if (strcasecmp(content_server_content_source_type.c_str(),"http") == 0)
    {
      httpContentClient = new ContentServer::HTTP::ClientImplementation();
      httpContentClient->init(content_server_content_source_http_url.c_str());
      contentService = httpContentClient;
    }

    if (contentService == nullptr)
    {
      Fmi::Exception exception(BCP,"No acceptable content source defined!");
      throw exception;
    }

    if (content_server_cache_enabled)
    {
      cacheImplementation = new ContentServer::CacheImplementation();
      cacheImplementation->setRequestForwardEnabled(content_server_cache_requestForwardEnabled);

      if (content_server_processing_log_enabled && content_server_processing_log_file.length() > 0)
      {
        mContentServerProcessingLog.init(true,content_server_processing_log_file.c_str(),content_server_processing_log_maxSize,content_server_processing_log_truncateSize);
        cacheImplementation->setProcessingLog(&mContentServerProcessingLog);
      }

      if (content_server_debug_log_enabled && content_server_debug_log_file.length() > 0)
      {
        mContentServerDebugLog.init(true,content_server_debug_log_file.c_str(),content_server_debug_log_maxSize,content_server_debug_log_truncateSize);
        cacheImplementation->setDebugLog(&mContentServerDebugLog);
      }

      cacheImplementation->init(0,contentService);
      cacheImplementation->setEventListMaxLength(content_server_cache_eventListMaxSize);

      cacheImplementation->startEventProcessing();
      contentService = cacheImplementation;
    }
    else
    {
      if (content_server_processing_log_enabled && content_server_processing_log_file.length() > 0)
      {
        mContentServerProcessingLog.init(true,content_server_processing_log_file.c_str(),content_server_processing_log_maxSize,content_server_processing_log_truncateSize);
        contentService->setProcessingLog(&mContentServerProcessingLog);
      }

      if (content_server_debug_log_enabled && content_server_debug_log_file.length() > 0)
      {
        mContentServerDebugLog.init(true,content_server_debug_log_file.c_str(),content_server_debug_log_maxSize,content_server_debug_log_truncateSize);
        contentService->setDebugLog(&mContentServerDebugLog);
      }
    }



    dataService = new DataServer::ServiceImplementation();
    queryService = new QueryServer::ServiceImplementation();

    corbaServer = new QueryServer::Corba::GridServer(corba_server_address.c_str(),corba_server_port.c_str());
    corbaServer->init(contentService,dataService,queryService);


    dataService->init(0,data_server_id,data_server_name.c_str(),corbaServer->getDataServiceIor().c_str(),data_server_grid_storage_directory.c_str(),contentService,data_server_luaFiles);
    queryService->init(contentService,dataService,grid_files_configFile,query_server_mappingFiles,query_server_aliasFiles,query_server_producerFile,query_server_producerAliasFiles,query_server_luaFiles,query_server_checkGeometryStatus,data_server_methods_enabled);





    if (data_server_processing_log_enabled && data_server_processing_log_file.length() > 0)
    {
      mDataServerProcessingLog.init(true,data_server_processing_log_file.c_str(),data_server_processing_log_maxSize,data_server_processing_log_truncateSize);
      dataService->setProcessingLog(&mDataServerProcessingLog);
    }

    if (data_server_debug_log_enabled && data_server_debug_log_file.length() > 0)
    {
      mDataServerDebugLog.init(true,data_server_debug_log_file.c_str(),data_server_debug_log_maxSize,data_server_debug_log_truncateSize);
      dataService->setDebugLog(&mDataServerDebugLog);
    }

    dataService->setVirtualContentEnabled(data_server_virtualFiles_enabled);

    if (data_server_virtualFiles_definitionFile.length() > 0)
    {
      DataServer::VirtualContentFactory_type1 *factory = new DataServer::VirtualContentFactory_type1();
      factory->init(data_server_virtualFiles_definitionFile);

      dataService->addVirtualContentFactory(factory);
    }

    dataService->startEventProcessing();


    if (query_server_processing_log_enabled && query_server_processing_log_file.length() > 0)
    {
      mQueryServerProcessingLog.init(true,query_server_processing_log_file.c_str(),query_server_processing_log_maxSize,query_server_processing_log_truncateSize);
      queryService->setProcessingLog(&mQueryServerProcessingLog);
    }

    if (query_server_debug_log_enabled  &&  query_server_debug_log_file.length() > 0)
    {
      mQueryServerDebugLog.init(true,query_server_debug_log_file.c_str(),query_server_debug_log_maxSize,query_server_debug_log_truncateSize);
      queryService->setDebugLog(&mQueryServerDebugLog);
    }

    // Let's print the service IOR. This is necessary for accessing the service. Usually the best way
    // to handle an IOR is to store it into an environment variable.

    std::string ior = corbaServer->getContentServiceIor();
    if (content_server_iorFile.length() == 0)
    {
      printf("\n\nContentServer\n%s\n",ior.c_str());
    }
    else
    {
      FILE *file = fopen(content_server_iorFile.c_str(),"we");
      if (file == nullptr)
      {
        Fmi::Exception exception(BCP,"Cannot create file for IOR!");
        exception.addParameter("IorFile",content_server_iorFile);
        throw exception;
      }
      fprintf(file,"%s\n",ior.c_str());
      fclose(file);
    }


    ior = corbaServer->getDataServiceIor();
    if (data_server_iorFile.length() == 0)
    {
      printf("\n\nDataServer\n%s\n",ior.c_str());
    }
    else
    {
      FILE *file = fopen(data_server_iorFile.c_str(),"we");
      if (file == nullptr)
      {
        Fmi::Exception exception(BCP,"Cannot create file for IOR!");
        exception.addParameter("IorFile",data_server_iorFile);
        throw exception;
      }
      fprintf(file,"%s\n",ior.c_str());
      fclose(file);
    }


    ior = corbaServer->getQueryServiceIor();
    if (query_server_iorFile.length() == 0)
    {
      printf("\n\nQueryServer\n%s\n",ior.c_str());
    }
    else
    {
      FILE *file = fopen(query_server_iorFile.c_str(),"we");
      if (file == nullptr)
      {
        Fmi::Exception exception(BCP,"Cannot create file for IOR!");
        exception.addParameter("IorFile",query_server_iorFile);
        throw exception;
      }
      fprintf(file,"%s\n",ior.c_str());
      fclose(file);
    }

    mProducerAliasFileCollection.init(query_server_producerAliasFiles,true);

    startUpdateProcessing();

    corbaServer->run();

    delete contentService;
    delete dataService;
    delete queryService;
    delete corbaServer;
    return 0;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP,"Operation failed!",nullptr);
    exception.printError();
    return -1;
  }
}
