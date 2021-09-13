#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#include "fileid.hpp"
#include "server.hpp"
#include "service.hpp"

using namespace std;

void * response(void * arg)
{
    
    int connect_id = ((pair<int, Server *> *)arg)->first;
    Server * server = ((pair<int, Server *> *)arg)->second;
    struct account user_account;
    Service service;

/*  用户需要先登录才能提出服务请求  */
login:
    if(read(connect_id, &user_account, sizeof(struct account)) != \
        sizeof(struct account))
        goto close_connection;
    switch (user_account.user_id)
    {
    case 0:
        //printf("%d\n%s\n", user_account.user_id, user_account.password);
        server->service_regist(connect_id, user_account);
        break;
    default:
        //printf("%d\n%s\n", user_account.user_id, user_account.password);
        if(server->service_login(connect_id, user_account)) goto addclient;
        break;
    }

    goto login;

addclient:
/*  登录成功，将用户信息与连接的套接字描述符建立映射存在表中，操作需加锁进行    */
    server->threadpool.resourceLock();
    server->clients[connect_id] = make_pair(user_account, map<ino_t, string>());
    server->threadpool.resourceUnlock();

services:
/*  成功则开始监听用户请求  */  
    if(read(connect_id, service.getRecive(), sizeof(Service::protocol_header)) 
        != sizeof(Service::protocol_header)) goto close_connection;

/*  用户请求恢复之前的备份，    */
    if(strcmp(service.getRecive()->request, "recover") == 0) 
    {   
        if(server->send_remote(connect_id, service.getRecive()->file_name))
            goto close_connection;
    }
    
/*  否则用户请求备份，将向服务器发送文件，服务器调用recive接收用户文件并保存    */
    else if(server->recive_remote(connect_id, service.getRecive())) {
        goto close_connection;
    }

    goto services;


close_connection:

    close(connect_id);
    server->threadpool.resourceLock();
    server->clients.erase(connect_id);
    server->threadpool.resourceUnlock();
    delete(((pair<int, Server *> *)arg));
    return (void *)(0);

}

bool Server::start()
{
    if(accounts_file() < 0) return false;
    
    {
        struct account acnt;
        struct stat file_state;
        if(stat(user_config, &file_state)) return false;

        if(file_state.st_size == 0) 
        {   acnt.user_id = (1 << 30);
            strncpy(acnt.password, "xuelanghaoshuai", 32);
            if(write(accounts_file(), &acnt, sizeof(struct account)) != 
                sizeof(struct account)) return false; 
        }
    }

    struct task client_task = {.run = response};
    if((false == socket.Success()) || \
        socket.setLocalAddress(ip, port)) return false;

    if(socket.Bindto() || socket.Listen()) return false;
    
    while(1)
    {
        int connect_id = socket.Accept();
        if(connect_id > 0) 
        {
            client_task.arg = new pair<int, Server *>(connect_id, this);
            threadpool.addTask(client_task);
        }
    }
}

bool Server::service_login(int connect_id, struct account & user_account)
{
/*  */
    off_t offset;
    Service service;
    struct account checked;
    service.set(0, 0, Service::Fail);
    file_id fileid(user_config, O_RDONLY, -1);

    offset = sizeof(checked) * (user_account.user_id - (1u << 30));
    printf("offset : %ld file id %d\n", offset, fileid());
    printf("-account : %d\n-password : %s\n", user_account.user_id, user_account.password);

    if(pread(7, &checked, sizeof(checked), offset) \
        != sizeof(checked)) 
        {
            printf("pread : %s\n", strerror(errno));
            goto fail;
        } 
    
    if(strcmp(user_account.password, checked.password)) goto fail;
    
    service.set(0, 0, Service::Sucs);
    if(write(connect_id, service.getOut(), sizeof(Service::protocol_header))
        != sizeof(Service::protocol_header)) return false;
    else 
    {
        printf("login\n-account : %d\n-password : %s\n", user_account.user_id, user_account.password);
        return true;
    }

fail:
    write(connect_id, service.getOut(), sizeof(Service::protocol_header));
    return false;

}

void Server::service_regist(int connect_id, struct account & user_account)
{
    Service service;
    struct account checked;
    service.set(0, 0, Service::Fail);

    threadpool.resourceLock();
    if(lseek(accounts_file(), -sizeof(struct account), SEEK_END)) goto end;
    if(read(accounts_file(), &checked, sizeof(checked)) != sizeof(checked))
        goto end;
    user_account.user_id = checked.user_id + 1;
    if(write(accounts_file(), &user_account, sizeof(struct account)) != \
        sizeof(struct account)) goto end;
    
    service.success(user_account.user_id);
end:
    threadpool.resourceUnlock();
    write(connect_id, service.getOut(), sizeof(Service::protocol_header));
}


int Server::send_remote(int socket_id, const char * path)
{
    int user_id;
    Service service;
    string newpath;
    if(clients.find(socket_id) == clients.end()) return 0;
    user_id = clients[socket_id].first.user_id;
    newpath = to_string(user_id) + '/' + path;

    printf("send_remote %s\n", newpath.c_str());
    
    if(service_send(socket_id, newpath.c_str()) != Success)
    {
        printf("failure\n");
        service.failure();
    }
    else 
    {
        service.success(0);
    }

    if(write(socket_id, service.getOut(), sizeof(Service::protocol_header)) 
        != sizeof(Service::protocol_header)) 
        {
            printf("ewrite\n");
            return eWrite;
        }

    return Success;
}


int Server::recive_remote(int socket_id, Service::protocol_header * request)
{
    printf("recive_remote : %s %s\n", request->file_name, request->request);
    if(update_path(socket_id, request->file_name) == nullptr) return 0;
    return service_recive(socket_id,request);
    
}

int Server::set_stat(const char * path, struct stat & file_state)
{
    std::string newpath(path);
    if(S_ISDIR(file_state.st_mode)) newpath += '/';
    newpath += ".stat";

    file_id fileid(newpath.c_str(), O_WRONLY | O_CREAT, 0644);
    if(fileid() < 0) return eOpen;

    if(write(fileid(), &file_state, sizeof(file_state)) != sizeof(file_state))
        return eWrite;

    return Success;
}

struct stat * Server::load_real_stat(const char * path, struct stat & file_state)
{
    std::string newpath(path);
    if(S_ISDIR(file_state.st_mode)) newpath += '/';
    newpath += ".stat";

    file_id fileid(newpath.c_str(), O_RDONLY, -1);
    if(fileid() < 0) return nullptr;

    if(read(fileid(), &file_state, sizeof(file_state)) != sizeof(file_state))
        return nullptr;

    return &file_state;
}

const char * Server::update_path(int socket_id, char * path)
{
    int user_id;
    string newpath;
    if(clients.find(socket_id) == clients.end()) return 0;
    user_id = clients[socket_id].first.user_id;
    newpath = to_string(user_id) + '/' + path;
    strncpy(path, newpath.c_str(), 256);

    return path;
}