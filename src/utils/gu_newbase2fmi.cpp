#include <macgyver/Exception.h>
#include "grid-files/identification/GridDef.h"
#include "grid-files/common/GeneralFunctions.h"


using namespace SmartMet;



void loadNewbaseParameterDefs(char *configDir,Identification::NewbaseParamDef_vec& parameters)
{
  try
  {
    char filename[200];
    sprintf(filename,"%s/newbase_parameters.csv",configDir);


    FILE *file = fopen(filename,"re");
    if (file == nullptr)
    {
      Fmi::Exception exception(BCP,"Cannot open file!");
      exception.addParameter("Filename",std::string(filename));
      throw exception;
    }

    char st[1000];

    while (!feof(file))
    {
      if (fgets(st,1000,file) != nullptr  &&  st[0] != '#')
      {
        bool ind = false;
        char *field[100];
        uint c = 1;
        field[0] = st;
        char *p = st;
        while (*p != '\0'  &&  c < 100)
        {
          if (*p == '"')
            ind = !ind;

          if ((*p == ';'  || *p == '\n') && !ind)
          {
            *p = '\0';
            p++;
            field[c] = p;
            c++;
          }
          else
          {
            p++;
          }
        }

        if (c > 1)
        {
          Identification::NewbaseParameterDef rec;

          if (field[0][0] != '\0')
            rec.mNewbaseParameterId = field[0];

          if (field[1][0] != '\0')
            rec.mParameterName = field[1];

          parameters.push_back(rec);
        }
      }
    }
    fclose(file);
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
    if (argc < 2)
    {
      fprintf(stderr,"USAGE: gu_newbase2fmi <grid-config-dir> [-rev]\n");
      return -1;
    }

    char *configDir = argv[1];
    bool reverse = false;
    if (argc == 3  &&  strcmp(argv[2],"-rev") == 0)
      reverse = true;

    Identification::gridDef.init(configDir);


    Identification::NewbaseParamDef_vec parameters;
    loadNewbaseParameterDefs(configDir,parameters);

    for (auto it = parameters.begin(); it != parameters.end(); ++it)
    {
      Identification::FmiParameterDef rec;
      if (Identification::gridDef.getFmiParameterDefByNewbaseId(it->mNewbaseParameterId,rec))
      {
        if (!reverse)
          std::cout << "newbase." << it->mParameterName << ";" << rec.mParameterName << "\n";
        else
          std::cout << it->mParameterName << ";newbase." << it->mParameterName << "\n";
      }
      else
      {
        if (!reverse)
          std::cout << "# newbase." << it->mParameterName << ";\n";
        else
          std::cout << "# " << it->mParameterName << ";newbase." << it->mParameterName << "\n";
      }
    }

    return 0;
  }
  catch (Fmi::Exception& e)
  {
    Fmi::Exception exception(BCP,"Service call failed!",nullptr);
    exception.printError();
    return -7;
  }
}

