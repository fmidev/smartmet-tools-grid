#include "grid-content/contentServer/corba/client/ClientImplementation.h"
#include "grid-content/queryServer/corba/client/ClientImplementation.h"
#include <macgyver/Exception.h>
#include "grid-files/common/GeneralFunctions.h"
#include "grid-files/common/ImageFunctions.h"
#include "grid-files/common/ImagePaint.h"
#include "grid-files/common/GraphFunctions.h"
#include "grid-files/identification/GridDef.h"

#include <macgyver/Astronomy.h>
#include <macgyver/CharsetTools.h>
#include <macgyver/StringConversion.h>
#include <macgyver/TimeFormatter.h>
#include <macgyver/TimeParser.h>
#include <macgyver/TimeZoneFactory.h>

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


    if (argc < 12)
    {
      fprintf(stdout,"USAGE: qs_getParameterIsobands <sessionId> <parameter> <attributeList> <areaInterpolation> <multiplyer> <rotate> <startTime> <timeStepSizeInMinutes> <timesteps> <filePrefix> <contourVal1> [..<contourValN>]\n");
      return -1;
    }

    init();

    QueryServer::Corba::ClientImplementation service;
    T::SessionId sessionId = toInt64(argv[1]);
    QueryServer::Query query;
    QueryServer::QueryParameter param;


    char *attributes = argv[3];
    uint areaInterpolation = toInt64(argv[4]);
    double mp = toDouble(argv[5]);
    bool rotate = (bool)atoi(argv[6]);

    service.init(serviceIor);

    query.mAttributeList.addAttribute("contour.coordinateType",std::to_string(T::CoordinateTypeValue::GRID_COORDINATES));
    query.mAttributeList.addAttribute("grid.areaInterpolationMethod",std::to_string(areaInterpolation));
    query.mAttributeList.addAttribute("grid.timeInterpolationMethod",std::to_string(areaInterpolation));

    std::vector<std::string> attrList;
    splitString(attributes,'|',attrList);
    for (auto it=attrList.begin(); it != attrList.end(); ++it)
    {
      std::vector<std::string> partList;
      splitString(*it,'=',partList);
      if (partList.size() == 2)
        query.mAttributeList.setAttribute(partList[0],partList[1]);
    }


    query.mStartTime = utcTimeToTimeT(argv[7]);

    uint timestep = atoi(argv[8]);
    uint timesteps = atoi(argv[9]);
    char *filePrefix = argv[10];

    param.mParam = argv[2];

    std::vector<uint> colorList;

    param.mType = QueryServer::QueryParameter::Type::Isoband;
    param.mLocationType = QueryServer::QueryParameter::LocationType::Geometry;

    for (int t=11; t<argc-1; t++)
    {
      std::vector<std::string> partList1;
      splitString(argv[t],':',partList1);
      param.mContourLowValues.push_back(toDouble(partList1[0].c_str()));
      if (partList1.size() == 2)
        colorList.push_back(strtoll(partList1[1].c_str(),nullptr,16));
      else
        colorList.push_back(0xFFFFFFFF);

      std::vector<std::string> partList2;
      splitString(argv[t+1],':',partList2);
      param.mContourHighValues.push_back(toDouble(partList2[0].c_str()));
    }

    query.mQueryParameterList.push_back(param);

    unsigned long long startTime = getTime();

    time_t s = query.mStartTime;
    for (uint t=0; t<timesteps; t++)
    {
      QueryServer::Query newQuery(query);

      newQuery.mForecastTimeList.insert(s);

      s = s + (timestep*60);

      int result = service.executeQuery(sessionId,newQuery);

      if (result == 0)
      {
        if (newQuery.mAttributeList.getAttributeValue("grid.width") != nullptr && newQuery.mAttributeList.getAttributeValue("grid.height") != nullptr)
        {
          int width = atoi(newQuery.mAttributeList.getAttributeValue("grid.width"));
          int height = atoi(newQuery.mAttributeList.getAttributeValue("grid.height"));

          for (auto it = newQuery.mQueryParameterList.begin(); it != newQuery.mQueryParameterList.end(); ++it)
          {
            for (auto v = it->mValueList.begin(); v != it->mValueList.end(); ++v)
            {
              //printf("%s (%lu):",v->mForecastTime.c_str(),v->mValueData.size());

              int imageWidth = width*mp;
              int imageHeight = height*mp;

              ImagePaint imagePaint(imageWidth,imageHeight,0xFFFFFFFF,0x00000,0xA0A0A0,false,rotate);

              if ((*v)->mValueData.size() > 0)
              {
                uint c = 250;
                uint step = 250 / (*v)->mValueData.size();

                uint a = 0;
                for (auto it = (*v)->mValueData.begin(); it != (*v)->mValueData.end(); ++it)
                {
                  uint col = colorList[a];
                  if (col == 0xFFFFFFFF)
                    col = (c << 16) + (c << 8) + c;

                  imagePaint.setFillColor(col);
                  imagePaint.paintWkb(mp,mp,0,0,*it);
                  c = c - step;
                  a++;
                }
              }

              char filename[200];
              sprintf(filename,"%s_%04u.png",filePrefix,t);
              printf("Saving image : %s\n",filename);
              imagePaint.savePngImage(filename);
            }
          }
        }
      }
    }

    unsigned long long endTime = getTime();
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

