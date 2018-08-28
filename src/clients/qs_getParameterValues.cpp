#include "grid-content/contentServer/corba/client/ClientImplementation.h"
#include "grid-content/queryServer/corba/client/ClientImplementation.h"
#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"

#include <stdlib.h>

using namespace SmartMet;



int main(int argc, char *argv[])
{
  try
  {
    char *contentServerIor = getenv("SMARTMET_CS_IOR");
    if (contentServerIor == nullptr)
    {
      fprintf(stdout,"SMARTMET_CS_IOR not defined!\n");
      return -2;
    }

    char *queryServerIor = getenv("SMARTMET_QS_IOR");
    if (queryServerIor == nullptr)
    {
      fprintf(stdout,"SMARTMET_QS_IOR not defined!\n");
      return -2;
    }

    if (argc < 17)
    {
      fprintf(stdout,"USAGE:\n");
      fprintf(stdout,"  qs_getParameterValues <sessionId>  <parameterIdType> <parameterKey> \n");
      fprintf(stdout,"     <parameterLevelIdType> <parameterLevelId> <minLevel> <maxLevel> <forecastType> <forecastNumber> \n");
      fprintf(stdout,"     <startTime> <endTime> <coordinatesType> <x-coordinate> <y-coordinate> \n");
      fprintf(stdout,"     <interpolationMethod>\n");
      fprintf(stdout,"WHERE:\n");
      fprintf(stdout,"  sessionId             = Session identifier\n");
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
      fprintf(stdout,"  minLevel               = Minimum parameter level\n");
      fprintf(stdout,"  maxLevel               = Maximum parameter level\n");
      fprintf(stdout,"  forcastType            = Forecast type\n");
      fprintf(stdout,"  forecastNumber         = Forecast number\n");
      fprintf(stdout,"  geometryId             = GeometryId\n");
      fprintf(stdout,"  startTime              = First accepted grid time\n");
      fprintf(stdout,"  endTime                = Last accepted grid time\n");
      fprintf(stdout,"  coordinatesType        = Coordinates type: \n");
      fprintf(stdout,"                            latlon       => Latitude-longitude\n");
      fprintf(stdout,"                            grid         => Grid table coordinates\n");
      fprintf(stdout,"                            original     => Grid original coordinates\n");
      fprintf(stdout,"  x-coordinate           = X-coordinate (=> longitude)\n");
      fprintf(stdout,"  y-coordinate           = Y-coordinate (=> latitude)\n");
      fprintf(stdout,"  interpolationMethod    = Interpolation method:\n");
      fprintf(stdout,"                             nearest      => Nearest value\n");
      fprintf(stdout,"                             linear       => Linear interpolation\n");
      return -2;
    }


    // ### Session:
    T::SessionId sessionId = (SmartMet::T::SessionId)atoll(argv[1]);


    // ### Service parameters:
    T::ParamKeyType paramKeyType = T::ParamKeyType::FMI_ID;
    if (strcmp(argv[2],"fmi-id") == 0)
      paramKeyType = T::ParamKeyType::FMI_ID;
    else
    if (strcmp(argv[2],"fmi-name") == 0)
      paramKeyType = T::ParamKeyType::FMI_NAME;
    else
    if (strcmp(argv[2],"grib-id") == 0)
      paramKeyType = T::ParamKeyType::GRIB_ID;
    else
    if (strcmp(argv[2],"cdm-id") == 0)
      paramKeyType = T::ParamKeyType::CDM_ID;
    else
    if (strcmp(argv[2],"cdm-name") == 0)
      paramKeyType = T::ParamKeyType::CDM_NAME;
    else
    if (strcmp(argv[2],"newbase-id") == 0)
      paramKeyType = T::ParamKeyType::NEWBASE_ID;
    else
    if (strcmp(argv[2],"newbase-name") == 0)
      paramKeyType = T::ParamKeyType::NEWBASE_NAME;

    T::ParamId parameterKey = argv[3];
    T::ParamLevelIdType parameterLevelIdType = T::ParamLevelIdType::ANY;

    if (strcmp(argv[4],"any") == 0)
      parameterLevelIdType = T::ParamLevelIdType::ANY;
    else
    if (strcmp(argv[4],"fmi") == 0)
      parameterLevelIdType = T::ParamLevelIdType::FMI;
    else
    if (strcmp(argv[4],"grib1") == 0)
      parameterLevelIdType = T::ParamLevelIdType::GRIB1;
    else
    if (strcmp(argv[4],"grib2") == 0)
      parameterLevelIdType = T::ParamLevelIdType::GRIB2;
    else
    if (strcmp(argv[4],"ignore") == 0)
      parameterLevelIdType = T::ParamLevelIdType::IGNORE;

    T::ParamLevelId parameterLevelId = (T::ParamLevelId)atoll(argv[5]);
    T::ParamLevel minLevel = (T::ParamLevel)atoll(argv[6]);
    T::ParamLevel maxLevel = (T::ParamLevel)atoll(argv[7]);
    T::ForecastType forecastType = (T::ForecastType)atoll(argv[8]);
    T::ForecastNumber forecastNumber = (T::ForecastNumber)atoll(argv[9]);
    T::GeometryId geometryId = (T::GeometryId)atoll(argv[10]);
    std::string start = argv[11];
    std::string end = argv[12];

    T::CoordinateType coordinateType = T::CoordinateType::LATLON_COORDINATES;

    if (strcmp(argv[13],"latlon") == 0)
      coordinateType = T::CoordinateType::LATLON_COORDINATES;
    else
    if (strcmp(argv[13],"grid") == 0)
      coordinateType = T::CoordinateType::GRID_COORDINATES;
    else
    if (strcmp(argv[13],"original") == 0)
      coordinateType = T::CoordinateType::ORIGINAL_COORDINATES;

    double x = atof(argv[14]);
    double y = atof(argv[15]);

    short interpolationMethod = T::AreaInterpolationMethod::Linear;

    if (strcmp(argv[16],"linear") == 0)
      interpolationMethod = T::AreaInterpolationMethod::Linear;
    else
    if (strcmp(argv[16],"nearest") == 0)
      interpolationMethod = T::AreaInterpolationMethod::Nearest;


    // ### Service:

    QueryServer::Corba::ClientImplementation queryService;
    queryService.init(queryServerIor);


    ContentServer::Corba::ClientImplementation contentService;
    contentService.init(contentServerIor);

    unsigned long long startTime = getTime();
    T::ContentInfoList contentInfoList;

    int result = contentService.getContentListByParameter(sessionId,paramKeyType,parameterKey,parameterLevelIdType,parameterLevelId,minLevel,maxLevel,forecastType,forecastNumber,geometryId,start,end,0,contentInfoList);

    T::GridPointValueList valueList;

    queryService.getValuesByGridPoint(sessionId,contentInfoList,coordinateType,x,y,interpolationMethod,valueList);

    unsigned long long endTime = getTime();

    if (result != 0)
    {
      fprintf(stdout,"ERROR (%d) : %s\n",result,QueryServer::getResultString(result).c_str());
      return -3;
    }

    uint len = contentInfoList.getLength();
    if (len != (uint)valueList.getLength())
    {
      printf("ERROR: The length of the value list (%u) should be the same as the length of content list (%u)!\n",(uint)valueList.getLength(),len);
    }

    for (uint t=0; t<len; t++)
    {
      T::ContentInfo *contentInfo = contentInfoList.getContentInfoByIndex(t);
      T::GridPointValue *point = valueList.getGridPointValueByIndex(t);
      if (point != nullptr  &&  point->mValue != ParamValueMissing)
        printf("%u;%u;%s;%u;%f\n",contentInfo->mGenerationId,contentInfo->mGrib1ParameterLevelId,contentInfo->mForecastTime.c_str(),contentInfo->mParameterLevel,point->mValue);
      else
        printf("%u;%u;%s;%u;None\n",contentInfo->mGenerationId,contentInfo->mGrib1ParameterLevelId,contentInfo->mForecastTime.c_str(),contentInfo->mParameterLevel);
    }

    // ### Result:
    //infoList.print(std::cout,0,0);

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

