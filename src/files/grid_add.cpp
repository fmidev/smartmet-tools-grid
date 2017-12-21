#include "contentServer/corba/client/ClientImplementation.h"
#include "contentServer/http/client/ClientImplementation.h"
#include "contentServer/redis/RedisImplementation.h"
#include "grid-files/grid/PhysicalGridFile.h"
#include "grid-files/grid/PrintOptions.h"
#include "grid-files/identification/GribDef.h"
#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"
#include "grid-files/common/GeneralDefinitions.h"
#include "grid-files/common/ImageFunctions.h"
#include "grid-files/grid/Typedefs.h"
#include "grid-files/grid/ValueCache.h"
#include "grid-files/identification/MessageIdentifier_grib1.h"
#include "grid-files/identification/MessageIdentifier_grib2.h"

#include <iostream>
#include <stdexcept>
#include <string.h>
#include <vector>
#include <sys/time.h>


using namespace SmartMet;


ContentServer::ServiceInterface *service = NULL;



void init()
{
  try
  {
    char *configDir = getenv("SMARTMET_GRID_CONFIG_DIR");
    if (configDir == NULL)
    {
      printf("SMARTMET_GRID_CONFIG_DIR not defined!\n");
      exit(-1);
    }

    // Initializing the global structures. These are needed when
    // extracting information from GRIB files.

    SmartMet::Identification::gribDef.init(configDir);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,NULL);
  }
}




void dump(SmartMet::GRID::GridFile& gridFile,uint optionFlags)
{
  try
  {
    printf("--------------------------------------------------------------------------------------------\n");
    printf(" FILE : %s\n",gridFile.getFileName().c_str());
    printf("--------------------------------------------------------------------------------------------\n");

    gridFile.print(std::cout,0,optionFlags);

    printf("\n");
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,NULL);
  }
}




void print_usage()
{
  printf("-------------------------------------------------------------------------------- \n");
  printf("PROGRAM :\n");
  printf("  The program adds the grid file information into the ContentServer.\n");
  printf("\n");
  printf("USAGE :\n");
  printf("  grid_add <file> [[-http <url>]|[-redis <address> <port> <tablePrefix>]]\n");
  printf("\n");
  printf("PARAMETERS :\n");
  printf("  <file>   -- The full name of the grid file (with the directory path)\n");
  printf("\n");
  printf("EXAMPLES :\n");
  printf("  grid_add /data/test/mygribfile.grib2\n");
  printf("  grid_add /data/test/mygribfile.grib2 -redis 127.0.0.1 6379 \"\"\n");
  printf("-------------------------------------------------------------------------------- \n");
  printf("\n");
}


int run(int argc, char **argv)
{
  try
  {
    uint optionFlags = 0;

    if (argc < 2)
    {
      print_usage();
      return -1;
    }

    init();
    std::string filename = argv[1];
    /*

    for (int t=2; t<argc; t++)

    if (strcmp(argv[t],"-http") == 0  &&  (t+1) < argc)
    {
      ContentServer::HTTP::ClientImplementation *client = new ContentServer::HTTP::ClientImplementation();
      client->init(argv[t+1]);
      service = client;
    }
    else
      if (strcmp(argv[t],"-redis") == 0  &&  (t+3) < argc)
    {
      ContentServer::RedisImplementation *redis = new ContentServer::RedisImplementation();
      redis->init(argv[t+1],atoi(argv[t+2),argv[t+3);
      service = redis;
    }
    else
    {
      char *serviceIor = getenv("SMARTMET_CS_IOR");

      if (strcmp(argv[argc-2],"-ior") == 0)
        serviceIor = argv[argc-1];

      if (serviceIor == NULL)
      {
        fprintf(stdout,"Service IOR not defined!\n");
        return -2;
      }

      ContentServer::Corba::ClientImplementation *client = new ContentServer::Corba::ClientImplementation();
      client->init(serviceIor);
      service = client;
    }
*/

    SmartMet::GRID::PhysicalGridFile gridFile;
    gridFile.read(filename);
    gridFile.print(std::cout,0,0);

    delete service;
    return 0;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,NULL);
  }
}


// ----------------------------------------------------------------------
/*
 * \brief Main program apart from exception handling
 */
// ----------------------------------------------------------------------

int main(int argc, char **argv) try
{
  return run(argc, argv);
}
catch (...)
{
  SmartMet::Spine::Exception exception(BCP,exception_operation_failed,NULL);
  exception.printError();
}
