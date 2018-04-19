#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"
#include "grid-files/common/ConfigurationFile.h"
#include "grid-files/common/CoordinateConversions.h"


using namespace SmartMet;




int main(int argc, char *argv[])
{
  try
  {
    if (argc < 2)
    {
      fprintf(stderr,"USAGE: gu_showConfigurationAttributes <configurationFile> [-val]\n");
      return -1;
    }

    char *filename = argv[1];

    ConfigurationFile configFile(filename);

    if (argc == 3  &&  strcmp(argv[2],"-val") == 0)
      configFile.print(std::cout,0,1);
    else
      configFile.print(std::cout,0,0);

    return 0;
  }
  catch (SmartMet::Spine::Exception& e)
  {
    SmartMet::Spine::Exception exception(BCP,"Service call failed!",NULL);
    exception.printError();
    return -7;
  }
}

