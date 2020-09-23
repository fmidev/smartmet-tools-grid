#include "grid-content/queryServer/corba/client/ClientImplementation.h"
#include <macgyver/Exception.h>
#include "grid-files/common/GeneralFunctions.h"

using namespace SmartMet;



int main(int argc, char *argv[])
{
  try
  {
    if (argc < 6)
    {
      fprintf(stdout,"USAGE: getParameterValueByPointAndTime <sessionId> <producer> <parameter> <x> <y> <time>\n");
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
    T::ParamValue value;

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
    result = service.getParameterValueByPointAndTime(sessionId,producer,parameter,T::CoordinateTypeValue::LATLON_COORDINATES,x,y,timeStr,1,1,1,value);
    ulonglong endTime = getTime();

    if (result != 0)
    {
      fprintf(stdout,"ERROR (%d) : %s\n",result,QueryServer::getResultString(result).c_str());
      return -3;
    }

    // ### Result:

    std::cout << "VALUE : " << value << "\n";

    printf("\nTIME : %f sec\n\n",(float)(endTime-startTime)/1000000);

    return 0;
  }
  catch (Fmi::Exception& e)
  {
    Fmi::Exception exception(BCP,"Service call failed!",nullptr);
    exception.printError();
    return -4;
  }
}

