#include "grid-content/contentServer/corba/client/ClientImplementation.h"
#include "grid-content/dataServer/corba/client/ClientImplementation.h"
#include <macgyver/Exception.h>
#include "grid-files/common/GeneralFunctions.h"
#include "grid-files/common/ImageFunctions.h"
#include "grid-files/common/ImagePaint.h"
#include "grid-files/common/GraphFunctions.h"
#include "grid-files/identification/GridDef.h"

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

    if (argc < 9)
    {
      fprintf(stdout,"USAGE:\n");
      fprintf(stdout,"  ds_getGridIsobandImageByGeometry <sessionId> <fileId> <messageIndex> <attributeList> <areaInterpolation> \n");
      fprintf(stdout,"    <multiplier> <rotateImage> <pngFile> <contourVal1> [<contourVal2>..<contourValN>] \n");
      return -1;
    }

    T::ByteData_vec contours;
    T::AttributeList attributeList;
    T::ParamValue_vec lowValues;
    T::ParamValue_vec highValues;
    T::SessionId sessionId = toInt64(argv[1]);
    uint fileId = toInt64(argv[2]);
    uint messageIndex = toInt64(argv[3]);
    char *attributes = argv[4];
    uint areaInterpolation = toInt64(argv[5]);
    double mp = toDouble(argv[6]);
    bool rotate = (bool)toInt64(argv[7]);
    char *pngFile = argv[8];

    std::vector<uint> colorList;

    for (int t=9; t<(argc-1); t++)
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

    attributeList.addAttribute("grid.areaInterpolationMethod",std::to_string(areaInterpolation));
    attributeList.addAttribute("contour.coordinateType",std::to_string(T::CoordinateTypeValue::GRID_COORDINATES));

    std::vector<std::string> attrList;
    splitString(attributes,',',attrList);
    for (auto it=attrList.begin(); it != attrList.end(); ++it)
    {
      std::vector<std::string> partList;
      splitString(*it,'=',partList);
      if (partList.size() == 2)
        attributeList.setAttribute(partList[0],partList[1]);
    }


    // ### Creating a dataServer client:

    DataServer::Corba::ClientImplementation dataServer;
    dataServer.init(serviceIor);

    // ### Calling the dataServer:

    unsigned long long startTime = getTime();
    int result = dataServer.getGridIsobandsByGeometry(sessionId,fileId,messageIndex,lowValues,highValues,attributeList,contours);
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
          col = (c << 16) + (c << 8) + c;

        imagePaint.setFillColor(col);
        imagePaint.paintWkb(mp,mp,0,0,*it);
        c = c - step;
        t++;
      }
    }

    // ### Saving the image:

    imagePaint.savePngImage(pngFile);

    return 0;
  }
  catch (Fmi::Exception& e)
  {
    Fmi::Exception exception(BCP,"Service call failed!",nullptr);
    exception.printError();
    return -6;
  }
}

