#include "grid-content/contentServer/corba/client/ClientImplementation.h"
#include "grid-content/dataServer/corba/client/ClientImplementation.h"
#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"
#include "grid-files/common/ImageFunctions.h"
#include "grid-files/common/GraphFunctions.h"
#include "grid-files/identification/GridDef.h"

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

    if (argc < 11)
    {
      fprintf(stdout,"USAGE:\n");
      fprintf(stdout,"  ds_getGridIsolineImageByTime <sessionId> <fileId1> <msgIdx1> <fileId2> <msgIdx2>\n");
      fprintf(stdout,"    <gridTime> <multiplier> <rotateImage> <imageFile> <contourVal1> [<contourVal2>..<contourValN>] \n");
      return -1;
    }

    T::WkbData_vec contours;
    T::AttributeList attributeList;
    T::ParamValue_vec values;
    T::SessionId sessionId = toInt64(argv[1]);
    uint fileId1 = toInt64(argv[2]);
    uint messageIndex1 = toInt64(argv[3]);
    uint fileId2 = toInt64(argv[4]);
    uint messageIndex2 = toInt64(argv[5]);
    std::string timestamp = argv[6];
    double mp = toDouble(argv[7]);
    bool rotate = (bool)toInt64(argv[8]);
    char *jpgFile = argv[9];

    for (int t=10; t<argc; t++)
      values.push_back(atof(argv[t]));


    attributeList.addAttribute("grid.timeInterpolationMethod",std::to_string(1));
    attributeList.addAttribute("contour.coordinateType",std::to_string(T::CoordinateTypeValue::GRID_COORDINATES));

    // ### Creating a dataServer client:

    DataServer::Corba::ClientImplementation dataServer;
    dataServer.init(serviceIor);

    // ### Calling the dataServer:

    unsigned long long startTime = getTime();
    int result = dataServer.getGridIsolinesByTime(sessionId,fileId1,messageIndex1,fileId2,messageIndex2,timestamp,values,attributeList,contours);
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
  catch (SmartMet::Spine::Exception& e)
  {
    SmartMet::Spine::Exception exception(BCP,"Service call failed!",nullptr);
    exception.printError();
    return -6;
  }
}

