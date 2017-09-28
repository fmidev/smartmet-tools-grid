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


enum ImageFlags
{
  IMGF_REVERSE    = 1 << 0,
  IMGF_PARAM      = 1 << 1
};


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




void saveMessageSubmap(const char *imageFile,const GRID::Message *message,double startLat,double startLon,uint width,uint height,double step,T::ParamValue minValue,T::ParamValue maxValue,T::InterpolationMethod interpolationMethod,uint valueLevels,uint flags)
{
  try
  {
    printf("SAVE SUBMAP %s  (minValue=%f, maxValue=%f)\n",imageFile,minValue,maxValue);

    auto d = message->getGridDimensions();
    if (!d)
      return;

    double dd = maxValue - minValue;
    double vstep = dd / 255;
    uint levelSize = 256/valueLevels;

    uint size = (uint)(width*height);
    unsigned long *image = new unsigned long[size];
    memset(image,0xFF,size*sizeof(unsigned long));

    unsigned char hue = 30;
    unsigned char saturation = 128;

    double lat = startLat;
    for (uint y = 0; y < height; y++)
    {
      if (lat > -90  &&  lat < 90)
      {
        double lon = startLon;
        for (uint x=0; x<width; x++)
        {
          if (lon < -180)
            lon += 360;

          if (lon > 180)
            lon -= 360;

          T::ParamValue val = message->getGridValueByLatLonCoordinate(lat,lon,interpolationMethod);
          uint v = 255 - (uint)((val - minValue) / vstep / levelSize);
          v = v * levelSize;

          if ((flags & IMGF_REVERSE) != 0)
            v = 255-v;

          uint col = hsv_to_rgb(hue,saturation,(unsigned char)v);

          if (val == ParamValueMissing)
            col = 0xE8E8E8;

          image[(height-y-1)*width + (x%width)] = col;
          lon += step;
        }
      }
      lat += step;
    }

    jpeg_save(imageFile,image,(int)height,(int)width,100);
    delete image;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,NULL);
  }
}



void saveSubmapsByParameterId(uint fileIndex,SmartMet::GRID::GridFile& gridFile,T::ParamId parameterId,double lat,double lon,uint width,uint height,double step,const char *imageDir,uint valueLevels,uint flags)
{
  try
  {
    printf("SAVE SUBMAPS %s\n",parameterId.c_str());

    T::ParamValue minValue = 0;
    T::ParamValue maxValue = 0;
    T::InterpolationMethod interpolationMethod = T::InterpolationMethod::Linear;
    Identification::ParameterDefinition_cptr def = Identification::gribDef.getGribParamDefById(parameterId);
    if (def != NULL)
      interpolationMethod = Identification::gribDef.getPreferredInterpolationMethodByUnits(def->mParameterUnits);

    if ((flags & IMGF_PARAM) != 0)
      gridFile.getGridMinAndMaxValues(parameterId,minValue,maxValue);

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
        sprintf(imageFile,"%s/submap-%04u-%s-%09u-%s-%04llu.jpg",imageDir,fileIndex,parameterId.c_str(),level,toString(message->getForecastTime()).c_str(),(unsigned long long)m);
        saveMessageSubmap(imageFile,message,lat,lon,width,height,step,minValue,maxValue,interpolationMethod,valueLevels,flags);
      }
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,NULL);
  }
}




void saveAllSubmaps(uint fileIndex,SmartMet::GRID::GridFile& gridFile,double lat,double lon,uint width,uint height,double step,const char *imageDir,uint valueLevels,uint flags)
{
  try
  {
    auto parameterIdList = gridFile.getParameterIdentifiers();

    std::size_t size = parameterIdList.size();
    for (std::size_t t=0; t<size; t++)
    {
      saveSubmapsByParameterId(fileIndex,gridFile,parameterIdList[t],lat,lon,width,height,step,imageDir,valueLevels,flags);
    }
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
  printf("  This program maps grid data into latlon maps and stores these maps \n");
  printf("  as JPG-images into the given directory.\n");
  printf("\n");
  printf("USAGE :\n");
  printf("  grid_submaps <imageDir> <lat> <lon> <width> <height> <step> [-p <paramId>] [-l <levels>] [-pmax] [-reverse] <file> [<file_2> .. <file_N>]\n");
  printf("\n");
  printf("PARAMETERS :\n");
  printf("  <imageDir>     -- Directory where grid images will be stored\n");
  printf("  <lat>          -- Latitude of the bottom left corner (-90..90)\n");
  printf("  <lon           -- Longitude of the bottom left corner (-180..180)\n");
  printf("  <width>        -- Image width in pixels\n");
  printf("  <height>       -- Image height in pixels\n");
  printf("  <step>         -- Pixel size in degrees (for example 0.05)\n");
  printf("  -p <paramId>   -- Requested parameter id (0 = all)\n");
  printf("  -l <levels>    -- Number of value levels (max 256)\n");
  printf("  -pmax          -- Count image max and min values on parameter level (i.e. over several messages)\n");
  printf("  -reverse       -- Use reverse values\n");
  printf("  <file>         -- Grib file\n");
  printf("\n");

  printf("EXAMPLES :\n");
  printf("  grid_submaps 0 70 10 1200 800 0.05 256 1 /MyImages mygribfile.grib2\n");
  printf("-------------------------------------------------------------------------------- \n");
  printf("\n");
}


int run(int argc, char **argv)
{
  try
  {
    if (argc < 8)
    {
      print_usage();
      return -1;
    }

    std::string imageDir = argv[1];
    double lat = atof(argv[2]);
    double lon = atof(argv[3]);
    uint width = (uint)atoi(argv[4]);
    uint height = (uint)atoi(argv[5]);
    double step = atof(argv[6]);

    T::ParamId parameterId;
    uint valueLevels = 256;
    uint flags = 0;
    std::vector<std::string>files;

    for (int t=7; t<argc;t++)
    {
      if (strcmp(argv[t],"-p") == 0  &&  (t+1) < argc)
      {
        parameterId = argv[t+1];
        t++;
      }
      else
      if (strcmp(argv[t],"-l") == 0  &&  (t+1) < argc)
      {
        valueLevels = (uint)atol(argv[t+1]);
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
      SmartMet::GRID::GridFile gridFile;
      gridFile.read(file);
      unsigned long long readEndTime = getTime();

      if (atoi(parameterId.c_str()) != 0)
        saveSubmapsByParameterId(fileIndex,gridFile,parameterId,lat,lon,width,height,step,imageDir.c_str(),valueLevels,flags);
      else
        saveAllSubmaps(fileIndex,gridFile,lat,lon,width,height,step,imageDir.c_str(),valueLevels,flags);

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
