#include "grid-content/contentServer/corba/client/ClientImplementation.h"
#include "grid-content/contentServer/http/client/ClientImplementation.h"
// #include "grid-content/contentServer/postgres/PostgresImplementation.h"
#include "grid-content/contentServer/redis/RedisImplementation.h"
#include <macgyver/Exception.h>
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

    if (argc < 14)
    {
      fprintf(stdout,"USAGE:\n");
      fprintf(stdout,"  cs_getContentListByParameter <sessionId>  <parameterIdType> <parameterKey> \n");
      fprintf(stdout,"     <parameterLevelIdType> <parameterLevelId> <minLevel> <maxLevel> \n");
      fprintf(stdout,"     <forecastType> <forecastNumber> <geometryId> <startTime> <endTime> <requestFlags>\n");
      fprintf(stdout,"WHERE:\n");
      fprintf(stdout,"  sessionId             = Session identifier\n");
      fprintf(stdout,"  parameterKeyType      = Parameter search key type:\n");
      fprintf(stdout,"                            fmi-id       => Radon identifier\n");
      fprintf(stdout,"                            fmi-name     => Radon name\n");
      fprintf(stdout,"                            grib-id      => GRIB identifier\n");
      fprintf(stdout,"                            newbase-id   => Newbase identifier\n");
      fprintf(stdout,"                            newbase-name => Newbase name\n");
      fprintf(stdout,"  parameterKey          = Parameter search key\n");
      fprintf(stdout,"  parameterLevelIdType  = Parameter level id type:\n");
      fprintf(stdout,"                             any         => All level types are accepted\n");
      fprintf(stdout,"                             fmi         => Radon level identifier\n");
      fprintf(stdout,"                             grib1       => GRIB 1 level identifier\n");
      fprintf(stdout,"                             grib2       => GRIB 2 level identifier\n");
      fprintf(stdout,"                             ignore      => All level types and values are accepted\n");
      fprintf(stdout,"  parameterLevelId       = Parameter level id\n");
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
    T::ParamKeyType paramKeyType = T::ParamKeyTypeValue::FMI_ID;
    if (strcmp(argv[2],"fmi-id") == 0)
      paramKeyType = T::ParamKeyTypeValue::FMI_ID;
    else
    if (strcmp(argv[2],"fmi-name") == 0)
      paramKeyType = T::ParamKeyTypeValue::FMI_NAME;
    else
    if (strcmp(argv[2],"grib-id") == 0)
      paramKeyType = T::ParamKeyTypeValue::GRIB_ID;
    else
    if (strcmp(argv[2],"newbase-id") == 0)
      paramKeyType = T::ParamKeyTypeValue::NEWBASE_ID;
    else
    if (strcmp(argv[2],"newbase-name") == 0)
      paramKeyType = T::ParamKeyTypeValue::NEWBASE_NAME;

    T::ParamId parameterKey = argv[3];
    T::ParamLevelIdType parameterLevelIdType = T::ParamLevelIdTypeValue::ANY;

    if (strcmp(argv[4],"any") == 0)
      parameterLevelIdType = T::ParamLevelIdTypeValue::ANY;
    else
    if (strcmp(argv[4],"fmi") == 0)
      parameterLevelIdType = T::ParamLevelIdTypeValue::FMI;
    else
    if (strcmp(argv[4],"grib1") == 0)
      parameterLevelIdType = T::ParamLevelIdTypeValue::GRIB1;
    else
    if (strcmp(argv[4],"grib2") == 0)
      parameterLevelIdType = T::ParamLevelIdTypeValue::GRIB2;
    else
    if (strcmp(argv[4],"ignore") == 0)
      parameterLevelIdType = T::ParamLevelIdTypeValue::IGNORE;

    T::ParamLevelId parameterLevelId = toInt64(argv[5]);
    T::ParamLevel minLevel = (T::ParamLevel)toInt64(argv[6]);
    T::ParamLevel maxLevel = (T::ParamLevel)toInt64(argv[7]);
    T::ForecastType forecastType = (T::ForecastType)toInt64(argv[8]);
    T::ForecastNumber forecastNumber = (T::ForecastNumber)toInt64(argv[9]);
    T::GeometryId geometryId = (T::GeometryId)toInt64(argv[10]);
    std::string start = argv[11];
    std::string end = argv[12];
    uint requestFlags = toInt64(argv[13]);
    T::ContentInfoList infoList;

    ContentServer::ServiceInterface *service = nullptr;

    if (strcmp(argv[argc-2],"-http") == 0)
    {
      ContentServer::HTTP::ClientImplementation *httpClient = new ContentServer::HTTP::ClientImplementation();
      httpClient->init(argv[argc-1]);
      service = httpClient;
    }
    else
    if (argc > 4  &&  strcmp(argv[argc-4],"-redis") == 0)
    {
      ContentServer::RedisImplementation *redis = new ContentServer::RedisImplementation();
      redis->init(argv[argc-3],toInt64(argv[argc-2]),argv[argc-1]);
      service = redis;
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

      ContentServer::Corba::ClientImplementation *corbaClient = new ContentServer::Corba::ClientImplementation();
      corbaClient->init(serviceIor);
      service = corbaClient;
    }


    if (service == nullptr)
    {
      fprintf(stdout,"ERROR : Service not defined!\n");
      return -3;
    }

    unsigned long long startTime = getTime();
    int result = service->getContentListByParameter(sessionId,paramKeyType,parameterKey,parameterLevelIdType,parameterLevelId,minLevel,maxLevel,forecastType,forecastNumber,geometryId,start,end,requestFlags,infoList);
    unsigned long long endTime = getTime();

    if (result != 0)
    {
      fprintf(stdout,"ERROR (%d) : %s\n",result,ContentServer::getResultString(result).c_str());
      return -3;
    }

    // ### Result:
    infoList.print(std::cout,0,0);

    printf("\nTIME : %f sec\n\n",(float)(endTime-startTime)/1000000);

    delete service;

    return 0;
  }
  catch (Fmi::Exception& e)
  {
    Fmi::Exception exception(BCP,"Service call failed!",nullptr);
    exception.printError();
    return -4;
  }
}

