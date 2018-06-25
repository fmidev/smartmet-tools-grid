#include "grid-content/contentServer/corba/client/ClientImplementation.h"
#include "grid-content/dataServer/corba/client/ClientImplementation.h"
#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"
#include "grid-files/common/ImageFunctions.h"

using namespace SmartMet;

bool rotate = false;



void saveImage(const char *imageFile,uint columns,uint rows,T::ParamValue_vec&  values)
{
  try
  {
    double maxValue = -1000000000;
    double minValue = 1000000000;

    uint sz = (uint)values.size();

    if (sz != (columns * rows))
    {
      printf("The number of values (%u) does not match to the grid size (%u x %u)1\n",sz,columns,rows);
      exit(-1);
    }

    for (uint t=0; t<sz; t++)
    {
      double val = values[t];
      if (val != ParamValueMissing)
      {
        if (val < minValue)
          minValue = val;

        if (val > maxValue)
          maxValue = val;
      }
    }

    double dd = maxValue - minValue;
    double step = dd / 255;

    int width = columns;
    int height = rows;

    unsigned long *image = new unsigned long[width*height];

    unsigned char hue = 30;
    unsigned char saturation = 128;
    uint c = 0;

    if (!rotate)
    {
      for (int y=0; y<height; y++)
      {
        for (int x=0; x<width; x++)
        {
          T::ParamValue val = values[c];
          uint v = 255 - (uint)((val - minValue) / step);
          uint col = hsv_to_rgb(hue,saturation,(unsigned char)v);
          if (val == ParamValueMissing)
            col = 0xE8E8E8;

          image[y*width + x] = col;
          c++;
        }
      }
    }
    else
    {
      for (int y=height-1; y>=0; y--)
      {
        for (int x=0; x<width; x++)
        {
          T::ParamValue val = values[c];
          uint v = 255 - (uint)((val - minValue) / step);
          uint col = hsv_to_rgb(hue,saturation,(unsigned char)v);
          if (val == ParamValueMissing)
            col = 0xE8E8E8;

          image[y*width + x] = col;
          c++;
        }
      }
    }

    jpeg_save(imageFile,image,height,width,100);
    delete image;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,NULL);
  }
}




int main(int argc, char *argv[])
{
  try
  {
    char *serviceIor = getenv("SMARTMET_DS_IOR");
    if (serviceIor == NULL)
    {
      fprintf(stdout,"SMARTMET_DS_IOR not defined!\n");
      return -2;
    }

    if (argc < 14)
    {
      fprintf(stdout,"USAGE: cs_getGridImageByArea <sessionId> <fileId> <messageIndex> <flags> <coordinateType> <columns> <rows> <x> <y> <xStep> <yStep> <interpolationMethod> <image.jpg> [-rotate]\n");
      return -1;
    }

    // ### Session:
    T::SessionId sessionId = (SmartMet::T::SessionId)atoll(argv[1]);


    // ### Creating a dataServer client:

    DataServer::Corba::ClientImplementation dataServer;
    dataServer.init(serviceIor);

    // ### Calling the dataServer:

    uint fileId = (uint)atoll(argv[2]);
    uint messageIndex = (uint)atoll(argv[3]);
    uint flags = (uint)atoll(argv[4]);
    T::CoordinateType coordinateType = (T::CoordinateType)atoll(argv[5]);
    uint columns = (uint)atoll(argv[6]);
    uint rows = (uint)atoll(argv[7]);
    double x = (double)atof(argv[8]);
    double y = (double)atof(argv[9]);
    double xStep = (double)atof(argv[10]);
    double yStep = (double)atof(argv[11]);
    short interpolationMethod = (short)atoll(argv[12]);
    char *imageFile = argv[13];
    T::ParamValue_vec values;

    unsigned long long startTime = getTime();
    int result = dataServer.getGridValueVectorByRectangle(sessionId,fileId,messageIndex,flags,coordinateType,columns,rows,x,y,xStep,yStep,interpolationMethod,values);
    unsigned long long endTime = getTime();

    if (result != 0)
    {
      fprintf(stdout,"ERROR (%d) : %s\n",result,DataServer::getResultString(result).c_str());
      return -5;
    }

    if (argc == 14 &&  strcmp(argv[13],"-rotate") == 0)
      rotate = true;

    // ### Save the image:
    saveImage(imageFile,columns,rows,values);

    printf("\nTIME : %f sec\n\n",(float)(endTime-startTime)/1000000);

    return 0;
  }
  catch (SmartMet::Spine::Exception& e)
  {
    SmartMet::Spine::Exception exception(BCP,"Service call failed!",NULL);
    exception.printError();
    return -7;
  }
}

