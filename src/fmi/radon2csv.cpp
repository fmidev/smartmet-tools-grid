#include <macgyver/Exception.h>
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

T::GenerationId globalGenerationId = 1;
T::FileId globalFileId = 1;
T::SourceId sourceId = 200;
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
      Fmi::Exception exception(BCP, "Cannot open the producer list file!");
      exception.addParameter("Filename",filename);
      throw exception;
    }

    char st[1000];
    while (fgets(st,1000,file) != nullptr)
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
    fclose(file);
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
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
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}





void readContent(PGconn *conn,char *producerId,T::GenerationId generationId,T::FileId fileId,uint fileType,char *geometryId,char *forecastTime,char *fmiParameterId,char *fmiLevelId,char *parameterLevel,char *forecastType,char *pertubationNumber)
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
    if (Identification::gridDef.getFmiParameterDefById(toUInt32(fmiParameterId),fmiDef))
    {
      fmiParameterName = fmiDef.mParameterName;
      fmiParameterUnits = fmiDef.mParameterUnits;

      Identification::NewbaseParameterDef newbaseDef;
      if (Identification::gridDef.getNewbaseParameterDefByFmiId(toUInt32(fmiParameterId),newbaseDef))
      {
        newbaseParameterId = std::to_string(newbaseDef.mNewbaseParameterId);
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


    fprintf(contentFile,"%lu;%u;%u;%s;%lu;%u;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%u;%u;%u;%s;\n",
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
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}





uint readFiles(PGconn *conn,char *producerId,T::GenerationId generationId,char *schemaName, char *partitionName,char *analysisTime)
{
  FUNCTION_TRACE
  try
  {
    std::string sql = std::string("SELECT file_location,param_id,level_id,level_value::int,to_char((analysis_time+forecast_period) at time zone 'utc','yyyymmddThh24MISS'),forecast_type_id,forecast_type_value::int,geometry_id FROM ") + schemaName + "." + partitionName + " WHERE to_char(analysis_time, 'yyyymmddThh24MISS') = '" + analysisTime + "' AND producer_id = " + producerId;

    PGresult *res = PQexec(conn,sql.c_str());
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
      error(PQresultErrorMessage(res));

    uint fileCount = 0;
    //int fieldCount = PQnfields(res);
    int rowCount = PQntuples(res);

    for(int i = 0; i < rowCount; i++)
    {
      if (strstr(PQgetvalue(res, i, 0),"masala") != nullptr)
      {
        fprintf(fileFile,"%lu;%u;%s;%s;%lu;%u;%u;%u\n",
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
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
  }
}





uint readGenerations(PGconn *conn,char *producerId,char *producerName)
{
  FUNCTION_TRACE
  try
  {
    std::string sql = std::string("SELECT producer_id, to_char(analysis_time, 'yyyymmddThh24MISS'),to_char(analysis_time at time zone 'utc', 'yyyymmddThh24MISS'),schema_name, partition_name FROM as_grid_v WHERE producer_id = ") + producerId + " ORDER BY analysis_time desc";
    PGresult *res = PQexec(conn, sql.c_str());
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
      error(PQresultErrorMessage(res));

    //int fieldCount = PQnfields(res);
    int rowCount = PQntuples(res);

    std::string prev;
    uint generationCount = 0;

    for (int i = 0; i < rowCount; i++)
    {
      std::string st = std::to_string(globalGenerationId) + ";0;" + producerId + ";" + producerName + ":" + PQgetvalue(res, i, 2) + ";Producer " + producerName + " generation " + PQgetvalue(res, i, 2) + ";" + PQgetvalue(res, i, 2) + ";1;0;" + std::to_string(sourceId) + ";";

      if (/*toInt64(producerId) == 1  &&*/  strcmp(PQgetvalue(res, i, 1),prev.c_str()) != 0)
      {
        uint fCount = readFiles(conn,producerId,globalGenerationId,PQgetvalue(res, i, 3), PQgetvalue(res, i, 4),PQgetvalue(res, i, 1));
        if (fCount > 0)
        {
          fprintf(generationFile,"%s\n",st.c_str());
          prev = PQgetvalue(res, i, 1);
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
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
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
    throw Fmi::Exception(BCP, "Operation failed!", nullptr);
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

    std::string filename;

    filename = std::string(dir) + "/producers.csv";
    producerFile = fopen(filename.c_str(),"we");
    if (producerFile == nullptr)
    {
      fprintf(stderr,"ERROR: Cannot create the file (%s)!\n",filename.c_str());
      return -3;
    }

    filename = std::string(dir) + "/generations.csv";
    generationFile = fopen(filename.c_str(),"we");
    if (generationFile == nullptr)
    {
      fprintf(stderr,"ERROR: Cannot create the file (%s)!\n",filename.c_str());
      return -4;
    }

    filename = std::string(dir) + "/files.csv";
    fileFile = fopen(filename.c_str(),"we");
    if (fileFile == nullptr)
    {
      fprintf(stderr,"ERROR: Cannot create the file (%s)!\n",filename.c_str());
      return -5;
    }

    filename = std::string(dir) + "/content.csv";
    contentFile = fopen(filename.c_str(),"we");
    if (contentFile == nullptr)
    {
      fprintf(stderr,"ERROR: Cannot create the file (%s)!\n",filename.c_str());
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
  catch (Fmi::Exception& e)
  {
    Fmi::Exception exception(BCP,"Service call failed!",nullptr);
    exception.printError();
    return -7;
  }
}

