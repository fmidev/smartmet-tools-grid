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
      fprintf(stdout,"USAGE: ds_getGridValueVectorByRectangle <sessionId> <fileId> <messageIndex> <coordinateType> <columns> <rows> <x> <y> <xStep> <yStep> <interpolationMethod>\n");
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
    uint columns = toInt64(argv[5]);
    uint rows = toInt64(argv[6]);
    double x = toDouble(argv[7]);
    double y = toDouble(argv[8]);
    double xStep = toDouble(argv[9]);
    double yStep = toDouble(argv[10]);
    short interpolationMethod = (short)toInt64(argv[11]);
    T::ParamValue_vec values;

    unsigned long long startTime = getTime();
    int result = dataServer.getGridValueVectorByRectangle(sessionId,fileId,messageIndex,coordinateType,columns,rows,x,y,xStep,yStep,interpolationMethod,values);
    unsigned long long endTime = getTime();


    if (result != 0)
    {
      fprintf(stdout,"ERROR (%d) : %s\n",result,DataServer::getResultString(result).c_str());
      return -5;
    }

    // ### Printing the result:

    if ((columns*rows) != values.size())
    {
      printf("The request returned incorrect number of values (%lu != %u x %u)\n",values.size(),columns,rows);
      return -6;
    }

    uint cnt = 0;
    double yy = y;
    for (uint r=0; r<rows; r++)
    {
      double xx = x;
      for (uint c=0; c<columns; c++)
      {
        printf("(%u,%u) (%f,%f) : %f\n",c,r,xx,yy,values[cnt]);
        cnt++;
        xx += xStep;
      }
      yy += yStep;
    }


    printf("\nTIME : %f sec\n\n",(float)(endTime-startTime)/1000000);

    return 0;
  }
  catch (Fmi::Exception& e)
  {
    Fmi::Exception exception(BCP,"Service call failed!",nullptr);
    exception.printError();
    return -7;
  }
}

