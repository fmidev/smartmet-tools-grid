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

    if (argc != 18)
    {
      fprintf(stdout,"USAGE:\n");
      fprintf(stdout,"  ds_getGridValueByTimeLevelAndPoint <sessionId> <fileId1> <msgIdx1> <fileId2> <msgIdx2> <fileId3> <msgIdx3> <fileId4> <msgIdx4>\n");
      fprintf(stdout,"      <timestamp> <level> <coordinateType> <x> <y> <areaInterpolationMethod> <timeInterpolationMethod> <levelInterpolationMethod>\n");
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
    uint fileId3 = toInt64(argv[6]);
    uint messageIndex3 = toInt64(argv[7]);
    uint fileId4 = toInt64(argv[8]);
    uint messageIndex4 = toInt64(argv[9]);
    std::string timestamp = argv[10];
    int level = toInt64(argv[11]);
    T::CoordinateType coordinateType = toInt64(argv[12]);
    double x = toDouble(argv[13]);
    double y = toDouble(argv[14]);
    short areaInterpolationMethod = toInt16(argv[15]);
    short timeInterpolationMethod = toInt16(argv[16]);
    short levelInterpolationMethod = toInt16(argv[17]);
    T::ParamValue value = 0;

    unsigned long long startTime = getTime();
    int result = dataServer.getGridValueByTimeLevelAndPoint(sessionId,fileId1,messageIndex1,-1,fileId2,messageIndex2,-1,fileId3,messageIndex3,-1,fileId4,messageIndex4,-1,
        timestamp,level,coordinateType,x,y,areaInterpolationMethod,timeInterpolationMethod,levelInterpolationMethod,value);
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

