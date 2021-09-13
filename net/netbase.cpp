#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#include "fileid.hpp"
#include "netbase.hpp"

int NetBase::service_recive(int socket_id, Service::protocol_header * request)
{
    printf("%s %s\n", request->file_name, request->request);
/*  接收请求协议头，根据请求类型执行不同操作，读取失败返回错误码    */
    if(strcmp(request->request, "file") == 0) return recive_normal_file(socket_id, request);
    
    else if(!strcmp(request->request, "hard")) return recive_hard_link(socket_id, request);
    
    else if(!strcmp(request->request, "dir")) return recive_directory(socket_id, request);
    
    else if(!strcmp(request->request, "soft")) return recive_soft_link(socket_id, request);
    
    else if(!strcmp(request->request, "fifo")) return recive_fifo_pipe(socket_id, request);

    else return 0;

}

int NetBase::recive_normal_file(int socket_id, Service::protocol_header * request)
{
    int dest_fd, result, n;
    struct stat file_state;
    char buffer[4096];

    file_id tempfd(request->file_name, O_WRONLY | O_CREAT, 0644);

    if((dest_fd = tempfd()) < 0) return eOpen;

/*  读取文件属性结构，保存到file_state中，读取出错返回错误码表示失败    */
    if(read(socket_id, &file_state, sizeof(file_state)) != sizeof(file_state)) 
        return eRead;

    n = request->length;

/*  接收文件内容，写入到目标文件中，连续读写直到接收字节数达到指定的数目  */
    while(n > 0)
    {   memset(buffer, 0, 4096);
        result = read(socket_id, buffer, (n > 4096) ? 4096 : n);
        if(result <= 0) return eRead;
        if(write(dest_fd, buffer, result) != result) return eWrite;
        n -= result;
    }
/*  调用虚函数set_stat采取不同的操作，服务器将属性保存到新文件中，客户端重设文件属性 */
    return set_stat(request->file_name, file_state);
}

int NetBase::recive_hard_link(int socket_id, Service::protocol_header * request)
{
    char buffer[256];
    struct stat file_state;

    if(read(socket_id, &file_state, sizeof(file_state)) != sizeof(file_state))
        return eRead;

    memset(buffer, 0 , 256);

/*  读取链接到源文件名，建立硬链接，处理文件状态信息    */
    if(read(socket_id, buffer, request->length) != request->length) return eRead;

    printf("name %s\n", buffer);
    if(link(update_path(socket_id, buffer), request->file_name) != 0) return eLink;

    return set_stat(request->file_name, file_state);
}

int NetBase::recive_directory(int socket_id, Service::protocol_header * request)
{
    printf("recive dir\n");
    struct stat file_state;

    if(mkdir(request->file_name, 0777) != 0) return eMkdir;
    if(read(socket_id, &file_state, sizeof(file_state)) != sizeof(file_state))
        return eRead;
    printf("dirname %s\n", request->file_name);

    printf("mtim %dsetstat : %d\n", file_state.st_mtim, set_stat(request->file_name, file_state));

    return Success;
}

int NetBase::recive_soft_link(int socket_id, Service::protocol_header * request)
{
    char buffer[256];
    struct stat file_state;
    
    if(read(socket_id, &file_state, sizeof(file_state)) != sizeof(file_state))
        return eRead;

    memset(buffer, 0 , 256);

    if(read(socket_id, buffer, request->length) != request->length) return eRead;

    if(symlink(buffer, request->file_name) != 0) return eSymlink;

    return set_stat(request->file_name, file_state);

}

int NetBase::service_send(int socket_id, const char * path)
{
    struct stat file_state;
    if(lstat(path, &file_state) != 0) return elstat;

    if(S_ISREG(file_state.st_mode))
        return  send_normal_file(socket_id, path, file_state);

    else if(S_ISDIR(file_state.st_mode))
        return send_directory(socket_id, path, file_state);
    
    else if(S_ISLNK(file_state.st_mode))
        return send_sofk_link(socket_id, path, file_state);
    
    else return 0;

}

int NetBase::send_normal_file(int socket_id, const char * path, struct stat & file_state)
{
    int n, result, ishard;
    struct stat * real_file_state;
    char buffer[4096];
    Service service;
    file_id tempfd(path, O_RDONLY, -1);
    int fileid = tempfd();
    
    if(ends_with_stat(path)) return Success;

    if(fileid < 0) return eOpen;

/*  硬链接，发送相应请求，无需发送文件内容  */
    ishard = check_inode(socket_id, file_state.st_ino);

/*  设置消息头部信息，表明需要采取的操作和传输的数据大小,若文件为硬链接则只需要发送链接到的文件名，
    长度为文件名的长度   */
    if(ishard) 
    {
        n = file_state.st_ino;
        service.set(path, inode_get(socket_id, file_state.st_ino).length(), Service::Hard);
    }
    else 
    {
        service.set(path, file_state.st_size, Service::Norm);

    /*  并在inode表中增加次文件的inode，用于判断此文件的硬链接  */
        add_inode(socket_id, file_state.st_ino, std::string(path));
    }

    if(write(socket_id, service.getOut(), sizeof(Service::protocol_header)) \
        != sizeof(Service::protocol_header)) return eWrite;

    if((real_file_state = load_real_stat(path, file_state)) == nullptr) return Fail;

/*  传输文件属性结构体  */
    if(write(socket_id, real_file_state, sizeof(struct stat)) != sizeof(struct stat))
    {
        return eWrite;
    }

/*  硬链接发送内容后直接返回，不能执行后面发送文件内容的代码    */
    if(ishard)
    {
        if(write(socket_id, inode_get(socket_id, n).c_str(), service.getOut()->length) 
            != service.getOut()->length) return eWrite;
        else return Success;
    }

/*  非硬链接文件发送文件内容，读取文件内容发送服务器  */
    n = file_state.st_size;

    while(n > 0)
    {
        memset(buffer, 0, 4096);
        result = read(fileid, buffer, (n > 4096) ? 4096 : n);
        if(result < 0) return eRead;
        if(write(socket_id, buffer, result) != result) return eWrite;
        n -= result;
    }

    return Success;

}

int NetBase::send_directory(int socket_id, const char * path, struct stat & file_state)
{
    DIR * directory;
    struct dirent * directory_entry;
    Service service;
    int result;
    struct stat * real_file_state;
    
    if((directory = opendir(path)) == nullptr) return eOpen;
    if((real_file_state = load_real_stat(path, file_state)) == nullptr) return Fail;

/*  设置请求报文头，将请求发送到远程连接    */
    service.set(path, file_state.st_size, Service::Dir);

    if(write(socket_id, service.getOut(), sizeof(Service::protocol_header)) \
        != sizeof(Service::protocol_header)) return eWrite;

/*  将文件属性结构体发送到远程连接  */
    if(write(socket_id, real_file_state, sizeof(struct stat)) != sizeof(struct stat))
        return eWrite;

/*  遍历目录项，递归执行发送服务    */
    while((directory_entry = readdir(directory)))
    {   
        if(!strcmp(directory_entry->d_name, ".") || !strcmp(directory_entry->d_name, ".."))
            continue;
        std::string newpath(path);
        newpath = newpath + '/' + directory_entry->d_name;
    /*  成功则继续遍历，失败则停止执行  */
        if((result = service_send(socket_id, newpath.c_str())) != 0) return result;
    }

    return Success;
}
int NetBase::send_sofk_link(int socket_id, const char * path, struct stat & file_state)
{
    Service service;
    char buffer[256];
    struct stat * real_file_state;
    int length = readlink(path, buffer, 256);

    if(length == -1) return eRead;
    if((real_file_state = load_real_stat(path, file_state)) == nullptr) return Fail;

    service.set(path, length, Service::Soft);

    if(write(socket_id, service.getOut(), sizeof(Service::protocol_header)) \
        != sizeof(Service::protocol_header)) return eWrite;
    
/*  将文件属性结构体发送到远程连接  */
    if(write(socket_id, real_file_state, sizeof(struct stat)) != sizeof(struct stat))
        return eWrite;

/*  发送链接到的文件名  */
    if(write(socket_id, buffer, length) != length) return eWrite;

    return Success;
}

bool NetBase::ends_with_stat(const char * path) const
{
    int length = strlen(path) - 1;
    const char _stat[] = ".stat";
    if(length < 4) return false;
    for(int i = 0; i < 5; ++i)
    {
        if(path[length - i] != _stat[4 - i]) return false;
    }

    return true;
}