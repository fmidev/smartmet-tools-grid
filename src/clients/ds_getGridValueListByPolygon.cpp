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


    if (argc < 7)
    {
      fprintf(stdout,"USAGE: ds_getGridValueListByPolygon <sessionId> <fileId> <messageIndex> <coordinateType> <x1,y1> <x2,y2> <x3,y3> [<x4,y4> ... <xN,yN>]\n");
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
    std::vector<T::Coordinate> polygonPoints;
    T::GridValueList valueList;

    char buf[100];
    for (int t=5; t<argc; t++)
    {
      strcpy(buf,argv[t]);
      char *p = strstr(buf,",");
      if (p != nullptr)
      {
        *p = '\0';
        p++;
        double x = toDouble(buf);
        double y = toDouble(p);
        polygonPoints.push_back(T::Coordinate(x,y));
        printf("Coordinate %f,%f\n",x,y);
      }
    }

    unsigned long long startTime = getTime();
    int result = dataServer.getGridValueListByPolygon(sessionId,fileId,messageIndex,coordinateType,polygonPoints,valueList);
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

