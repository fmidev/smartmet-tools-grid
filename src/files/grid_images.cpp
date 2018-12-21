#include "grid-files/grid/PhysicalGridFile.h"
#include "grid-files/grib1/GribFile.h"
#include "grid-files/grib2/GribFile.h"
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

enum ImageFlags
{
  IMGF_ROTATE     = 1 << 0,
  IMGF_REVERSE    = 1 << 1,
  IMGF_PARAM      = 1 << 2
};



void getGridMinAndMaxValues(GRID::GridFile& gridFile,T::ParamId parameterId,T::ParamValue& minValue,T::ParamValue& maxValue)
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
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,nullptr);
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
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,nullptr);
  }
}




void saveMessageImage(const char *imageFile,const GRID::Message *message,T::ParamValue minValue,T::ParamValue maxValue,short interpolationMethod,double scaleFactor,uint valueLevels,uint flags)
{
  try
  {
    printf("SAVE IMAGE %s  (minValue=%f, maxValue=%f)\n",imageFile,minValue,maxValue);

    auto d = message->getGridDimensions();
    if (d.getDimensions() != 2)
      return;


    double dd = maxValue - minValue;
    double step = dd / 255;
    uint levelSize = 256/valueLevels;

    int width = (int)(d.nx() * scaleFactor);
    int height = (int)(d.ny() * scaleFactor);

    if (width == 0)
      width = (int)((int)message->getGridOriginalColumnCount() * scaleFactor);

    uint *image = new uint[width*height];

    unsigned char hue = 30;
    unsigned char saturation = 128;

    if ((flags & IMGF_ROTATE) == 0)
    {
      for (int y=0; y<height; y++)
      {
        for (int x=0; x<width; x++)
        {
          T::ParamValue val = message->getGridValueByGridPoint(C_DOUBLE(x)/scaleFactor,C_DOUBLE(y)/scaleFactor,interpolationMethod);
          uint v = 255 - ((val - minValue) / step / levelSize);
          v = v * levelSize;
          if ((flags & IMGF_REVERSE) != 0)
            v = 255-v;

          uint col = hsv_to_rgb(hue,saturation,(unsigned char)v);
          if (val == ParamValueMissing)
            col = 0xE8E8E8;

          image[y*width + x] = col;
        }
      }
    }
    else
    {
      for (int y=0; y<height; y++)
      {
        for (int x=0; x<width; x++)
        {
          T::ParamValue val = message->getGridValueByGridPoint(C_DOUBLE(x)/scaleFactor,C_DOUBLE(y)/scaleFactor,interpolationMethod);
          uint v = 255 - ((val - minValue) / step / levelSize);
          v = v * levelSize;
          if ((flags & IMGF_REVERSE) != 0)
            v = 255-v;

          uint col = hsv_to_rgb(hue,saturation,(unsigned char)v);
          if (val == ParamValueMissing)
            col = 0xE8E8E8;

          image[(height-y-1)*width + x] = col;
        }
      }
    }

    jpeg_save(imageFile,image,height,width,100);
    delete[] image;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,nullptr);
  }
}



void saveImagesByParameterId(uint fileIndex,SmartMet::GRID::GridFile& gridFile,T::ParamId parameterId,const char *imageDir,double scaleFactor,uint valueLevels,uint flags)
{
  try
  {
    printf("SAVE IMAGES %s\n",parameterId.c_str());

    T::ParamValue minValue = 0;
    T::ParamValue maxValue = 0;
    short interpolationMethod = T::AreaInterpolationMethod::Linear;
    Identification::GribParameterDef def;
    if (Identification::gridDef.getGribParamDefById(parameterId,def))
      interpolationMethod = Identification::gridDef.getPreferredInterpolationMethodByUnits(def.mParameterUnits);

    if ((flags & IMGF_PARAM) != 0)
      getGridMinAndMaxValues(gridFile,parameterId,minValue,maxValue);

    std::size_t messageCount = gridFile.getNumberOfMessages();
    for (std::size_t m=0; m<messageCount; m++)
    {
      const GRID::Message *message = gridFile.getMessageByIndex(m);
      if (message->getGribParameterId() == parameterId)
      {
        auto level = message->getGridParameterLevel();
        if ((flags & IMGF_PARAM) == 0)
          message->getGridMinAndMaxValues(minValue,maxValue);

        char imageFile[300];
        sprintf(imageFile,"%s/image-%04u-%s-%09u-%s-%04llu.jpg",imageDir,fileIndex,parameterId.c_str(),level,toString(message->getForecastTime()).c_str(),(unsigned long long)m);
        saveMessageImage(imageFile,message,minValue,maxValue,interpolationMethod,scaleFactor,valueLevels,flags);
      }
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,nullptr);
  }
}



void saveAllImages(uint fileIndex,SmartMet::GRID::GridFile& gridFile,const char *imageDir,double scaleFactor,uint valueLevels,uint flags)
{
  try
  {
    printf("SAVE ALL IMAGES\n");

    T::ParamValue minValue = 0;
    T::ParamValue maxValue = 0;
    short interpolationMethod = T::AreaInterpolationMethod::Linear;

    std::size_t messageCount = gridFile.getNumberOfMessages();
    for (std::size_t m=0; m<messageCount; m++)
    {
      const GRID::Message *message = gridFile.getMessageByIndex(m);
      auto level = message->getGridParameterLevel();
      message->getGridMinAndMaxValues(minValue,maxValue);

      char imageFile[300];
      sprintf(imageFile,"%s/image-%04u-%04lu-%09u-%s.jpg",imageDir,fileIndex,m,level,toString(message->getForecastTime()).c_str());
      saveMessageImage(imageFile,message,minValue,maxValue,interpolationMethod,scaleFactor,valueLevels,flags);
    }
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
  printf("  This program creates JPG-images from the grid data and stores images\n");
  printf("  into the given directory.\n");
  printf("\n");
  printf("USAGE :\n");
  printf("  grid_images <imageDir> [-p <paramId>] [-s <scaleFactor>] [-l <levels>] [-pmax] [-rotate] [-reverse] <file> [<file_2> .. <file_N>]\n");
  printf("\n");
  printf("PARAMETERS :\n");
  printf("  <imageDir>        -- Directory where grid images will be stored\n");
  printf("  -p <paramId>      -- Requested parameter id (0 = all)\n");
  printf("  -l <levels>       -- Number of value levels (max 256)\n");
  printf("  -s <scaleFactor>  -- Image scale factor (use 1.0 as default)\n");
  printf("  -pmax             -- Count image max and min values on parameter level (i.e. over several messages)\n");
  printf("  -rotate           -- Rotate image\n");
  printf("  -reverse          -- Use reverse values\n");
  printf("  <file>            -- Grib file\n");
  printf("\n");
  printf("EXAMPLES :\n");
  printf("  grid_images /MyImages mygribfile.grib2\n");
  printf("  grid_images /MyImages -p 130 -s 2.0 -l 8 -rotate mygribfile.grib2\n");
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
    T::ParamId parameterId;
    uint valueLevels = 256;
    double scaleFactor = 1.0;
    uint flags = 0;
    std::vector<std::string>files;

    for (int t=2; t<argc;t++)
    {
      if (strcmp(argv[t],"-p") == 0  &&  (t+1) < argc)
      {
        parameterId = argv[t+1];
        t++;
      }
      else
      if (strcmp(argv[t],"-s") == 0  &&  (t+1) < argc)
      {
        scaleFactor = toDouble(argv[t+1]);
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
      if (strcmp(argv[t],"-rotate") == 0)
        flags |= IMGF_ROTATE;
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

      if (toInt64(parameterId.c_str()) != 0)
        saveImagesByParameterId(fileIndex,gridFile,parameterId,imageDir.c_str(),scaleFactor,valueLevels,flags);
      else
        saveAllImages(fileIndex,gridFile,imageDir.c_str(),scaleFactor,valueLevels,flags);

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
