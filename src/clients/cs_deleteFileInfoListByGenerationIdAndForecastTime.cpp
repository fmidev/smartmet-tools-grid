#include "contentServer/corba/client/ClientImplementation.h"
#include "contentServer/http/client/ClientImplementation.h"
#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"

using namespace SmartMet;


int main(int argc, char *argv[])
{
  try
  {
    if (argc < 7)
    {
      fprintf(stdout,"USAGE: cs_deleteFileInfoListByGenerationIdAndForecastTime <sessionId> <generationId> <geometryId> <forecastType> <forecastPertubationNumber> <forecastTime> [-http <url>]\n");
      return -1;
    }

    T::SessionId sessionId = (SmartMet::T::SessionId)atoll(argv[1]);
    uint generationId = (uint)atoll(argv[2]);
    uint geometryId = (uint)atoll(argv[3]);
    short forecastType = (short)atoll(argv[4]);
    short pertubationNumber = (short)atoll(argv[5]);
    std::string forecastTime = argv[6];

    int result = 0;
    unsigned long long startTime = 0;
    unsigned long long endTime = 0;

    if (strcmp(argv[argc-2],"-http") == 0)
    {
      ContentServer::HTTP::ClientImplementation service;
      service.init(argv[argc-1]);

      startTime = getTime();
      result = service.deleteFileInfoListByGenerationIdAndForecastTime(sessionId,generationId,geometryId,forecastType,pertubationNumber,forecastTime);
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
      result = service.deleteFileInfoListByGenerationIdAndForecastTime(sessionId,generationId,geometryId,forecastType,pertubationNumber,forecastTime);
      endTime = getTime();
    }

    if (result != 0)
    {
      fprintf(stdout,"ERROR (%d) : %s\n",result,ContentServer::getResultString(result).c_str());
      return -3;
    }

    // ### Result:
    printf("OK\n");

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

