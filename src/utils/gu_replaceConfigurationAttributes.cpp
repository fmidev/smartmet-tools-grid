#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"
#include "grid-files/common/ConfigurationFile.h"
#include "grid-files/common/CoordinateConversions.h"


using namespace SmartMet;




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
  catch (SmartMet::Spine::Exception& e)
  {
    SmartMet::Spine::Exception exception(BCP,"Service call failed!",nullptr);
    exception.printError();
    return -7;
  }
}

