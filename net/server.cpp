#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#include "fileid.hpp"
#include "server.hpp"
#include "service.hpp"

using namespace std;

void * response(void * arg)
{
    
    int connect_id = ((pair<int, pair<unsigned int, Server *>> *)arg)->first;
    Server * server = ((pair<int, pair<unsigned int ,Server *>> *)arg)->second.second;
    
    struct account user_account;
    Service service;

/*  用户需要先登录才能提出服务请求  */
login:
    if(read(connect_id, &user_account, sizeof(struct account)) != \
        sizeof(struct account))
        goto close_connection;
    if(strcmp("RGT", user_account.request) == 0)
        server->service_regist(connect_id, user_account);
    else if(server->service_login(connect_id, user_account)) goto addclient;

    goto login;

addclient:
/*  登录成功，将用户信息与连接的套接字描述符建立映射存在表中，操作需加锁进行    */
    server->threadpool.resourceLock();
    server->clients[connect_id].user = user_account;
    server->clients[connect_id].address = 0;
    server->threadpool.resourceUnlock();

    server->serverlog("login", connect_id);

services:
/*  成功则开始监听用户请求  */  
    if(read(connect_id, service.getRecive(), server->HEAD_SIZE) 
        != server->HEAD_SIZE) goto close_connection;

/*  用户请求恢复之前的备份，    */
    if(strcmp(service.getRecive()->request, "recover") == 0) 
    {   
        if(server->send_remote(connect_id, service.getRecive()->file_name))
            goto close_connection;
    }
/*  用户请求检查数据是否一致，  */
    else if(strcmp(service.getRecive()->request, "check") == 0)
    {
        if(server->check(connect_id, service.getRecive()))
            goto close_connection;
    }

/*  用户请求删除文件或文件夹    */
    else if(strcmp(service.getRecive()->request, "remove") == 0)
    {
        if(server->remove(connect_id, service.getRecive()->file_name))
            goto close_connection;
    }
/*  否则用户请求备份，将向服务器发送文件，服务器调用recive接收用户文件并保存    */
    else if(server->recive_remote(connect_id, service.getRecive())) 
        goto close_connection;
    

    goto services;


close_connection:

    server->serverlog("close", connect_id);
    close(connect_id);

    server->threadpool.resourceLock();
    server->clients.erase(connect_id);
    server->threadpool.resourceUnlock();
    
    delete(((pair<int, pair<unsigned int, Server *>> *)arg));
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
    /*
    while(1){
    int connect_id = socket.Accept();
    response(new pair(connect_id, make_pair(socket.remote_ip(), this)));
    }
    /*
    */
    while(1)
    {
        int connect_id = socket.Accept();
        if(connect_id > 0) 
        {
            client_task.arg = new pair<int, pair<unsigned int, Server *>>(\
                connect_id, make_pair(socket.remote_ip(), this));
            threadpool.addTask(client_task);
        }
    }
    /**/
}

bool Server::service_login(int connect_id, struct account & user_account)
{
/*  */
    off_t offset;
    Service service;
    struct account checked;
    service.set(0, 0, Service::Fail);

    offset = sizeof(checked) * (user_account.user_id - (1u << 30));

    if(pread(accounts_file(), &checked, sizeof(checked), offset) \
        != sizeof(checked)) goto fail;
    
    if(strcmp(user_account.password, checked.password)) goto fail;
    
    service.set(0, 0, Service::Sucs);
    if(write(connect_id, service.getOut(), HEAD_SIZE)
        != HEAD_SIZE) return false;
    else return true;

fail:
    write(connect_id, service.getOut(), HEAD_SIZE);
    return false;

}

void Server::service_regist(int connect_id, struct account & user_account)
{
    Service service;
    struct account checked;
    service.set(0, 0, Service::Fail);

    threadpool.resourceLock();

    if(lseek(accounts_file(), -sizeof(struct account), SEEK_END) == -1) 
        goto end;

    if(read(accounts_file(), &checked, sizeof(checked)) != sizeof(checked))
        goto end;

    user_account.user_id = checked.user_id + 1;

    if(mkdir(to_string(user_account.user_id).c_str(), 0744)) goto end;

    if(write(accounts_file(), &user_account, sizeof(struct account)) != \
        sizeof(struct account)) goto end;

    serverlog("register", 0);

    service.success(user_account.user_id);
end:
    threadpool.resourceUnlock();
    write(connect_id, service.getOut(), HEAD_SIZE);
}


int Server::send_remote(int socket_id, const char * path)
{
    int user_id;
    Service service;
    string newpath;
    if(clients.find(socket_id) == clients.end()) return 0;
    user_id = clients[socket_id].user.user_id;
    newpath = to_string(user_id) + '/' + path;
    
    if(service_send(socket_id, newpath.c_str()) != Success)
        service.failure();
    else service.success(0);

    if(write(socket_id, service.getOut(), HEAD_SIZE) 
        != HEAD_SIZE) return eWrite;

    serverlog((string("recover ") + newpath).c_str(), socket_id);

    return Success;
}


int Server::recive_remote(int socket_id, Service::protocol_header * request)
{
    if(update_path(socket_id, request->file_name) == nullptr) return 0;
    serverlog((string("recive ") + request->file_name).c_str(), socket_id);
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
    user_id = clients[socket_id].user.user_id;
    newpath = to_string(user_id) + '/' + path;
    strncpy(path, newpath.c_str(), 256);

    return path;
}

#include <time.h>

void Server::serverlog(const char * message, int socket_id)
{
    int address, user_id; 
    time_t current_time;
    
    time(&current_time);
    if(clients.find(socket_id) == clients.end())
        user_id = address = 0;
    else
    {
        user_id = clients[socket_id].user.user_id;
        address = clients[socket_id].address;
    }

    printf("[server log]\t%10d@%s\t:%s\t%s",
        user_id,socket.ip_string(address),message,ctime(&current_time));
}


int Server::service_check(int connect_id, Service::protocol_header * request)
{
    serverlog((string("check ") + request->file_name).c_str(), connect_id);

    Service service;
    char hashcode[64];

    service.set(0, 0, Service::Fail);
    update_path(connect_id, request->file_name);

/*  检查文件是否存在，文件不存在则说明原文件与备份文件不一致    */
    if(access(request->file_name, F_OK) == -1) goto end;
    
/*  目录文件或管道文件不用检查哈希值  */
    if(request->length == 1) service.set(0, 0, Service::Sucs);
    else 
    {
        file_id fileid((string(request->file_name) + ".stat").c_str(), O_RDWR, -1);
        if(fileid() < 0) goto end;
        if(lseek(fileid(), sizeof(struct stat), SEEK_SET) <= 0) goto end;
        if(read(fileid(), hashcode, 64) != 64) goto end;
    
        if(strcmp(hashcode, request->hashcode) == 0) service.set(0, 0, Service::Sucs);
    }

end:

    return (write(connect_id, service.getOut(), HEAD_SIZE) != HEAD_SIZE);
}

int Server::recive_code(int socket_id, Service::protocol_header * request)
{
    file_id fileid((string(request->file_name) + ".stat").c_str(), O_RDWR | O_CREAT, 0644);
    if((fileid() < 0) || (lseek(fileid(), sizeof(struct stat), SEEK_SET) <= 0)) return -1;
    
    if(write(fileid(), request->hashcode, 64) != 64) return -1;

    return 0;
}

int Server::remove(int connect_id, const char * path)
{
    serverlog((string("remove ") + to_string(clients[connect_id].user.user_id) + '/' + path).c_str(), connect_id);
    return this->remove((to_string(clients[connect_id].user.user_id) + '/' + path).c_str());
}

#include <dirent.h>

int Server::remove(const char * path)
{

    DIR * directory;
    struct dirent * directory_entry;
    struct stat file_state;

    if(lstat(path, &file_state)) return false;
    if(!S_ISDIR(file_state.st_mode)) 
    {
        ::remove(path);
        ::remove((string(path) + ".stat").c_str());

        return 0;
    }
    if (nullptr == (directory = opendir(path)))
        return -1;

    while ((directory_entry = readdir(directory)) != nullptr)
    {
        if (!strcmp(".", directory_entry->d_name) \
            || !strcmp("..", directory_entry->d_name))
            continue;

        if (0 != this->remove((string(path) + '/' + directory_entry->d_name).c_str()))
        {
            closedir(directory);
            return -1;
        }
    }
    closedir(directory);

    return ::remove(path);
    
}

int Server::count_files(const char * path)
{
    int count;
    DIR * directory;
    struct dirent * directory_entry;
    struct stat file_state;

    if(lstat(path, &file_state)) return 0;

/*  不是目录，直接返回1     */
    if(!S_ISDIR(file_state.st_mode)) return 1;

/*  递归遍历统计子目录中文件数目    */
    count = 1;

    if((directory = opendir(path)) == nullptr) return count;

    while ((directory_entry = readdir(directory)) != nullptr)
    {
        if(strcmp(".", directory_entry->d_name) == 0) continue;
        if(strcmp("..", directory_entry->d_name) == 0) continue;
        count += count_files((string(path) + '/' + directory_entry->d_name).c_str());
    }
    
    closedir(directory);

    return count;
}


int Server::check(int connect_id, Service::protocol_header * request)
{
    int count, sum;
    Service service;
    char path[256];

    strncpy(path, request->file_name, 256);

/*  统计文件总数，用于判断原目录是否缺少文件    */
    sum = count_files(update_path(connect_id, path)) / 2;

/*  已检查文件数目  */
    count = 0;

    do
    {
/*  检查文件是否一致，循环读取直到客户停止文件检查请求    */
        if(service_check(connect_id, request)) return -1;
        if(read(connect_id, service.getRecive(), HEAD_SIZE) != HEAD_SIZE) return -1;
        request = service.getRecive();

        ++count;

    } while (!strcmp(service.getRecive()->request, "check"));

    if (count == sum) service.set(0, 0, Service::Sucs);
    else service.set(0, 0, Service::Fail);

    return (write(connect_id, service.getOut(), HEAD_SIZE) != HEAD_SIZE);
    
}