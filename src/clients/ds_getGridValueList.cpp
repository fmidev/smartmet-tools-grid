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

    if (argc != 7)
    {
      fprintf(stdout,"USAGE: cs_getGridValueList <sessionId> <startFileId> <endfileId> <messageIndex> <lon> <lat> \n");
      return -1;
    }


    // ### Session:
    T::SessionId sessionId = (SmartMet::T::SessionId)atoll(argv[1]);


    // ### Creating a dataServer client:

    DataServer::Corba::ClientImplementation dataServer;
    dataServer.init(serviceIor);

    // ### Calling the dataServer:

    uint startFileId = (uint)atoll(argv[2]);
    uint endFileId = (uint)atoll(argv[3]);
    uint messageIndex = (uint)atoll(argv[4]);
    double lon = (double)atof(argv[5]);
    double lat = (double)atof(argv[6]);
    T::ValueRecordList valueRecordList;

    for (uint t=startFileId;t<=endFileId;t++)
    {
      T::ValueRecord *rec = new T::ValueRecord();
      rec->mFileId = t;
      rec->mMessageIndex = messageIndex;
      rec->mCoordinateType = T::CoordinateType::LATLON_COORDINATES;
      rec->mInterpolationMethod = T::InterpolationMethod::Linear;
      rec->mX = lon;
      rec->mY = lat;
      valueRecordList.addValueRecord(rec);
    }

    unsigned long long startTime = getTime();
    int result = dataServer.getMultipleGridValues(sessionId,valueRecordList);
    unsigned long long endTime = getTime();


    if (result != 0)
    {
      fprintf(stdout,"ERROR (%d) : %s\n",result,DataServer::getResultString(result).c_str());
      return -5;
    }

    // ### Printing the result:

    valueRecordList.print(std::cout,0,0);

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

