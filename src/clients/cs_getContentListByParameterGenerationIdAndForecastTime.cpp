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
    if (argc != 11)
    {
      fprintf(stdout,"USAGE:\n");
      fprintf(stdout,"  cs_getContentListByParameterGenerationIdAndForecastTime <sessionId>  <generationId> <parameterIdType>\n");
      fprintf(stdout,"     <parameterKey> <parameterLevelId> <level> <geometryId>\n");
      fprintf(stdout,"     <forecastType> <forecastNumber> <forecastTime>\n");
      fprintf(stdout,"WHERE:\n");
      fprintf(stdout,"  sessionId             = Session identifier\n");
      fprintf(stdout,"  generationId          = Generation identifier\n");
      fprintf(stdout,"  parameterKeyType      = Parameter search key type:\n");
      fprintf(stdout,"                            fmi-id       => Radon identifier\n");
      fprintf(stdout,"                            fmi-name     => Radon name\n");
      fprintf(stdout,"                            grib-id      => GRIB identifier\n");
      fprintf(stdout,"                            newbase-id   => Newbase identifier\n");
      fprintf(stdout,"                            newbase-name => Newbase name\n");
      fprintf(stdout,"  parameterKey          = Parameter search key\n");
      fprintf(stdout,"  level                 = Parameter level\n");
      fprintf(stdout,"  forecastType          = Forecast type\n");
      fprintf(stdout,"  forecastNumber        = Forecast number\n");
      fprintf(stdout,"  geometryId            = Geometry identifier\n");
      fprintf(stdout,"  forecastTime          = Forecast time\n");

      return -1;
    }

    // ### Session:
    T::SessionId sessionId = toInt64(argv[1]);

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
    if (strcmp(argv[3],"newbase-id") == 0)
      paramKeyType = T::ParamKeyTypeValue::NEWBASE_ID;
    else
    if (strcmp(argv[3],"newbase-name") == 0)
      paramKeyType = T::ParamKeyTypeValue::NEWBASE_NAME;

    T::ParamId parameterKey = argv[4];
    T::ParamLevelId parameterLevelId = toInt64(argv[5]);
    T::ParamLevel level = (T::ParamLevel)toInt64(argv[6]);
    T::ForecastType forecastType = (T::ForecastType)toInt64(argv[7]);
    T::ForecastNumber forecastNumber = (T::ForecastNumber)toInt64(argv[8]);
    T::GeometryId geometryId = (T::GeometryId)toInt64(argv[9]);
    std::string forecastTime = argv[10];
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
    int result = service->getContentListByParameterGenerationIdAndForecastTime(sessionId,generationId,paramKeyType,parameterKey,parameterLevelId,level,forecastType,forecastNumber,geometryId,forecastTime,infoList);
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

