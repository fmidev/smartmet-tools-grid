#include "contentServer/corba/client/ClientImplementation.h"
#include "contentServer/http/client/ClientImplementation.h"
#include "contentServer/redis/RedisImplementation.h"
#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"
#include "grid-files/identification/GridDef.h"

using namespace SmartMet;




int main(int argc, char *argv[])
{
  try
  {
    if (argc < 4)
    {
      fprintf(stdout,"USAGE: cs_getProducerParameterList <sessionId> <sourceParameterKeyType> <targetParameterKeyType> [[-http <url>]|[-redis <address> <port> <tablePrefix>]]\n");
      return -1;
    }

    T::SessionId sessionId = (SmartMet::T::SessionId)atoll(argv[1]);
    T::ParamKeyType sourceParameterKeyType = (T::ParamKeyType)atoll(argv[2]);
    T::ParamKeyType targetParameterKeyType = (T::ParamKeyType)atoll(argv[3]);

    std::set<std::string> infoList;
    int result = 0;
    unsigned long long startTime = 0;
    unsigned long long endTime = 0;

    char *configFile = getenv(SMARTMET_GRID_CONFIG_FILE);
    if (configFile == NULL)
    {
      fprintf(stderr,"%s not defined!\n",SMARTMET_GRID_CONFIG_FILE);
      return -1;
    }

    Identification::gridDef.init(configFile);


    if (strcmp(argv[argc-2],"-http") == 0)
    {
      ContentServer::HTTP::ClientImplementation service;
      service.init(argv[argc-1]);

      startTime = getTime();
      result = service.getProducerParameterList(sessionId,sourceParameterKeyType,targetParameterKeyType,infoList);
      endTime = getTime();
    }
    else
    if (argc > 4  &&  strcmp(argv[argc-4],"-redis") == 0)
    {
      ContentServer::RedisImplementation service;
      service.init(argv[argc-3],atoi(argv[argc-2]),argv[argc-1]);

      startTime = getTime();
      result = service.getProducerParameterList(sessionId,sourceParameterKeyType,targetParameterKeyType,infoList);
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
      result = service.getProducerParameterList(sessionId,sourceParameterKeyType,targetParameterKeyType,infoList);
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
      std::cout << *it << "\n";
      std::vector<std::string> partList;

      splitString(it->c_str(),';',partList);
      if (partList.size() >= 7)
      {
        std::cout << partList[0] << ";" << partList[1] << ";" << partList[2] << ";" << partList[3] << ";" << partList[4] << ";" << partList[5] << ";" << partList[6] << ";";

        Identification::FmiParameterDef paramDef;

        bool found = false;
        if (targetParameterKeyType == T::ParamKeyType::FMI_NAME)
          found = Identification::gridDef.getFmiParameterDefByName(partList[3],paramDef);
        else
        if (targetParameterKeyType == T::ParamKeyType::FMI_ID)
          found = Identification::gridDef.getFmiParameterDefById(partList[3],paramDef);
        else
        if (targetParameterKeyType == T::ParamKeyType::NEWBASE_ID)
          found = Identification::gridDef.getFmiParameterDefByNewbaseId(partList[3],paramDef);

        if (found)
        {
          if (paramDef.mAreaInterpolationMethod >= 0)
            std::cout << (int)paramDef.mAreaInterpolationMethod << ";";
          else
            std::cout << ";";

          if (paramDef.mTimeInterpolationMethod >= 0)
            std::cout << (int)paramDef.mTimeInterpolationMethod << ";";
          else
            std::cout << ";";

          if (paramDef.mLevelInterpolationMethod >= 0)
            std::cout << (int)paramDef.mLevelInterpolationMethod << ";";
          else
            std::cout << ";";

          std::cout << "D;";

          if (sourceParameterKeyType == T::ParamKeyType::NEWBASE_ID || sourceParameterKeyType == T::ParamKeyType::NEWBASE_NAME)
          {
            Identification::FmiParameterId_newbase paramMapping;
            if (Identification::gridDef.getNewbaseParameterMappingByFmiId(paramDef.mFmiParameterId,paramMapping))
            {
              std::cout << paramMapping.mConversionFunction;
            }
          }
          std::cout << ";\n";
        }
        else
        {
          std::cout << "1;1;1;D;;\n";
        }
      }

      //printf("%s\n",it->c_str());
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

