#include <macgyver/Exception.h>
#include "grid-files/identification/GridDef.h"
#include "grid-files/common/GeneralFunctions.h"
#include "grid-files/common/GraphFunctions.h"


using namespace SmartMet;



int main(int argc, char *argv[])
{
  try
  {
    if (argc != 6)
    {
      fprintf(stderr,"USAGE: gu_getGridPointsInsideCircle <gridWidth> <gridHeight> <origo-x> <origo-y> <radius>\n");
      return -1;
    }


    int width = toInt64(argv[1]);
    int height = toInt64(argv[2]);
    double origoX = toDouble(argv[3]);
    double origoY = toDouble(argv[4]);
    double radius = toDouble(argv[5]);
    std::vector<T::Point> gridPoints;


    unsigned long long startTime = getTime();
    getPointsInsideCircle(width,height,origoX,origoY,radius,gridPoints);
    unsigned long long endTime = getTime();

    for (auto it = gridPoints.begin(); it != gridPoints.end(); ++it)
    {
      printf("%d,%d\n",it->x(),it->y());
    }

/*
    for (int y=0; y<height;y++)
    {
      for (int x=0; x<width;x++)
      {
        bool found = false;
        for (auto it = gridPoints.begin(); it != gridPoints.end()  &&  !found; ++it)
        {
          if (it->x() == x  &&  it->y() == y)
            found = true;
        }

        if (found)
          printf("X");
        else
          printf("-");
      }
      printf("\n");
    }
*/
    printf("\nTIME : %f sec\n\n",(float)(endTime-startTime)/1000000);
    return 0;
  }
  catch (Fmi::Exception& e)
  {
    Fmi::Exception exception(BCP,"Service call failed!",nullptr);
    exception.printError();
    return -7;
  }
}

