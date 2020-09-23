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

    if (argc != 7)
    {
      fprintf(stdout,"USAGE: cs_getGridValueList <sessionId> <startFileId> <endfileId> <messageIndex> <lon> <lat> \n");
      return -1;
    }


    // ### Session:
    T::SessionId sessionId = toInt64(argv[1]);


    // ### Creating a dataServer client:

    DataServer::Corba::ClientImplementation dataServer;
    dataServer.init(serviceIor);

    // ### Calling the dataServer:

    uint startFileId = toInt64(argv[2]);
    uint endFileId = toInt64(argv[3]);
    uint messageIndex = toInt64(argv[4]);
    double lon = toDouble(argv[5]);
    double lat = toDouble(argv[6]);
    T::ValueRecordList valueRecordList;

    for (uint t=startFileId;t<=endFileId;t++)
    {
      T::ValueRecord *rec = new T::ValueRecord();
      rec->mFileId = t;
      rec->mMessageIndex = messageIndex;
      rec->mCoordinateType = T::CoordinateTypeValue::LATLON_COORDINATES;
      rec->mAreaInterpolationMethod = T::AreaInterpolationMethod::Linear;
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
  catch (Fmi::Exception& e)
  {
    Fmi::Exception exception(BCP,"Service call failed!",nullptr);
    exception.printError();
    return -6;
  }
}

