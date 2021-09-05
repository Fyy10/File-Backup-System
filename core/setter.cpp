#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include "setter.hpp"

int StatSetter::updateAttributes(const char * path, struct stat & file_state)
{
    struct timeval timeval_list[2] = 
    {
        {
            .tv_sec = file_state.st_atim.tv_sec,
            .tv_usec = file_state.st_atim.tv_nsec / 1000
        },
        {
            .tv_sec = file_state.st_mtim.tv_sec,
            .tv_usec = file_state.st_mtim.tv_nsec / 1000
        }
    };

    return updateAttributes(path,file_state.st_uid,file_state.st_gid,
        timeval_list,file_state.st_mode);
}

int StatSetter::updateAttributes(const char * path, uid_t ownner,
        gid_t group, struct timeval * timeval_list, mode_t mode)
{
    if(setownner(path, ownner, group) != 0) return -1;
    else if(settime(path, timeval_list) != 0) return -2;
    else if(setmode(path, mode) != 0) return -3;
    else return 0;
}

int StatSetter::updateOwnnerTime(const char * path, struct stat & file_state)
{
    struct timeval timeval_list[2] = 
    {
        {
            .tv_sec = file_state.st_atim.tv_sec,
            .tv_usec = file_state.st_atim.tv_nsec / 1000
        },
        {
            .tv_sec = file_state.st_mtim.tv_sec,
            .tv_usec = file_state.st_mtim.tv_nsec / 1000
        }
    };

    if(setownner(path, file_state.st_uid, file_state.st_gid)) return -1;
    else if(settime(path, timeval_list)) return -2;
    else return 0;
}

int StatSetter::setownner(const char * path, uid_t ownner, gid_t group)
{
    return lchown(path, ownner, group);
}

int StatSetter::settime(const char * path, struct timeval * timeval_list)
{
    return lutimes(path,timeval_list);
}

int StatSetter::setmode(const char * path, mode_t mode)
{
    return chmod(path,mode);
}