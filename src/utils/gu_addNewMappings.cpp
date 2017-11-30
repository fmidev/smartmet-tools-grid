#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"
#include "grid-content/queryServer/definition/ParameterMappingFile.h"


using namespace SmartMet;




int main(int argc, char *argv[])
{
  try
  {
    if (argc != 3)
    {
      fprintf(stderr,"USAGE: gu_addNewMappings <originalFile> <newMappings>\n");
      return -1;
    }

    char *originalFile = argv[1];
    char *mergeFile = argv[2];

    QueryServer::ParameterMappingFile file1(originalFile);
    file1.init();

    QueryServer::ParameterMappingFile file2(mergeFile);
    file2.init();

    FILE *file = fopen(originalFile,"a");
    if (file == NULL)
    {
      fprintf(stderr,"Cannot open the parameter file (%s)!\n",originalFile);
      return -2;
    }

    uint len2 = file2.getNumberOfMappings();
    for (uint t=0; t<len2; t++)
    {
      QueryServer::ParameterMapping *m2 = file2.getParameterMappingByIndex(t);
      QueryServer::ParameterMapping *m1 = file1.getMapping(*m2);
      if (m1 == NULL)
      {
        fprintf(file,"%s;%s;%d;%s;%d;%d;%05d;%d;D\n",
            m2->mProducerName.c_str(),
            m2->mParameterName.c_str(),
            (int)m2->mParameterKeyType,
            m2->mParameterKey.c_str(),
            (int)m2->mParameterLevelIdType,
            (int)m2->mParameterLevelId,
            (int)m2->mParameterLevel,
            (int)m2->mInterpolationMethod);
      }
    }

    return 0;

    fclose(file);
  }
  catch (SmartMet::Spine::Exception& e)
  {
    SmartMet::Spine::Exception exception(BCP,"Service call failed!",NULL);
    exception.printError();
    return -7;
  }
}

