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
    if (configFile == NULL)
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
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,NULL);
  }
}





void showInfo(SmartMet::GRID::GridFile& gridFile)
{
  try
  {
    /*

    auto parameterIdList = gridFile.getParameterIdentifiers();
    std::size_t idCount = parameterIdList.size();

    printf("------------------------------------------------------------------------------------------------------\n");
    printf(" FILE : %s (type: %s)\n",gridFile.getFileName().c_str(),gridFile.getFileTypeString().c_str());
    printf("------------------------------------------------------------------------------------------------------\n");
    printf(" ParamId   Level     StartTime       EndTime         Msg  Area          Name    Description\n");
    printf("------------------------------------------------------------------------------------------------------\n");

    for (std::size_t t=0; t<idCount; t++)
    {
      T::ParamId id = parameterIdList[t];

      Identification::ParamDef_cptr paramDef = Identification::gridDef.getGribParamDefById(id);

      T::TimeString startTime;
      T::TimeString endTime;
      std::size_t messages = 0;
      std::vector<T::ParamLevel> levels = gridFile.getLevelsByParameterId(id);

      auto levelCount = levels.size();
      for (std::size_t l=0; l<levelCount; l++)
      {
        auto level = levels[l];
        gridFile.getTimeRangeByParameterIdAndLevel(id,level,startTime,endTime,messages);


        printf(" %-9s %9u %s %s %-4llu %-9s %s (%s)\n",
            id.c_str(),
            level,
            startTime.c_str(),
            endTime.c_str(),
            (unsigned long long)messages,
            paramDef->mParameterName.c_str(),
            paramDef->mParameterDescription.c_str(),
            paramDef->mParameterUnits.c_str()
            );
      }
    }
    printf("\n");
      */
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,NULL);
  }
}





void showFullInfo(SmartMet::GRID::GridFile& gridFile)
{
  try
  {
/*
    auto parameterIdList = gridFile.getParameterIdentifiers();
    std::size_t idCount = parameterIdList.size();

    printf("------------------------------------------------------------------------------------------------------\n");
    printf(" FILE : %s (type: %s)\n",gridFile.getFileName().c_str(),gridFile.getFileTypeString().c_str());
    printf("------------------------------------------------------------------------------------------------------\n");

    for (std::size_t t=0; t<idCount; t++)
    {
      T::ParamId id = parameterIdList[t];

      Identification::ParamDef_cptr paramDef = Identification::gridDef.getGribParamDefById(id);


      T::TimeString startTime;
      T::TimeString endTime;
      std::size_t messages = 0;

      std::vector<T::ParamLevel> levels = gridFile.getLevelsByParameterId(id);

      auto levelCount = levels.size();
      for (std::size_t l=0; l<levelCount; l++)
      {
        auto level = levels[l];

        T::Index_vec messageIndex = gridFile.getMessagesIndexListByParameterIdAndLevel(id,level);
        gridFile.getTimeRangeByParameterIdAndLevel(id,level,startTime,endTime,messages);

        const GRID::Message *msg = gridFile.getMessageByIndex(messageIndex[0]);
        T::Hash hash = msg->getGridHash();

        T::Dimensions_opt d = msg->getGridDimensions();

        printf(" PARAMETER [%s] : %s\n",id.c_str(),paramDef->mParameterDescription.c_str());
        printf(" - Name                          : %s\n",paramDef->mParameterName.c_str());
        printf(" - Level type                    : %s\n",msg->getGridParameterLevelIdString().c_str());
        printf(" - Level                         : %u\n",level);
        printf(" - Projection                    : %s\n",msg->getGridProjectionString().c_str());
        printf(" - Grid type                     : %s\n",T::get_gridProjectionString(msg->getGridProjection()).c_str());
        printf(" - Grid layout                   : %s\n",T::get_gridLayoutString(msg->getGridLayout()).c_str());
        printf(" - Grid hash                     : %llu\n",(unsigned long long)hash);
        printf(" - Grid geometry id              : %d\n",msg->getGridGeometryId());

        if (d)
          printf(" - Dimensions                    : %u x %u\n",d->nx(),d->ny());
        else
          printf(" - Dimensions                    :\n");

        printf(" - Grid rows                     : %u\n",(uint)msg->getGridOriginalRowCount());
        printf(" - Grid columns                  : %u\n",(uint)msg->getGridOriginalColumnCount());
        printf(" - Units                         : %s\n",paramDef->mParameterUnits.c_str());
        printf(" - Time range                    : %s - %s\n",startTime.c_str(),endTime.c_str());
        printf(" - Number of messages            : %llu\n",(unsigned long long)messages);
        printf("------------------------------------------------------------------------------------------------------\n");
      }
    }
    printf("\n");
*/
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
  printf("  The program lists the data content of the given files.\n");
  printf("\n");
  printf("USAGE :\n");
  printf("  grid_info [-full] <file> [<file_2> .. <file_N>]\n");
  printf("\n");
  printf("PARAMETERS :\n");
  printf("  -full    -- Show file content in details\n");
  printf("  <file>   -- Grib file\n");
  printf("\n");
  printf("EXAMPLES :\n");
  printf("  grid_info mygribfile.grib2\n");
  printf("  grid_info -full mygribfile.grib2\n");
  printf("-------------------------------------------------------------------------------- \n");
  printf("\n");
}




int run(int argc, char **argv)
{
  try
  {
    if (argc < 2)
    {
      print_usage();
      return -1;
    }

    std::vector<std::string>files;
    bool fullInfo = false;

    for (int t=1; t<argc;t++)
    {
      if (strcmp(argv[t],"-full") == 0)
        fullInfo = true;
      else
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

      if (fullInfo)
        showFullInfo(gridFile);
      else
        showInfo(gridFile);

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
