#include <macgyver/Exception.h>
#include <grid-files/common/GeneralFunctions.h>
#include <grid-files/common/BitLine.h>
#include <gis/LandCover.h>


using namespace SmartMet;


int main(int argc, char *argv[])
{
  try
  {
    if (argc != 5)
    {
      fprintf(stderr,"USAGE: createLandSeaMap <coverDir> <width> <height> <mapFile>\n");
      return -1;
    }


    Fmi::LandCover landCover(argv[1]);

    uint byteIdx = 0;
    uint bitIdx = 0;

    uint w = atoi(argv[2]);
    uint h = atoi(argv[3]);

    double xstep = 360.0 / (double)w;
    double ystep = 180.0 / (double)h;

    uint bits = w*h;
    uint bytes = bits/8 + 1;
    uchar *data = new uchar[bytes];

    memset(data,0,bytes);

    double lat = -90;
    for (uint y=0; y<h; y++)
    {
      double lon = -180;
      for (uint x=0; x<w; x++)
      {
        auto coverType = landCover.coverType(lon,lat);
        if (coverType == Fmi::LandCover::Sea || coverType == Fmi::LandCover::CaspianSea || coverType == Fmi::LandCover::Lakes)
        {
        }
        else
        {
          data[byteIdx] |= (1 << bitIdx);
        }

        //printf("%u,%u  %f,%f %d  %u:%u %02X\n",x,y,lon,lat,(int)coverType,byteIdx,bitIdx,data[byteIdx]);

        bitIdx++;
        if (bitIdx == 8)
        {
          byteIdx++;
          bitIdx = 0;
        }

        lon = lon + xstep;
      }
      lat = lat + ystep;
    }
    FILE *file = fopen(argv[4],"w");
    fwrite(&w,4,1,file);
    fwrite(&h,4,1,file);
    BitLine bitLine(data,bits);
    bitLine.writeToFile(file);
    fclose(file);
    delete data;


    return 0;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP,"Operation failed!",nullptr);
    exception.printError();
    return -1;
  }
}


