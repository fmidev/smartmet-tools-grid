#include "grid-files/grid/PhysicalGridFile.h"
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


enum ImageFlags
{
  IMGF_REVERSE    = 1 << 0,
  IMGF_PARAM      = 1 << 1
};




void getGridMinAndMaxValues(GRID::GridFile& gridFile,T::GribParamId parameterId,T::ParamValue& minValue,T::ParamValue& maxValue)
{
  try
  {
    minValue = 1000000000;
    maxValue = -1000000000;

    std::size_t messageCount = gridFile.getNumberOfMessages();
    for (std::size_t t=0; t<messageCount; t++)
    {
      auto message = gridFile.getMessageByIndex(t);
      if (message != nullptr  &&  message->getGribParameterId() == parameterId)
      {
        T::ParamValue min = 1000000000;
        T::ParamValue max = -1000000000;
        message->getGridMinAndMaxValues(min,max);

        if (min != 1000000000  &&  max != -1000000000)
        {
          if (min < minValue)
            minValue = min;

          if (max > maxValue)
            maxValue = max;
        }
      }
    }
  }
  catch (...)
  {
    throw Fmi::Exception(BCP,"Operation failed!",nullptr);
  }
}





void getParameterIdentifiers(GRID::GridFile& gridFile,std::set<T::GribParamId>& parameterIdList)
{
  try
  {
    std::size_t messageCount = gridFile.getNumberOfMessages();
    for (std::size_t t=0; t<messageCount; t++)
    {
      auto message = gridFile.getMessageByIndex(t);
      if (message != nullptr)
      {
        auto paramId = message->getGribParameterId();
        if (parameterIdList.find(paramId) == parameterIdList.end())
          parameterIdList.insert(paramId);
      }
    }
  }
  catch (...)
  {
    throw Fmi::Exception(BCP,"Operation failed!",nullptr);
  }
}




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



void saveMessageMap(const char *imageFile,const GRID::Message *message,T::ParamValue minValue,T::ParamValue maxValue,short interpolationMethod,uint valueLevels,uint flags)
{
  try
  {
    printf("SAVE MAP %s  (minValue=%f, maxValue=%f)\n",imageFile,minValue,maxValue);

    double dd = maxValue - minValue;
    double step = dd / 255;
    uint levelSize = 256/valueLevels;

    uint width = 1800;
    uint height = 900;

    uint size = (width*height);
    uint *image = new uint[size];
    memset(image,0xFF,size*sizeof(uint));

    unsigned char hue = 30;
    unsigned char saturation = 128;
    uint startLon = 0;
    uint y = 0;

    for (double lat = -89.8; lat < 90; lat=lat+0.2)
    {
      uint x = startLon;
      for (double lon = -179.8; lon < 180; lon=lon + 0.2)
      {
        T::ParamValue val = message->getGridValueByLatLonCoordinate(lat,lon,interpolationMethod);

        uint v = 255 - ((val - minValue) / step / levelSize);
        v = v * levelSize;
        if ((flags & IMGF_REVERSE) != 0)
          v = 255-v;

        uint col = hsv_to_rgb(hue,saturation,(unsigned char)v);

        if (val == ParamValueMissing)
          col = 0xE8E8E8;

        if (((x-startLon) % 75) == 0  ||  (y % 75) == 0)
          col = 0xFFFFFF;


        image[(height-y-1)*width + (x%width)] = col;
        x++;
      }
      y++;
    }

    jpeg_save(imageFile,image,(int)height,(int)width,100);
    delete[] image;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP,"Operation failed!",nullptr);
  }
}



void saveMapsByParameterId(uint fileIndex,SmartMet::GRID::GridFile& gridFile,T::GribParamId parameterId,const char *imageDir,uint valueLevels,uint flags)
{
  try
  {
    printf("SAVE MAPS %u\n",parameterId);

    T::ParamValue minValue = 0;
    T::ParamValue maxValue = 0;
    short interpolationMethod = T::AreaInterpolationMethod::Linear;
    Identification::GribParameterDef def;
    if (Identification::gridDef.getGribParameterDefById(parameterId,def))
      interpolationMethod = Identification::gridDef.getPreferredInterpolationMethodByUnits(def.mParameterUnits);

    if ((flags & IMGF_PARAM) != 0)
      getGridMinAndMaxValues(gridFile,parameterId,minValue,maxValue);

    std::size_t messageCount = gridFile.getNumberOfMessages();
    for (std::size_t m=0; m<messageCount; m++)
    {
      const GRID::Message *message = gridFile.getMessageByIndex(m);
      if (message->getGribParameterId() == parameterId)
      {
        if ((flags & IMGF_PARAM) == 0)
          message->getGridMinAndMaxValues(minValue,maxValue);

        auto level = message->getGridParameterLevel();
        char imageFile[300];
        sprintf(imageFile,"%s/map-%04u-%u-%09u-%s-%04llu.jpg",imageDir,fileIndex,parameterId,level,toString(message->getForecastTime()).c_str(),(unsigned long long)m);
        saveMessageMap(imageFile,message,minValue,maxValue,interpolationMethod,valueLevels,flags);
      }
    }
  }
  catch (...)
  {
    throw Fmi::Exception(BCP,"Operation failed!",nullptr);
  }
}




void saveAllMaps(uint fileIndex,SmartMet::GRID::GridFile& gridFile,const char *imageDir,uint valueLevels,uint minMaxLevel)
{
  try
  {
    std::set<T::GribParamId> parameterIdList;
    getParameterIdentifiers(gridFile,parameterIdList);

    for (auto id = parameterIdList.begin();id != parameterIdList.end(); ++id)
    {
      saveMapsByParameterId(fileIndex,gridFile,*id,imageDir,valueLevels,minMaxLevel);
    }
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
  printf("  This program maps grid data into the world maps and stores these maps \n");
  printf("   as JPG-images into the given directory.\n");
  printf("\n");
  printf("USAGE :\n");
  printf("  grid_maps <imageDir> [-p <paramId>] [-l <levels>] [-pmax] [-reverse] <file> [<file_2> .. <file_N>]\n");
  printf("\n");
  printf("PARAMETERS :\n");
  printf("  <imageDir>        -- Directory where grid images will be stored\n");
  printf("  -p <paramId>      -- Requested parameter id (0 = all)\n");
  printf("  -l <levels>       -- Number of value levels (max 256)\n");
  printf("  -pmax             -- Count image max and min values on parameter level (i.e. over several messages)\n");
  printf("  -reverse          -- Use reverse values\n");
  printf("  <file>            -- Grib file\n");
  printf("\n");
  printf("EXAMPLES :\n");
  printf("  grid_maps /MyImages mygribfile.grib2\n");
  printf("  grid_maps /MyImages -p 130 -l 8 -pmax mygribfile.grib2\n");
  printf("-------------------------------------------------------------------------------- \n");
  printf("\n");
}


int run(int argc, char **argv)
{
  try
  {
    if (argc < 3)
    {
      print_usage();
      return -1;
    }

    std::string imageDir = argv[1];
    T::GribParamId parameterId = 0;
    uint valueLevels = 256;
    uint flags = 0;
    std::vector<std::string>files;

    for (int t=2; t<argc;t++)
    {
      if (strcmp(argv[t],"-p") == 0  &&  (t+1) < argc)
      {
        parameterId = toUInt32(argv[t+1]);
        t++;
      }
      else
      if (strcmp(argv[t],"-l") == 0  &&  (t+1) < argc)
      {
        valueLevels = atol(argv[t+1]);
        t++;
      }
      else
      if (strcmp(argv[t],"-pmax") == 0)
        flags |= IMGF_PARAM;
      else
      if (strcmp(argv[t],"-reverse") == 0)
        flags |= IMGF_REVERSE;
      else
        files.push_back(std::string(argv[t]));
    }

    if (valueLevels > 256  ||  valueLevels <= 1)
      valueLevels = 256;


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

      if (parameterId != 0)
        saveMapsByParameterId(fileIndex,gridFile,parameterId,imageDir.c_str(),valueLevels,flags);
      else
        saveAllMaps(fileIndex,gridFile,imageDir.c_str(),valueLevels,flags);

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
