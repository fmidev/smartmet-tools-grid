#include "grid-files/common/Exception.h"
#include "grid-files/identification/GribDef.h"
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

    char *configDir = getenv("SMARTMET_GRID_CONFIG_DIR");
    if (configDir == NULL)
    {
      fprintf(stderr,"SMARTMET_GRID_CONFIG_DIR not defined!\n");
      return -2;
    }

    Identification::gribDef.init(configDir);

    uint geometryId = atoll(argv[1]);

    unsigned long long startTime = getTime();
    GRIB2::GridDefinition_ptr def = Identification::gribDef.getGridDefinition2ByGeometryId(geometryId);
    unsigned long long endTime = getTime();

    if (def == NULL)
    {
      printf("Geometry (%u) not found!\n",geometryId);
      return -3;
    }

    //T::Dimensions_opt d = def->getGridDimensions();
    T::Coordinate_vec coordinates = def->getGridLatLonCoordinates();

    for (auto it = coordinates.begin(); it != coordinates.end(); ++it)
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

