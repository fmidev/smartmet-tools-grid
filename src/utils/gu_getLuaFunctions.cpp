#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"
#include "grid-content/lua/LuaFile.h"


using namespace SmartMet;




int main(int argc, char *argv[])
{
  try
  {
    if (argc != 2)
    {
      fprintf(stderr,"USAGE: gu_getLuaFunctions <luaFile>\n");
      return -1;
    }

    char *filename = argv[1];

    Lua::LuaFile luaFile(filename);
    luaFile.init();

    luaFile.print(std::cout,0,0);
    return 0;
  }
  catch (SmartMet::Spine::Exception& e)
  {
    SmartMet::Spine::Exception exception(BCP,"Service call failed!",NULL);
    exception.printError();
    return -7;
  }
}

