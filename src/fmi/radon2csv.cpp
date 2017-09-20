#include "grid-files/common/Exception.h"
#include "grid-files/common/ShowFunction.h"
#include "grid-files/common/GeneralFunctions.h"
#include "grid-files/common/ShowFunction.h"
#include "grid-files/identification/GribDef.h"

#include <postgresql/libpq-fe.h>
#include <stdlib.h>
#include <string.h>


#define FUNCTION_TRACE FUNCTION_TRACE_OFF



using namespace SmartMet;


FILE *producerFile = NULL;
FILE *generationFile = NULL;
FILE *fileFile = NULL;
FILE *contentFile = NULL;

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
    FILE *file = fopen(filename,"r");
    if (file == NULL)
    {
      SmartMet::Spine::Exception exception(BCP, "Cannot open the producer list file!");
      exception.addParameter("Filename",filename);
      throw exception;
    }

    char st[1000];
    while (!feof(file))
    {
      if (fgets(st,1000,file) != NULL)
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
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
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
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
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


    const Identification::ParameterDefinition_fmi_cptr fmiDef = Identification::gribDef.mMessageIdentifier_fmi.getParameterDefById(fmiParameterId);

    if (fmiDef != NULL)
    {
      fmiParameterName = fmiDef->mParameterName;
      fmiParameterUnits = fmiDef->mParameterUnits;
      newbaseParameterId = fmiDef->mNewbaseId;

      const Identification::Parameter_newbase_cptr  nbDef = Identification::gribDef.mMessageIdentifier_fmi.getParameter_newbaseId(fmiDef->mNewbaseId);
      if (nbDef != NULL)
        newbaseParameterName = nbDef->mParameterName;

      const Identification::Parameter_grib1_fmi_cptr g1Def = Identification::gribDef.mMessageIdentifier_fmi.getParameter_grib1(fmiParameterId);
      if (g1Def != NULL)
        grib1ParameterLevelId = toString(g1Def->mGribParameterLevelId);

      const Identification::Parameter_grib2_fmi_cptr g2Def = Identification::gribDef.mMessageIdentifier_fmi.getParameter_grib2(fmiParameterId);
      if (g2Def != NULL)
        grib2ParameterLevelId = toString(g2Def->mGribParameterLevelId);

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
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}





uint readFiles(PGconn *conn,char *producerId,uint generationId,char *schemaName, char *partitionName,char *analysisTime)
{
  FUNCTION_TRACE
  try
  {
    char sql[1000];
    sprintf(sql,"SELECT file_location,param_id,level_id,level_value::int,to_char((analysis_time+forecast_period) at time zone 'utc','yyyymmddThh240000'),forecast_type_id,forecast_type_value::int,geometry_id FROM %s.%s WHERE to_char(analysis_time, 'yyyymmddThh240000') = '%s' AND producer_id = %s",schemaName,partitionName,analysisTime,producerId);

    PGresult *res = PQexec(conn,sql);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
      error(PQresultErrorMessage(res));

    uint fileCount = 0;
    //int fieldCount = PQnfields(res);
    int rowCount = PQntuples(res);

    for(int i = 0; i < rowCount; i++)
    {
      if (strstr(PQgetvalue(res, i, 0),"masala") != NULL)
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
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}





uint readGenerations(PGconn *conn,char *producerId,char *producerName)
{
  FUNCTION_TRACE
  try
  {
    char sql[1000];
    sprintf(sql,"SELECT producer_id, to_char(analysis_time, 'yyyymmddThh240000'),to_char(analysis_time at time zone 'utc', 'yyyymmddThh240000'),schema_name, partition_name FROM as_grid_v WHERE producer_id = %s ORDER BY analysis_time desc",producerId);
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
      sprintf(st,"%u;%u;%s;%s:%s;Producer %s generation %s;%u;%u;%u;",
              globalGenerationId,
              0,  // type
              producerId,
              producerName,PQgetvalue(res, i, 2),  // Name,
              producerName,PQgetvalue(res, i, 2),  // Description,
              1,  // status,
              0,  // flags
              sourceId
             );

      if (/*atoi(producerId) == 1  &&*/  strcmp(PQgetvalue(res, i, 1),prev) != 0)
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
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
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
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
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

    char *configDir = getenv("SMARTMET_GRID_CONFIG_DIR");
    if (configDir == NULL)
    {
      fprintf(stderr,"SMARTMET_GRID_CONFIG_DIR not defined!\n");
      return -2;
    }

    SmartMet::Identification::gribDef.init(configDir);


    char *dir = argv[1];
    sourceId = (uint)atoi(argv[2]);
    char *producerListFile = argv[3];
    char *connectionString = argv[4];

    PGconn *conn = PQconnectdb(connectionString);
    if (PQstatus(conn) != CONNECTION_OK)
      error(PQerrorMessage(conn));

    char filename[200];

    sprintf(filename,"%s/producers.csv",dir);
    producerFile = fopen(filename,"w");
    if (producerFile == NULL)
    {
      fprintf(stderr,"ERROR: Cannot create the file (%s)!\n",filename);
      return -3;
    }

    sprintf(filename,"%s/generations.csv",dir);
    generationFile = fopen(filename,"w");
    if (generationFile == NULL)
    {
      fprintf(stderr,"ERROR: Cannot create the file (%s)!\n",filename);
      return -4;
    }

    sprintf(filename,"%s/files.csv",dir);
    fileFile = fopen(filename,"w");
    if (fileFile == NULL)
    {
      fprintf(stderr,"ERROR: Cannot create the file (%s)!\n",filename);
      return -5;
    }

    sprintf(filename,"%s/content.csv",dir);
    contentFile = fopen(filename,"w");
    if (contentFile == NULL)
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
    SmartMet::Spine::Exception exception(BCP,"Service call failed!",NULL);
    exception.printError();
    return -7;
  }
}

