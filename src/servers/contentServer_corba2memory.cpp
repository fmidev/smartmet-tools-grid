#include "contentServer/cache/MemoryImplementation.h"
#include "contentServer/corba/client/ClientImplementation.h"
#include "contentServer/corba/server/Server.h"
#include "grid-files/common/Exception.h"
#include "contentServer/corba/server/ServerInterface.h"
#include "grid-files/identification/GribDef.h"
#include <signal.h>

using namespace SmartMet;


ContentServer::Corba::Server *server = NULL;
ContentServer::MemoryImplementation *memoryImplementation = NULL;


void sig_handler(int signum)
{
  {
    try
    {
      if (memoryImplementation != NULL)
      {
        printf("\n**** SHUTTING DOWN ****\n");
        memoryImplementation->shutdown();
        server->shutdown();
      }
      else
        exit(-1);
    }
    catch (...)
    {
      SmartMet::Spine::Exception exception(BCP,exception_operation_failed,NULL);
      exception.printError();
      exit(-1);
    }
  }
}




int main(int argc, char *argv[])
{
  try
  {
    if (argc < 3)
    {
      printf("\n");
      printf("##################################################################################\n");
      printf("                        contentServer_corba2memory\n");
      printf("##################################################################################\n");
      printf(" DESCRIPTION:\n");
      printf("   This is a contentServer that offers a CORBA service interface for accessing\n");
      printf("   its services. The actual content information is stored into the memory.\n");
      printf("\n");
      printf("   When the server starts, it prints its IOR (International Object Reference) \n");
      printf("   on the screen. You should store this into the environment variable SMARTMET_CS_IOR\n");
      printf("   so that client programs can access this service easily. Notice that the IOR\n");
      printf("   does not change as long as the server address and ports do not change. I.e. you\n");
      printf("   can store it in your shell start-up scripts (like '.bashrc').\n");
      printf("\n");
      printf(" USAGE:\n");
      printf("   contentServer_corba2memory <corbaAddress> <corbaPort> [-log logFile]\n");
      printf("\n");
      printf(" WHERE:\n");
      printf("   <corbaAddress>    => The IP address of the server.\n");
      printf("   <corbaPort>       => The TCP port of the server.\n");
      printf("##################################################################################\n");
      printf("\n");
      return -1;
    }

    signal(SIGINT, sig_handler);

    char *corbaAddress = (char*)argv[1];
    char *corbaPort = (char*)argv[2];

    memoryImplementation = new ContentServer::MemoryImplementation();
    memoryImplementation->init();

    server = new ContentServer::Corba::Server(corbaAddress,corbaPort);
    server->init(memoryImplementation);

    Log processingLog;
    if (argc == 5  && strcmp(argv[3],"-log") == 0)
    {
      processingLog.init(true,argv[4],100000000,50000000);
      memoryImplementation->setProcessingLog(&processingLog);
    }

    // Let's print the service IOR. This is necessary for accessing the service. Usually the best way
    // to handle an IOR is to store it into an environment variable.

    std::string ior = server->getServiceIor();
    printf("\n%s\n",ior.c_str());

    server->run();

    delete memoryImplementation;

    return 0;
  }
  catch (SmartMet::Spine::Exception& e)
  {
    SmartMet::Spine::Exception exception(BCP,exception_operation_failed,NULL);
    exception.printError();
    return -1;
  }
}
