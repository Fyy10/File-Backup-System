#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "filter.hpp"
#include "socket.hpp"
#include "service.hpp"
#include "netbase.hpp"

#include <map>

class Client : public NetBase
{

    public:

    Client(int coding) : NetBase(coding) {}

    bool connectServer();

    unsigned int regist(const char * password)
    {
        char request[4] = {"RGT"};
        return this->request(*(unsigned int *)request, password);
    }
    unsigned int login(unsigned int user_id, const char * password)
    {
        return request(user_id, password);
    }

    bool check_file(const std::string & path);
    
    int backup(const char * path);

    int recover(const char * path);

    int update(const char * path);

    int remove(const char * path);

    int remove_dir(const char* path);

    private:

    bool service_check(const std::string & path, int flag);

    bool check(const std::string & path);

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

    virtual std::string gethashcode(const std::string & path)
    {
        return HashList::gethashcode(path);
    }

    unsigned int request(unsigned int user_id, const char * password);

    private:
    
    Service service;
    
    const uint16_t port = 12800u;

    std::string origin_prefix;

    const char * server_address = "127.0.0.1";

    std::map<ino_t, std::string> inodes_map;
};

#endif