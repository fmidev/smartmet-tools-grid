#include "contentServer/corba/client/ClientImplementation.h"
#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"

using namespace SmartMet;


int main(int argc, char *argv[])
{

  try
  {
    char *serviceIor = getenv("SMARTMET_CS_IOR");
    if (serviceIor == NULL)
    {
      fprintf(stdout,"SMARTMET_CS_IOR not defined!\n");
      return -2;
    }

    if (argc != 11)
    {
      fprintf(stdout,"USAGE:\n");
      fprintf(stdout,"  cs_getContentListByParameter <sessionId>  <parameterIdType> <parameterKey> \n");
      fprintf(stdout,"     <parameterLevelIdType> <parameterLevelId> <minLevel> <maxLevel> \n");
      fprintf(stdout,"     <startTime> <endTime> <requestFlags>\n");
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
      fprintf(stdout,"  startTime              = First accepted grid time\n");
      fprintf(stdout,"  endTime                = Last accepted grid time\n");
      fprintf(stdout,"  requestFlags           = Request flags\n");

      return -1;
    }

    // ### Session:
    T::SessionId sessionId = (SmartMet::T::SessionId)atoll(argv[1]);

    // ### Service:
    ContentServer::Corba::ClientImplementation contentServer;
    contentServer.init(serviceIor);

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
    std::string start = argv[8];
    std::string end = argv[9];
    uint requestFlags = (uint)atoll(argv[10]);
    T::ContentInfoList infoList;

    unsigned long long startTime = getTime();
    int result = contentServer.getContentListByParameter(sessionId,paramKeyType,parameterKey,parameterLevelIdType,parameterLevelId,minLevel,maxLevel,start,end,requestFlags,infoList);
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
    SmartMet::Spine::Exception exception(BCP,"Service call failed!",NULL);
    exception.printError();
    return -4;
  }
}

