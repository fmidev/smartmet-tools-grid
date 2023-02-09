#include "grid-content/contentServer/corba/client/ClientImplementation.h"
#include "grid-content/dataServer/corba/client/ClientImplementation.h"
#include <macgyver/Exception.h>
#include "grid-files/common/GeneralFunctions.h"
#include "grid-files/common/ImageFunctions.h"
#include "grid-files/common/ImagePaint.h"
#include "grid-files/common/GraphFunctions.h"
#include "grid-files/identification/GridDef.h"

using namespace SmartMet;


CImage  itsLandSeaMask;


bool isLand(double lon,double lat)
{
  try
  {
    if (itsLandSeaMask.pixel == nullptr)
      return false;

    if (lon >= 180)
      lon = lon - 360;

    if (lat >= 90)
      lat = lat - 90;

    int x = C_INT(round((lon+180)*10));
    int y = C_INT(round((lat+90)*10));

    if (x >= 0  &&  x < itsLandSeaMask.width  &&  y >= 0  &&  y < itsLandSeaMask.height)
    {
      int pos = ((itsLandSeaMask.height-y-1)*itsLandSeaMask.width + x);

      if (pos >= 0  &&  pos < (itsLandSeaMask.height*itsLandSeaMask.width)  &&  itsLandSeaMask.pixel[pos] < 0x808080)
        return true;
    }
    return false;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP, "Operation failed!", nullptr);
    throw exception;
  }
}


int main(int argc, char *argv[])
{
  try
  {
    char *serviceIor = getenv("SMARTMET_DS_IOR");
    if (serviceIor == nullptr)
    {
      fprintf(stdout,"SMARTMET_DS_IOR not defined!\n");
      return -1;
    }

    // ### Parsing command-line parameters:

    if (argc < 9)
    {
      fprintf(stdout,"USAGE:\n");
      fprintf(stdout,"  ds_getGridIsobandImage <sessionId> <fileId> <messageIndex> <areaInterpolation>\n");
      fprintf(stdout,"    <multiplier> <rotateImage> <pngFile> [-map <mapfile> <bordercolor> <landcolor> <seacolor>] -contours <contourVal1> [<contourVal2>..<contourValN>] \n");
      return -1;
    }

    T::ByteData_vec contours;
    T::AttributeList attributeList;
    T::ParamValue_vec lowValues;
    T::ParamValue_vec highValues;
    T::SessionId sessionId = toInt64(argv[1]);
    uint fileId = toInt64(argv[2]);
    uint messageIndex = toInt64(argv[3]);
    uint areaInterpolation = toInt64(argv[4]);
    double mp = toDouble(argv[5]);
    bool rotate = (bool)toInt64(argv[6]);
    char *pngFile = argv[7];
    uint modificationOperation = 0;
    double_vec modificationParameters;

    std::vector<uint> colorList;

    uint borderColor = 0xFFFFFFFF;
    uint landColor = 0xFFFFFFFF;
    uint seaColor = 0xFFFFFFFF;
    bool map = false;

    for (int a=8; a<(argc-1); a++)
    {
      if (strcmp(argv[a],"-map") == 0  &&  (a+4) < argc)
      {
        jpg_load(argv[a+1],itsLandSeaMask);
        borderColor = strtoll(argv[a+2],nullptr,16);
        landColor = strtoll(argv[a+3],nullptr,16);
        seaColor = strtoll(argv[a+4],nullptr,16);
        map = true;
      }
      if (strcmp(argv[a],"-contours") == 0)
      {
        for (int t=a+1; t<(argc-1); t++)
        {
          std::vector<std::string> partList1;
          splitString(argv[t],':',partList1);
          lowValues.push_back(toDouble(partList1[0].c_str()));
          if (partList1.size() == 2)
            colorList.push_back(strtoll(partList1[1].c_str(),nullptr,16));
          else
            colorList.push_back(0xFFFFFFFF);

          std::vector<std::string> partList2;
          splitString(argv[t+1],':',partList2);
          highValues.push_back(toDouble(partList2[0].c_str()));
        }
        a = argc;
      }
    }

    attributeList.addAttribute("grid.areaInterpolationMethod",std::to_string(areaInterpolation));
    attributeList.addAttribute("contour.coordinateType",std::to_string(T::CoordinateTypeValue::GRID_COORDINATES));



    // ### Creating a dataServer client:

    DataServer::Corba::ClientImplementation dataServer;
    dataServer.init(serviceIor);

    // ### Calling the dataServer:

    unsigned long long startTime = getTime();
    int result = dataServer.getGridIsobands(sessionId,fileId,messageIndex,lowValues,highValues,attributeList,modificationOperation,modificationParameters,contours);
    if (result != 0)
    {
      fprintf(stdout,"ERROR (%d) : %s\n",result,DataServer::getResultString(result).c_str());
      return -5;
    }

    T::GridCoordinates coordinates;
    if (map)
    {
      result = dataServer.getGridCoordinates(sessionId,fileId,messageIndex,T::CoordinateTypeValue::LATLON_COORDINATES,coordinates);
      if (result != 0)
      {
        fprintf(stdout,"ERROR (%d) : %s\n",result,DataServer::getResultString(result).c_str());
        return -5;
      }
    }

    unsigned long long endTime = getTime();

    // ### Printing the result:

    printf("\nContours : %ld\n",contours.size());

    for (auto contour = contours.begin(); contour != contours.end(); ++contour)
    {
      printf("Lines %ld\n",contour->size());
    }

    printf("\nTIME : %f sec\n\n",(float)(endTime-startTime)/1000000);

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

    ImagePaint imagePaint(imageWidth,imageHeight,0xFFFFFFFF,0x000000,0xA0A0A0,false,rotate);

    unsigned int *image = imagePaint.getImage();

    // ### Painting contours into the image:

    if (contours.size() > 0)
    {
      uint c = 250;
      uint step = 250 / contours.size();

      uint t = 0;
      for (auto it = contours.begin(); it != contours.end(); ++it)
      {
        uint col = colorList[t];
        if (col == 0xFFFFFFFF)
          col = c ;// (c << 16) + (c << 8) + c;

        imagePaint.setFillColor(col);
        imagePaint.paintWkb(mp,mp,0,0,*it);
        c = c - step;
        t++;
      }
    }

    // ### Saving the image:


    if (map)
    {
      for (int x=0; x<width; x++)
      {
        bool land = false;
        for (int y=0; y<height; y++)
        {
          uint idx = y*width + x;
          T::Coordinate cc = coordinates.mCoordinateList[idx];

          bool ind = isLand(cc.x(),cc.y());
          if (ind)
          {
            if (landColor != 0xFFFFFFFF)
              image[idx] = landColor;

            if (borderColor != 0xFFFFFFFF)
            {
              if (!land)
              {
                image[idx] = borderColor;
                if ((y+1) < height)
                  image[idx+width] = borderColor;
              }

              land = true;
            }
          }
          else
          {
            if (seaColor != 0xFFFFFFFF)
              image[idx] = seaColor;

            if (borderColor != 0xFFFFFFFF)
            {
              if (land)
              {
                if (y > 0)
                  image[idx-width] = borderColor;

                if (y > 2)
                  image[idx-2*width] = borderColor;
              }
            }
            land = false;
          }
        }
      }

      if (borderColor != 0xFFFFFFFF)
      {
        for (int y=0; y<height; y++)
        {
          bool land = false;
          for (int x=0; x<width; x++)
          {
            uint idx = y*width + x;
            T::Coordinate cc = coordinates.mCoordinateList[idx];
            bool ind = isLand(cc.x(),cc.y());
            if (ind)
            {
              if (!land)
              {
                image[idx] = borderColor;
                if ((x+1) < width)
                  image[idx+1] = borderColor;

                //if ((x+2) < width)
                //  image[idx+2] = landborder;

                land = true;
              }
            }
            else
            {
              if (land)
              {
                if (x > 0)
                  image[idx-1] = borderColor;

                if (x > 1)
                  image[idx-2] = borderColor;

                //if (x > 3)
                //  image[idx-3] = landborder;
              }
              land = false;
            }
          }
        }
      }

    }

    png_save(pngFile,image,width,height);
    //imagePaint.savePngImage(pngFile);

    return 0;
  }
  catch (Fmi::Exception& e)
  {
    Fmi::Exception exception(BCP,"Service call failed!",nullptr);
    exception.printError();
    return -6;
  }
}

