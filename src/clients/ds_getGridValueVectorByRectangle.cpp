#include "contentServer/corba/client/ClientImplementation.h"
#include "dataServer/corba/client/ClientImplementation.h"
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

    if (argc != 13)
    {
      fprintf(stdout,"USAGE: ds_getGridValueVectorByRectangle <sessionId> <fileId> <messageIndex> <flags> <coordinateType> <columns> <rows> <x> <y> <xStep> <yStep> <interpolationMethod>\n");
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
    uint columns = (uint)atoll(argv[6]);
    uint rows = (uint)atoll(argv[7]);
    double x = (double)atof(argv[8]);
    double y = (double)atof(argv[9]);
    double xStep = (double)atof(argv[10]);
    double yStep = (double)atof(argv[11]);
    T::InterpolationMethod interpolationMethod = (T::InterpolationMethod)atoll(argv[12]);
    T::ParamValue_vec values;

    unsigned long long startTime = getTime();
    int result = dataServer.getGridValueVectorByRectangle(sessionId,fileId,messageIndex,flags,coordinateType,columns,rows,x,y,xStep,yStep,interpolationMethod,values);
    unsigned long long endTime = getTime();


    if (result != 0)
    {
      fprintf(stdout,"ERROR (%d) : %s\n",result,DataServer::getResultString(result).c_str());
      return -5;
    }

    // ### Printing the result:

    if ((columns*rows) != (uint)values.size())
    {
      printf("The request returned incorrect number of values (%u != %u x %u)\n",(uint)values.size(),columns,rows);
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
  catch (SmartMet::Spine::Exception& e)
  {
    SmartMet::Spine::Exception exception(BCP,"Service call failed!",NULL);
    exception.printError();
    return -7;
  }
}

