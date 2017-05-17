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
      fprintf(stdout,"USAGE: cs_addGenerationInfo <sessionId> <producerId> <generationType> <name> <description> <status> [-http <url>]\n");
      return -1;
    }

    T::GenerationInfo info;
    T::SessionId sessionId = (SmartMet::T::SessionId)atoll(argv[1]);
    info.mProducerId = (uint)atoll(argv[2]);
    info.mGenerationType = (uint)atoll(argv[3]);
    info.mName = argv[4];
    info.mDescription = argv[5];
    info.mStatus = (T::GenerationStatus)atoll(argv[6]);

    int result = 0;
    unsigned long long startTime = 0;
    unsigned long long endTime = 0;

    if (argc == 9  &&  strcmp(argv[7],"-http") == 0)
    {
      ContentServer::HTTP::ClientImplementation service;
      service.init(argv[8]);

      startTime = getTime();
      result = service.addGenerationInfo(sessionId,info);
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
      result = service.addGenerationInfo(sessionId,info);
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

    return info.mGenerationId;
  }
  catch (SmartMet::Spine::Exception& e)
  {
    SmartMet::Spine::Exception exception(BCP,"Service call failed!",NULL);
    exception.printError();
    return -4;
  }
}

