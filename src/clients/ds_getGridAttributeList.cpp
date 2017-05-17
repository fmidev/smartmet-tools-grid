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


    if (argc != 5)
    {
      fprintf(stdout,"USAGE: cs_getGridAttributeList <sessionId> <dataServerId> <fileId> <messageIndex>\n");
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
    T::AttributeList attributeList;

    unsigned long long startTime = getTime();
    result = dataServer.getGridAttributeList(sessionId,fileId,messageIndex,attributeList);
    unsigned long long endTime = getTime();


    if (result != 0)
    {
      fprintf(stdout,"ERROR (%d) : %s\n",result,DataServer::getResultString(result).c_str());
      return -5;
    }

    // ### Printing the result:

    attributeList.print(std::cout,0,0);

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

