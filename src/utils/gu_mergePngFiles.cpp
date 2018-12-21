#include "grid-files/common/Exception.h"
#include "grid-files/common/ImageFunctions.h"

using namespace SmartMet;



int main(int argc, char *argv[])
{
  try
  {
    if (argc < 4)
    {
      fprintf(stdout,"USAGE:\n");
      fprintf(stdout,"  gu_mergePngFiles <newPngFile> <pngFile1> <pngFile2> [<pngFile3> .. <pngFileN>]\n");
      return -1;
    }

    char *newPngFile = argv[1];

    CImage image;
    if (png_load(argv[2],image) != 0)
    {
      printf("Not found (%s)!\n",argv[2]);
      return -1;
    }

    int sz = image.width * image.height;

    for (int t = 3; t < argc; t++)
    {
      CImage img;
      if (png_load(argv[t],img) != 0  ||  img.width != image.width ||  img.height != image.height)
      {
        printf("Not found (%s)!\n",argv[t]);
        return -1;
      }

      if (img.width != image.width ||  img.height != image.height)
      {
        printf("Size do not match (%s)!\n",argv[t]);
        return -1;
      }

      for (int a=0; a<sz; a++)
      {
        unsigned long col = img.pixel[a];
        if ((col & 0xFF000000) == 0)
          image.pixel[a] = col;
      }
    }

    // The function writes an image data to the PNG file.

    png_save(newPngFile,image.pixel,image.width,image.height);

    return 0;
  }
  catch (SmartMet::Spine::Exception& e)
  {
    SmartMet::Spine::Exception exception(BCP,"Service call failed!",nullptr);
    exception.printError();
    return -4;
  }
}

