#include "grid-files/common/Exception.h"
#include "grid-files/identification/GribDef.h"
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

    char *configDir = getenv("SMARTMET_GRID_CONFIG_DIR");
    if (configDir == NULL)
    {
      fprintf(stderr,"SMARTMET_GRID_CONFIG_DIR not defined!\n");
      return -2;
    }

    Identification::gribDef.init(configDir);

    double lat = atof(argv[1]);
    double lon = atof(argv[2]);
    std::set<uint> geometryIdList;

    unsigned long long startTime = getTime();
    Identification::gribDef.getGeometryIdListByLatLon(lat,lon,geometryIdList);
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

