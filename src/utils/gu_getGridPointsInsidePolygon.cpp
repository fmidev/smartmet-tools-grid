/*! \file
 *  \brief Utility program that lists all grid point coordinates falling inside a polygon defined by command-line vertices.
 */

#include <macgyver/Exception.h>
#include "grid-files/identification/GridDef.h"
#include "grid-files/common/GeneralFunctions.h"
#include "grid-files/common/GraphFunctions.h"


using namespace SmartMet;



/*! \brief Program entry point. */

int main(int argc, char *argv[])
{
  try
  {
    if (argc < 6)
    {
      fprintf(stderr,"USAGE: gu_getGridPointsInsidePolygon <gridWidth> <gridHeight> <x1,y1> <x2,y2> <x3,y3> [<x4,y4> ... <xn,yn>]\n");
      return -1;
    }


    int width = toInt64(argv[1]);
    int height = toInt64(argv[2]);
    std::vector<T::Coordinate> polygonPoints;
    std::vector<T::Point> gridPoints;

    for (int t=3; t<argc; t++)
    {
      std::string bufStr(argv[t]);  char *buf = bufStr.data();
      char *p = strstr(buf,",");
      if (p != nullptr)
      {
        *p = '\0';
        p++;
        double x = toDouble(buf);
        double y = toDouble(p);
        polygonPoints.push_back(T::Coordinate(x,y));
        printf("Coordinate %f,%f\n",x,y);
      }
    }

    UInt64 startTime = getTime();
    getPointsInsidePolygon(width,height,polygonPoints,gridPoints);
    UInt64 endTime = getTime();

    for (auto it = gridPoints.begin(); it != gridPoints.end(); ++it)
    {
      printf("%d,%d\n",it->x(),it->y());
    }

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

