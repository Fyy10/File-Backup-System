#include <unistd.h>
#include <sys/stat.h>

#include "setter.hpp"
#include "generator.hpp"

#include <iostream>

using namespace std;

void Generator::initialize()
{
/*  删除路径名末尾的‘/’字符，保证递归构建过程中文件名的正确性 */
    if(oldpath[oldpath.length() - 1] == '/') oldpath.pop_back();
    if(newpath[newpath.length() - 1] == '/') newpath.pop_back();
    int position = oldpath.rfind("/");
    if(position != string::npos)
        newpath = newpath + oldpath.substr(position, oldpath.length());
    else newpath = newpath + '/' + oldpath;
    cout << newpath << ":" << oldpath << '\n';
}

bool Generator::autobuild(string & src, string & dest, Export & destination)
{
    bool result;
    struct stat src_file_state;

    if(lstat(src.c_str(), &src_file_state)) return false;

    if(S_ISREG(src_file_state.st_mode))
        result = create_normal_file(src, dest, src_file_state, destination);
    else if(S_ISDIR(src_file_state.st_mode))
        result = recursion_of_dir(src, dest, src_file_state, destination);

    else if(S_ISLNK(src_file_state.st_mode))
        result = create_soft_link(src, dest, src_file_state, destination);
    else if(S_ISFIFO(src_file_state.st_mode))
        result = create_fifo_pipe(src, dest, src_file_state, destination);

    if(result == false) return false;
    if(S_ISDIR(src_file_state.st_mode)) hashlist.add(src, HashList::Dir);
    else hashlist.add(src, HashList::File); 

    return true;
    
}
int Generator::add_inodes(const char * src, const char * dest, struct stat & src_file_state)
{
    struct stat dest_file_state;
    if(lstat(src, &dest_file_state) != 0) return -1;
    inodes_map[src_file_state.st_ino] = make_pair(dest_file_state.st_ino, dest);
    return 0;
}
bool Generator::build(Export & destination)
{
    if(access(newpath.c_str(), F_OK) == 0) 
    {
        printf("Operation failed : Destination exists\n");
        return false;
    }
    if(autobuild(oldpath, newpath, destination) == false)
        return errorProcessing();
    return true;
}

#include <dirent.h>

bool Generator::check(const string & path)
{
    DIR * directory;
    struct dirent * directory_entry;
    struct stat file_state;

    if(lstat(path.c_str(), &file_state)) return false;

    if(!filter.ismatch(file_state, path.c_str())) return true;

    if(!S_ISDIR(file_state.st_mode)) 
        return hashlist.contains(path, HashList::File);
    
    if(!hashlist.contains(path.c_str(), HashList::Dir)) return false;

    if((directory = opendir(path.c_str())) == nullptr) return false;

    while((directory_entry = readdir(directory)) != nullptr)
    {
        if(isparentdir(directory_entry->d_name)) continue;
        if(check(path + '/' + directory_entry->d_name) == false)
        {   
            closedir(directory); 
            return false;
        }
    }
    
    closedir(directory);
    return true;
}