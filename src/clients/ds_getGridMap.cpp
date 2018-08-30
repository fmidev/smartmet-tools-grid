#include "grid-content/contentServer/corba/client/ClientImplementation.h"
#include "grid-content/dataServer/corba/client/ClientImplementation.h"
#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"
#include "grid-files/common/ImageFunctions.h"

using namespace SmartMet;




void saveImage(const char *imageFile,uint columns,uint rows,T::ParamValue_vec&  values)
{
  try
  {
    double maxValue = -1000000000;
    double minValue = 1000000000;

    uint sz = values.size();

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

    uint xx = columns / 36;
    uint yy = rows / 18;

    double dd = maxValue - minValue;
    double step = dd / 255;

    int width = columns;
    int height = rows;

    unsigned long *image = new unsigned long[width*height];

    unsigned char hue = 0;
    unsigned char saturation = 0;
    uint c = 0;

    for (int y=0; y<height; y++)
    {
      for (int x=0; x<width; x++)
      {
        T::ParamValue val = values[c];
        //printf("Val(%u,%u) : %f\n",x,y,val);
        uint v = 255 - ((val - minValue) / step);
        uint col = hsv_to_rgb(hue,saturation,(unsigned char)v);
        if (val == ParamValueMissing)
          col = 0xE8E8E8;

        if ((x % xx) == 0  ||  (y % yy) == 0)
          col = 0xFFFFFF;

        image[y*width + x] = col;
        c++;
      }
    }

    jpeg_save(imageFile,image,height,width,100);
    delete image;
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
      return -2;
    }

    if (argc != 6)
    {
      fprintf(stdout,"USAGE: cs_getGridMap <sessionId> <fileId> <messageIndex> <flags> <jpgFile>\n");
      return -1;
    }

    // ### Session:
    T::SessionId sessionId = toInt64(argv[1]);


    // ### Creating a dataServer client:

    DataServer::Corba::ClientImplementation dataServer;
    dataServer.init(serviceIor);

    // ### Calling the data server:

    uint fileId = toInt64(argv[2]);
    uint messageIndex = toInt64(argv[3]);
    uint flags = toInt64(argv[4]);
    uint columns = 1800;
    uint rows = 900;
    T::ParamValue_vec values;

    unsigned long long startTime = getTime();
    int result = dataServer.getGridValueVectorByRectangle(sessionId,fileId,messageIndex,flags,T::CoordinateTypeValue::LATLON_COORDINATES,columns,rows,-180,90,360/C_DOUBLE(columns),-180/C_DOUBLE(rows),T::AreaInterpolationMethod::Nearest,values);
    unsigned long long endTime = getTime();

    if (result != 0)
    {
      fprintf(stdout,"ERROR (%d) : %s\n",result,DataServer::getResultString(result).c_str());
      return -5;
    }

    // ### Saving the grid data as an image:

    saveImage(argv[5],columns,rows,values);

    printf("\nTIME : %f sec\n\n",(float)(endTime-startTime)/1000000);

    return 0;
  }
  catch (SmartMet::Spine::Exception& e)
  {
    SmartMet::Spine::Exception exception(BCP,"Service call failed!",nullptr);
    exception.printError();
    return -6;
  }
}

