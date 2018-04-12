#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"
#include "grid-files/common/ConfigurationFile.h"
#include "grid-files/common/CoordinateConversions.h"


using namespace SmartMet;




int main(int argc, char *argv[])
{
  try
  {
    double southPoleLat = -30;
    double southPoleLon = 0;


    for (double lat=-89; lat < 90; lat= lat+1)
    {
      for (double lon=-179; lon < 180; lon = lon+1)
      {
        double rotLat = lat;
        double rotLon = lon;
        double newLat = 0;
        double newLon = 0;

        //latlon_to_rotatedLatlon(lat,lon,southPoleLat,southPoleLon,rotLat,rotLon);
        rotatedLatlon_to_latlon(rotLat,rotLon,southPoleLat,southPoleLon,newLat,newLon);
        //if (round(lat) != round(newLat)  ||  round(lon) != round(newLon))
          printf("%f,%f => %f,%f => %f,%f\n",lat,lon,rotLat,rotLon,newLat,newLon);
      }

    }

    return 0;

    if (argc < 2)
    {
      fprintf(stderr,"USAGE: gu_showConfigurationAttributes <configurationFile> [-val]\n");
      return -1;
    }

    char *filename = argv[1];

    ConfigurationFile configFile(filename);

    if (argc == 3  &&  strcmp(argv[2],"-val") == 0)
      configFile.print(std::cout,0,1);
    else
      configFile.print(std::cout,0,0);

    return 0;
  }
  catch (SmartMet::Spine::Exception& e)
  {
    SmartMet::Spine::Exception exception(BCP,"Service call failed!",NULL);
    exception.printError();
    return -7;
  }
}

