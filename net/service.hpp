#ifndef SERVICE_HPP
#define SERVICE_HPP

class Service
{
    public:

    enum { Norm = 1, Dir, Soft, Hard, Pipe, Sucs, Fail, Reco };

    struct protocol_header
    {
        unsigned int length;
        char request[8];
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
    void failure();

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