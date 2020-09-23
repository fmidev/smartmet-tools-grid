#include <macgyver/Exception.h>
#include "grid-files/common/CoordinateConversions.h"
#include "grid-files/common/GeneralFunctions.h"


using namespace SmartMet;



int main(int argc, char *argv[])
{
  try
  {
    if (argc != 5)
    {
      fprintf(stderr,"USAGE: gu_getLatLonDistance <lat1> <lon1> <lat2> <lon2>\n");
      return -1;
    }

    double lat1 = toDouble(argv[1]);
    double lon1 = toDouble(argv[2]);
    double lat2 = toDouble(argv[3]);
    double lon2 = toDouble(argv[4]);

    double dist = latlon_distance(lat1,lon1,lat2,lon2);
    printf("\nDistance : %f\n",dist);

    return 0;
  }
  catch (Fmi::Exception& e)
  {
    Fmi::Exception exception(BCP,"Service call failed!",nullptr);
    exception.printError();
    return -7;
  }
}

