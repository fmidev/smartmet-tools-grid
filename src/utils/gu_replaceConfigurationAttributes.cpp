/*! \file
 *  \brief Utility program that replaces attribute name placeholders in a text file with their values from a configuration file.
 */

#include <macgyver/Exception.h>
#include "grid-files/common/GeneralFunctions.h"
#include "grid-files/common/ConfigurationFile.h"
#include "grid-files/common/CoordinateConversions.h"


using namespace SmartMet;




/*! \brief Program entry point. */

int main(int argc, char *argv[])
{
  try
  {
    if (argc != 4)
    {
      fprintf(stderr,"USAGE: gu_replaceConfigurationAttributes <configurationFile> <inputFile> <outpufFile>\n");
      return -1;
    }

    char *configFile = argv[1];
    char *inputFile = argv[2];
    char *outputFile = argv[3];

    ConfigurationFile config(configFile);
    config.replaceAttributeNamesWithValues(std::string(inputFile),std::string(outputFile));

    return 0;
  }
  catch (Fmi::Exception& e)
  {
    Fmi::Exception exception(BCP,"Service call failed!",nullptr);
    exception.printError();
    return -7;
  }
}

