#include "fileid.hpp"
#include "setter.hpp"
#include "client.hpp"
#include "account.h"
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include <string>

using namespace std;

bool Client::connectServer()
{
    if((socket.Success() == false) || \
        socket.setRemoteAddress(server_address, port)) return false;
    socket.printRemote();
    return (socket.Connect() == 0);
}

int Client::request(unsigned int user_id, const char * password)
{
    int socket_id = socket.getfd();
    int length = sizeof(typename Service::protocol_header);
    struct account user_account = {.user_id = user_id};

/* 检查密码长度是否大于32，长度过大则请求失败   */
    if(strlen(password) >= LENGTH) return -eOverflow;
    strncpy(user_account.password, password, LENGTH);
    
/*  发送用户帐号密码到服务器，接收服务器响应，根据服务器相应判断登录是否成功    */
    if(write(socket_id, &user_account, sizeof(user_account)) \
        != sizeof(user_account)) return -eWrite;
    if(read(socket_id, service.getRecive(), length) != length)
        return -eRead;
    if(strcmp(service.getRecive()->request, "success")) return Fail;

    return Success;
}

int Client::set_stat(const char * path, struct stat & file_state)
{
    if(S_ISLNK(file_state.st_mode)) 
        return StatSetter::updateOwnnerTime(path, file_state) ? eSetst : Success;
    return StatSetter::updateAttributes(path, file_state) ? eSetst : Success;
}

int Client::backup(const char * path)
{
    string path_str(path);
    if(path_str[path_str.length() - 1] == '/') path_str.pop_back();

    if(service_send(socket.getfd(), path_str.c_str()) == 0) return 0;

    perror("send");
    return -1; 
}

int Client::recover(char * path)
{
/*  发送恢复命令到服务器，并指明需要恢复的文件名    */
    service.set(path, 0, Service::Reco);
    if(write(socket.getfd(), service.getOut(), sizeof(Service::protocol_header))
        != sizeof(Service::protocol_header)) return eWrite;
    
    while(1)
    {
        if(read(socket.getfd(), service.getRecive(), sizeof(Service::protocol_header)) 
            != sizeof(Service::protocol_header)) return eRead;
        if(!strcmp(service.getRecive()->request, "success") || 
            !strcmp(service.getRecive()->request, "failure")) break;
        update_path(socket.getfd(), service.getRecive()->file_name);
        
        service_recive(socket.getfd(), service.getRecive());
    }

    return Success;
    
}