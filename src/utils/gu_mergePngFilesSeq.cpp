#include <macgyver/Exception.h>
#include "grid-files/common/ImageFunctions.h"

using namespace SmartMet;



int main(int argc, char *argv[])
{
  try
  {
    if (argc < 5)
    {
      fprintf(stdout,"USAGE:\n");
      fprintf(stdout,"  gu_mergePngFilesSeq <numOfiles> <newPngFile> <pngFile1> <pngFile2> [<pngFile3> .. <pngFileN>]\n");
      return -1;
    }

    uint numOfFiles = atoi(argv[1]);
    char filename[200];

    for (uint a=0; a<numOfFiles; a++)
    {
      strcpy(filename,argv[3]);
      char *p = strstr(filename,"@");
      if (p != nullptr)
        sprintf(p,"%04u.png",a);

      CImage image;
      if (png_load(filename,image) != 0)
      {
        printf("Not found (%s)!\n",filename);
        return -1;
      }

      int sz = image.width * image.height;

      for (int t = 4; t < argc; t++)
      {
        strcpy(filename,argv[t]);
        char *p = strstr(filename,"@");
        if (p != nullptr)
          sprintf(p,"%04u.png",a);

        CImage img;
        if (png_load(filename,img) != 0  ||  img.width != image.width ||  img.height != image.height)
        {
          printf("Not found (%s,%u,%u,%u,%u)!\n",filename,img.width,image.width,img.height,image.height);
          return -1;
        }

        if (img.width != image.width ||  img.height != image.height)
        {
          printf("Size do not match (%s)!\n",argv[t]);
          return -1;
        }

        for (int n=0; n<sz; n++)
        {
          unsigned long col = img.pixel[n];
          if ((col & 0xFF000000) == 0)
            image.pixel[n] = col;
        }
      }

      // The function writes an image data to the PNG file.

      strcpy(filename,argv[2]);
      p = strstr(filename,"@");
      if (p != nullptr)
        sprintf(p,"%04u.png",a);

      png_save(filename,image.pixel,image.width,image.height);
    }

    return 0;
  }
  catch (Fmi::Exception& e)
  {
    Fmi::Exception exception(BCP,"Service call failed!",nullptr);
    exception.printError();
    return -4;
  }
}

