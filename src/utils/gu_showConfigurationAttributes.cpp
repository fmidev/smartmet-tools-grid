#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"
#include "grid-files/common/ConfigurationFile.h"


using namespace SmartMet;




int main(int argc, char *argv[])
{
  try
  {
    if (argc != 2)
    {
      fprintf(stderr,"USAGE: gu_showConfigurationAttributes <configurationFile>\n");
      return -1;
    }

    char *filename = argv[1];

    ConfigurationFile configFile(filename);

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

