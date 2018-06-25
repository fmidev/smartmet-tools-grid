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
    if (argc < 24)
    {
      fprintf(stdout,"USAGE:\n");
      fprintf(stdout,"  cs_addContentInfo <1:sessionId> <2:fileId> <3:fileType> <4:messageIndex>\n");
      fprintf(stdout,"     <5:producerId> <6:generationId> <7:groupFlags> <8:startTime> <9:endTime> \n");
      fprintf(stdout,"     <10:fmiParameterId> <11:gribParameterId> <12:cdmParameterId> <13:cdmParameterName>\n");
      fprintf(stdout,"     <14:newbaseParameterId> <15:newbaseParameterName>\n");
      fprintf(stdout,"     <16:fmiParameterLevelId> <17:grib1ParameterLevelId> <18:grib2ParameterLevelId> \n");
      fprintf(stdout,"     <19:parameterLevel> <20:fmiParameterUnits> <21:gribParameterUnits> <22:sourceId> <23:flags> [[-http <url>]|[-redis <address> <port> <tablePrefix>]]\n");
      return -1;
    }

    T::ContentInfo info;
    T::SessionId sessionId = (SmartMet::T::SessionId)atoll(argv[1]);
    info.mFileId  = (uint)atoll(argv[2]);
    info.mFileType  = (T::FileType)atoll(argv[3]);
    info.mMessageIndex  = (uint)atoll(argv[4]);
    info.mProducerId  = (uint)atoll(argv[5]);
    info.mGenerationId  = (uint)atoll(argv[6]);
    info.mGroupFlags  = (uint)atoll(argv[7]);
    info.mForecastTime = argv[8];
    info.mFmiParameterId = argv[9];
    info.mGribParameterId = argv[10];
    info.mCdmParameterId = argv[11];
    info.mCdmParameterName = argv[12];
    info.mNewbaseParameterId = argv[13];
    info.mNewbaseParameterName = argv[14];
    info.mFmiParameterLevelId  = (T::ParamLevelId)atoll(argv[15]);
    info.mGrib1ParameterLevelId  = (T::ParamLevelId)atoll(argv[16]);
    info.mGrib2ParameterLevelId  = (T::ParamLevelId)atoll(argv[17]);
    info.mParameterLevel  = (uint)atoll(argv[18]);
    info.mFmiParameterUnits = argv[19];
    info.mGribParameterUnits = argv[20];
    info.mSourceId  = (uint)atoll(argv[21]);
    info.mFlags  = (uint)atoll(argv[22]);

    int result = 0;
    unsigned long long startTime = 0;
    unsigned long long endTime = 0;

    if (strcmp(argv[argc-2],"-http") == 0)
    {
      ContentServer::HTTP::ClientImplementation service;
      service.init(argv[argc-1]);

      startTime = getTime();
      result = service.addContentInfo(sessionId,info);
      endTime = getTime();
    }
    else
    if (strcmp(argv[argc-4],"-redis") == 0)
    {
      ContentServer::RedisImplementation service;
      service.init(argv[argc-3],atoi(argv[argc-2]),argv[argc-1]);

      startTime = getTime();
      result = service.addContentInfo(sessionId,info);
      endTime = getTime();
    }
    else
    {
      char *serviceIor = getenv("SMARTMET_CS_IOR");

      if (strcmp(argv[argc-2],"-ior") == 0)
        serviceIor = argv[argc-1];

      if (serviceIor == NULL)
      {
        fprintf(stdout,"Service IOR not defined!\n");
        return -2;
      }

      ContentServer::Corba::ClientImplementation service;
      service.init(serviceIor);

      startTime = getTime();
      result = service.addContentInfo(sessionId,info);
      endTime = getTime();
    }

    if (result != 0)
    {
      fprintf(stdout,"ERROR (%d) : %s\n",result,ContentServer::getResultString(result).c_str());
      return -3;
    }

    // ### Result:
    info.print(std::cout,0,0);

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

