#include "grid-content/contentServer/corba/client/ClientImplementation.h"
#include "grid-content/dataServer/corba/client/ClientImplementation.h"
#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"

using namespace SmartMet;


int main(int argc, char *argv[])
{
  try
  {
    char *serviceIor = getenv("SMARTMET_DS_IOR");
    if (serviceIor == NULL)
    {
      fprintf(stdout,"SMARTMET_DS_IOR not defined!\n");
      return -2;
    }

    if (argc != 9)
    {
      fprintf(stdout,"USAGE: ds_getGridValueVectorByPoint <sessionId> <fileId> <messageIndex> <flags> <coordinateType> <x> <y> <vectorType>\n");
      return -1;
    }


    // ### Session:
    T::SessionId sessionId = (SmartMet::T::SessionId)atoll(argv[1]);


    // ### Creating a dataServer client:

    DataServer::Corba::ClientImplementation dataServer;
    dataServer.init(serviceIor);

    // ### Calling the dataServer:

    uint fileId = (uint)atoll(argv[2]);
    uint messageIndex = (uint)atoll(argv[3]);
    uint flags = (uint)atoll(argv[4]);
    T::CoordinateType coordinateType = (T::CoordinateType)atoll(argv[5]);
    double x = (double)atof(argv[6]);
    double y = (double)atof(argv[7]);
    uint vectorType = (uint)atoll(argv[8]);
    double_vec valueVector;

    unsigned long long startTime = getTime();
    int result = dataServer.getGridValueVectorByPoint(sessionId,fileId,messageIndex,flags,coordinateType,x,y,vectorType,valueVector);
    unsigned long long endTime = getTime();


    if (result != 0)
    {
      fprintf(stdout,"ERROR (%d) : %s\n",result,DataServer::getResultString(result).c_str());
      return -5;
    }

    // ### Printing the result:

    for (auto it = valueVector.begin(); it != valueVector.end(); ++it)
      std::cout << *it << "\n";

    printf("\nTIME : %f sec\n\n",(float)(endTime-startTime)/1000000);

    return 0;
  }
  catch (SmartMet::Spine::Exception& e)
  {
    SmartMet::Spine::Exception exception(BCP,"Service call failed!",NULL);
    exception.printError();
    return -6;
  }
}

