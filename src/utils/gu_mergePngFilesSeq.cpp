/*! \file
 *  \brief Utility program that merges sequences of numbered PNG files into a corresponding sequence of composited output PNGs.
 */

#include <macgyver/Exception.h>
#include "grid-files/common/GeneralFunctions.h"
#include "grid-files/common/ImageFunctions.h"
#include <string>

using namespace SmartMet;



/*! \brief Returns the file name for the sequence number. The part of the pattern starting
    from '@' is replaced by the four digit sequence number followed by ".png". */

std::string sequenceFileName(const char *pattern,uint a)
{
  std::string filename(pattern);
  auto pos = filename.find('@');
  if (pos != std::string::npos)
  {
    char tmp[30];
    snprintf(tmp,sizeof(tmp),"%04u.png",a);
    filename = filename.substr(0,pos) + tmp;
  }
  return filename;
}





/*! \brief Draws the non-transparent pixels of the numbered PNG file over the image. Returns
    false if the file cannot be loaded or if its size does not match. */

bool overlayImage(CImage& image,const char *pattern,uint a)
{
  std::string filename = sequenceFileName(pattern,a);

  int sz = image.width * image.height;

  CImage img;
  if (png_load(filename.c_str(),img) != 0  ||  img.width != image.width ||  img.height != image.height)
  {
    printf("Not found (%s,%d,%d,%d,%d)!\n",filename.c_str(),img.width,image.width,img.height,image.height);
    return false;
  }

  if (img.width != image.width ||  img.height != image.height)
  {
    printf("Size do not match (%s)!\n",pattern);
    return false;
  }

  for (int n=0; n<sz; n++)
  {
    unsigned long col = img.pixel[n];
    if ((col & 0xFF000000) == 0)
      image.pixel[n] = col;
  }
  return true;
}





/*! \brief Program entry point. */

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

    uint numOfFiles = toInt32(argv[1]);

    for (uint a=0; a<numOfFiles; a++)
    {
      std::string filename = sequenceFileName(argv[3],a);

      CImage image;
      if (png_load(filename.c_str(),image) != 0)
      {
        printf("Not found (%s)!\n",filename.c_str());
        return -1;
      }

      for (int t = 4; t < argc; t++)
      {
        if (!overlayImage(image,argv[t],a))
          return -1;
      }

      // The function writes an image data to the PNG file.

      filename = sequenceFileName(argv[2],a);
      png_save(filename.c_str(),image.pixel,image.width,image.height);
    }

    return 0;
  }
  catch (Fmi::Exception&)
  {
    Fmi::Exception exception(BCP,"Service call failed!",nullptr);
    exception.printError();
    return -4;
  }
}
