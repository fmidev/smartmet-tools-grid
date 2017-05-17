#include "contentServer/corba/client/ClientImplementation.h"
#include "dataServer/corba/client/ClientImplementation.h"
#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"

using namespace SmartMet;

long long values[3000000] = {0};
uint valueCount = 0;


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


    if (argc != 5)
    {
      fprintf(stdout,"USAGE: cs_getGridData <sessionId> <dataServerId> <fileId> <messageIndex>\n");
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
      fprintf(stdout,"Unknown data server id (%s)!\n",argv[2]);
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
    T::GridData gridData;

    unsigned long long startTime = getTime();
    result = dataServer.getGridData(sessionId,fileId,messageIndex,gridData);
    unsigned long long endTime = getTime();


    if (result != 0)
    {
      fprintf(stdout,"ERROR (%d) : %s\n",result,DataServer::getResultString(result).c_str());
      return -5;
    }

    // ### Printing the result:

    gridData.print(std::cout,0,0);

    printf("\nTIME : %f sec\n\n",(float)(endTime-startTime)/1000000);

    uint sz = (uint)gridData.mValues.size();
    for (uint t=0; t<sz; t++)
    {
      long long val = (long long)(gridData.mValues[t] * 100);

      uint a=0;
      while (a < valueCount)
      {
        if (values[a] == val)
        {
          a = valueCount + 100;
        }
        a++;
      }
      if (a <= valueCount)
      {
        values[valueCount] = val;
        valueCount++;
        if ((valueCount % 1000) == 0)
          printf("%u\n",valueCount);
      }
    }

    printf("VALUES %u\n",valueCount);

    return 0;
  }
  catch (SmartMet::Spine::Exception& e)
  {
    SmartMet::Spine::Exception exception(BCP,"Service call failed!",NULL);
    exception.printError();
    return -6;
  }
}

