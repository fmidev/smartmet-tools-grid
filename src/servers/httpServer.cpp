#include <microhttpd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <string>
#include <map>
#include <vector>

#include "grid-files/common/ThreadLock.h"
#include "grid-files/common/AutoThreadLock.h"


// MHD_Result used as return value in newer versions of microhttpd. Earlier defines
// MHD_YES and MHD_NO are being used. Workaround ABI compatibility by using typedef
// for older versions
#if defined(MHD_YES) && defined(MHD_NO)
typedef int MHD_Result;
#endif


std::string rootDir = "";

struct FileHandle
{
  FILE *fileHandle;
  SmartMet::ThreadLock threadLock;
  time_t lastUsed;
};

std::map<std::string,FileHandle*> fileHandles;
SmartMet::ThreadLock fileHandles_threadLock;
time_t fileHandles_lastChecked = time(0);


struct Request
{
    std::string url;
    std::map<std::string,std::string> headers;
    std::map<std::string,std::string> parameters;
};


FileHandle* getFileHandle(const char *filename)
{
  time_t currentTime = time(0);

  SmartMet::AutoThreadLock lock(&fileHandles_threadLock);
  if ((currentTime - fileHandles_lastChecked) > 3600)
  {
    std::vector<std::string> deleteList;
    time_t old = currentTime - 3600;
    for (auto it = fileHandles.begin(); it != fileHandles.end(); ++it)
    {
      if (it->second->lastUsed < old)
      {
        deleteList.push_back(it->first);
      }
    }

    for (auto it = deleteList.begin(); it != deleteList.end(); ++it)
    {
      auto itm = fileHandles.find(*it);
      if (itm != fileHandles.end())
      {
        //printf("DELETE %s\n",it->c_str());
        fclose(itm->second->fileHandle);
        delete itm->second;
        fileHandles.erase(itm);
      }
    }

    fileHandles_lastChecked = time(0);
  }

  auto fh = fileHandles.find(filename);
  if (fh != fileHandles.end())
  {
    fh->second->lastUsed = currentTime;
    return fh->second;
  }

  FileHandle *f = new FileHandle;
  f->fileHandle = fopen(filename,"r");
  if (f->fileHandle != NULL)
  {
    f->lastUsed = currentTime;
    fileHandles.insert(std::pair<std::string,FileHandle*>(filename,f));
    return f;
  }
  delete f;
  return nullptr;
}



static MHD_Result addParameter(void *cls, enum MHD_ValueKind kind, const char *key,const char *value)
{
  //printf("%s: %s\n",key,value);

  if (cls != nullptr)
  {
    std::map<std::string,std::string> *list = (std::map<std::string,std::string>*)cls;
    list->insert(std::pair<std::string,std::string>(key,value));
  }

  return MHD_YES;
}





static void requestCompleted (void *cls, struct MHD_Connection *connection,void **ptr, enum MHD_RequestTerminationCode toe)
{
  Request *request = (Request*)*ptr;
  if (request != nullptr)
    delete request;
}





static MHD_Result processRequest(void *cls,struct MHD_Connection *connection,const char *url,const char *method,const char *version,
        const char *upload_data,size_t *upload_data_size,void **ptr)
{
  printf("PROCESS REQUEST : %s : %lld : %s\n",url,(long long)*ptr,method);
  if (*ptr == nullptr)
  {
    // This is the first call. Let's initialize the request structure.

    Request *request = new Request;

    request->url = url;

    *ptr = request;
    *upload_data_size = 0;
    return MHD_YES;
  }


  // This is the second or N:th call. Let's pick up the request structure.
  //SmartMet::T::RequestMessage *requestMessage = (SmartMet::T::RequestMessage*)*ptr;

  Request *request = (Request*)*ptr;

  if (strcmp(method, "GET") == 0)
  {
    // This is a GET request, which means that all parameter can be read from the URL.

    MHD_get_connection_values(connection, MHD_HEADER_KIND, addParameter,&request->headers);
    MHD_get_connection_values(connection, MHD_GET_ARGUMENT_KIND, addParameter,&request->parameters);
  }
  else
  {
    return MHD_NO; /* unexpected method */
  }

  char filename[1000];
  if (!rootDir.empty() &&  rootDir != "/")
    sprintf(filename,"%s/%s",rootDir.c_str(),request->url.c_str());
  else
    strcpy(filename,request->url.c_str());

  char *content = NULL;
  std::size_t contentSize = 0;
  auto range = request->headers.find("Range");
  if (range != request->headers.end())
  {
    char buf[1000];
    strcpy(buf,range->second.c_str());
    if (strncasecmp(buf,"bytes=",6) == 0)
    {
      char *p1 = buf+6;
      char *p2 = strchr(p1,'-');
      if (p2 != NULL)
      {
        *p2 = '\0';
        p2++;

        std::size_t filePosition1 = atoll(p1);
        std::size_t filePosition2 = atoll(p2);

        if (filePosition2 > filePosition1)
        {
          contentSize = filePosition2 - filePosition1 + 1;
          content = new char[contentSize];

          auto fh = getFileHandle(filename);
          if (fh != NULL)
          {
            SmartMet::AutoThreadLock lock(&fh->threadLock);
            fseek(fh->fileHandle,filePosition1,SEEK_SET);
            int n = fread(content,1,contentSize,fh->fileHandle);
          }
        }
      }
    }
  }

  *ptr = nullptr; /* clear context pointer */
  if (content != NULL)
  {
    printf("SEND %ld\n",contentSize);
    struct MHD_Response *response = MHD_create_response_from_buffer(contentSize,(void*)content,MHD_RESPMEM_MUST_FREE);
    MHD_add_response_header(response, MHD_HTTP_HEADER_CONNECTION,"keep-alive");
    MHD_Result ret = MHD_queue_response(connection,MHD_HTTP_OK,response);
    MHD_destroy_response(response);
    return ret;
  }
  else
  {
    return MHD_NO;
  }
}





int main(int argc,char ** argv)
{
  if (argc != 3)
  {
    fprintf(stderr,"USAGE: httpServer <port> <rootDir>\n");
    exit(-1);
  }

  int port = atoi(argv[1]);
  rootDir = argv[2];


  struct MHD_Daemon *daemon =
      MHD_start_daemon (MHD_USE_SELECT_INTERNALLY  | MHD_USE_DEBUG,
      port,
      nullptr,
      nullptr,
      &processRequest,
      nullptr,
      MHD_OPTION_NOTIFY_COMPLETED,
      requestCompleted,
      nullptr,
      MHD_OPTION_END);

  if (daemon == nullptr)
    return 1;

  (void) getc(stdin);
  MHD_stop_daemon(daemon);

  return 0;
}

