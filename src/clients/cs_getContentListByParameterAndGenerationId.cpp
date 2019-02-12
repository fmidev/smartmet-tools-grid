#include "grid-content/contentServer/corba/client/ClientImplementation.h"
#include "grid-content/contentServer/http/client/ClientImplementation.h"
#include "grid-content/contentServer/redis/RedisImplementation.h"
#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"

using namespace SmartMet;


int main(int argc, char *argv[])
{

  try
  {
    char *serviceIor = getenv("SMARTMET_CS_IOR");
    if (serviceIor == nullptr)
    {
      fprintf(stdout,"SMARTMET_CS_IOR not defined!\n");
      return -2;
    }

    if (argc < 15)
    {
      fprintf(stdout,"USAGE:\n");
      fprintf(stdout,"  cs_getContentListByParameterAndGenerationId <sessionId>  <generationId> <parameterIdType>\n");
      fprintf(stdout,"     <parameterKey>  <parameterLevelIdType> <parameterLevelId> <minLevel> <maxLevel> \n");
      fprintf(stdout,"     <forecastType> <forecastNumber> <geometryId> <startTime> <endTime> <requestFlags>\n");
      fprintf(stdout,"WHERE:\n");
      fprintf(stdout,"  sessionId             = Session identifier\n");
      fprintf(stdout,"  generationId          = Generation identifier\n");
      fprintf(stdout,"  parameterKeyType      = Parameter search key type:\n");
      fprintf(stdout,"                            fmi-id       => Radon identifier\n");
      fprintf(stdout,"                            fmi-name     => Radon name\n");
      fprintf(stdout,"                            grib-id      => GRIB identifier\n");
      fprintf(stdout,"                            cdm-id       => CDM identifier\n");
      fprintf(stdout,"                            cdm-name     => CDM name\n");
      fprintf(stdout,"                            newbase-id   => Newbase identifier\n");
      fprintf(stdout,"                            newbase-name => Newbase name\n");
      fprintf(stdout,"  parameterKey          = Parameter search key\n");
      fprintf(stdout,"  parameterLevelIdType  = Parameter level id type:\n");
      fprintf(stdout,"                             any         => All level types are accepted\n");
      fprintf(stdout,"                             fmi         => Radon level identifier\n");
      fprintf(stdout,"                             grib1       => GRIB 1 level identifier\n");
      fprintf(stdout,"                             grib2       => GRIB 2 level identifier\n");
      fprintf(stdout,"                             ignore      => All level types and values are accepted\n");
      fprintf(stdout,"  parameterLevelId       = Parameter level type\n");
      fprintf(stdout,"  minLevel               = Minimum parameter level\n");
      fprintf(stdout,"  maxLevel               = Maximum parameter level\n");
      fprintf(stdout,"  forecastType           = Forecast type\n");
      fprintf(stdout,"  forecastNumber         = Forecast number\n");
      fprintf(stdout,"  geometryId             = Geometry identifier\n");
      fprintf(stdout,"  startTime              = First accepted grid time\n");
      fprintf(stdout,"  endTime                = Last accepted grid time\n");
      fprintf(stdout,"  requestFlags           = Request flags\n");

      return -1;
    }

    // ### Session:
    T::SessionId sessionId = toInt64(argv[1]);

    // ### Service:
    ContentServer::Corba::ClientImplementation contentServer;
    contentServer.init(serviceIor);

    // ### Service parameters:
    uint generationId = toInt64(argv[2]);

    T::ParamKeyType paramKeyType = T::ParamKeyTypeValue::FMI_ID;
    if (strcmp(argv[3],"fmi-id") == 0)
      paramKeyType = T::ParamKeyTypeValue::FMI_ID;
    else
    if (strcmp(argv[3],"fmi-name") == 0)
      paramKeyType = T::ParamKeyTypeValue::FMI_NAME;
    else
    if (strcmp(argv[3],"grib-id") == 0)
      paramKeyType = T::ParamKeyTypeValue::GRIB_ID;
    else
    if (strcmp(argv[3],"cdm-id") == 0)
      paramKeyType = T::ParamKeyTypeValue::CDM_ID;
    else
    if (strcmp(argv[3],"cdm-name") == 0)
      paramKeyType = T::ParamKeyTypeValue::CDM_NAME;
    else
    if (strcmp(argv[3],"newbase-id") == 0)
      paramKeyType = T::ParamKeyTypeValue::NEWBASE_ID;
    else
    if (strcmp(argv[3],"newbase-name") == 0)
      paramKeyType = T::ParamKeyTypeValue::NEWBASE_NAME;

    T::ParamId parameterKey = argv[4];
    T::ParamLevelIdType parameterLevelIdType = T::ParamLevelIdTypeValue::ANY;

    if (strcmp(argv[5],"any") == 0)
      parameterLevelIdType = T::ParamLevelIdTypeValue::ANY;
    else
    if (strcmp(argv[5],"fmi") == 0)
      parameterLevelIdType = T::ParamLevelIdTypeValue::FMI;
    else
    if (strcmp(argv[5],"grib1") == 0)
      parameterLevelIdType = T::ParamLevelIdTypeValue::GRIB1;
    else
    if (strcmp(argv[5],"grib2") == 0)
      parameterLevelIdType = T::ParamLevelIdTypeValue::GRIB2;
    else
    if (strcmp(argv[5],"ignore") == 0)
      parameterLevelIdType = T::ParamLevelIdTypeValue::IGNORE;

    T::ParamLevelId parameterLevelId = toInt64(argv[6]);
    T::ParamLevel minLevel = (T::ParamLevel)toInt64(argv[7]);
    T::ParamLevel maxLevel = (T::ParamLevel)toInt64(argv[8]);
    T::ForecastType forecastType = (T::ForecastType)toInt64(argv[9]);
    T::ForecastNumber forecastNumber = (T::ForecastNumber)toInt64(argv[10]);
    T::GeometryId geometryId = (T::GeometryId)toInt64(argv[11]);
    std::string start = argv[12];
    std::string end = argv[13];
    uint requestFlags = toInt64(argv[14]);
    T::ContentInfoList infoList;
    //Log processingLog;

    int result = 0;
    unsigned long long startTime = 0;
    unsigned long long endTime = 0;

    if (strcmp(argv[argc-2],"-http") == 0)
    {
      ContentServer::HTTP::ClientImplementation service;
      service.init(argv[argc-1]);

      startTime = getTime();
      result = service.getContentListByParameterAndGenerationId(sessionId,generationId,paramKeyType,parameterKey,parameterLevelIdType,parameterLevelId,minLevel,maxLevel,forecastType,forecastNumber,geometryId,start,end,requestFlags,infoList);
      endTime = getTime();
    }
    else
    if (strcmp(argv[argc-4],"-redis") == 0)
    {
      ContentServer::RedisImplementation service;
      service.init(argv[argc-3],toInt64(argv[argc-2]),argv[argc-1]);

      //processingLog.init(true,"/dev/stdout",10000000,5000000);
      //service.setProcessingLog(&processingLog);

      startTime = getTime();
      result = service.getContentListByParameterAndGenerationId(sessionId,generationId,paramKeyType,parameterKey,parameterLevelIdType,parameterLevelId,minLevel,maxLevel,forecastType,forecastNumber,geometryId,start,end,requestFlags,infoList);
      endTime = getTime();
    }
    else
    {
      char *serviceIor = getenv("SMARTMET_CS_IOR");

      if (strcmp(argv[argc-2],"-ior") == 0)
        serviceIor = argv[argc-1];

      if (serviceIor == nullptr)
      {
        fprintf(stdout,"Service IOR not defined!\n");
        return -2;
      }

      ContentServer::Corba::ClientImplementation service;
      service.init(serviceIor);

      startTime = getTime();
      result = service.getContentListByParameterAndGenerationId(sessionId,generationId,paramKeyType,parameterKey,parameterLevelIdType,parameterLevelId,minLevel,maxLevel,forecastType,forecastNumber,geometryId,start,end,requestFlags,infoList);
      endTime = getTime();
    }

    if (result != 0)
    {
      fprintf(stdout,"ERROR (%d) : %s\n",result,ContentServer::getResultString(result).c_str());
      return -3;
    }

    // ### Result:
    infoList.print(std::cout,0,0);

    printf("\nTIME : %f sec\n\n",(float)(endTime-startTime)/1000000);

    return 0;
  }
  catch (SmartMet::Spine::Exception& e)
  {
    SmartMet::Spine::Exception exception(BCP,"Service call failed!",nullptr);
    exception.printError();
    return -4;
  }
}

