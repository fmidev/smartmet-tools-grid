#include "grid-files/grid/PhysicalGridFile.h"
#include "grid-files/grib1/GribFile.h"
#include "grid-files/grib2/GribFile.h"
#include "grid-files/grid/PrintOptions.h"
#include "grid-files/identification/GridDef.h"
#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"
#include "grid-files/common/GeneralDefinitions.h"
#include "grid-files/common/ImageFunctions.h"
#include "grid-files/common/GraphFunctions.h"
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





void print_usage()
{
  printf("-------------------------------------------------------------------------------- \n");
  printf("PROGRAM :\n");
  printf("  The program saves isoline image of a grid.\n");
  printf("\n");
  printf("USAGE :\n");
  printf("  grid_getIsolineImage <gridFile> <msgIndex> <multiplier>\n");
  printf("         <rotateImage> <imageFile> <contourVal1> [<contourVal2>..<contourValN>] \n");
  printf("\n");
  printf("-------------------------------------------------------------------------------- \n");
  printf("\n");
}




int main(int argc, char **argv)
{
  try
  {
    if (argc < 7)
    {
      print_usage();
      return -1;
    }

    init();

    T::WkbData_vec contours;
    T::AttributeList attributeList;
    T::ParamValue_vec values;

    std::string gridFilename = argv[1];
    uint messageIndex = atoi(argv[2]);
    double mp = atof(argv[3]);
    bool rotate = (bool)atoi(argv[4]);
    char *jpgFile = argv[5];

    SmartMet::GRID::PhysicalGridFile gridFile;
    gridFile.read(gridFilename);

    for (int t=6; t<argc; t++)
    {
      values.push_back(atof(argv[t]));
    }

    attributeList.addAttribute("grid.areaInterpolationMethod","1");
    attributeList.addAttribute("contour.coordinateType",std::to_string(T::CoordinateTypeValue::GRID_COORDINATES));


    GRID::Message *message = gridFile.getMessageByIndex(messageIndex);
    if (message == nullptr)
    {
      fprintf(stdout,"ERROR : Message (%u) not found!\n",messageIndex);
      return -5;
    }

    message->getGridIsolines(values,attributeList,contours);

    printf("\nContours : %ld\n",contours.size());

    for (auto contour = contours.begin(); contour != contours.end(); ++contour)
    {
      printf("Lines %ld\n",contour->size());
    }

    // ### Allocating and initializing the image:

    int width = atoi(attributeList.getAttributeValue("grid.width"));
    int height = atoi(attributeList.getAttributeValue("grid.height"));

    if (width <= 0 ||  height <= 0)
    {
      fprintf(stdout,"ERROR: Invalid grid size (%d x %d)!\n",width,height);
      return -5;
    }

    int imageWidth = width*mp;
    int imageHeight = height*mp;

    int sz = imageWidth * imageHeight;
    unsigned long *image = new unsigned long[sz];
    for (int t=0; t<sz; t++)
      image[t] = 0xFFFFFF;

    // ### Painting contours into the image:

    if (contours.size() > 0)
      paintWkb(image,imageWidth,imageHeight,false,rotate,mp,0,0,contours,0x00);

    // ### Saving the image and releasing the image data:

    jpeg_save(jpgFile,image,imageHeight,imageWidth,100);
    delete[] image;

    return 0;
  }
  catch (...)
  {
    SmartMet::Spine::Exception exception(BCP,exception_operation_failed,nullptr);
    exception.printError();
  }
}

