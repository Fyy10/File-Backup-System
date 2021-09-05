#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "setter.hpp"
#include "localgenerator.hpp"

using namespace std;

bool LocalGenerator::create_normal_file(std::string & src, std::string & dest,
    struct stat & src_file_state, const FileFilter & filter, Export & destination)
{
    int source_file_id, dest_file_id, result;

/*  调用过滤器执行文件过滤，若文件不符合要求，则不需要对此文件采取操作    */
    if(!filter.ismatch(src_file_state, src.c_str())) return true;

/*  对比文件inode表，若表中已存在相同的ionde则说明此文件是一个硬链接，此时不需要再创建文件  */
    if(inodes_map.find(src_file_state.st_ino) != inodes_map.end())
    {
        if(link(inodes_map[src_file_state.st_ino].second.c_str(), dest.c_str()))
            return false;
        return (0 == StatSetter::updateAttributes(dest.c_str(), src_file_state));
    }

/*  打开源文件读取内容，创建目标文件进行写入    */
    if((source_file_id = open(src.c_str(), O_RDONLY)) <= 0) return false;
    if((dest_file_id = open(dest.c_str(), O_CREAT | O_WRONLY, 0644)) <= 0) 
    {
        close(source_file_id);
        return false;
    }

/*  调用输出类将按指定方式导出内容到目标文件，更新文件属性，执行完成后关闭文件描述符号    */
    result = destination.exportContent(source_file_id, dest_file_id);
    result = result ? result : \
        StatSetter::updateAttributes(dest.c_str(),src_file_state);

/*  添加新的inode映射记录   */
    result = result ? result : add_inodes(src.c_str(), dest.c_str(), src_file_state);

    close(source_file_id);
    close(dest_file_id);

    return (result == 0);
}

bool LocalGenerator::recursion_of_dir(std::string & src, std::string & dest,
    struct stat & src_file_state, const FileFilter & filter, Export & destination)
{
    DIR * src_directory;
    struct dirent * src_dirent;
    string newsrc,newdest;

    if((src_directory = opendir(src.c_str())) == nullptr) return false;

/*  创建新目录，并将inode的映射关系加入到inode表中，出错直接返回    */
    if(!mkdir(dest.c_str(),0777) && !add_inodes(src.c_str(), dest.c_str(), src_file_state)) {}
    else 
    {
        closedir(src_directory);
        return false;
    }

/*  遍历目录项，递归处理每个子文件  */
    while((src_dirent = readdir(src_directory)) != nullptr)
    {
        if(isparentdir(src_dirent->d_name)) continue;
        newsrc = src + '/' + src_dirent->d_name;
        newdest = dest + '/' + src_dirent->d_name;
        if(!autobuild(newsrc, newdest, filter, destination))
        {
            closedir(src_directory);
            return false;
        }
    }
/*  先关闭文件夹再修改属性，否则访问时间将被更新    */
    closedir(src_directory);
    return (0 == StatSetter::updateAttributes(dest.c_str(), src_file_state));
}

bool LocalGenerator::create_soft_link(std::string & src, std::string & dest,
    struct stat & src_file_state, const FileFilter & filter, Export & destination)
{
    int n;
    char buf[1024];
    if((n = readlink(src.c_str(), buf, 1024)) == -1) return false;
    buf[n] = 0;
    if(symlink(buf, dest.c_str()) || \
        add_inodes(src.c_str(), dest.c_str(), src_file_state)) return false;
    return (0 == StatSetter::updateOwnnerTime(dest.c_str(), src_file_state));
}

bool LocalGenerator::errorProcessing()
{
    perror("Operation failed");
    printf("Removing intermediate files...\n");
    if(false == Remove(newpath)) perror("\tremove error");
    else printf("\tfinish\n");
    return false;
}

bool LocalGenerator::Remove(string & path)
{
    DIR * directory;
    struct dirent * directory_entry;
    struct stat file_state;
    string newpath;

    if(lstat(path.c_str(), &file_state)) return false;
    if(S_ISDIR(file_state.st_mode))
    {
        if(nullptr == (directory = opendir(path.c_str()))) return false;
        while((directory_entry = readdir(directory)) != nullptr)
        {
            if(isparentdir(directory_entry->d_name)) continue;
            newpath = path + '/' + directory_entry->d_name;
            if(false == Remove(newpath))
            {
                closedir(directory);
                return false;
            }
        }
        closedir(directory);
        if(remove(path.c_str())) return false;
        return true;
    }
    else if(remove(path.c_str())) return false;
    return true;

}
