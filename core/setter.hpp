#ifndef SETTER_HPP
#define SETTER_HPP

#include <sys/types.h>

class StatSetter
{
    public:

    static int updateAttributes(const char * path, struct stat & file_state);
    static int updateAttributes(const char * path, uid_t ownner, gid_t group,
        struct timeval * timeval_list, mode_t mode);
    static int updateOwnnerTime(const char * path, struct stat & file_state);
    private:
    static int setownner(const char * path, uid_t ownner, gid_t group);
    static int settime(const char * path, struct timeval * timeval_list);
    static int setmode(const char * path, mode_t mode);

};

#endif