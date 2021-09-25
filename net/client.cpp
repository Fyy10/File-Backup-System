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

    return (socket.Connect() == 0);
}

unsigned int Client::request(unsigned int user_id, const char * password)
{
    int socket_id = socket.getfd();
    struct account user_account = {.user_id = user_id};

/* 检查密码长度是否大于32，长度过大则请求失败   */
    if(strlen(password) >= LENGTH) return -eOverflow;
    strncpy(user_account.password, password, LENGTH);
    
/*  发送用户帐号密码到服务器，接收服务器响应，根据服务器相应判断登录是否成功    */
    if(write(socket_id, &user_account, sizeof(user_account)) \
        != sizeof(user_account)) return -eWrite;
    if(read(socket_id, service.getRecive(), HEAD_SIZE) != HEAD_SIZE)
        return -eRead;
    if(strcmp(service.getRecive()->request, "success")) return Fail;

    return service.getRecive()->length;
}

int Client::set_stat(const char * path, struct stat & file_state)
{
    if(S_ISLNK(file_state.st_mode)) 
        return StatSetter::updateOwnnerTime(path, file_state) ? eSetst : Success;
    return StatSetter::updateAttributes(path, file_state) ? eSetst : Success;
}

int Client::backup(const char * path)
{
    char pwd[256];
    string path_str(path), newpath;

    if(getcwd(pwd, 256) == nullptr) return -1;

    if(path_str[path_str.length() - 1] == '/') path_str.pop_back();

    int pos = path_str.rfind('/');
    if(pos != string::npos) 
    {
        newpath = string(path_str.c_str() + pos + 1);
        origin_prefix = path_str.erase(pos);

        if(chdir(path_str.c_str()) != 0) return -1;

    }
    else newpath = path_str;

    if(service_send(socket.getfd(), newpath.c_str()) == 0) 
    {
        chdir(pwd);
        return 0;
    }

    chdir(pwd);
    return -1; 
}

int Client::recover(const char * path)
{
    int pos;
    string newpath, path_str(path);
    if((pos = path_str.rfind('/')) != string::npos)
    {
        newpath = string(path_str.c_str() + pos + 1);
    }
    else newpath = path;
/*  发送恢复命令到服务器，并指明需要恢复的文件名    */
    service.set(newpath.c_str(), 0, Service::Reco);
    if(write(socket.getfd(), service.getOut(), HEAD_SIZE) != HEAD_SIZE) 
        return eWrite;
    
/* 迭代接收文件直到文件传送完毕 */
    while(1)
    {
        if(read(socket.getfd(), service.getRecive(), HEAD_SIZE) != HEAD_SIZE)
            return eRead;
        
        if(!strcmp(service.getRecive()->request, "success")) break; 
        if(!strcmp(service.getRecive()->request, "failure")) return Fail;
/* 设置文件恢复到的位置 */   
        update_path(socket.getfd(), service.getRecive()->file_name);
        
        service_recive(socket.getfd(), service.getRecive());
    }

    return Success;
    
}

bool Client::check_file(const string & path)
{
    bool result;
    char pwd[256];
    string path_str(path), newpath;

    if(getcwd(pwd, 256) == nullptr) return -1;

    if(path_str[path_str.length() - 1] == '/') path_str.pop_back();
    int pos = path_str.rfind('/');

    if(pos != string::npos) 
    {
        newpath = string(path_str.c_str() + pos + 1);
        path_str.erase(pos);
        if(chdir(path_str.c_str()) != 0) return -1;

    }
    else newpath = path_str;

    result = check(newpath);

    service.set(0, 0, Service::Sucs);

    if(write(socket.getfd(), service.getOut(), HEAD_SIZE) != HEAD_SIZE)
        result = false;
        
    if((read(socket.getfd(), service.getRecive(), HEAD_SIZE) != HEAD_SIZE) 
        || strcmp("success", service.getRecive()->request))
        result = false;

    chdir(pwd);
    return result;

}

bool Client::check(const string & path)
{
    DIR * directory;
    struct dirent * directory_entry;
    struct stat file_state;

    if(lstat(path.c_str(), &file_state)) return false;

    if(!S_ISDIR(file_state.st_mode)) 
    {
        if(S_ISFIFO(file_state.st_mode)) return service_check(path, 1);
        else return service_check(path, 0);
    }
    if(service_check(path, 1) == false) return false;

    if((directory = opendir(path.c_str())) == nullptr) return false;

    while((directory_entry = readdir(directory)))
    {
        if(!strcmp(directory_entry->d_name, ".") || 
            !strcmp(directory_entry->d_name, "..")) continue;
        
        if(!check(path + '/' + directory_entry->d_name))
        {
            closedir(directory);
            return false;
        }
    }

    closedir(directory);
    return true;

}

#include "hashlist.hpp"

bool Client::service_check(const std::string & path, int flag)
{
    Service sservice;
    sservice.set(path.c_str(), flag, Service::Check);
    if(flag == 0) sservice.setcode(HashList::gethashcode(path));

    if(write(socket.getfd(), sservice.getOut(), HEAD_SIZE) != HEAD_SIZE)
        return false;
    
    if(read(socket.getfd(), sservice.getRecive(), HEAD_SIZE) != HEAD_SIZE)
    {   
        return false;
    }
    
    if(strcmp(sservice.getRecive()->request, "success") == 0) return true;
    else 
    {
        return false;
    }
}

int Client::update(const char * path)
{
    int result;
    struct stat file_state;
    char pwd[256];

    string newpath(path + origin_prefix.length() + 1);

    if (lstat(path, &file_state)) return -1;
    if ((getcwd(pwd, 256) == nullptr) || chdir(origin_prefix.c_str())) return -1;
    
    if (S_ISDIR(file_state.st_mode))
    {
        service.set(newpath.c_str(), 0, Service::Dir);
        if((write(socket.getfd(), service.getOut(), HEAD_SIZE) != HEAD_SIZE) ||
        (write(socket.getfd(), &file_state, sizeof(struct stat)) != sizeof(struct stat)))
            result = -1;
    }
    else result = service_send(socket.getfd(), path);

    if (chdir(pwd)) return -1;
    else return result;
}

int Client::remove(const char * path)
{
    service.set(string(path + origin_prefix.length() + 1).c_str(), 0, Service::Remv);
    return write(socket.getfd(), service.getOut(), HEAD_SIZE);
}