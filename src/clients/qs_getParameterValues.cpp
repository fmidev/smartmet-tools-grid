#include "grid-content/contentServer/corba/client/ClientImplementation.h"
#include "grid-content/queryServer/corba/client/ClientImplementation.h"
#include <macgyver/Exception.h>
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
    throw Fmi::Exception(BCP,"Operation failed!",nullptr);
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


    if (argc < 6)
    {
      fprintf(stdout,"USAGE: qs_getParameterValues <sessionId> <parameter> <startTime> <endTime> <csvFile> [<attrName=attrValue> ... <attrName=attrValue>]\n");
      return -1;
    }

    T::SessionId sessionId = toInt64(argv[1]);
    QueryServer::Query query;
    QueryServer::QueryParameter param;

    T::Coordinate_vec coordinates;
    //T::AttributeList attributeList;

    for (int t=6; t<argc; t++)
    {
      std::vector<std::string> partList;
      splitString(argv[t],'=',partList);
      if (partList.size() == 2)
        query.mAttributeList.addAttribute(partList[0],partList[1]);
    }

    param.mType = QueryServer::QueryParameter::Type::Vector;
    param.mLocationType = QueryServer::QueryParameter::LocationType::Geometry;

    query.mStartTime = utcTimeToTimeT(argv[3]);
    query.mEndTime = utcTimeToTimeT(argv[4]);
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

    const char *gridWidthStr = query.mAttributeList.getAttributeValue("grid.width");
    const char *gridHeightStr = query.mAttributeList.getAttributeValue("grid.height");
    uint columns = toInt32(gridWidthStr);
    uint rows = toInt32(gridHeightStr);
    auto sz = query.mQueryParameterList[0].mValueList[0]->mValueVector.size();

    if (sz == (columns*rows))
    {
      FILE *file = fopen(argv[5],"w");
      if (file)
      {
        uint c = 0;
        for (uint y=0; y<rows; y++)
        {
          for (uint x=0; x<columns; x++)
          {
            fprintf(file,"%f;",query.mQueryParameterList[0].mValueList[0]->mValueVector[c]);
            c++;
          }
          fprintf(file,"\n");
        }
        fclose(file);
      }
    }

    printf("\nTIME : %f sec\n\n",(float)(endTime-startTime)/1000000);

    return 0;
  }
  catch (Fmi::Exception& e)
  {
    Fmi::Exception exception(BCP,"Service call failed!",nullptr);
    exception.printError();
    return -4;
  }
}

