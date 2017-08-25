#include "grid-files/common/Exception.h"
#include "grid-files/common/ShowFunction.h"
#include "grid-files/common/GeneralFunctions.h"
#include "grid-files/common/ShowFunction.h"

#include <postgresql/libpq-fe.h>
#include <stdlib.h>
#include <string.h>

#define FUNCTION_TRACE FUNCTION_TRACE_OFF


using namespace SmartMet;




void error(char *mess)
{
  fprintf(stderr, "### %s\n", mess);
  exit(1);
}




void create_levelDef_grib2_fmi(PGconn *conn,const char *dir)
{
  FUNCTION_TRACE
  try
  {
    char filename[300];
    sprintf(filename,"%s/levelDef_grib2_fmi.csv",dir);

    FILE *file = fopen(filename,"w");
    if (file == NULL)
    {
      SmartMet::Spine::Exception exception(BCP,"Cannot create the file!");
      exception.addParameter("Filename",filename);
      throw exception;
    }

    char sql[3000];
    char *p = sql;

    p += sprintf(p,"SELECT DISTINCT\n");
    p += sprintf(p,"  level_grib2.producer_id,\n");
    p += sprintf(p,"  producer_grib.ident,\n");
    p += sprintf(p,"  producer_grib.centre,\n");
    p += sprintf(p,"  level_grib2.grib_level_id,\n");
    p += sprintf(p,"  level_grib2.level_id\n");
    p += sprintf(p,"FROM\n");
    p += sprintf(p,"  level_grib2 LEFT OUTER JOIN producer_grib ON (level_grib2.producer_id = producer_grib.producer_id)\n");
    p += sprintf(p,"ORDER BY\n");
    p += sprintf(p,"  level_grib2.producer_id,producer_grib.ident,producer_grib.centre,level_grib2.grib_level_id,level_grib2.level_id;\n");


    PGresult *res = PQexec(conn,sql);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
      error(PQresultErrorMessage(res));

    int fieldCount = PQnfields(res);
    int rowCount = PQntuples(res);

    for (int i = 0; i < rowCount; i++)
    {
      for (int f=0; f< fieldCount; f++)
      {
        fprintf(file,"%s;",PQgetvalue(res,i,f));
      }
      fprintf(file,"\n");
    }

    fclose(file);
    PQclear(res);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}





void create_levelDef_grib1_fmi(PGconn *conn,const char *dir)
{
  FUNCTION_TRACE
  try
  {
    char filename[300];
    sprintf(filename,"%s/levelDef_grib1_fmi.csv",dir);

    FILE *file = fopen(filename,"w");
    if (file == NULL)
    {
      SmartMet::Spine::Exception exception(BCP,"Cannot create the file!");
      exception.addParameter("Filename",filename);
      throw exception;
    }

    char sql[3000];
    char *p = sql;

    p += sprintf(p,"SELECT DISTINCT\n");
    p += sprintf(p,"  level_grib1.producer_id,\n");
    p += sprintf(p,"  producer_grib.ident,\n");
    p += sprintf(p,"  producer_grib.centre,\n");
    p += sprintf(p,"  level_grib1.grib_level_id,\n");
    p += sprintf(p,"  level_grib1.level_id\n");
    p += sprintf(p,"FROM\n");
    p += sprintf(p,"  level_grib1 LEFT OUTER JOIN producer_grib ON (level_grib1.producer_id = producer_grib.producer_id)\n");
    p += sprintf(p,"ORDER BY\n");
    p += sprintf(p,"  level_grib1.producer_id,producer_grib.ident,producer_grib.centre,level_grib1.grib_level_id,level_grib1.level_id;\n");


    PGresult *res = PQexec(conn,sql);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
      error(PQresultErrorMessage(res));

    int fieldCount = PQnfields(res);
    int rowCount = PQntuples(res);

    for (int i = 0; i < rowCount; i++)
    {
      for (int f=0; f< fieldCount; f++)
      {
        fprintf(file,"%s;",PQgetvalue(res,i,f));
      }
      fprintf(file,"\n");
    }

    fclose(file);
    PQclear(res);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}





void create_parameterDef_grib2_fmi(PGconn *conn,const char *dir)
{
  FUNCTION_TRACE
  try
  {
    char filename[300];
    sprintf(filename,"%s/parameterDef_grib2_fmi.csv",dir);

    FILE *file = fopen(filename,"w");
    if (file == NULL)
    {
      SmartMet::Spine::Exception exception(BCP,"Cannot create the file!");
      exception.addParameter("Filename",filename);
      throw exception;
    }

    char sql[3000];
    char *p = sql;

    p += sprintf(p,"SELECT DISTINCT\n");
    p += sprintf(p,"  param_grib2.id,\n");
    p += sprintf(p,"  param_grib2.param_id,\n");
    p += sprintf(p,"  param_grib2.producer_id,\n");
    p += sprintf(p,"  producer_grib.ident,\n");
    p += sprintf(p,"  producer_grib.centre,\n");
    p += sprintf(p,"  param_grib2.discipline,\n");
    p += sprintf(p,"  param_grib2.category,\n");
    p += sprintf(p,"  param_grib2.number,\n");
    p += sprintf(p,"  param_grib2.level_id,\n");
    p += sprintf(p,"  level_grib2.grib_level_id,\n");
    p += sprintf(p,"  param_grib2.level_value\n");
    p += sprintf(p,"FROM\n");
    p += sprintf(p,"  param_grib2 LEFT OUTER JOIN producer_grib ON (param_grib2.producer_id=producer_grib.producer_id)\n");
    p += sprintf(p,"  LEFT OUTER JOIN level_grib2 ON (param_grib2.level_id=level_grib2.level_id)\n");
    p += sprintf(p,"ORDER BY\n");
    p += sprintf(p,"  param_grib2.param_id,param_grib2.producer_id,producer_grib.ident;\n");

    PGresult *res = PQexec(conn,sql);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
      error(PQresultErrorMessage(res));

    int fieldCount = PQnfields(res);
    int rowCount = PQntuples(res);

    for (int i = 0; i < rowCount; i++)
    {
      for (int f=0; f< fieldCount; f++)
      {
        fprintf(file,"%s;",PQgetvalue(res,i,f));
      }
      fprintf(file,"\n");
    }

    fclose(file);
    PQclear(res);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}




void create_parameterDef_grib1_fmi(PGconn *conn,const char *dir)
{
  FUNCTION_TRACE
  try
  {
    char filename[300];
    sprintf(filename,"%s/parameterDef_grib1_fmi.csv",dir);

    FILE *file = fopen(filename,"w");
    if (file == NULL)
    {
      SmartMet::Spine::Exception exception(BCP,"Cannot create the file!");
      exception.addParameter("Filename",filename);
      throw exception;
    }

    char sql[3000];
    char *p = sql;

    p += sprintf(p,"SELECT DISTINCT\n");
    p += sprintf(p,"  param_grib1.id,\n");
    p += sprintf(p,"  param_grib1.param_id,\n");
    p += sprintf(p,"  param_grib1.producer_id,\n");
    p += sprintf(p,"  producer_grib.ident,\n");
    p += sprintf(p,"  producer_grib.centre,\n");
    p += sprintf(p,"  param_grib1.table_version,\n");
    p += sprintf(p,"  param_grib1.number,\n");
    p += sprintf(p,"  param_grib1.timerange_indicator,\n");
    p += sprintf(p,"  param_grib1.level_id,\n");
    p += sprintf(p,"  level_grib1.grib_level_id,\n");
    p += sprintf(p,"  param_grib1.level_value\n");
    p += sprintf(p,"FROM\n");
    p += sprintf(p,"  param_grib1 LEFT OUTER JOIN producer_grib ON (param_grib1.producer_id=producer_grib.producer_id)\n");
    p += sprintf(p,"  LEFT OUTER JOIN level_grib1 ON (param_grib1.level_id=level_grib1.level_id)\n");
    p += sprintf(p,"ORDER BY\n");
    p += sprintf(p,"  param_grib1.param_id,param_grib1.producer_id,producer_grib.ident;\n");


    PGresult *res = PQexec(conn,sql);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
      error(PQresultErrorMessage(res));

    int fieldCount = PQnfields(res);
    int rowCount = PQntuples(res);

    for (int i = 0; i < rowCount; i++)
    {
      for (int f=0; f< fieldCount; f++)
      {
        fprintf(file,"%s;",PQgetvalue(res,i,f));
      }
      fprintf(file,"\n");
    }

    fclose(file);
    PQclear(res);
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP, "Operation failed!", NULL);
  }
}




void create_parameterDef_fmi(PGconn *conn,const char *dir)
{
  FUNCTION_TRACE
  try
  {
    char filename[300];
    sprintf(filename,"%s/parameterDef_fmi.csv",dir);

    FILE *file = fopen(filename,"w");
    if (file == NULL)
    {
      SmartMet::Spine::Exception exception(BCP,"Cannot create the file!");
      exception.addParameter("Filename",filename);
      throw exception;
    }

    char sql[3000];
    char *p = sql;

    p += sprintf(p,"SELECT DISTINCT\n");
    p += sprintf(p,"  param.id,\n");
    p += sprintf(p,"  param.version,\n");
    p += sprintf(p,"  param.name,\n");
    p += sprintf(p,"  param_unit.name,\n");
    p += sprintf(p,"  param.description,\n");
    p += sprintf(p,"  param.interpolation_id,\n");
    p += sprintf(p,"  coalesce(param_newbase.univ_id,0)\n");
    p += sprintf(p,"FROM\n");
    p += sprintf(p,"  param LEFT OUTER JOIN param_newbase ON (param.id = param_newbase.param_id),param_unit\n");
    p += sprintf(p,"WHERE\n");
    p += sprintf(p,"  param.unit_id=param_unit.id\n");
    p += sprintf(p,"ORDER BY\n");
    p += sprintf(p,"  param.id;\n");


    PGresult *res = PQexec(conn,sql);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
      error(PQresultErrorMessage(res));

    int fieldCount = PQnfields(res);
    int rowCount = PQntuples(res);

    for (int i = 0; i < rowCount; i++)
    {
      for (int f=0; f< fieldCount; f++)
      {
        fprintf(file,"%s;",PQgetvalue(res,i,f));
      }
      fprintf(file,"\n");
    }

    fclose(file);
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
    if (argc != 3)
    {
      fprintf(stderr,"USAGE: radon2config <configDir> <dbConnectionString>\n");
      return -1;
    }


    char *dir = argv[1];
    char *connectionString = argv[2];


    PGconn *conn = PQconnectdb(connectionString);
    if (PQstatus(conn) != CONNECTION_OK)
      error(PQerrorMessage(conn));

    create_parameterDef_fmi(conn,dir);
    create_parameterDef_grib1_fmi(conn,dir);
    create_parameterDef_grib2_fmi(conn,dir);
    create_levelDef_grib1_fmi(conn,dir);
    create_levelDef_grib2_fmi(conn,dir);

    PQfinish(conn);

    return 0;
  }
  catch (SmartMet::Spine::Exception& e)
  {
    SmartMet::Spine::Exception exception(BCP,"Service call failed!",NULL);
    exception.printError();
    return -7;
  }
}

