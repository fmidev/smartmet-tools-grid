#include "contentServer/corba/client/ClientImplementation.h"
#include "contentServer/http/client/ClientImplementation.h"
#include "contentServer/redis/RedisImplementation.h"
#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"
#include "grid-files/identification/GridDef.h"

using namespace SmartMet;




void init()
{
  try
  {
    char *configFile = getenv(SMARTMET_GRID_CONFIG_FILE);
    if (configFile == NULL)
    {
      printf("%s not defined!\n",SMARTMET_GRID_CONFIG_FILE);
      exit(-1);
    }

    // Initializing the global structures. These are needed when
    // extracting information from GRIB files.

    Identification::gridDef.init(configFile);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,NULL);
  }
}



int main(int argc, char *argv[])
{
  try
  {
    if (argc < 2)
    {
      fprintf(stdout,"USAGE: cs_getProducerNameAndGeometryList <sessionId> [[-http <url>]|[-redis <address> <port> <tablePrefix>]]\n");
      return -1;
    }

    init();

    T::SessionId sessionId = (SmartMet::T::SessionId)atoll(argv[1]);
    std::set<std::string> infoList;
    int result = 0;
    unsigned long long startTime = 0;
    unsigned long long endTime = 0;

    if (strcmp(argv[argc-2],"-http") == 0)
    {
      ContentServer::HTTP::ClientImplementation service;
      service.init(argv[argc-1]);

      startTime = getTime();
      result = service.getProducerNameAndGeometryList(sessionId,infoList);
      endTime = getTime();
    }
    else
    if (argc > 4  &&  strcmp(argv[argc-4],"-redis") == 0)
    {
      ContentServer::RedisImplementation service;
      service.init(argv[argc-3],atoi(argv[argc-2]),argv[argc-1]);

      startTime = getTime();
      result = service.getProducerNameAndGeometryList(sessionId,infoList);
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
      result = service.getProducerNameAndGeometryList(sessionId,infoList);
      endTime = getTime();
    }

    if (result != 0)
    {
      fprintf(stdout,"ERROR (%d) : %s\n",result,ContentServer::getResultString(result).c_str());
      return -3;
    }

    // ### Result:
    for (auto it=infoList.begin(); it != infoList.end(); ++it)
    {
      std::vector<std::string> partList;
      splitString(it->c_str(),';',partList);
      if (partList.size() >= 2)
      {
        double width = 0, height = 0;
        if (Identification::gridDef.getGridCellAverageSizeByGeometryId((T::GeometryId)atoi(partList[1].c_str()),width,height))
          printf("%s;%s;%.1fkm x %.1fkm;\n",partList[0].c_str(),partList[1].c_str(),width,height);
        else
          printf("%s;%s;;\n",partList[0].c_str(),partList[1].c_str());
      }

    }

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

