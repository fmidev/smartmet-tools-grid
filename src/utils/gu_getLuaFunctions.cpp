/*! \file
 *  \brief Utility program that loads a Lua file and prints the list of functions it defines.
 */

#include <macgyver/Exception.h>
#include "grid-files/common/GeneralFunctions.h"
#include "grid-content/lua/LuaFile.h"


using namespace SmartMet;




/*! \brief Program entry point. */

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
  catch (Fmi::Exception& e)
  {
    Fmi::Exception exception(BCP,"Service call failed!",nullptr);
    exception.printError();
    return -7;
  }
}

