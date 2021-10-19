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

    if (argc != 12)
    {
      fprintf(stdout,"USAGE: ds_getGridValueByTimeAndPoint <sessionId> <fileId1> <msgIndex1> <fileId2> <msgIndex2> <timestamp> <coordinateType> <x> <y> <areaInterpolationMethod> <levelInterpolationMethod>\n");
      return -1;
    }


    // ### Session:
    T::SessionId sessionId = toInt64(argv[1]);


    // ### Creating a dataServer client:

    DataServer::Corba::ClientImplementation dataServer;
    dataServer.init(serviceIor);

    // ### Calling the dataServer:

    uint fileId1 = toInt64(argv[2]);
    uint messageIndex1 = toInt64(argv[3]);
    uint fileId2 = toInt64(argv[4]);
    uint messageIndex2 = toInt64(argv[5]);
    std::string timestamp = argv[6];
    T::CoordinateType coordinateType = toInt64(argv[7]);
    double x = toDouble(argv[8]);
    double y = toDouble(argv[9]);
    short areaInterpolationMethod = toInt16(argv[10]);
    short timeInterpolationMethod = toInt16(argv[11]);
    T::ParamValue value = 0;
    uint modificationOperation = 0;
    double_vec modificationParameters;

    unsigned long long startTime = getTime();
    int result = dataServer.getGridValueByTimeAndPoint(sessionId,fileId1,messageIndex1,fileId2,messageIndex2,timestamp,coordinateType,x,y,areaInterpolationMethod,timeInterpolationMethod,modificationOperation,modificationParameters,value);
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

