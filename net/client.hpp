#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "socket.hpp"
#include "service.hpp"
#include "netbase.hpp"

#include <map>

class Client : public NetBase
{

    public:

    bool connectServer();

    int regist(const char * password)
    {
        return request(0u, password);
    }
    int login(unsigned int user_id, const char * password)
    {
        return request(user_id, password);
    }

    int backup(const char * path);

    int recover(char * path);

    private:

    virtual bool check_inode(int socket_id, ino_t inode)
    {
        return (inodes_map.find(inode) != inodes_map.end());
    }

    virtual void add_inode(int socket_id, ino_t inode, const std::string & path)
    {
        inodes_map[inode] = path;
    }

    virtual std::string & inode_get(int socket_id, ino_t inode)
    {
        return inodes_map[inode];
    }
    
    virtual const char * update_path(int socket_id, char * path) 
    {
        std::string newpath(path);
        int i, n = newpath.find_first_of('/') + 1;
        for(i = n; i < newpath.length(); ++i) 
        {
            path[i - n] = newpath[i];
        }
        path[i - n] = 0;
        return path;
    }

    using NetBase::service_recive;

    virtual struct stat * load_real_stat(const char * path, struct stat & file_state)
    {
        return &file_state;
    }

    virtual int set_stat(const char * path, struct stat & file_state);


    int request(unsigned int user_id, const char * password);

    private:
    
    Service service;
    
    const uint16_t port = 12800u;

    const char * server_address = "127.0.0.1";

    std::map<ino_t, std::string> inodes_map;
};

#endif