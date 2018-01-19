#include "grid-files/common/Exception.h"
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


    int width = atoll(argv[1]);
    int height = atoll(argv[2]);
    double origoX = atof(argv[3]);
    double origoY = atof(argv[4]);
    double radius = atof(argv[5]);
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
  catch (SmartMet::Spine::Exception& e)
  {
    SmartMet::Spine::Exception exception(BCP,"Service call failed!",NULL);
    exception.printError();
    return -7;
  }
}

