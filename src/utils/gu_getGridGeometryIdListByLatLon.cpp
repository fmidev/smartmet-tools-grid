/*! \file
 *  \brief Utility program that lists all grid geometry IDs whose bounding area contains a given lat/lon coordinate.
 */

#include <macgyver/Exception.h>
#include "grid-files/identification/GridDef.h"
#include "grid-files/common/GeneralFunctions.h"


using namespace SmartMet;



/*! \brief Program entry point. */

int main(int argc, char *argv[])
{
  try
  {
    if (argc != 3)
    {
      fprintf(stderr,"USAGE: gu_getGridGeometryIdListByLatLon <latitude> <longitude>\n");
      return -1;
    }

    char *configFile = getenv(SMARTMET_GRID_CONFIG_FILE);
    if (configFile == nullptr)
    {
      printf("%s not defined!\n",SMARTMET_GRID_CONFIG_FILE);
      exit(-1);
    }

    // Initializing the global structures. These are needed when
    // extracting information from GRIB files.

    Identification::gridDef.init(configFile);

    double lat = toDouble(argv[1]);
    double lon = toDouble(argv[2]);
    std::set<T::GeometryId> geometryIdList;

    UInt64 startTime = getTime();
    Identification::gridDef.getGeometryIdListByLatLon(lat,lon,geometryIdList);
    UInt64 endTime = getTime();

    for (auto it = geometryIdList.begin(); it != geometryIdList.end(); ++it)
    {
      printf("GeometryId : %u\n",*it);
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

