#include "grid-content/contentServer/corba/client/ClientImplementation.h"
#include "grid-content/dataServer/corba/client/ClientImplementation.h"
#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"
#include "grid-files/common/ImageFunctions.h"
#include "grid-files/common/ImagePaint.h"
#include "grid-files/common/GraphFunctions.h"

using namespace SmartMet;




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


    if (argc < 14)
    {
      fprintf(stdout,"USAGE: ds_getGridIsolineMap <sessionId> <fileId> <messageIndex> <startLon> <startLat> <gridWidth> <gridHeight> <lonStep> <latStep> <areaInterpolation> <multiplier> <pngFile> <contourVal1> [<contourVal2>..<contourValN>] \n");
      return -1;
    }


    // ### Session:
    T::SessionId sessionId = toInt64(argv[1]);


    // ### Creating a dataServer client:

    DataServer::Corba::ClientImplementation dataServer;
    dataServer.init(serviceIor);

    // ### Calling the dataServer:

    uint fileId = toInt64(argv[2]);
    uint messageIndex = toInt64(argv[3]);
    double startLon = toDouble(argv[4]);
    double startLat = toDouble(argv[5]);
    int width = toInt64(argv[6]);
    int height = toInt64(argv[7]);
    double lonStep = toDouble(argv[8]);
    double latStep = toDouble(argv[9]);
    uint areaInterpolation = toInt64(argv[10]);
    double mp = toDouble(argv[11]);
    char *pngFile = argv[12];

    int imageWidth = mp * width;
    int imageHeight = mp * height;

    T::ParamValue_vec values;
    std::vector<uint> colorList;

    for (int t=13; t<argc; t++)
    {
      std::vector<std::string> partList1;
      splitString(argv[t],':',partList1);
      values.push_back(atof(partList1[0].c_str()));
      if (partList1.size() == 2)
        colorList.push_back(strtoll(partList1[1].c_str(),nullptr,16));
      else
        colorList.push_back(0xFFFFFFFF);
    }

    T::WkbData_vec contours;


    std::vector<T::Coordinate> coordinates;


    double lat = startLat;
    for (int y=0; y<height; y++)
    {
      double lon = startLon;
      for (int x=0; x<width; x++)
      {
        lon = lon + lonStep;
        coordinates.push_back(T::Coordinate(lon,lat));
      }
      lat = lat + latStep;
    }

    T::AttributeList attributeList;
    attributeList.addAttribute("grid.areaInterpolationMethod",std::to_string(areaInterpolation));
    attributeList.addAttribute("contour.coordinateType",std::to_string(T::CoordinateTypeValue::LATLON_COORDINATES));

    unsigned long long startTime = getTime();
    int result = dataServer.getGridIsolinesByGrid(sessionId,fileId,messageIndex,values,width,height,coordinates,attributeList,contours);
    unsigned long long endTime = getTime();


    if (result != 0)
    {
      fprintf(stdout,"ERROR (%d) : %s\n",result,DataServer::getResultString(result).c_str());
      return -5;
    }

    // ### Printing the result:

    printf("\nContours : %ld\n",contours.size());

    for (auto contour = contours.begin(); contour != contours.end(); ++contour)
    {
      printf("Lines %ld\n",contour->size());
    }

    printf("\nTIME : %f sec\n\n",(float)(endTime-startTime)/1000000);


    ImagePaint imagePaint(imageWidth,imageHeight,0xFFFFFFFF,false,false);


    if (contours.size() > 0)
    {
      uint t = 0;
      for (auto it = contours.begin(); it != contours.end(); ++it)
      {
        uint col = colorList[t];
        if (col == 0xFFFFFFFF)
          col = 0;

        imagePaint.paintWkb(mp,mp,180,90,*it,col);
        t++;
      }
    }

    imagePaint.savePngImage(pngFile);


    return 0;
  }
  catch (SmartMet::Spine::Exception& e)
  {
    SmartMet::Spine::Exception exception(BCP,"Service call failed!",nullptr);
    exception.printError();
    return -6;
  }
}

