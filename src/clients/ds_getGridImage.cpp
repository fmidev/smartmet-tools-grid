#include "grid-content/contentServer/corba/client/ClientImplementation.h"
#include "grid-content/dataServer/corba/client/ClientImplementation.h"
#include <macgyver/Exception.h>
#include "grid-files/common/GeneralFunctions.h"
#include "grid-files/common/ImageFunctions.h"

using namespace SmartMet;

bool rotate = false;



void saveImage(const char *imageFile,T::GridData&  gridData)
{
  try
  {
    double maxValue = -1000000000;
    double minValue = 1000000000;

    int width = gridData.mColumns;
    int height = gridData.mRows;

    uint size = width*height;
    std::size_t sz = gridData.mValues.size();

    if (sz != size)
    {
      printf("ERROR: There are not enough values (= %lu) for the grid (%u x %u)!\n",sz,width,height);
      return;
    }

    for (std::size_t t=0; t<sz; t++)
    {
      double val = gridData.mValues[t];
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



    uint *image = new uint[size];

    unsigned char hue = 0;
    unsigned char saturation = 0;
    uint c = 0;

    if (!rotate)
    {
      for (int y=0; y<height; y++)
      {
        for (int x=0; x<width; x++)
        {
          T::ParamValue val = gridData.mValues[c];
          uint v = 255 - ((val - minValue) / step);
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
          T::ParamValue val = gridData.mValues[c];
          uint v = 255 - ((val - minValue) / step);
          uint col = hsv_to_rgb(hue,saturation,(unsigned char)v);
          if (val == ParamValueMissing)
            col = 0xE8E8E8;

          image[y*width + x] = col;
          c++;
        }
      }
    }

    jpeg_save(imageFile,image,height,width,100);
    delete[] image;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP,"Operation failed!",nullptr);
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

    if (argc < 5)
    {
      fprintf(stdout,"USAGE: cs_getGridData <sessionId> <fileId> <messageIndex> <jpgFile> [-rotate]\n");
      return -1;
    }

    if (argc == 6  &&  strcmp(argv[5],"-rotate") == 0)
      rotate = true;


    // ### Session:
    T::SessionId sessionId = toInt64(argv[1]);


    // ### Creating a dataServer client:

    DataServer::Corba::ClientImplementation dataServer;
    dataServer.init(serviceIor);

    // ### Calling the data server:

    uint fileId = toInt64(argv[2]);
    uint messageIndex = toInt64(argv[3]);
    T::GridData gridData;

    unsigned long long startTime = getTime();
    int result = dataServer.getGridData(sessionId,fileId,messageIndex,gridData);
    unsigned long long endTime = getTime();

    if (result != 0)
    {
      fprintf(stdout,"ERROR (%d) : %s\n",result,DataServer::getResultString(result).c_str());
      return -5;
    }

    // ### Printing the grid data:

    gridData.print(std::cout,0,0);

    // ### Saving the grid data as an image:

    saveImage(argv[4],gridData);

    printf("\nTIME : %f sec\n\n",(float)(endTime-startTime)/1000000);

    return 0;
  }
  catch (Fmi::Exception& e)
  {
    Fmi::Exception exception(BCP,"Service call failed!",nullptr);
    exception.printError();
    return -6;
  }
}

