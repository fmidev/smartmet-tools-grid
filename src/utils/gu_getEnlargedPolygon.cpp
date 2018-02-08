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
      fprintf(stderr,"USAGE: gu_getEnlargedPolygon <xAddition> <yAddition> <x1,y1> <x2,y2> <x3,y3> [<x4,y4> ... <xn,yn>]\n");
      return -1;
    }


    double xAddition = atof(argv[1]);
    double yAddition = atof(argv[2]);
    std::vector<T::Coordinate> polygonPoints;

    char buf[100];
    for (int t=3; t<argc; t++)
    {
      strcpy(buf,argv[t]);
      char *p = strstr(buf,",");
      if (p != NULL)
      {
        *p = '\0';
        p++;
        double x = atof(buf);
        double y = atof(p);
        polygonPoints.push_back(T::Coordinate(x,y));
        printf("Coordinate %f,%f\n",x,y);
      }
    }

    unsigned long long startTime = getTime();
    std::vector<T::Coordinate> newCoordinates = getEnlargedPolygon(polygonPoints,xAddition,yAddition);
    unsigned long long endTime = getTime();



    for (auto it = newCoordinates.begin(); it != newCoordinates.end(); ++it)
    {
      printf("%f,%f\n",it->x(),it->y());
    }

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

