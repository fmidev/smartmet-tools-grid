#include <macgyver/Exception.h>
#include "grid-files/common/ImageFunctions.h"

using namespace SmartMet;



int main(int argc, char *argv[])
{
  try
  {
    if (argc < 4)
    {
      fprintf(stdout,"USAGE:\n");
      fprintf(stdout,"  gu_mergePngFiles <newPngFile> <backColor> <pngFile1> <pngFile2> [<pngFile3> .. <pngFileN>]\n");
      return -1;
    }

    char *newPngFile = argv[1];

    CImage image;
    if (png_load(argv[3],image) != 0)
    {
      printf("Not found (%s)!\n",argv[3]);
      return -1;
    }

    uint backColor = strtoll(argv[2],nullptr,16);

    int sz = image.width * image.height;

    unsigned long col1 = 0;
    unsigned long col2 = 0;
    unsigned long nc = 0;
    uchar *c1 = (uchar*)&col1;
    uchar *c2 = (uchar*)&col2;
    uchar *c = (uchar*)&nc;

    for (int t = 4; t < argc; t++)
    {
      CImage img;
      if (png_load(argv[t],img) != 0)
      {
        printf("Load failed (%s)!\n",argv[t]);
        return -1;
      }

      if (img.width != image.width ||  img.height != image.height)
      {
        printf("Size do not match (%s  %d x %d != %d x %d)!\n",argv[t],img.width,img.height,image.width,image.height);
        return -1;
      }

      for (int a=0; a<sz; a++)
      {
        col1 = image.pixel[a];
        col2 = img.pixel[a];

        if ((col1 & 0xFF000000) < 0x0F000000)
          col1 = backColor;

        uint r = col2 & 0xFF000000;

        if (r == 0)
        {
          // Pixel is transparent
          image.pixel[a] = col1;
        }
        else
        if (r == 0xFF000000)
        {

          // Pixel has no tranparency
          image.pixel[a] = col2;
        }
        else
        {

          double m2 = (double)(col2 >> 24) / (double)0xFF;
          double m1 = 1.0 - m2;

          c[0] = m1*c1[0] + m2*c2[0];
          c[1] = m1*c1[1] + m2*c2[1];
          c[2] = m1*c1[2] + m2*c2[2];
          c[3] = 0xFF; //m1*c1[3] + m2*c2[3];

          image.pixel[a] = nc;
        }
        if ((image.pixel[a] & 0xFF000000) != 0xFF000000)
          image.pixel[a] = image.pixel[a] | 0xFF000000;
      }
    }

    // The function writes an image data to the PNG file.

    png_save(newPngFile,image.pixel,image.width,image.height);

    return 0;
  }
  catch (Fmi::Exception& e)
  {
    Fmi::Exception exception(BCP,"Service call failed!",nullptr);
    exception.printError();
    return -4;
  }
}

