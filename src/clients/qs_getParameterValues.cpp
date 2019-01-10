#include "grid-content/contentServer/corba/client/ClientImplementation.h"
#include "grid-content/queryServer/corba/client/ClientImplementation.h"
#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"
#include "grid-files/identification/GridDef.h"

#include <stdlib.h>

using namespace SmartMet;


void init()
{
  try
  {
    char *configFile = getenv(SMARTMET_GRID_CONFIG_FILE);
    if (configFile == nullptr)
    {
      printf("%s not defined!\n",SMARTMET_GRID_CONFIG_FILE);
      exit(-1);
    }

    // Initializing the global structures. These are needed when
    // extracting information from GRIB files.

    Identification::gridDef.init(configFile);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,exception_operation_failed,nullptr);
  }
}




int main(int argc, char *argv[])
{
  try
  {
    char *serviceIor = getenv("SMARTMET_QS_IOR");
    if (serviceIor == NULL)
    {
      fprintf(stdout,"SMARTMET_QS_IOR not defined!\n");
      return -1;
    }


    if (argc < 7)
    {
      fprintf(stdout,"USAGE: qs_getParameterValues <sessionId> <parameter> <startTime> <endTime> <lat> <lon>\n");
      return -1;
    }

    T::SessionId sessionId = toInt64(argv[1]);
    QueryServer::Query query;
    QueryServer::QueryParameter param;


    uint geometryId = 0;
    if (argc >= 7)
      geometryId = atoi(argv[7]);

    T::Coordinate_vec coordinates;


    if (geometryId > 0)
    {
      query.mAttributeList.setAttribute("grid.geometryId",std::to_string(geometryId));
      /*

      init();
      uint cols = 0;
      uint rows = 0;
      if (Identification::gridDef.getGridDimensionsByGeometryId(geometryId,cols,rows))
      {
        query.mGridWidth = cols;
        query.mGridHeight = rows;
      }
      else
      {
        fprintf(stdout,"ERROR: Unknow geometry!\n");
        return -2;
      }

      if (!Identification::gridDef.getGridLatLonCoordinatesByGeometryId(geometryId,coordinates))
      {
        fprintf(stdout,"ERROR: Unknow geometry!\n");
        return -2;
      }
      */
      param.mType = QueryServer::Query::Type::Isoband;
      param.mLocationType = QueryServer::Query::LocationType::Point;

      param.mContourLowValues.push_back(270);
      param.mContourLowValues.push_back(275);
      param.mContourLowValues.push_back(280);
      param.mContourLowValues.push_back(285);
    }
    else
    {
      coordinates.push_back(T::Coordinate(atof(argv[6]),atof(argv[5])));
    }

    query.mAreaCoordinates.push_back(coordinates);
    query.mStartTime = argv[3];
    query.mEndTime = argv[4];
    query.mSearchType = QueryServer::Query::SearchType::TimeRange;

    param.mParam = argv[2];
    query.mQueryParameterList.push_back(param);


    QueryServer::Corba::ClientImplementation service;
    service.init(serviceIor);

    unsigned long long startTime = getTime();
    int result = service.executeQuery(sessionId,query);
    unsigned long long endTime = getTime();

    if (result != 0)
    {
      fprintf(stdout,"ERROR (%d) : %s\n",result,QueryServer::getResultString(result).c_str());
      return -3;
    }

    // ### Result:

    query.print(std::cout,0,0);

    for (auto it = query.mQueryParameterList.begin(); it != query.mQueryParameterList.end(); ++it)
    {
      for (auto v = it->mValueList.begin(); v != it->mValueList.end(); ++v)
      {
        printf("%s (%lu):",v->mForecastTime.c_str(),v->mWkbList.size());
        uint len = v->mValueList.getLength();
        for (uint t=0; t<len; t++)
        {
          T::GridValue *rec = v->mValueList.getGridValueByIndex(t);
          if (rec != NULL)
            printf(" %f",rec->mValue);

        }
        printf("\n");
      }
    }


    printf("\nTIME : %f sec\n\n",(float)(endTime-startTime)/1000000);

    return 0;
  }
  catch (SmartMet::Spine::Exception& e)
  {
    SmartMet::Spine::Exception exception(BCP,"Service call failed!",nullptr);
    exception.printError();
    return -4;
  }
}

