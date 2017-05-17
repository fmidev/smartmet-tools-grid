#include "contentServer/corba/client/ClientImplementation.h"
#include "contentServer/http/client/ClientImplementation.h"
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

    if (argc < 23)
    {
      fprintf(stdout,"USAGE:\n");
      fprintf(stdout,"  cs_addContentInfo <1:sessionId> <2:fileId> <3:fileType> <4:messageIndex>\n");
      fprintf(stdout,"     <5:producerId> <6:generationId> <7:groupFlags> <8:startTime> <9:endTime> \n");
      fprintf(stdout,"     <10:fmiParameterId> <11:gribParameterId> <12:cdmParameterId> <13:cdmParameterName>\n");
      fprintf(stdout,"     <14:newbaseParameterId> <15:newbaseParameterName>\n");
      fprintf(stdout,"     <16:fmiParameterLevelId> <17:grib1ParameterLevelId> <18:grib2ParameterLevelId> \n");
      fprintf(stdout,"     <19:parameterLevel> <20:fmiParameterUnits> <21:gribParameterUnits> <22:flags> [-http <url>]\n");
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
    info.mStartTime = argv[8];
    info.mEndTime = argv[9];
    info.mFmiParameterId = argv[10];
    info.mGribParameterId = argv[11];
    info.mCdmParameterId = argv[12];
    info.mCdmParameterName = argv[13];
    info.mNewbaseParameterId = argv[14];
    info.mNewbaseParameterName = argv[15];
    info.mFmiParameterLevelId  = (T::ParamLevelId)atoll(argv[16]);
    info.mGrib1ParameterLevelId  = (T::ParamLevelId)atoll(argv[17]);
    info.mGrib2ParameterLevelId  = (T::ParamLevelId)atoll(argv[18]);
    info.mParameterLevel  = (uint)atoll(argv[19]);
    info.mFmiParameterUnits = argv[20];
    info.mGribParameterUnits = argv[21];
    info.mFlags  = (uint)atoll(argv[22]);

    int result = 0;
    unsigned long long startTime = 0;
    unsigned long long endTime = 0;

    if (argc == 25  &&  strcmp(argv[23],"-http") == 0)
    {
      ContentServer::HTTP::ClientImplementation service;
      service.init(argv[24]);

      startTime = getTime();
      result = service.addContentInfo(sessionId,info);
      endTime = getTime();
    }
    else
    {
      char *serviceIor = getenv("SMARTMET_CS_IOR");
      if (serviceIor == NULL)
      {
        fprintf(stdout,"SMARTMET_CS_IOR not defined!\n");
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

