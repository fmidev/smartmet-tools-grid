/*! \file
 *  \brief Utility program that prints the mapping between Newbase and FMI parameter names, optionally in reverse order.
 */

#include <macgyver/Exception.h>
#include "grid-files/identification/GridDef.h"
#include "grid-files/common/GeneralFunctions.h"


using namespace SmartMet;



namespace
{
/*! \brief Splits the line in place into fields separated by ';' or newline, ignoring
    separators inside quotes. Returns the number of field pointers set (at most 100),
    which includes the field starting after the last separator. */

uint splitFields(char *st,char *field[100])
{
  bool ind = false;
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
  return c;
}



/*! \brief Load newbase parameter defs. */

void loadNewbaseParameterDefs(const std::string& configDir,Identification::NewbaseParamDef_vec& parameters)
{
  try
  {
    std::string filename = configDir + "/newbase_parameters.csv";


    FILE *file = fopen(filename.c_str(),"re");
    if (file == nullptr)
    {
      Fmi::Exception exception(BCP,"Cannot open file!");
      exception.addParameter("Filename",filename);
      throw exception;
    }

    char st[1000];

    while (!feof(file))
    {
      if (fgets(st,1000,file) != nullptr  &&  st[0] != '#')
      {
        char *field[100];
        uint c = splitFields(st,field);

        if (c > 1)
        {
          Identification::NewbaseParameterDef rec;

          if (field[0][0] != '\0')
            rec.mNewbaseParameterId = toUInt32(field[0]);

          if (field[1][0] != '\0')
            rec.mParameterName = field[1];

          parameters.push_back(rec);
        }
      }
    }
    (void)fclose(file);
  }
  catch (...)
  {
    throw Fmi::Exception(BCP,"Operation failed!",nullptr);
  }
}
}  // namespace





/*! \brief Program entry point. */

int main(int argc, char *argv[])
{
  try
  {
    if (argc < 2)
    {
      fprintf(stderr,"USAGE: gu_newbase2fmi <grid-config-file> [-rev]\n");
      return -1;
    }

    // The grid-files configuration file, newbase_parameters.csv is read from the same directory
    char *configFile = argv[1];
    std::string configDir = getFileDir(configFile);
    bool reverse = false;
    if (argc == 3  &&  strcmp(argv[2],"-rev") == 0)
      reverse = true;

    Identification::gridDef.init(configFile);


    Identification::NewbaseParamDef_vec parameters;
    loadNewbaseParameterDefs(configDir,parameters);

    for (auto &parameter : parameters)
    {
      Identification::FmiParameterDef rec;
      if (Identification::gridDef.getFmiParameterDefByNewbaseId(parameter.mNewbaseParameterId,rec))
      {
        if (!reverse)
          std::cout << "newbase." << parameter.mParameterName << ";" << rec.mParameterName << "\n";
        else
          std::cout << parameter.mParameterName << ";newbase." << parameter.mParameterName << "\n";
      }
      else
      {
        if (!reverse)
          std::cout << "# newbase." << parameter.mParameterName << ";\n";
        else
          std::cout << "# " << parameter.mParameterName << ";newbase." << parameter.mParameterName << "\n";
      }
    }

    return 0;
  }
  catch (Fmi::Exception&)
  {
    Fmi::Exception exception(BCP,"Service call failed!",nullptr);
    exception.printError();
    return -7;
  }
}

