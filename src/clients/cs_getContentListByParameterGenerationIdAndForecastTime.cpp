#include "grid-content/contentServer/corba/client/ClientImplementation.h"
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

    if (argc != 12)
    {
      fprintf(stdout,"USAGE:\n");
      fprintf(stdout,"  cs_getContentListByParameterGenerationIdAndForecastTime <sessionId>  <generationId> <parameterIdType>\n");
      fprintf(stdout,"     <parameterKey>  <parameterLevelIdType> <parameterLevelId> <level> <geometryId>\n");
      fprintf(stdout,"     <forecastType> <forecastNumber> <forecastTime>\n");
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
      fprintf(stdout,"  level                  = Parameter level\n");
      fprintf(stdout,"  forecastType           = Forecast type\n");
      fprintf(stdout,"  forecastNumber         = Forecast number\n");
      fprintf(stdout,"  geometryId             = Geometry identifier\n");
      fprintf(stdout,"  forecastTime           = Forecast time\n");

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
    T::ParamLevel level = (T::ParamLevel)toInt64(argv[7]);
    T::ForecastType forecastType = (T::ForecastType)toInt64(argv[8]);
    T::ForecastNumber forecastNumber = (T::ForecastNumber)toInt64(argv[9]);
    T::GeometryId geometryId = (T::GeometryId)toInt64(argv[10]);
    std::string forecastTime = argv[11];
    T::ContentInfoList infoList;

    unsigned long long startTime = getTime();
    int result = contentServer.getContentListByParameterGenerationIdAndForecastTime(sessionId,generationId,paramKeyType,parameterKey,parameterLevelIdType,parameterLevelId,level,forecastType,forecastNumber,geometryId,forecastTime,infoList);
    unsigned long long endTime = getTime();

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

