#include "grid-files/grid/GridFile.h"
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
  printf("  The program prints detailed structures of the given files.\n");
  printf("\n");
  printf("USAGE :\n");
  printf("  grid_dump [-coordinates][-data][-bitmaps] <file> [<file_2> .. <file_N>]\n");
  printf("\n");
  printf("PARAMETERS :\n");
  printf("  -coordinates   -- Print coordinate values\n");
  printf("  -data          -- Print data values\n");
  printf("  -bitmaps       -- Print bitmaps\n");
  printf("\n");
  printf("EXAMPLES :\n");
  printf("  grid_dump mygribfile.grib2\n");
  printf("  grid_dump -coordinates -data mygribfile.grib2\n");
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

    std::vector<std::string>files;

    for (int t=1; t<argc;t++)
    {
      if (strcmp(argv[t],"-coordinates") == 0)
        optionFlags |= SmartMet::GRID::PrintFlag::coordinates;
      else
      if (strcmp(argv[t],"-data") == 0)
        optionFlags |= SmartMet::GRID::PrintFlag::data;
      else
      if (strcmp(argv[t],"-bitmap") == 0)
         optionFlags |= SmartMet::GRID::PrintFlag::bitmap;
      else
      if (argv[t][0] != '-')
        files.push_back(std::string(argv[t]));
    }


    init();

    unsigned long long startTime = getTime();
    uint fileIndex = 0;

    for (const auto &file : files)
    {
      fileIndex++;
      unsigned long long readStartTime = getTime();
      SmartMet::GRID::GridFile gridFile;
      gridFile.read(file);
      unsigned long long readEndTime = getTime();

      dump(gridFile,optionFlags);

      unsigned long long commandEndTime = getTime();

      printf("\nFile read time  : %f sec\n",(double)(readEndTime-readStartTime)/1000000);
      printf("Processing time : %f sec\n",(double)(commandEndTime-readEndTime)/1000000);
    }

    unsigned long long endTime = getTime();
    printf("Total time      : %f sec\n",(double)(endTime-startTime)/1000000);

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
