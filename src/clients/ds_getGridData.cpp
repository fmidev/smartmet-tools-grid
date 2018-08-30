#include "grid-content/contentServer/corba/client/ClientImplementation.h"
#include "grid-content/dataServer/corba/client/ClientImplementation.h"
#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"

using namespace SmartMet;

std::set<T::ParamValue> values;


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


    if (argc != 6)
    {
      fprintf(stdout,"USAGE: ds_getGridData <sessionId> <fileId> <messageIndex> <flags> <outputCsvFile> \n");
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
    uint flags = toInt64(argv[4]);
    char *outputFile = argv[5];
    T::GridData gridData;

    unsigned long long startTime = getTime();
    int result = dataServer.getGridData(sessionId,fileId,messageIndex,flags,gridData);
    unsigned long long endTime = getTime();


    if (result != 0)
    {
      fprintf(stdout,"ERROR (%d) : %s\n",result,DataServer::getResultString(result).c_str());
      return -5;
    }

    // ### Printing the result:

    gridData.print(std::cout,0,0);

    printf("\nTIME : %f sec\n\n",(float)(endTime-startTime)/1000000);

    FILE *file = fopen(outputFile,"we");
    if (file == nullptr)
    {
      fprintf(stdout,"ERROR: Cannot create the ouput file (%s)!\n",outputFile);
      return -6;
    }

    uint sz = gridData.mValues.size();

    if ((gridData.mColumns*gridData.mRows) != sz)
    {
      fprintf(stdout,"ERROR: The size of the grid (%u x %u) and the number of the values (%u) no not match!\n",gridData.mColumns,gridData.mRows,sz);
      return -7;
    }

    uint t = 0;
    for (uint r=0; r<gridData.mRows; r++)
    {
      for (uint c=0; c<gridData.mColumns; c++)
      {
        T::ParamValue val = gridData.mValues[t];
        fprintf(file,"%f;",val);
        t++;

        if (values.find(val) == values.end())
          values.insert(val);
      }
      fprintf(file,"\n");
    }
    printf("VALUES %lu\n",values.size());

    return 0;
  }
  catch (SmartMet::Spine::Exception& e)
  {
    SmartMet::Spine::Exception exception(BCP,"Service call failed!",nullptr);
    exception.printError();
    return -6;
  }
}

