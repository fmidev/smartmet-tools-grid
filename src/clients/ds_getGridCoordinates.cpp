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


    if (argc != 5)
    {
      fprintf(stdout,"USAGE: cs_getGridCoordinates <sessionId> <fileId> <messageIndex> <coordinateType>\n");
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
    T::GridCoordinates gridCoordinates;

    unsigned long long startTime = getTime();
    int result = dataServer.getGridCoordinates(sessionId,fileId,messageIndex,coordinateType,gridCoordinates);
    unsigned long long endTime = getTime();


    if (result != 0)
    {
      fprintf(stdout,"ERROR (%d) : %s\n",result,DataServer::getResultString(result).c_str());
      return -5;
    }

    // ### Printing the result:

    gridCoordinates.print(std::cout,0,0);

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

