#include <unistd.h>
#include <sys/stat.h>

#include "setter.hpp"
#include "generator.hpp"

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
}

bool Generator::autobuild(string & src, string & dest, Export & destination)
{
    struct stat src_file_state;

    if(lstat(src.c_str(), &src_file_state)) return false;

    if(S_ISREG(src_file_state.st_mode))
        return create_normal_file(src, dest, src_file_state, destination);

    else if(S_ISDIR(src_file_state.st_mode))
        return recursion_of_dir(src, dest, src_file_state, destination);

    else if(S_ISLNK(src_file_state.st_mode))
        return create_soft_link(src, dest, src_file_state, destination);

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