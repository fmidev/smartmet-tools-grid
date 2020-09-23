#include "grid-content/queryServer/corba/client/ClientImplementation.h"
#include <macgyver/Exception.h>
#include "grid-files/common/GeneralFunctions.h"

using namespace SmartMet;



int main(int argc, char *argv[])
{
  try
  {
    if (argc < 2)
    {
      fprintf(stdout,"USAGE: qs_getProducerList <sessionId> \n");
      return -1;
    }

    T::SessionId sessionId = toInt64(argv[1]);
    string_vec producerList;
    int result = 0;
    unsigned long long startTime = 0;
    unsigned long long endTime = 0;

    char *serviceIor = getenv("SMARTMET_QS_IOR");

    if (strcmp(argv[argc-2],"-ior") == 0)
      serviceIor = argv[argc-1];

    if (serviceIor == nullptr)
    {
      fprintf(stdout,"Service IOR not defined!\n");
      return -2;
    }

    QueryServer::Corba::ClientImplementation service;
    service.init(serviceIor);

    startTime = getTime();
    result = service.getProducerList(sessionId,producerList);
    endTime = getTime();

    if (result != 0)
    {
      fprintf(stdout,"ERROR (%d) : %s\n",result,QueryServer::getResultString(result).c_str());
      return -3;
    }

    // ### Result:

    for (auto it = producerList.begin(); it != producerList.end(); ++it)
      std::cout << *it << "\n";

    printf("\nTIME : %f sec\n\n",(float)(endTime-startTime)/1000000);

    return 0;
  }
  catch (Fmi::Exception& e)
  {
    Fmi::Exception exception(BCP,"Service call failed!",nullptr);
    exception.printError();
    return -4;
  }
}

