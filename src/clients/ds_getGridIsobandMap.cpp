#include "grid-content/contentServer/corba/client/ClientImplementation.h"
#include "grid-content/dataServer/corba/client/ClientImplementation.h"
#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"
#include "grid-files/common/ImageFunctions.h"
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
      return -2;
    }


    if (argc < 13)
    {
      fprintf(stdout,"USAGE: ds_getGridIsobandMap <sessionId> <fileId> <messageIndex> <startLon> <startLat> <gridWidth> <gridHeight> <lonStep> <latStep> <imageMultiplier> <imageFile> <contourVal1> [<contourVal2>..<contourValN>] \n");
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
    double mp = toDouble(argv[10]);
    char *jpgFile = argv[11];

    int imageWidth = mp * width;
    int imageHeight = mp * height;

    T::ParamValue_vec lowValues;
    T::ParamValue_vec highValues;
    for (int t=12; t<(argc-1); t++)
    {
      lowValues.push_back(atof(argv[t]));
      highValues.push_back(atof(argv[t+1]));
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
    attributeList.addAttribute("grid.areaInterpolationMethod",std::to_string(1));
    attributeList.addAttribute("contour.coordinateType",std::to_string(T::CoordinateTypeValue::LATLON_COORDINATES));

    unsigned long long startTime = getTime();
    int result = dataServer.getGridIsobandsByGrid(sessionId,fileId,messageIndex,lowValues,highValues,width,height,coordinates,attributeList,contours);
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


    int sz = imageWidth * imageHeight;
    unsigned long *image = new unsigned long[sz];
    for (int t=0; t<sz; t++)
      image[t] = 0xFFFFFF;


    uint c = 250;
    uint step = 250 / contours.size();

    for (auto it = contours.begin(); it != contours.end(); ++it)
    {
      uint col = (c << 16) + (c << 8) + c;
      paintWkb(image,imageWidth,imageHeight,false,false,mp,0,0,*it,col);
      c = c - step;
    }

    //paintWkb(image,imageWidth,imageHeight,false,mp,0,0,contours,0x00);

    jpeg_save(jpgFile,image,imageHeight,imageWidth,100);

    delete [] image;

    return 0;
  }
  catch (SmartMet::Spine::Exception& e)
  {
    SmartMet::Spine::Exception exception(BCP,"Service call failed!",nullptr);
    exception.printError();
    return -6;
  }
}

