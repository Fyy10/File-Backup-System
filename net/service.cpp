#include <string.h>
#include "service.hpp"


void Service::setpath(const char * path)
{
    memset(out_header.file_name, 0, 256);
    memcpy(out_header.file_name, path, strlen(path));
    out_header.file_name[strlen(path)] = 0;
}

void Service::setrequest(const char * request)
{
    memset(out_header.request, 0 , 8);
    memcpy(out_header.request, request, strlen(request));
}
void Service::normal(const char * path, unsigned int length)
{  
    setrequest("file");
    out_header.length = length;
    setpath(path);
}

void Service::success(unsigned int length)
{
    setrequest("success");
    out_header.length = length;   
}

void Service::failure()
{
    setrequest("failure");
}


void Service::set(const char * path, unsigned int length, int request)
{
    switch(request)
    {
    case Norm:
        return normal(path, length);
    case Dir:
        return directory(path, length);
    case Soft:
        return symbol(path, length);
    case Hard:
        return hardlink(path, length);
    case Pipe:
        return fifopipe(path, length);
    case Reco:
        return recover(path);
    case Sucs:
        return success(length);
    case Fail:
        return failure();
    default:
        break;
    }
}

void Service::symbol(const char * path, unsigned int length)
{
    setrequest("soft");
    out_header.length = length;
    setpath(path);

}
void Service::directory(const char * path, unsigned int length)
{
    setrequest("dir");
    out_header.length = length;
    setpath(path);
}

void Service::recover(const char * path)
{
    setrequest("recover");
    setpath(path);
}


void Service::hardlink(const char * path, unsigned int length)
{
    setrequest("hard");
    out_header.length = length;
    setpath(path);
}
void Service::fifopipe(const char * path, unsigned int length)
{
    setrequest("pipe");
    out_header.length = length;
    setpath(path);
}