#include "grid-content/contentServer/corba/client/ClientImplementation.h"
#include "grid-content/dataServer/corba/client/ClientImplementation.h"
#include <macgyver/Exception.h>
#include "grid-files/common/GeneralFunctions.h"

using namespace SmartMet;


int main(int argc, char *argv[])
{
  try
  {
    char *serviceIor = getenv("SMARTMET_DS_IOR");
    if (serviceIor == nullptr)
    {
      fprintf(stdout,"SMARTMET_DS_IOR not defined!\n");
      return -2;
    }

    if (argc < 8)
    {
      fprintf(stdout,"USAGE: ds_getGridValueByPoint <sessionId> <fileId> <messageIndex> <coordinateType> <x> <y> <areaInterpolationMethod> [<modificationParameters>]\n");
      return -1;
    }


    // ### Session:
    T::SessionId sessionId = toInt64(argv[1]);


    // ### Creating a dataServer client:

    DataServer::Corba::ClientImplementation dataServer;
    dataServer.init(serviceIor);

    // ### Calling the dataServer:

    uint fileId = toInt64(argv[2]);
    uint messageIndex = toInt64(argv[3]);
    T::CoordinateType coordinateType = toInt64(argv[4]);
    double x = toDouble(argv[5]);
    double y = toDouble(argv[6]);
    short areaInterpolationMethod = toInt16(argv[7]);
    T::ParamValue value = 0;
    uint modificationOperation = 0;
    double_vec modificationParameters;

    if (argc == 9)
      splitString(argv[8],',',modificationParameters);

    unsigned long long startTime = getTime();
    int result = dataServer.getGridValueByPoint(sessionId,fileId,messageIndex,coordinateType,x,y,areaInterpolationMethod,modificationOperation,modificationParameters,value);
    unsigned long long endTime = getTime();


    if (result != 0)
    {
      fprintf(stdout,"ERROR (%d) : %s\n",result,DataServer::getResultString(result).c_str());
      return -5;
    }

    // ### Printing the result:

    printf("VALUE %f\n",value);

    printf("\nTIME : %f sec\n\n",(float)(endTime-startTime)/1000000);

    return 0;
  }
  catch (Fmi::Exception& e)
  {
    Fmi::Exception exception(BCP,"Service call failed!",nullptr);
    exception.printError();
    return -6;
  }
}

