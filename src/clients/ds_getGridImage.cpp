#include "contentServer/corba/client/ClientImplementation.h"
#include "dataServer/corba/client/ClientImplementation.h"
#include "grid-files/common/Exception.h"
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

    if (sz != (uint)size)
    {
      printf("ERROR: There are not enough values (= %u) for the grid (%u x %u)!\n",(uint)sz,width,height);
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



    unsigned long *image = new unsigned long[size];

    unsigned char hue = 30;
    unsigned char saturation = 128;
    uint c = 0;

    if (!rotate)
    {
      for (int y=0; y<height; y++)
      {
        for (int x=0; x<width; x++)
        {
          T::ParamValue val = gridData.mValues[c];
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
          T::ParamValue val = gridData.mValues[c];
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
    throw SmartMet::Spine::Exception(BCP,"Operation failed!",NULL);
  }
}




int main(int argc, char *argv[])
{
  try
  {
    char *contentServiceIor = getenv("SMARTMET_CS_IOR");
    if (contentServiceIor == NULL)
    {
      fprintf(stdout,"SMARTMET_CS_IOR not defined!\n");
      return -2;
    }


    if (argc < 6)
    {
      fprintf(stdout,"USAGE: cs_getGridData <sessionId> <dataServerId> <fileId> <messageIndex> <jpgFile> [-rotate]\n");
      return -1;
    }

    if (argc == 7  &&  strcmp(argv[6],"-rotate") == 0)
      rotate = true;


    // ### Session:
    T::SessionId sessionId = (SmartMet::T::SessionId)atoll(argv[1]);


    // #######################################################################
    // ### STEP 1: Getting the dataServer IOR from the contentServer.
    // #######################################################################

    // ### Creating a contentServer client:

    ContentServer::Corba::ClientImplementation contentServer;
    contentServer.init(contentServiceIor);

    // ### Calling the contentServer:

    uint dataServerId = (uint)atoll(argv[2]);
    T::ServerInfo dataServerInfo;
    int result = 0;

    if (dataServerId != 0)
      result = contentServer.getDataServerInfoById(sessionId,dataServerId,dataServerInfo);
    else
      result = contentServer.getDataServerInfoByName(sessionId,std::string(argv[2]),dataServerInfo);


    if (result == ContentServer::Result::DATA_NOT_FOUND)
    {
      fprintf(stdout,"Unknown data server id (%s)!\n",argv[2]);
      return -3;
    }

    if (result != 0)
    {
      fprintf(stdout,"ERROR (%d) : %s\n",result,ContentServer::getResultString(result).c_str());
      return -4;
    }


    // #######################################################################
    // ### STEP 2: Requesting data from the dataServer.
    // #######################################################################

    // ### Creating a dataServer client:

    DataServer::Corba::ClientImplementation dataServer;
    dataServer.init(dataServerInfo.mServerIor);

    // ### Calling the data server:

    uint fileId = (uint)atoll(argv[3]);
    uint messageIndex = (uint)atoll(argv[4]);
    T::GridData gridData;

    unsigned long long startTime = getTime();
    result = dataServer.getGridData(sessionId,fileId,messageIndex,gridData);
    unsigned long long endTime = getTime();

    if (result != 0)
    {
      fprintf(stdout,"ERROR (%d) : %s\n",result,DataServer::getResultString(result).c_str());
      return -5;
    }

    // ### Printing the grid data:

    gridData.print(std::cout,0,0);

    // ### Saving the grid data as an image:

    saveImage(argv[5],gridData);

    printf("\nTIME : %f sec\n\n",(float)(endTime-startTime)/1000000);

    return 0;
  }
  catch (SmartMet::Spine::Exception& e)
  {
    SmartMet::Spine::Exception exception(BCP,"Service call failed!",NULL);
    exception.printError();
    return -6;
  }
}

