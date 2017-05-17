#include "contentServer/corba/client/ClientImplementation.h"
#include "dataServer/corba/client/ClientImplementation.h"
#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"

using namespace SmartMet;


int main(int argc, char *argv[])
{
  try
  {
    char *contentServiceIor = getenv("SMARTMET_CS_IOR");
    if (contentServiceIor == NULL)
    {
      fprintf(stdout,"SMARTMET_CS_IOR not defined!\n");
      return -2;
    }


    if (argc != 13)
    {
      fprintf(stdout,"USAGE: cs_getGridValuesByArea <sessionId> <dataServerId> <fileId> <messageIndex> <coordinateType> <columns> <rows> <x> <y> <xStep> <yStep> <interpolationMethod>\n");
      return -1;
    }


    // ### Session:
    T::SessionId sessionId = (SmartMet::T::SessionId)atoll(argv[1]);


    // #######################################################################
    // ### STEP 1: Getting the dataServer IOR from the contentServer.
    // #######################################################################

    // ### Creating a contentServer client:

    ContentServer::Corba::ClientImplementation contentServer;
    contentServer.init(contentServiceIor);

    // ### Calling the contentServer:

    uint dataServerId = (uint)atoll(argv[2]);
    T::ServerInfo dataServerInfo;
    int result = 0;

    if (dataServerId != 0)
      result = contentServer.getDataServerInfoById(sessionId,dataServerId,dataServerInfo);
    else
      result = contentServer.getDataServerInfoByName(sessionId,std::string(argv[2]),dataServerInfo);


    if (result == ContentServer::Result::DATA_NOT_FOUND)
    {
      fprintf(stdout,"Unknown data server (%s)!\n",argv[2]);
      return -3;
    }

    if (result != 0)
    {
      fprintf(stdout,"ERROR (%d) : %s\n",result,ContentServer::getResultString(result).c_str());
      return -4;
    }


    // #######################################################################
    // ### STEP 2: Requesting data from the dataServer.
    // #######################################################################

    // ### Creating a dataServer client:

    DataServer::Corba::ClientImplementation dataServer;
    dataServer.init(dataServerInfo.mServerIor);

    // ### Calling the dataServer:

    uint fileId = (uint)atoll(argv[3]);
    uint messageIndex = (uint)atoll(argv[4]);
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
    result = dataServer.getGridValuesByArea(sessionId,fileId,messageIndex,coordinateType,columns,rows,x,y,xStep,yStep,interpolationMethod,values);
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

