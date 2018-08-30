#include "grid-files/grid/PhysicalGridFile.h"
#include "grid-files/grid/PrintOptions.h"
#include "grid-files/identification/GridDef.h"
#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"
#include "grid-files/common/GeneralDefinitions.h"
#include "grid-files/common/ImageFunctions.h"
#include "grid-files/grid/Typedefs.h"
#include "grid-files/grid/ValueCache.h"

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
    char *configFile = getenv(SMARTMET_GRID_CONFIG_FILE);
    if (configFile == nullptr)
    {
      printf("%s not defined!\n",SMARTMET_GRID_CONFIG_FILE);
      exit(-1);
    }

    // Initializing the global structures. These are needed when
    // extracting information from GRIB files.

    Identification::gridDef.init(configFile);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,nullptr);
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

    std::string fname = "koe1.grib";
    gridFile.write(fname);

    printf("\n");
/*
    auto newGridFile = new SmartMet::GRID::GridFile(gridFile);
    newGridFile->print(std::cout,0,optionFlags);

    fname = "koe2.grib";
    newGridFile->write(fname);
*/
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,nullptr);
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
      SmartMet::GRID::PhysicalGridFile gridFile;
      gridFile.read(file);
      unsigned long long readEndTime = getTime();

      dump(gridFile,optionFlags);

      unsigned long long commandEndTime = getTime();

      printf("\nFile read time  : %f sec\n",C_DOUBLE(readEndTime-readStartTime)/1000000);
      printf("Processing time : %f sec\n",C_DOUBLE(commandEndTime-readEndTime)/1000000);
    }

    unsigned long long endTime = getTime();
    printf("Total time      : %f sec\n",C_DOUBLE(endTime-startTime)/1000000);

    return 0;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,nullptr);
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
  SmartMet::Spine::Exception exception(BCP,exception_operation_failed,nullptr);
  exception.printError();
}
