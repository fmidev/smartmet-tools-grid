#include "grid-content/queryServer/corba/client/ClientImplementation.h"
#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"

using namespace SmartMet;



int main(int argc, char *argv[])
{
  try
  {
    if (argc < 6)
    {
      fprintf(stdout,"USAGE: getParameterValuesByPointAndTimeList <sessionId> <producer> <parameter> <x> <y> <time,time,time,...,time>\n");
      return -1;
    }

    T::SessionId sessionId = toInt64(argv[1]);
    string_vec producerList;
    int result = 0;
    std::string producer = argv[2];
    std::string parameter = argv[3];
    double x = toDouble(argv[4]);
    double y = toDouble(argv[5]);
    std::string timeStr = argv[6];

    std::vector<std::string> tList;
    splitString(timeStr,',',tList);

    T::ParamValue_vec values;

    char *serviceIor = getenv("SMARTMET_QS_IOR");

    if (strcmp(argv[argc-2],"-ior") == 0)
      serviceIor = argv[argc-1];

    if (serviceIor == nullptr)
    {
      fprintf(stdout,"Service IOR not defined!\n");
      return -2;
    }

    QueryServer::Corba::ClientImplementation service;
    service.init(serviceIor);

    ulonglong startTime = getTime();
    result = service.getParameterValuesByPointAndTimeList(sessionId,producer,parameter,T::CoordinateTypeValue::LATLON_COORDINATES,x,y,tList,1,1,1,values);
    ulonglong endTime = getTime();

    if (result != 0)
    {
      fprintf(stdout,"ERROR (%d) : %s\n",result,QueryServer::getResultString(result).c_str());
      return -3;
    }

    // ### Result:

    std::cout << "VALUES:\n";
    uint len = values.size();
    if (tList.size() == len)
    {
      for (uint t=0; t<len; t++)
      {
        std::cout << "  "  << tList[t] << " : " << values[t] << "\n";
      }
    }

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

