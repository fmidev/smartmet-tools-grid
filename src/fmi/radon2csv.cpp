#include "grid-files/common/Exception.h"
#include "grid-files/common/ShowFunction.h"
#include "grid-files/common/GeneralFunctions.h"
#include "grid-files/common/ShowFunction.h"
#include "grid-files/identification/GridDef.h"

#include <libpq-fe.h>
#include <stdlib.h>
#include <string.h>


#define FUNCTION_TRACE FUNCTION_TRACE_OFF



using namespace SmartMet;


FILE *producerFile = nullptr;
FILE *generationFile = nullptr;
FILE *fileFile = nullptr;
FILE *contentFile = nullptr;

uint globalGenerationId = 1;
uint globalFileId = 1;
uint sourceId = 200;
std::vector<std::string> producerList;




void error(char *mess)
{
  fprintf(stderr, "### %s\n", mess);
  exit(1);
}



void readProducerList(const char *filename)
{
  try
  {
    FILE *file = fopen(filename,"re");
    if (file == nullptr)
    {
      SmartMet::Spine::Exception exception(BCP, "Cannot open the producer list file!");
      exception.addParameter("Filename",filename);
      throw exception;
    }

    char st[1000];
    while (!feof(file))
    {
      if (fgets(st,1000,file) != nullptr)
      {
        if (st[0] != '#')
        {
          char *p = st;
          while (*p != '\0')
          {
            if (*p <= ' ')
              *p = '\0';
            else
              p++;
          }
          //printf("Add producer [%s]\n",st);
          producerList.push_back(std::string(st));
        }
      }
    }
    fclose(file);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, exception_operation_failed, nullptr);
  }
}





bool producerEnabled(const char *producerName)
{
  try
  {
    for (auto it = producerList.begin(); it != producerList.end(); ++it)
    {
      if (strcasecmp(it->c_str(),producerName) == 0)
        return true;
    }
    return false;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, exception_operation_failed, nullptr);
  }
}





void readContent(PGconn *conn,char *producerId,uint generationId,uint fileId,uint fileType,char *geometryId,char *forecastTime,char *fmiParameterId,char *fmiLevelId,char *parameterLevel,char *forecastType,char *pertubationNumber)
{
  FUNCTION_TRACE
  try
  {

    std::string fmiParameterName;
    std::string fmiParameterUnits;
    std::string newbaseParameterId;
    std::string newbaseParameterName;
    std::string gribParameterId;
    std::string gribParameterUnits;

    std::string grib1ParameterLevelId;
    std::string grib2ParameterLevelId;


    Identification::FmiParameterDef fmiDef;
    if (Identification::gridDef.getFmiParameterDefById(fmiParameterId,fmiDef))
    {
      fmiParameterName = fmiDef.mParameterName;
      fmiParameterUnits = fmiDef.mParameterUnits;

      Identification::NewbaseParameterDef newbaseDef;
      if (Identification::gridDef.getNewbaseParameterDefByFmiId(fmiParameterId,newbaseDef))
      {
        newbaseParameterId = newbaseDef.mNewbaseParameterId;
        newbaseParameterName = newbaseDef.mParameterName;
      }
/*
      Identification::FmiParameterId_grib1 g1Def;
      if (Identification::gridDef.getGrib1ParameterDefByFmiId(fmiParameterId,g1Def))
        grib1ParameterLevelId = toString(g1Def.mGribParameterLevelId);

      Identification::FmiParameterId_grib2 g2Def;
      if (Identification::gridDef.getGrib2ParameterDef(fmiParameterId,g2Def))
        grib2ParameterLevelId = toString(g2Def.mGribParameterLevelId);
        */
    }


    fprintf(contentFile,"%u;%u;%u;%s;%u;%u;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%u;%u;%u;%s;\n",
           fileId,
           0, // messageIndex
           fileType,
           producerId,
           generationId,
           0, // groupFlags
           forecastTime,
           fmiParameterId,
           fmiParameterName.c_str(),
           gribParameterId.c_str(),
           "", // cdmParameterId
           "", // cdmParameterName
           newbaseParameterId.c_str(),
           newbaseParameterName.c_str(),
           fmiLevelId,
           grib1ParameterLevelId.c_str(),
           grib2ParameterLevelId.c_str(),
           parameterLevel,
           fmiParameterUnits.c_str(),
           gribParameterId.c_str(),
           forecastType,
           pertubationNumber,
           0, // serverFlags,
           0, // flags,
           sourceId,
           geometryId
        );
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, exception_operation_failed, nullptr);
  }
}





uint readFiles(PGconn *conn,char *producerId,uint generationId,char *schemaName, char *partitionName,char *analysisTime)
{
  FUNCTION_TRACE
  try
  {
    char sql[1000];
    sprintf(sql,"SELECT file_location,param_id,level_id,level_value::int,to_char((analysis_time+forecast_period) at time zone 'utc','yyyymmddThh24MISS'),forecast_type_id,forecast_type_value::int,geometry_id FROM %s.%s WHERE to_char(analysis_time, 'yyyymmddThh24MISS') = '%s' AND producer_id = %s",schemaName,partitionName,analysisTime,producerId);

    PGresult *res = PQexec(conn,sql);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
      error(PQresultErrorMessage(res));

    uint fileCount = 0;
    //int fieldCount = PQnfields(res);
    int rowCount = PQntuples(res);

    for(int i = 0; i < rowCount; i++)
    {
      if (strstr(PQgetvalue(res, i, 0),"masala") != nullptr)
      {
        fprintf(fileFile,"%u;%u;%s;%s;%u;%u;%u;%u\n",
               globalFileId,
               0,   // Type
               PQgetvalue(res, i, 0),  // fileName
               producerId,
               generationId,
               0,   // GroupFlas
               1,   // Flags (1 = content predefined)
               sourceId
              );

        readContent(conn,producerId,generationId,globalFileId,0,PQgetvalue(res, i, 7),PQgetvalue(res, i, 4),PQgetvalue(res, i, 1),PQgetvalue(res, i, 2),PQgetvalue(res, i, 3),PQgetvalue(res, i, 5),PQgetvalue(res, i, 6));

        globalFileId++;
        fileCount++;
      }
    }
    PQclear(res);
    return fileCount;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, exception_operation_failed, nullptr);
  }
}





uint readGenerations(PGconn *conn,char *producerId,char *producerName)
{
  FUNCTION_TRACE
  try
  {
    char sql[1000];
    sprintf(sql,"SELECT producer_id, to_char(analysis_time, 'yyyymmddThh24MISS'),to_char(analysis_time at time zone 'utc', 'yyyymmddThh24MISS'),schema_name, partition_name FROM as_grid_v WHERE producer_id = %s ORDER BY analysis_time desc",producerId);
    PGresult *res = PQexec(conn, sql);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
      error(PQresultErrorMessage(res));

    //int fieldCount = PQnfields(res);
    int rowCount = PQntuples(res);

    char prev[1000];
    prev[0] = '\0';
    uint generationCount = 0;

    for (int i = 0; i < rowCount; i++)
    {
      char st[1000];
      sprintf(st,"%u;%u;%s;%s:%s;Producer %s generation %s;%s;%u;%u;%u;",
              globalGenerationId,
              0,  // type
              producerId,
              producerName,PQgetvalue(res, i, 2),  // Name,
              producerName,PQgetvalue(res, i, 2),  // Description,
              PQgetvalue(res, i, 2),
              1,  // status,
              0,  // flags
              sourceId
             );

      if (/*toInt64(producerId) == 1  &&*/  strcmp(PQgetvalue(res, i, 1),prev) != 0)
      {
        uint fCount = readFiles(conn,producerId,globalGenerationId,PQgetvalue(res, i, 3), PQgetvalue(res, i, 4),PQgetvalue(res, i, 1));
        if (fCount > 0)
        {
          fprintf(generationFile,"%s\n",st);
          strcpy(prev,PQgetvalue(res, i, 1));
          globalGenerationId++;
          generationCount++;
          if (generationCount == 1)
          {
            PQclear(res);
            return generationCount;
          }
        }
      }
    }
    PQclear(res);

    return generationCount;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, exception_operation_failed, nullptr);
  }
}





void readProducers(PGconn *conn)
{
  FUNCTION_TRACE
  try
  {
    PGresult *res = PQexec(conn, "SELECT id,name,description FROM fmi_producer ORDER BY id");
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
      error(PQresultErrorMessage(res));


    //int fieldCount = PQnfields(res);
    int rowCount = PQntuples(res);

    for (int i = 0; i < rowCount; i++)
    {
      //printf("Producer :%s\n",PQgetvalue(res, i, 1));
      if (producerEnabled(PQgetvalue(res, i, 1)))
      {
        printf("Producer : %s\n",PQgetvalue(res, i, 1));
        //printf("-- read generations\n");
        if (readGenerations(conn,PQgetvalue(res, i, 0),PQgetvalue(res, i, 1)) > 0)
        {
          fprintf(producerFile,"%s;%s;%s;%s;%u;%u\n",
                 PQgetvalue(res, i, 0), // producerId
                 PQgetvalue(res, i, 1), // name
                 PQgetvalue(res, i, 1), // title
                 PQgetvalue(res, i, 2), // description
                 0,   // flags
                 sourceId
                );
        }
      }
    }
    PQclear(res);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, exception_operation_failed, nullptr);
  }
}





int main(int argc, char *argv[])
{
  FUNCTION_TRACE
  try
  {
    if (argc != 5)
    {
      fprintf(stderr,"USAGE: radon2csv <outputDir> <sourceId> <producerListFile> <dbConnectionString>\n");
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

    char *dir = argv[1];
    sourceId = toInt64(argv[2]);
    char *producerListFile = argv[3];
    char *connectionString = argv[4];

    PGconn *conn = PQconnectdb(connectionString);
    if (PQstatus(conn) != CONNECTION_OK)
      error(PQerrorMessage(conn));

    char filename[200];

    sprintf(filename,"%s/producers.csv",dir);
    producerFile = fopen(filename,"we");
    if (producerFile == nullptr)
    {
      fprintf(stderr,"ERROR: Cannot create the file (%s)!\n",filename);
      return -3;
    }

    sprintf(filename,"%s/generations.csv",dir);
    generationFile = fopen(filename,"we");
    if (generationFile == nullptr)
    {
      fprintf(stderr,"ERROR: Cannot create the file (%s)!\n",filename);
      return -4;
    }

    sprintf(filename,"%s/files.csv",dir);
    fileFile = fopen(filename,"we");
    if (fileFile == nullptr)
    {
      fprintf(stderr,"ERROR: Cannot create the file (%s)!\n",filename);
      return -5;
    }

    sprintf(filename,"%s/content.csv",dir);
    contentFile = fopen(filename,"we");
    if (contentFile == nullptr)
    {
      fprintf(stderr,"ERROR: Cannot create the file (%s)!\n",filename);
      return -6;
    }


    readProducerList(producerListFile);

    readProducers(conn);

    PQfinish(conn);

    fclose(producerFile);
    fclose(generationFile);
    fclose(fileFile);
    fclose(contentFile);

    return 0;
  }
  catch (SmartMet::Spine::Exception& e)
  {
    SmartMet::Spine::Exception exception(BCP,"Service call failed!",nullptr);
    exception.printError();
    return -7;
  }
}

