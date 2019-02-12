#include "grid-content/contentServer/corba/client/ClientImplementation.h"
#include "grid-content/contentServer/redis/RedisImplementation.h"
#include "grid-content/contentServer/cache/CacheImplementation.h"
#include "grid-content/dataServer/corba/client/ClientImplementation.h"
#include "grid-content/dataServer/cache/CacheImplementation.h"
#include "grid-content/dataServer/implementation/ServiceImplementation.h"
#include "grid-content/dataServer/implementation/VirtualContentFactory_type1.h"
#include "grid-content/queryServer/corba/server/ServerInterface.h"
#include "grid-content/queryServer/corba/server/Server.h"
#include "grid-content/queryServer/implementation/ServiceImplementation.h"
#include "grid-files/common/Exception.h"
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

QueryServer::Corba::Server *corbaServer = nullptr;
QueryServer::ServiceImplementation *queryServer = nullptr;
ContentServer::Corba::ClientImplementation *contentServer = nullptr;
DataServer::Corba::ClientImplementation *dataServer = nullptr;




bool                mShutdownRequested = false;

ConfigurationFile   mConfigurationFile;

std::string         mServerAddress;
std::string         mServerPort;
std::string         mServerIorFile;

std::string         mContentServerIor;
bool                mContentServerProcessingLogEnabled;
std::string         mContentServerProcessingLogFile;
int                 mContentServerProcessingLogMaxSize = 100000000;
int                 mContentServerProcessingLogTruncateSize = 20000000;
Log                 mContentServerProcessingLog;
bool                mContentServerDebugLogEnabled = false;
std::string         mContentServerDebugLogFile;
int                 mContentServerDebugLogMaxSize = 100000000;
int                 mContentServerDebugLogTruncateSize = 20000000;
Log                 mContentServerDebugLog;

std::string         mDataServerIor;
bool                mDataServerProcessingLogEnabled = false;
std::string         mDataServerProcessingLogFile;
int                 mDataServerProcessingLogMaxSize = 100000000;
int                 mDataServerProcessingLogTruncateSize = 20000000;
Log                 mDataServerProcessingLog;
bool                mDataServerDebugLogEnabled = false;
std::string         mDataServerDebugLogFile;
int                 mDataServerDebugLogMaxSize = 100000000;
int                 mDataServerDebugLogTruncateSize = 20000000;
Log                 mDataServerDebugLog;

string_vec          mQueryServerLuaFiles;
bool                mQueryServerProcessingLogEnabled = false;
std::string         mQueryServerProcessingLogFile;
int                 mQueryServerProcessingLogMaxSize = 100000000;
int                 mQueryServerProcessingLogTruncateSize = 20000000;
Log                 mQueryServerProcessingLog;
bool                mQueryServerDebugLogEnabled = false;
std::string         mQueryServerDebugLogFile;
int                 mQueryServerDebugLogMaxSize = 100000000;
int                 mQueryServerDebugLogTruncateSize = 20000000;
Log                 mQueryServerDebugLog;

std::string         mGridConfigFile;
uint                mNumOfCachedGrids = 50000;
uint                mMaxCompressedMegaBytesOfCachedGrids = 10000;
uint                mMaxUncompressedMegaBytesOfCachedGrids = 10000;

std::string         mProducerFile;
std::string         mProducerAliasFile;
bool                mVirtualFilesEnabled = false;
std::string         mVirtualFileDefinitions;
bool                mContentPreloadEnabled = false;

string_vec          mParameterAliasFiles;
string_vec          mParameterMappingFiles;

pthread_t           mThread;
std::string         mParameterMappingUpdateFile_fmi;
std::string         mParameterMappingUpdateFile_newbase;
time_t              mParameterMappingUpdateTime = 0;
T::ParamKeyType     mMappingTargetKeyType = T::ParamKeyTypeValue::FMI_NAME;

QueryServer::AliasFile mProducerAliases;






void loadMappings(QueryServer::ParamMappingFile_vec& parameterMappings)
{
  try
  {
    for (auto it = mParameterMappingFiles.begin(); it != mParameterMappingFiles.end(); ++it)
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
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", nullptr);
  }
}





FILE* openMappingFile(std::string mappingFile)
{
  try
  {
    FILE *file = fopen(mappingFile.c_str(),"we");
    if (file == nullptr)
    {
      SmartMet::Spine::Exception exception(BCP, "Cannot open a mapping file for writing!");
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
    fprintf(file,"#         6 = CDM_ID\n");
    fprintf(file,"#         7 = CDM_NAME\n");
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
    fprintf(file,"#  8) Area interpolation method\n");
    fprintf(file,"#         0 = None\n");
    fprintf(file,"#         1 = Linear\n");
    fprintf(file,"#         2 = Nearest\n");
    fprintf(file,"#         3 = Min\n");
    fprintf(file,"#         4 = Max\n");
    fprintf(file,"#         500..999 = List\n");
    fprintf(file,"#         1000..65535 = External (interpolated by an external function)\n");
    fprintf(file,"#  9) Time interpolation method\n");
    fprintf(file,"#         0 = None\n");
    fprintf(file,"#         1 = Linear\n");
    fprintf(file,"#         2 = Nearest\n");
    fprintf(file,"#         3 = Min\n");
    fprintf(file,"#         4 = Max\n");
    fprintf(file,"#         1000..65535 = External (interpolated by an external function)\n");
    fprintf(file,"# 10) Level interpolation method\n");
    fprintf(file,"#         0 = None\n");
    fprintf(file,"#         1 = Linear\n");
    fprintf(file,"#         2 = Nearest\n");
    fprintf(file,"#         3 = Min\n");
    fprintf(file,"#         4 = Max\n");
    fprintf(file,"#         5 = Logarithmic\n");
    fprintf(file,"#         1000..65535 = External (interpolated by an external function)\n");
    fprintf(file,"# 11) Group flags\n");
    fprintf(file,"#         bit 0 = Climatological parameter (=> ignore year when searching) \n");
    fprintf(file,"# 12) Search match (Can this mapping used when searching mappings for incomplete parameters)\n");
    fprintf(file,"#         E = Enabled\n");
    fprintf(file,"#         D = Disabled\n");
    fprintf(file,"# 13) Mapping function (enables data conversions during the mapping)\n");
    fprintf(file,"# 14) Reverse mapping function\n");
    fprintf(file,"# 15) Default precision\n");
    fprintf(file,"# \n");

    return file;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", nullptr);
  }
}





void updateMappings(T::ParamKeyType sourceParameterKeyType,T::ParamKeyType targetParameterKeyType,std::string mappingFile,QueryServer::ParamMappingFile_vec& parameterMappings)
{
  try
  {
    T::SessionId sessionId = 0;

    std::set<std::string> infoList;
    int result = 0;

    result = contentServer->getProducerParameterList(sessionId,sourceParameterKeyType,targetParameterKeyType,infoList);
    if (result != 0)
    {
      std::cerr << CODE_LOCATION << "The 'contentServer.getProducerParameterList()' service call returns an error!  Result : " << result << " : " << ContentServer::getResultString(result).c_str() << "\n";
      return;
    }

    FILE *file = nullptr;

    uint numOfNewMappings = 0;
    std::set<std::string> mapList;
    std::set<std::string> searchList;

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
        m.mParameterLevelIdType = toInt16(pl[5].c_str());
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
              found = Identification::gridDef.getFmiParameterDefById(pl[3],paramDef);
            else
            if (targetParameterKeyType == T::ParamKeyTypeValue::NEWBASE_ID)
              found = Identification::gridDef.getFmiParameterDefByNewbaseId(pl[3],paramDef);

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
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", nullptr);
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

      if (!mParameterMappingUpdateFile_fmi.empty())
      {
        updateMappings(T::ParamKeyTypeValue::FMI_NAME,mMappingTargetKeyType,mParameterMappingUpdateFile_fmi,parameterMappings);
      }

      if (mShutdownRequested)
        return;

      if (!mParameterMappingUpdateFile_newbase.empty())
      {
        updateMappings(T::ParamKeyTypeValue::NEWBASE_NAME,mMappingTargetKeyType,mParameterMappingUpdateFile_newbase,parameterMappings);
      }
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", nullptr);
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
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,nullptr);
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
    SmartMet::Spine::Exception exception(BCP,exception_operation_failed,nullptr);
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

        "smartmet.tools.grid.query-server.address",
        "smartmet.tools.grid.query-server.port",
        "smartmet.tools.grid.query-server.iorFile",

        "smartmet.tools.grid.query-server.producerFile",
        "smartmet.tools.grid.query-server.producerAliasFile",
        "smartmet.tools.grid.query-server.luaFiles",
        "smartmet.tools.grid.query-server.mappingFiles",
        "smartmet.tools.grid.query-server.mappingTargetKeyType",
        "smartmet.tools.grid.query-server.mappingUpdateFile.fmi",
        "smartmet.tools.grid.query-server.mappingUpdateFile.newbase",
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
        SmartMet::Spine::Exception exception(BCP, "Missing configuration attribute!");
        exception.addParameter("File",configFile);
        exception.addParameter("Attribute",configAttribute[t]);
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


    mConfigurationFile.getAttributeValue("smartmet.tools.grid.query-server.address", mServerAddress);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.query-server.port", mServerPort);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.query-server.iorFile", mServerIorFile);

    mConfigurationFile.getAttributeValue("smartmet.tools.grid.query-server.producerFile",mProducerFile);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.query-server.producerAliasFile",mProducerAliasFile);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.query-server.processing-log.enabled", mQueryServerProcessingLogEnabled);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.query-server.processing-log.file", mQueryServerProcessingLogFile);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.query-server.processing-log.maxSize", mQueryServerProcessingLogMaxSize);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.query-server.processing-log.truncateSize", mQueryServerProcessingLogTruncateSize);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.query-server.debug-log.enabled", mQueryServerDebugLogEnabled);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.query-server.debug-log.file", mQueryServerDebugLogFile);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.query-server.debug-log.maxSize", mQueryServerDebugLogMaxSize);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.query-server.debug-log.truncateSize", mQueryServerDebugLogTruncateSize);

    int tmp = 0;
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.query-server.mappingTargetKeyType",tmp);
    mMappingTargetKeyType = tmp;

    mConfigurationFile.getAttributeValue("smartmet.tools.grid.query-server.mappingUpdateFile.fmi",mParameterMappingUpdateFile_fmi);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.query-server.mappingUpdateFile.newbase",mParameterMappingUpdateFile_newbase);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.query-server.mappingFiles",mParameterMappingFiles);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.query-server.aliasFiles",mParameterAliasFiles);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.query-server.luaFiles",mQueryServerLuaFiles);


    // Initializing information that is needed for identifying the content of the grid files.

    SmartMet::Identification::gridDef.init(mGridConfigFile.c_str());
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Constructor failed!", nullptr);
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
      printf("                             corbaQueryServer\n");
      printf("##################################################################################\n");
      printf(" DESCRIPTION:\n");
      printf("\n");
      printf(" USAGE:\n");
      printf("   queryServer <configFile>\n");
      printf("\n");
      printf("##################################################################################\n");
      printf("\n");
      return -1;
    }

    //signal(SIGINT, sig_handler);

    readConfigFile(argv[1]);


    queryServer = new QueryServer::ServiceImplementation();

    corbaServer = new QueryServer::Corba::Server(mServerAddress.c_str(),mServerPort.c_str());
    corbaServer->init(queryServer);

    contentServer = new ContentServer::Corba::ClientImplementation();
    contentServer->init(mContentServerIor);

    dataServer = new DataServer::Corba::ClientImplementation();
    dataServer->init(mDataServerIor);


    queryServer->init(contentServer,dataServer,mGridConfigFile,mParameterMappingFiles,mParameterAliasFiles,mProducerFile,mProducerAliasFile,mQueryServerLuaFiles);

    if (mContentServerProcessingLogEnabled &&  mContentServerProcessingLogFile.length() > 0)
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

    if (mQueryServerProcessingLogEnabled && mQueryServerProcessingLogFile.length() > 0)
    {
      mQueryServerProcessingLog.init(true,mQueryServerProcessingLogFile.c_str(),mQueryServerProcessingLogMaxSize,mQueryServerProcessingLogTruncateSize);
      queryServer->setProcessingLog(&mQueryServerProcessingLog);
    }

    if (mQueryServerDebugLogEnabled && mQueryServerDebugLogFile.length() > 0)
    {
      mQueryServerDebugLog.init(true,mQueryServerDebugLogFile.c_str(),mQueryServerDebugLogMaxSize,mQueryServerDebugLogTruncateSize);
      queryServer->setDebugLog(&mQueryServerDebugLog);
    }

    // Let's print the service IOR. This is necessary for accessing the service. Usually the best way
    // to handle an IOR is to store it into an environment variable.

    std::string ior = corbaServer->getServiceIor();
    if (mServerIorFile.length() == 0)
    {
      printf("\n%s\n",ior.c_str());
    }
    else
    {
      FILE *file = fopen(mServerIorFile.c_str(),"we");
      if (file == nullptr)
      {
        SmartMet::Spine::Exception exception(BCP,"Cannot create file for IOR!");
        exception.addParameter("IorFile",mServerIorFile);
        throw exception;
      }
      fprintf(file,"%s\n",ior.c_str());
      fclose(file);
    }

    mProducerAliases.init(mProducerAliasFile,true);

    startUpdateProcessing();

    corbaServer->run();

    delete contentServer;
    delete dataServer;
    delete queryServer;
    delete corbaServer;
    return 0;
  }
  catch (...)
  {
    SmartMet::Spine::Exception exception(BCP,exception_operation_failed,nullptr);
    exception.printError();
    return -1;
  }
}
