#ifndef FILEID_HPP
#define FILEID_HPP

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

class file_id
{
    public:
    file_id(const char * path, int flags, int mode)
    {
        if(mode != -1) fd = open(path, flags, mode);
        else  fd = open(path, flags);
    }
    ~file_id() 
    {
        printf("close %d\n", fd);
        close(fd);
    }

    int operator () ()
    {
        return fd;
    }
    
    private:
    int fd;
};

#endif