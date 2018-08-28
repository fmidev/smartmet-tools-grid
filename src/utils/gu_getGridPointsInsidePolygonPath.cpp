#include "grid-files/common/Exception.h"
#include "grid-files/identification/GridDef.h"
#include "grid-files/common/GeneralFunctions.h"
#include "grid-files/common/GraphFunctions.h"


using namespace SmartMet;



int main(int argc, char *argv[])
{
  try
  {
    if (argc < 6)
    {
      fprintf(stderr,"USAGE: gu_getGridPointsInsideMultiPolygon <gridWidth> <gridHeight> <x1,y1> <x2,y2> <x3,y3> [ / <x4,y4> ... <xn,yn>]\n");
      return -1;
    }


    int width = atoll(argv[1]);
    int height = atoll(argv[2]);
    std::vector<std::vector<T::Coordinate>> polygonPath;
    std::vector<T::Coordinate> polygonPoints;
    std::vector<T::Point> gridPoints;

    char buf[100];
    for (int t=3; t<argc; t++)
    {
      if (strcmp(argv[t],"/") == 0  &&  polygonPoints.size() > 0)
      {
        polygonPath.push_back(polygonPoints);
        polygonPoints.clear();
        printf("\n");
      }

      strcpy(buf,argv[t]);
      char *p = strstr(buf,",");
      if (p != nullptr)
      {
        *p = '\0';
        p++;
        double x = atof(buf);
        double y = atof(p);
        polygonPoints.push_back(T::Coordinate(x,y));
        printf("Coordinate %f,%f\n",x,y);
      }
    }

    if (polygonPoints.size() > 0)
      polygonPath.push_back(polygonPoints);


    unsigned long long startTime = getTime();
    getPointsInsidePolygonPath(width,height,polygonPath,gridPoints);
    unsigned long long endTime = getTime();

    for (auto it = gridPoints.begin(); it != gridPoints.end(); ++it)
    {
      printf("%d,%d\n",it->x(),it->y());
    }

    printf("\nTIME : %f sec\n\n",(float)(endTime-startTime)/1000000);
    return 0;
  }
  catch (SmartMet::Spine::Exception& e)
  {
    SmartMet::Spine::Exception exception(BCP,"Service call failed!",nullptr);
    exception.printError();
    return -7;
  }
}

