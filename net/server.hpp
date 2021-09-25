#ifndef SERVER_HPP
#define SERVER_HPP

#include <map>

#include "fileid.hpp"
#include "account.h"
#include "socket.hpp"
#include "netbase.hpp"
#include "threadspool.hpp"

#include <fcntl.h>
#include <netinet/in.h>

class Server : public NetBase
{

    public:

    Server() : accounts_file(user_config, O_RDWR | O_CREAT | O_APPEND, 0644) {}
    bool start();

    private:

    bool service_login(int connect_id, struct account & user_account);
    void service_regist(int connect_id, struct account & user_account);

    int service_check(int connect_id, Service::protocol_header * request);

    int send_remote(int socket_id, const char * path);

    int recive_remote(int socket_id, Service::protocol_header * request);

    int remove(int connect_id, const char * path);

    virtual const char * update_path(int socket_id, char * path);

    virtual int set_stat(const char * path, struct stat & file_state);

    virtual struct stat * load_real_stat(const char * path, struct stat & file_state);

    friend void * response(void * arg);

    virtual bool check_inode(int socket_id, ino_t inode)
    {
        return clients[socket_id].file_map.find(inode) != clients[socket_id].file_map.end();
    }

    virtual void add_inode(int socket_id, ino_t inode, const std::string & path)
    {
        clients[socket_id].file_map[inode] = path;
    }

    virtual std::string & inode_get(int socket_id, ino_t inode)
    {
        return clients[socket_id].file_map[inode];
    }

    virtual int recive_code(int socket_id, Service::protocol_header * request);

    void serverlog(const char * message, int socket_id);

    int remove(const char * path);

    int count_files(const char * path);

    int check(int connect_id, Service::protocol_header * request);

    private:

    struct client_details 
    {
        unsigned int address;
        struct account user;
        std::map<ino_t, std::string> file_map;
    };

    Pthreadspool threadpool;
    Socket socket;
    const uint16_t port = 12800u;
    const unsigned int ip = INADDR_ANY;
    const char * user_config = ".user_config";
    file_id accounts_file;
    std::map<int, struct client_details>  clients;
};

#endif