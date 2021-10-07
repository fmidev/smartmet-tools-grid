#include "grid-content/contentServer/http/server/ServerInterface.h"
#include "grid-content/contentServer/redis/RedisImplementation.h"
#include "grid-content/contentServer/cache/CacheImplementation.h"
#include "grid-content/contentServer/corba/client/ClientImplementation.h"
#include "grid-content/contentServer/http/client/ClientImplementation.h"
#include "grid-content/contentServer/memory/MemoryImplementation.h"
#include "grid-files/common/ConfigurationFile.h"
#include "grid-files/common/Log.h"
#include "grid-files/common/GeneralFunctions.h"

#include <microhttpd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

using namespace SmartMet;


ContentServer::ServiceInterface *contentSource = nullptr;
ContentServer::Corba::ClientImplementation *corbaClient = nullptr;
ContentServer::HTTP::ClientImplementation *httpClient = nullptr;
ContentServer::RedisImplementation *redisImplementation = nullptr;
ContentServer::HTTP::ServerInterface *httpServer = nullptr;
ContentServer::MemoryImplementation *memoryImplementation = nullptr;
ContentServer::CacheImplementation *cacheImplementation = nullptr;

bool shutdownRequested = false;

ConfigurationFile   mConfigurationFile;
std::string         mServerAddress;
std::string         mServerPort;
std::string         mHelpFile;
bool                mCacheEnabled = false;
std::string         mContentSourceType;
bool                mDataLoadEnabled = false;
bool                mDataSyncEnabled = false;
bool                mDataSaveEnabled = false;
std::string         mDataDir;
uint                mDataEventListMaxSize = 3000000;
uint                mDataSaveInterval;
bool                mProcessingLogEnabled = false;
std::string         mProcessingLogFile;
int                 mProcessingLogMaxSize = 100000000;
int                 mProcessingLogTruncateSize = 20000000;
Log                 mProcessingLog;
bool                mDebugLogEnabled = false;
std::string         mDebugLogFile;
int                 mDebugLogMaxSize = 100000000;
int                 mDebugLogTruncateSize = 20000000;
Log                 mDebugLog;
std::string         mRedisAddress;
int                 mRedisPort;
std::string         mRedisTablePrefix;
std::string         mCorbaIor;
std::string         mHttpUrl;


void readConfigFile(const char* configFile)
{
  try
  {
    const char *configAttribute[] =
    {
        "smartmet.tools.grid.content-server.address",
        "smartmet.tools.grid.content-server.port",
        "smartmet.tools.grid.content-server.helpFile",
        "smartmet.tools.grid.content-server.cache.enabled",
        "smartmet.tools.grid.content-server.content-source.type",
        "smartmet.tools.grid.content-server.content-source.redis.address",
        "smartmet.tools.grid.content-server.content-source.redis.port",
        "smartmet.tools.grid.content-server.content-source.redis.tablePrefix",
        "smartmet.tools.grid.content-server.content-source.corba.ior",
        "smartmet.tools.grid.content-server.content-source.http.url",
        "smartmet.tools.grid.content-server.content-source.memory.contentLoadEnabled",
        "smartmet.tools.grid.content-server.content-source.memory.contentSyncEnabled",
        "smartmet.tools.grid.content-server.content-source.memory.contentSaveEnabled",
        "smartmet.tools.grid.content-server.content-source.memory.contentDir",
        "smartmet.tools.grid.content-server.content-source.memory.eventListMaxSize",
        "smartmet.tools.grid.content-server.content-source.memory.contentSaveInterval",
        "smartmet.tools.grid.content-server.processing-log.enabled",
        "smartmet.tools.grid.content-server.processing-log.file",
        "smartmet.tools.grid.content-server.processing-log.maxSize",
        "smartmet.tools.grid.content-server.processing-log.truncateSize",
        "smartmet.tools.grid.content-server.debug-log.enabled",
        "smartmet.tools.grid.content-server.debug-log.file",
        "smartmet.tools.grid.content-server.debug-log.maxSize",
        "smartmet.tools.grid.content-server.debug-log.truncateSize",
        nullptr
    };


    mConfigurationFile.readFile(configFile);
    //mConfigurationFile.print(std::cout,0,0);

    uint t=0;
    while (configAttribute[t] != nullptr)
    {
      if (!mConfigurationFile.findAttribute(configAttribute[t]))
      {
        Fmi::Exception exception(BCP, "Missing configuration attribute!");
        exception.addParameter("File",configFile);
        exception.addParameter("Attribute",configAttribute[t]);
        throw exception;
      }
      t++;
    }

    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.address", mServerAddress);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.port", mServerPort);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.helpFile", mHelpFile);

    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.cache.enabled", mCacheEnabled);

    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.content-source.type", mContentSourceType);

    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.content-source.redis.address", mRedisAddress);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.content-source.redis.port", mRedisPort);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.content-source.redis.tablePrefix", mRedisTablePrefix);

    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.content-source.corba.ior", mCorbaIor);

    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.content-source.http.url", mHttpUrl);

    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.content-source.memory.contentLoadEnabled", mDataLoadEnabled);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.content-source.memory.contentSyncEnabled", mDataSyncEnabled);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.content-source.memory.contentSaveEnabled", mDataSaveEnabled);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.content-source.memory.contentDir", mDataDir);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.content-source.memory.eventListMaxSize", mDataEventListMaxSize);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.content-source.memory.contentSaveInterval", mDataSaveInterval);

    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.processing-log.enabled", mProcessingLogEnabled);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.processing-log.file", mProcessingLogFile);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.processing-log.maxSize", mProcessingLogMaxSize);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.processing-log.truncateSize", mProcessingLogTruncateSize);

    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.debug-log.enabled", mDebugLogEnabled);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.debug-log.file", mDebugLogFile);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.debug-log.maxSize", mDebugLogMaxSize);
    mConfigurationFile.getAttributeValue("smartmet.tools.grid.content-server.debug-log.truncateSize", mDebugLogTruncateSize);
  }
  catch (...)
  {
    throw Fmi::Exception(BCP, "Constructor failed!", nullptr);
  }
}





void getMainPage(SmartMet::T::ResponseMessage& response)
{
  try
  {
    FILE *file = fopen(mHelpFile.c_str(),"re");
    if (file != nullptr)
    {
      char st[10000];

      while (!feof(file))
      {
        if (fgets(st,10000,file) != nullptr)
        {
          char *p = strstr(st,"\r");
          if (p == nullptr)
            p = strstr(st,"\n");
          if (p != nullptr)
            *p = '\0';
          response.addLine(st);
        }
      }
      fclose(file);
    }
    else
    {
      response.addLine("Help file not found!");
    }
  }
  catch (...)
  {
    throw Fmi::Exception(BCP,"Operation failed!",nullptr);
  }
}





static int addParameter(void *cls, enum MHD_ValueKind kind, const char *key,const char *value)
{
  try
  {
    //printf("%s: %s\n",key,value);

    if (cls != nullptr)
    {
      SmartMet::T::RequestMessage *httpRequest = (SmartMet::T::RequestMessage*)cls;
      if (key != nullptr  &&  value != nullptr)
        httpRequest->addLine(key,value);

      if (key != nullptr  &&  value == nullptr)
        httpRequest->addLine(key,"");

      if (key == nullptr  &&  value != nullptr)
        httpRequest->addLine("",value);
    }

    return MHD_YES;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP,"Operation failed!",nullptr);
  }
}





static void requestCompleted (void *cls, struct MHD_Connection *connection,void **ptr, enum MHD_RequestTerminationCode toe)
{
  try
  {
    SmartMet::T::RequestMessage *request = (SmartMet::T::RequestMessage*)*ptr;
    if (request != nullptr)
      delete request;
  }
  catch (...)
  {
    throw Fmi::Exception(BCP,"Operation failed!",nullptr);
  }
}





static int processRequest(void *cls,struct MHD_Connection *connection,const char *url,const char *method,const char *version,
        const char *upload_data,size_t *upload_data_size,void **ptr)
{
  try
  {
    if (*ptr == nullptr)
    {
      // This is the first call. Let's initialize the request structure.

      *ptr = new SmartMet::T::RequestMessage();
      // *upload_data_size = 0;
      return MHD_YES;
    }


    // This is the second or N:th call. Let's pick up the request structure.
    SmartMet::T::RequestMessage *requestMessage = (SmartMet::T::RequestMessage*)*ptr;


    if (strcmp(method, "GET") == 0)
    {
      // This is a GET request, which means that all parameter can be read from the URL.

      MHD_get_connection_values(connection, MHD_GET_ARGUMENT_KIND, addParameter, requestMessage);
    }
    else
    if (strcmp(method, "POST") == 0)
    {
      // This is a POST message, which means that parameters are in the content part of the message.

      size_t sz = *upload_data_size;

      // If the content part contains data then we should process it and exit the function. We should
      // do this as long as the content size is bigger than zero.

      if (sz > 0)
      {
        char line[10000];
        uint c = 0;

        uint lCount = requestMessage->getLineCount();
        if (lCount > 0)
        {
          std::string lastLine = requestMessage->getLineByIndex(lCount-1);
          if (lastLine.length() > 0  &&  lastLine[lastLine.length()-1] == '\n')
          {
            c = lastLine.length() - 1;
            strncpy(line,lastLine.c_str(),c);
            requestMessage->deleteLineByIndex(lCount-1);
          }
        }

        for (size_t t=0; t<sz; t++)
        {
          char ch = upload_data[t];
          if (ch == '\r'  || ch == '\n')
          {
            line[c] = '\0';
            if (c > 0)
             requestMessage->addLine(line);

            c = 0;
          }
          else
          {
            line[c] = ch;
            c++;
          }
        }

        if (c > 0)
        {
          line[c] = '\n';
          c++;
          line[c] = '\0';
          requestMessage->addLine(line);
        }
        *upload_data_size = 0;

        return MHD_YES;
      }

      // The content size is zero. So we have read all the data and now we can process the message.
    }
    else
    {
      return MHD_NO; /* unexpected method */
    }

    // Processing the message.

    SmartMet::T::ResponseMessage responseMessage;

    if (requestMessage->getLineCount() > 0)
      httpServer->processRequest(*requestMessage,responseMessage);
    else
      getMainPage(responseMessage);

    //requestMessage->print(std::cout,0,0);
    //responseMessage.print(std::cout,0,0);

    uint lineCount = responseMessage.getLineCount();
    uint size = responseMessage.getContentSize();
    char *content = new char[size+30000];
    char *p = content;

    //p += sprintf(p,"<HTML><BODY><PRE>\n");
    for (uint t=0; t<lineCount; t++)
    {
      std::string s = responseMessage.getLineByIndex(t);
      p += sprintf(p,"%s\n",s.c_str());
    }

    //printf("%s",content);

    *ptr = nullptr; /* clear context pointer */
    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(content),(void*)content,MHD_RESPMEM_MUST_FREE);
    int ret = MHD_queue_response(connection,MHD_HTTP_OK,response);
    MHD_destroy_response(response);
    return ret;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP,"Operation failed!",nullptr);
    exception.printError();
    return MHD_NO;
  }
}





int main(int argc,char ** argv)
{
  try
  {
    if (argc != 2)
    {
      printf("\n");
      printf("##################################################################################\n");
      printf("                            httpContentServer\n");
      printf("##################################################################################\n");
      printf(" DESCRIPTION:\n");
      printf("   This is a contentServer that offers a HTTP service interface for accessing\n");
      printf("   content information in the content source. Notice that this is just an interface");
      printf("   and it does not contain any caching capabilities.");
      printf("\n");
      printf(" USAGE:\n");
      printf("   httpContentServer <configFile>\n");
      printf("\n");
      printf("##################################################################################\n");
      printf("\n");
      return -1;
    }


    readConfigFile(argv[1]);


    if (strcasecmp(mContentSourceType.c_str(),"redis") == 0)
    {
      redisImplementation = new ContentServer::RedisImplementation();
      redisImplementation->init(mRedisAddress.c_str(),mRedisPort,mRedisTablePrefix.c_str());
      contentSource = redisImplementation;
    }
    else
    if (strcasecmp(mContentSourceType.c_str(),"corba") == 0)
    {
      corbaClient = new ContentServer::Corba::ClientImplementation();
      std::cout << mCorbaIor << " *** IOR\n";
      corbaClient->init(mCorbaIor.c_str());
      contentSource = corbaClient;
    }
    else
    if (strcasecmp(mContentSourceType.c_str(),"http") == 0)
    {
      httpClient = new ContentServer::HTTP::ClientImplementation();
      httpClient->init(mHttpUrl.c_str());
      contentSource = httpClient;
    }
    else
    if (strcasecmp(mContentSourceType.c_str(),"memory") == 0)
    {
      memoryImplementation = new ContentServer::MemoryImplementation();
      memoryImplementation->init(mDataLoadEnabled,mDataSaveEnabled,mDataSyncEnabled,true,mDataDir,mDataSaveInterval);
      memoryImplementation->setEventListMaxLength(mDataEventListMaxSize);
      contentSource = memoryImplementation;
    }

    if (contentSource == nullptr)
    {
      Fmi::Exception exception(BCP,"No acceptable content source defined!");
      throw exception;
    }

    if (mCacheEnabled)
    {
      cacheImplementation = new ContentServer::CacheImplementation();
      if (mProcessingLogEnabled)
      {
        mProcessingLog.init(true,mProcessingLogFile.c_str(),mProcessingLogMaxSize,mProcessingLogTruncateSize);
        cacheImplementation->setProcessingLog(&mProcessingLog);
      }

      if (mDebugLogEnabled)
      {
        mDebugLog.init(true,mDebugLogFile.c_str(),mDebugLogMaxSize,mDebugLogTruncateSize);
        cacheImplementation->setDebugLog(&mDebugLog);
      }

      cacheImplementation->init(0,contentSource);
      cacheImplementation->startEventProcessing();
      contentSource = cacheImplementation;
    }
    else
    {
      if (mProcessingLogEnabled)
      {
        mProcessingLog.init(true,mProcessingLogFile.c_str(),mProcessingLogMaxSize,mProcessingLogTruncateSize);
        contentSource->setProcessingLog(&mProcessingLog);
      }

      if (mDebugLogEnabled)
      {
        mDebugLog.init(true,mDebugLogFile.c_str(),mDebugLogMaxSize,mDebugLogTruncateSize);
        contentSource->setDebugLog(&mDebugLog);
      }
    }


    httpServer = new SmartMet::ContentServer::HTTP::ServerInterface();
    httpServer->init(contentSource);


    struct MHD_Daemon *daemon = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY, toInt64(mServerPort.c_str()), nullptr, nullptr,&processRequest, nullptr,
                                 MHD_OPTION_NOTIFY_COMPLETED, requestCompleted,nullptr, MHD_OPTION_END);
    if (daemon == nullptr)
      return 1;

    (void) getc(stdin);
    MHD_stop_daemon(daemon);

    if (redisImplementation != nullptr)
      delete redisImplementation;

    if (corbaClient != nullptr)
      delete corbaClient;

    if (httpClient != nullptr)
      delete httpClient;

    if (memoryImplementation != nullptr)
      delete memoryImplementation;

    if (cacheImplementation != nullptr)
      delete cacheImplementation;

    delete httpServer;

    return 0;
  }
  catch (...)
  {
    Fmi::Exception exception(BCP,"Operation failed!",nullptr);
    exception.printError();
    return -1;
  }
}

