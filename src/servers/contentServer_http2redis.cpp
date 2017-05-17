#include "contentServer/http/server/ServerInterface.h"
#include "contentServer/redis/RedisImplementation.h"

#include <microhttpd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>



SmartMet::ContentServer::HTTP::ServerInterface *httpServer = NULL;
SmartMet::ContentServer::RedisImplementation *redisImplementation = NULL;
std::string mainPage;


void getMainPage(SmartMet::T::ResponseMessage& response)
{
  try
  {
    if (mainPage.length() == 0)
      return;

    FILE *file = fopen(mainPage.c_str(),"r");
    if (file != NULL)
    {
      char st[10000];

      while (!feof(file))
      {
        if (fgets(st,10000,file) != NULL)
        {
          char *p = strstr(st,"\r");
          if (p == NULL)
            p = strstr(st,"\n");
          if (p != NULL)
            *p = '\0';
          response.addLine(st);
        }
      }
      fclose(file);
    }
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,"Operation failed!",NULL);
  }
}





static int addParameter(void *cls, enum MHD_ValueKind kind, const char *key,const char *value)
{
  try
  {
    //printf("%s: %s\n",key,value);

    if (cls != NULL)
    {
      SmartMet::T::RequestMessage *httpRequest = (SmartMet::T::RequestMessage*)cls;
      if (key != NULL  &&  value != NULL)
        httpRequest->addLine(key,value);

      if (key != NULL  &&  value == NULL)
        httpRequest->addLine(key,"");

      if (key == NULL  &&  value != NULL)
        httpRequest->addLine("",value);
    }

    return MHD_YES;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,"Operation failed!",NULL);
  }
}





static void requestCompleted (void *cls, struct MHD_Connection *connection,void **ptr, enum MHD_RequestTerminationCode toe)
{
  try
  {
    SmartMet::T::RequestMessage *request = (SmartMet::T::RequestMessage*)*ptr;
    if (request != NULL)
      delete request;
  }
  catch (...)
  {
    throw SmartMet::Spine::Exception(BCP,"Operation failed!",NULL);
  }
}





static int processRequest(void *cls,struct MHD_Connection *connection,const char *url,const char *method,const char *version,
        const char *upload_data,size_t *upload_data_size,void **ptr)
{
  try
  {
    if (*ptr == NULL)
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
    char *content = new char[size+100];
    char *p = content;

    //p += sprintf(p,"<HTML><BODY><PRE>\n");
    for (uint t=0; t<lineCount; t++)
    {
      std::string s = responseMessage.getLineByIndex(t);
      p += sprintf(p,"%s\n",s.c_str());
    }

    //printf("%s",content);

    *ptr = NULL; /* clear context pointer */
    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(content),(void*)content,MHD_RESPMEM_MUST_FREE);
    int ret = MHD_queue_response(connection,MHD_HTTP_OK,response);
    MHD_destroy_response(response);
    return ret;
  }
  catch (...)
  {
    SmartMet::Spine::Exception exception(BCP,"Operation failed!",NULL);
    exception.printError();
    return MHD_NO;
  }
}





int main(int argc,char ** argv)
{
  try
  {
    if (argc != 5)
    {
      printf("\n");
      printf("##################################################################################\n");
      printf("                        contentServer_http2redis\n");
      printf("##################################################################################\n");
      printf(" DESCRIPTION:\n");
      printf("   This is a contentServer that offers an HTTP service interface for accessing\n");
      printf("   its services. The actual content information is stored into the Redis database.\n");
      printf("\n");
      printf(" USAGE:\n");
      printf("   contentServer_http2redis <httpPort> <redisAddress> <redisPort>\n");
      printf("\n");
      printf(" WHERE:\n");
      printf("   <httpPort>        => The TCP port of the HTTP server.\n");
      printf("   <redisAddress>    => The IP address of the Redis database.\n");
      printf("   <redisPort>       => The TCP port of the Redis database.\n");
      printf("   <mainPage>        => The HTTP file that is returned when there is no \n");
      printf("                        parameters in the HTTP request.\n");
      printf("##################################################################################\n");
      printf("\n");
      return -1;
    }

    unsigned short httpPort = (unsigned short)atoi(argv[1]);
    char *redisAddress = (char*)argv[2];
    int redisPort = atoi(argv[3]);
    mainPage = argv[4];

    redisImplementation = new SmartMet::ContentServer::RedisImplementation();
    redisImplementation->init(redisAddress,redisPort);

    httpServer = new SmartMet::ContentServer::HTTP::ServerInterface();
    httpServer->init(redisImplementation);


    struct MHD_Daemon *daemon = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY, httpPort, NULL, NULL,&processRequest, NULL,
                                 MHD_OPTION_NOTIFY_COMPLETED, requestCompleted,NULL, MHD_OPTION_END);
    if (daemon == NULL)
      return 1;

    (void) getc(stdin);
    MHD_stop_daemon(daemon);

    delete redisImplementation;
    delete httpServer;

    return 0;
  }
  catch (...)
  {
    SmartMet::Spine::Exception exception(BCP,"Operation failed!",NULL);
    exception.printError();
    return -1;
  }
}

