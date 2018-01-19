#include "grid-files/common/Exception.h"
#include "grid-files/identification/GridDef.h"
#include "grid-files/common/GeneralFunctions.h"


using namespace SmartMet;



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
    if (configFile == NULL)
    {
      printf("%s not defined!\n",SMARTMET_GRID_CONFIG_FILE);
      exit(-1);
    }

    // Initializing the global structures. These are needed when
    // extracting information from GRIB files.

    Identification::gridDef.init(configFile);

    double lat = atof(argv[1]);
    double lon = atof(argv[2]);
    std::set<T::GeometryId> geometryIdList;

    unsigned long long startTime = getTime();
    Identification::gridDef.getGeometryIdListByLatLon(lat,lon,geometryIdList);
    unsigned long long endTime = getTime();

    for (auto it = geometryIdList.begin(); it != geometryIdList.end(); ++it)
    {
      printf("GeometryId : %u\n",*it);
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

