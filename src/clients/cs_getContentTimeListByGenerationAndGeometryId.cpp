#include "contentServer/corba/client/ClientImplementation.h"
#include "contentServer/http/client/ClientImplementation.h"
#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"
#include <vector>


using namespace SmartMet;


int main(int argc, char *argv[])
{
  try
  {
    if (argc < 4)
    {
      fprintf(stdout,"USAGE: cs_getContentTimeListByGenerationAndGeometryId <sessionId> <generationId> <geometryId>[-http <url>]\n");
      return -1;
    }

    T::SessionId sessionId = (SmartMet::T::SessionId)atoll(argv[1]);
    uint generationId = (uint)atoll(argv[2]);
    uint geometryId = (uint)atoll(argv[2]);
    std::set<std::string> timeList;

    int result = 0;
    unsigned long long startTime = 0;
    unsigned long long endTime = 0;

    if (argc == 6  &&  strcmp(argv[4],"-http") == 0)
    {
      ContentServer::HTTP::ClientImplementation service;
      service.init(argv[5]);

      startTime = getTime();
      result = service.getContentTimeListByGenerationAndGeometryId(sessionId,generationId,geometryId,timeList);
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
      result = service.getContentTimeListByGenerationAndGeometryId(sessionId,generationId,geometryId,timeList);
      endTime = getTime();
    }

    if (result != 0)
    {
      fprintf(stdout,"ERROR (%d) : %s\n",result,ContentServer::getResultString(result).c_str());
      return -3;
    }

    // ### Result:
    for (auto it=timeList.begin(); it!=timeList.end(); ++it)
      printf("%s\n",it->c_str());


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

