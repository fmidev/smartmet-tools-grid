#include "grid-files/grid/GridFile.h"
#include "grid-files/grid/PrintOptions.h"
#include "grid-files/identification/GridDef.h"
#include <macgyver/Exception.h>
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
    throw Fmi::Exception(BCP,"Operation failed!",nullptr);
  }
}




void executeQuery(SmartMet::GRID::GridFile& gridFile,double lat,double lon,std::vector<T::ParamId>& parameterIdList)
{
  try
  {
#if 0
    std::size_t pos[100] = {0};
    typedef std::vector<SmartMet::T::GridPointValue> ValueVector;

    T::TimeString startTime = "19000101T000000";
    T::TimeString endTime = "22000101T000000";

    std::size_t idCount = parameterIdList.size();
    ValueVector values[idCount];


    printf("----------------");
    for (std::size_t i=0; i<idCount; i++)
      printf("-----------");
    printf("\n");

    printf(" FILE : %s\n",gridFile.getFileName().c_str());

    printf("----------------");
    for (std::size_t i=0; i<idCount; i++)
      printf("-----------");
    printf("\n");

    printf(" Time            ");

    for (std::size_t i=0; i<idCount; i++)
    {
      auto id = parameterIdList[i];
      values[i] = gridFile.getParameterValuesByLatLon(id,startTime,endTime,lat,lon);

      printf("%10s ",id.c_str());
    }
    printf("\n");

    printf("----------------");
    for (std::size_t i=0; i<idCount; i++)
      printf("-----------");
    printf("\n");


    //std::size_t valueCount = values[0].size();
    std::size_t c=0;
    std::size_t endCount = 0;

    while (endCount < idCount)
    {
      endCount = 0;
      T::TimeString forecastTime;
      forecastTime == values[0][pos[0]].mTime;

      for (std::size_t i=0; i<idCount; i++)
      {
        if (pos[i] < values[i].size())
        {
          if (forecastTime > values[i][pos[i]].mTime)
            forecastTime = values[i][pos[i]].mTime;
        }
        else
        {
          endCount++;
        }
      }

      if (endCount < idCount)
      {
        for (std::size_t i=0; i<idCount; i++)
        {
          std::string value;
          if (pos[i] < values[i].size())
          {
            if (forecastTime == values[i][pos[i]].mTime)
            {
              value = toString(values[i][pos[i]].mValue);
              pos[i]++;
            }
            else
            {
              value = "---";
            }
          }
          else
          {
            value = "---";
          }

          if (i == 0)
            printf(" %s ",toString(forecastTime).c_str());

          printf("%10s ",value.c_str());
        }
        printf("\n");
      }
      c++;
    }
#endif
  }
  catch (...)
  {
    throw Fmi::Exception(BCP,"Operation failed!",nullptr);
  }
}



void print_usage()
{
  printf("-------------------------------------------------------------------------------- \n");
  printf("PROGRAM :\n");
  printf("  This program can be used in order to query parameter values in the given \n");
  printf("   latlon coordinates.\n");
  printf("\n");
  printf("USAGE :\n");
  printf("   grid_query <lat> <lon> @<paramId> [@<paramId_2> .. @<paramId_N>] <file> [<file_2> .. <file_N>]\n");
  printf("\n");
  printf("PARAMETERS :\n");
  printf("  <lat> <lon>    -- Requested coordinates\n");
  printf("  <paramId>      -- Requested parameter id (=> grib-id)\n");
  printf("  <file>         -- Grib file\n");
  printf("\n");
  printf("EXAMPLES :\n");
  printf("  grid_query 72 24 @130 @121 @154 mygribfile.grib2\n");
  printf("-------------------------------------------------------------------------------- \n");
  printf("\n");
}


int run(int argc, char **argv)
{
  try
  {
    if (argc < 4)
    {
      print_usage();
      return -1;
    }

    std::vector<std::string>files;
    std::vector<T::ParamId> parameterIdList;

    double lat = toDouble(argv[1]);
    double lon = toDouble(argv[2]);


    for (int t=3; t<argc;t++)
    {
      if (argv[t][0] == '@')
      {
        T::ParamId parameterId = (argv[t]+1);
        parameterIdList.push_back(parameterId);
      }
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

      executeQuery(gridFile,lat,lon,parameterIdList);

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
    throw Fmi::Exception(BCP,"Operation failed!",nullptr);
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
  Fmi::Exception exception(BCP,"Operation failed!",nullptr);
  exception.printError();
}
