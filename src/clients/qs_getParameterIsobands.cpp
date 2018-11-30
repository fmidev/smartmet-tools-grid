#include "grid-content/contentServer/corba/client/ClientImplementation.h"
#include "grid-content/queryServer/corba/client/ClientImplementation.h"
#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"
#include "grid-files/common/ImageFunctions.h"
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


std::vector<QueryServer::Query> queryList;
QueryServer::Corba::ClientImplementation service;
uint threadEndCount = 0;
ThreadLock threadLock;
T::SessionId sessionId = 0;



static void* processThread(void *arg)
{
  try
  {
    uint sz = queryList.size();
    uint readyCount = 0;
    while (readyCount < sz)
    {
      readyCount = 0;
      uint idx = sz;
      {
        AutoThreadLock lock(&threadLock);
        for (uint t=0; t<sz; t++)
        {
          if (queryList[t].mFlags & 0x80000000)
          {
            readyCount++;
          }
          else
          {
            idx = t;
            queryList[t].mFlags |= 0x80000000;
            t = sz;
          }
        }
      }

      if (idx < sz)
      {
        int result = service.executeQuery(sessionId,queryList[idx]);

        if (result != 0)
        {
          fprintf(stdout,"ERROR (%d) : %s\n",result,QueryServer::getResultString(result).c_str());
          {
            AutoThreadLock lock(&threadLock);
            threadEndCount++;
          }
          return nullptr;
        }
      }
    }

    {
      AutoThreadLock lock(&threadLock);
      threadEndCount++;
    }
    return nullptr;
  }
  catch (...)
  {
    SmartMet::Spine::Exception exception(BCP,exception_operation_failed,nullptr);
    exception.printError();
    exit(-1);
  }
}





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


    if (argc < 11)
    {
      fprintf(stdout,"USAGE: qs_getParameterIsobands <sessionId> <parameter> <gridGeometryId> <multiplyer> <rotate> <startTime> <timeStepSizeInMinutes> <timesteps> <filePrefix> <contourVal1> [..<contourValN>]\n");
      return -1;
    }

    init();

    sessionId = toInt64(argv[1]);
    QueryServer::Query query;
    QueryServer::QueryParameter param;

    query.mLocationType = QueryServer::Query::LocationType::Point;

    uint geometryId = atoi(argv[3]);
    double mp = atof(argv[4]);
    bool rotate = (bool)atoi(argv[5]);

    uint width = 0;
    uint height = 0;

    uint cols = 0;
    uint rows = 0;
    if (Identification::gridDef.getGridDimensionsByGeometryId(geometryId,cols,rows))
    {
      width = cols;
      height = rows;
    }
    else
    {
      fprintf(stdout,"ERROR: Unknow geometry!\n");
      return -2;
    }

    service.init(serviceIor);

    query.mAttributeList.setAttribute("grid.geometryId",std::to_string(geometryId));
    query.mStartTime = argv[6];

    uint timestep = atoi(argv[7]);
    uint timesteps = atoi(argv[8]);
    char *filePrefix = argv[9];

    param.mParam = argv[2];

    query.mType = QueryServer::Query::Type::Isoband;
    for (int t=10; t<argc-1; t++)
    {
      param.mContourLowValues.push_back(atof(argv[t]));
      param.mContourHighValues.push_back(atof(argv[t+1]));
    }

    query.mQueryParameterList.push_back(param);


    auto s = boost::posix_time::from_iso_string(query.mStartTime);
    for (uint t=0; t<timesteps; t++)
    {
      QueryServer::Query newQuery(query);

      std::string str = Fmi::to_iso_string(s);
      newQuery.mForecastTimeList.insert(str);

      s = s + boost::posix_time::seconds(timestep*60);
      queryList.push_back(newQuery);
    }

    unsigned long long startTime = getTime();

    pthread_t mThread[10];

    for (uint t=0; t<5; t++)
    {
      pthread_create(&mThread[t],nullptr,processThread,nullptr);
      time_usleep(0,100);
    }

    while (threadEndCount < 5)
    {
      //printf("ThreadEndCount %u\n",threadEndCount);
      time_usleep(0,100);
    }

    unsigned long long endTime = getTime();


    for (uint t=0; t<timesteps; t++)
    {
      for (auto it = queryList[t].mQueryParameterList.begin(); it != queryList[t].mQueryParameterList.end(); ++it)
      {
        for (auto v = it->mValueList.begin(); v != it->mValueList.end(); ++v)
        {
          //printf("%s (%lu):",v->mForecastTime.c_str(),v->mWkbList.size());

          int imageWidth = width*mp;
          int imageHeight = height*mp;

          int sz = imageWidth * imageHeight;
          unsigned long *image = new unsigned long[sz];
          for (int t=0; t<sz; t++)
            image[t] = 0xFFFFFF;

          uint c = 250;
          uint step = 250 / v->mWkbList.size();

          for (auto it = v->mWkbList.begin(); it != v->mWkbList.end(); ++it)
          {
            uint col = (c << 16) + (c << 8) + c;
            paintWkb(image,imageWidth,imageHeight,false,rotate,mp,0,0,*it,col);
            c = c - step;
          }

          char filename[200];
          sprintf(filename,"%s_%04u.jpg",filePrefix,t);
          printf("Saving image : %s\n",filename);
          jpeg_save(filename,image,imageHeight,imageWidth,100);
          delete[] image;
        }
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

