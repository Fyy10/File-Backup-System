#ifndef NETBASE_HPP
#define NETBASE_HPP

#include <map>
#include <string>

#include "socket.hpp"
#include "service.hpp"
#include "hashlist.hpp"

class NetBase
{

    public:

    NetBase() = default;
    NetBase(int coding) : coding(coding) {}

    enum { Success, eRead, eWrite, eOpen, eLink, eSymlink, eMkdir, elstat, eSetst, eOverflow, eMkfifo, Fail };

    protected:

    int service_recive(int socket_id, Service::protocol_header * request);

    int service_send(int socket_id, const char * path);

    virtual const char * update_path(int socket_id, char * path) = 0;
    
    virtual int set_stat(const char * path, struct stat & file_state) = 0;

    virtual struct stat * load_real_stat(const char * path, struct stat & file_state) = 0;

    virtual int recive_code(int socket_id, Service::protocol_header * request)
    {
        return 0;
    }
    
    virtual std::string gethashcode(const std::string & path)
    {
        return HashList::gethashcode(path);
    }

    protected:

    Socket socket;

    enum { HEAD_SIZE = sizeof(Service::protocol_header) };

    protected:

    int recive_normal_file(int fd, Service::protocol_header * request);

    int recive_hard_link(int fd, Service::protocol_header * request);
    
    int recive_directory(int fd, Service::protocol_header * request);

    int recive_soft_link(int fd, Service::protocol_header * request);

    int recive_fifo_pipe(int fd, Service::protocol_header * request);

    virtual bool check_inode(int socket_id, ino_t inode) = 0;

    virtual void add_inode(int socket_id, ino_t inode, const std::string & path) = 0;

    virtual std::string & inode_get(int socket_id, ino_t inode) = 0;


    int send_normal_file(int socket_id, const char * path, struct stat & file_state);

    int send_directory(int socket_id, const char * path, struct stat & file_state);

    int send_sofk_link(int socket_id, const char * path, struct stat & file_state);

    int send_fifo_pipe(int socket_id, const char * path, struct stat & file_state);

    bool ends_with_stat(const char * path) const;

    protected:

    int coding;

};

#endif