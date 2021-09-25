#ifndef SERVICE_HPP
#define SERVICE_HPP

#include <string>

class Service
{
    public:


    enum { Norm = 1, Dir, Soft, Hard, Pipe, Sucs, Reco, Check, Remv, Fail };

    struct protocol_header
    {
        unsigned int length;
        char request[8];
        char hashcode[64];
        char file_name[256];
    };

    void set(const char * path, unsigned int length, int request);


    void normal(const char * path, unsigned int length);
    void symbol(const char * path, unsigned int length);
    void hardlink(const char * path, unsigned int length);
    void fifopipe(const char * path, unsigned int length);
    void directory(const char * path, unsigned int length);
    void recover(const char * path);
    void success(unsigned int length);
    void check(const char * path, int length);
    void remove(const char * path);
    void failure();

    void setcode(const std::string & code)
    {
        strncpy(out_header.hashcode, code.c_str(), 64);
    }

    typename Service::protocol_header * getRecive()
    {
        return &recive;
    }

    typename Service::protocol_header * getOut()
    {
        return &out_header;
    }

    private:
    void setpath(const char * path);
    void setrequest(const char * request);
    struct protocol_header out_header, recive;
};

#endif