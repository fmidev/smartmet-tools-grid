#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"
#include "grid-content/lua/LuaFile.h"


using namespace SmartMet;




int main(int argc, char *argv[])
{
  try
  {
    if (argc < 5)
    {
      fprintf(stderr,"USAGE: gu_executeLuaFunction <luaFile> <functionType> <function> <parama1> [<param2> ... <paramN>]\n");
      return -1;
    }

    char *filename = argv[1];
    int functionType = toInt64(argv[2]);
    std::string function = argv[3];

    std::vector<double> inParams;

    for (int t=4;t<argc;t++)
      inParams.push_back(toDouble(argv[t]));

    //for (int t=0; t<100000; t++)
//      inParams.push_back((double)t);

    Lua::LuaFile luaFile(filename);
    luaFile.init();

    switch (functionType)
    {
      case 1:
      {
        double val = luaFile.executeFunctionCall1(function,inParams);
        printf("RESULT : %f\n",val);
      }
      break;

      case 2:
      {
        std::vector<double> outParams;
        luaFile.executeFunctionCall2(function,inParams.size(),1,inParams,outParams);
        printf("RESULT : ");
        for (auto it = outParams.begin(); it != outParams.end(); ++it)
        {
          printf("%f ",*it);
        }
        printf("\n");
      }
      break;

      default:
        printf("Invalid function type (must be 1 or 2)!\n");
        break;
    }

    return 0;
  }
  catch (SmartMet::Spine::Exception& e)
  {
    SmartMet::Spine::Exception exception(BCP,"Service call failed!",nullptr);
    exception.printError();
    return -7;
  }
}

