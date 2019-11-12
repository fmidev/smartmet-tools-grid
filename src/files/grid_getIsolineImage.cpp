#include "grid-files/grid/PhysicalGridFile.h"
#include "grid-files/grib1/GribFile.h"
#include "grid-files/grib2/GribFile.h"
#include "grid-files/grid/PrintOptions.h"
#include "grid-files/identification/GridDef.h"
#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"
#include "grid-files/common/GeneralDefinitions.h"
#include "grid-files/common/ImageFunctions.h"
#include "grid-files/common/ImagePaint.h"
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
  printf("  grid_getIsolineImage <gridFile> <msgIndex> <areaInterpolation> <multiplier>\n");
  printf("         <rotateImage> <pngFile> <contourVal1> [<contourVal2>..<contourValN>] \n");
  printf("\n");
  printf("-------------------------------------------------------------------------------- \n");
  printf("\n");
}




int main(int argc, char **argv)
{
  try
  {
    if (argc < 8)
    {
      print_usage();
      return -1;
    }

    init();

    T::ByteData_vec contours;
    T::AttributeList attributeList;
    T::ParamValue_vec values;

    std::string gridFilename = argv[1];
    uint messageIndex = atoi(argv[2]);
    uint areaInterpolation = toInt64(argv[3]);
    double mp = toDouble(argv[4]);
    bool rotate = (bool)atoi(argv[5]);
    char *pngFile = argv[6];

    SmartMet::GRID::PhysicalGridFile gridFile;
    gridFile.read(gridFilename);

    std::vector<uint> colorList;

    for (int t=7; t<argc; t++)
    {
      std::vector<std::string> partList1;
      splitString(argv[t],':',partList1);
      values.push_back(toDouble(partList1[0].c_str()));
      if (partList1.size() == 2)
        colorList.push_back(strtoll(partList1[1].c_str(),nullptr,16));
      else
        colorList.push_back(0xFFFFFFFF);
    }

    attributeList.addAttribute("grid.areaInterpolationMethod",std::to_string(areaInterpolation));
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

    ImagePaint imagePaint(imageWidth,imageHeight,0xFFFFFFFF,false,rotate);

    // ### Painting contours into the image:

    if (contours.size() > 0)
    {
      uint t = 0;
      for (auto it = contours.begin(); it != contours.end(); ++it)
      {
        uint col = colorList[t];
        if (col == 0xFFFFFFFF)
          col = 0;

        imagePaint.paintWkb(mp,mp,0,0,*it,col);
        t++;
      }
    }

    // ### Saving the image and releasing the image data:

    imagePaint.savePngImage(pngFile);

    return 0;
  }
  catch (...)
  {
    SmartMet::Spine::Exception exception(BCP,exception_operation_failed,nullptr);
    exception.printError();
  }
}

