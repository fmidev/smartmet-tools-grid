#include "contentServer/corba/client/ClientImplementation.h"
#include "queryServer/corba/server/ServerInterface.h"
#include "queryServer/corba/server/Server.h"
#include "queryServer/implementation/ServiceImplementation.h"
#include "grid-files/common/Exception.h"

#include <iostream>
#include <stdexcept>
#include <string.h>
#include <vector>
#include <pthread.h>
#include <sys/time.h>
#include <signal.h>


using namespace SmartMet;

QueryServer::ServiceImplementation *queryServer = NULL;




bool shutdownRequested = false;


void sig_handler(int signum)
{
  {
    try
    {
      if (shutdownRequested)
        sprintf(NULL,"Crashing the system for the core dump");

      if (queryServer != NULL)
      {
        printf("\n**** SHUTTING DOWN ****\n");
        queryServer->shutdown();
      }
      else
        exit(-1);
    }
    catch (...)
    {
      SmartMet::Spine::Exception exception(BCP,"Operation failed!",NULL);
      exception.printError();
      exit(-1);
    }
  }
}





int main(int argc, char *argv[])
{
  try
  {
    if (argc != 4)
    {
      printf("\n");
      printf("##################################################################################\n");
      printf("                             queryServer\n");
      printf("##################################################################################\n");
      printf(" DESCRIPTION:\n");
      printf("\n");
      printf(" USAGE:\n");
      printf("   queryServer <corbaAddress> <corbaPort> <contentServerIor>\n");
      printf("\n");
      printf(" WHERE:\n");
      printf("   <corbaAddress>     => The IP address of the server.\n");
      printf("   <corbaPort>        => The TCP port of the server.\n");
      printf("   <contentServerIor> => The IOR of the ContentServer.\n");
      printf("##################################################################################\n");
      printf("\n");
      return -1;
    }

    signal(SIGINT, sig_handler);

    char *corbaAddress = (char*)argv[1];
    char *corbaPort = (char*)argv[2];
    char *contentServerIor = (char*)argv[3];

    queryServer = new QueryServer::ServiceImplementation();

    QueryServer::Corba::Server corbaServer(corbaAddress,corbaPort);
    corbaServer.init(queryServer);

    ContentServer::Corba::ClientImplementation contentServerClient;
    contentServerClient.init(contentServerIor);

    queryServer->init (&contentServerClient,NULL);;

    // Let's print the service IOR. This is necessary for accessing the service. Usually the best way
    // to handle an IOR is to store it into an environment variable.

    std::string ior = corbaServer.getServiceIor();
    printf("\n%s\n",ior.c_str());

    corbaServer.run();

    delete queryServer;
    return 0;
  }
  catch (...)
  {
    SmartMet::Spine::Exception exception(BCP,"Operation failed!",NULL);
    exception.printError();
    return -1;
  }
}
