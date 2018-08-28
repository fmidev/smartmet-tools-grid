#include "grid-content/contentServer/corba/client/ClientImplementation.h"
#include "grid-content/contentServer/http/client/ClientImplementation.h"
#include "grid-content/contentServer/redis/RedisImplementation.h"
#include "grid-files/common/Exception.h"
#include "grid-files/common/GeneralFunctions.h"

using namespace SmartMet;


int main(int argc, char *argv[])
{
  try
  {
    char *serviceIor = getenv("SMARTMET_CS_IOR");
    if (serviceIor == nullptr)
    {
      fprintf(stdout,"SMARTMET_CS_IOR not defined!\n");
      return -2;
    }


    if (argc < 7)
    {
      fprintf(stdout,"USAGE: cs_addFileInfoWithContentList <sessionId> <groupFlags> <producerId> <generationId> <fileType> <filename>\n");
      fprintf(stdout,"       [-m <messageIndex> <timestamp> <fmiParamId> <gribParamId> ] [-m ...] [-m ...]\n");
      return -1;
    }

    // ### Session:
    T::SessionId sessionId = (SmartMet::T::SessionId)atoll(argv[1]);


    // ### Service:
    ContentServer::Corba::ClientImplementation service;
    service.init(serviceIor);

    // ### Service parameters:
    T::FileInfo info;
    info.mGroupFlags  = (uint)atoll(argv[2]);
    info.mProducerId = (uint)atoll(argv[3]);
    info.mGenerationId = (uint)atoll(argv[4]);
    info.mFileType = (T::FileType)atoll(argv[5]);
    info.mName = argv[6];
    info.mFlags = T::FileInfo::Flags::PredefinedContent;

    T::ContentInfoList contentList;
    for (int t=7;t<argc; t++)
    {
      if (strcmp(argv[t],"-m") == 0  &&  (t + 4) < argc)
      {
        T::ContentInfo *cinfo = new T::ContentInfo();
        cinfo->mFileId = info.mFileId;
        cinfo->mFileType = info.mFileType;
        cinfo->mGenerationId = info.mGenerationId;
        cinfo->mGroupFlags = info.mGroupFlags;
        cinfo->mProducerId = info.mProducerId;
        cinfo->mMessageIndex = (uint)atoll(argv[t+1]);
        cinfo->mForecastTime = argv[t+2];
        cinfo->mFmiParameterId = argv[t+3];
        cinfo->mGribParameterId = argv[t+4];

        contentList.addContentInfo(cinfo);
      }
    }

    // ### Service call
    unsigned long long startTime = getTime();
    int result = service.addFileInfoWithContentList(sessionId,info,contentList);
    unsigned long long endTime = getTime();

    if (result != 0)
    {
      fprintf(stdout,"ERROR (%d) : %s\n",result,ContentServer::getResultString(result).c_str());
      return -3;
    }

    // ### Result:
    info.print(std::cout,0,0);

    printf("\nTIME : %f sec\n\n",(float)(endTime-startTime)/1000000);

    return 0;
  }
  catch (SmartMet::Spine::Exception& e)
  {
    SmartMet::Spine::Exception exception(BCP,"Service call failed!",nullptr);
    exception.printError();
    return -4;
  }
}

