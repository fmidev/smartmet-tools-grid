#include "grid-files/common/Exception.h"
#include "grid-files/identification/GridDef.h"
#include "grid-files/common/GeneralFunctions.h"


using namespace SmartMet;



int main(int argc, char *argv[])
{
  try
  {
    if (argc != 2)
    {
      fprintf(stderr,"USAGE: gu_getGridLatLonCoordinatesByGeometryId <geometryId>\n");
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

    uint geometryId = atoll(argv[1]);
    T::Coordinate_vec coordinates;

    unsigned long long startTime = getTime();

    if (!Identification::gridDef.getGridLatLonCoordinatesByGeometryId(geometryId,coordinates))
    {
      printf("Geometry (%u) not found!\n",geometryId);
      return -3;
    }

    unsigned long long endTime = getTime();

    for (auto it = coordinates.begin(); it != coordinates.end(); ++it)
    {
      printf("%f,%f\n",it->x(),it->y());
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

