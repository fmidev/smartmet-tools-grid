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
    if (serviceIor == nullptr)
    {
      fprintf(stdout,"SMARTMET_DS_IOR not defined!\n");
      return -2;
    }


    if (argc != 10)
    {
      fprintf(stdout,"USAGE: ds_getGridValueListByArea <sessionId> <fileId> <messageIndex> <flags> <coordinateType> <x1> <y1> <x2> <y2>\n");
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
    double x1 = (double)atof(argv[6]);
    double y1 = (double)atof(argv[7]);
    double x2 = (double)atof(argv[8]);
    double y2 = (double)atof(argv[9]);
    T::GridValueList valueList;

    unsigned long long startTime = getTime();
    int result = dataServer.getGridValueListByRectangle(sessionId,fileId,messageIndex,flags,coordinateType,x1,y1,x2,y2,valueList);
    unsigned long long endTime = getTime();


    if (result != 0)
    {
      fprintf(stdout,"ERROR (%d) : %s\n",result,DataServer::getResultString(result).c_str());
      return -5;
    }

    // ### Printing the result:

    valueList.print(std::cout,0,0);

    printf("\n");
    printf("- Values = %u\n",valueList.getLength());
    printf("- Min    = %f\n",valueList.getMinValue());
    printf("- Max    = %f\n",valueList.getMaxValue());
    printf("- Avg    = %f\n",valueList.getAverageValue());

    printf("\nTIME : %f sec\n\n",(float)(endTime-startTime)/1000000);

    return 0;
  }
  catch (SmartMet::Spine::Exception& e)
  {
    SmartMet::Spine::Exception exception(BCP,"Service call failed!",nullptr);
    exception.printError();
    return -6;
  }
}

