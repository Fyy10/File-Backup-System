#ifndef GENERATOR_HPP
#define GENERATOR_HPP

#include <map>
#include <string>
#include "filter.hpp"
#include "export.hpp"
#include "hashlist.hpp"

class Generator
{

    public:
    
    Generator(const char * oldpath, const char * newpath, const char * key) 
        : Generator(std::string(oldpath), std::string(newpath), key, FileFilter()) {}
    Generator(const std::string & oldpath, const std::string & newpath, const std::string & key)
        : Generator(oldpath, newpath, key, FileFilter()) {}
    Generator(const std::string & oldpath, const std::string & newpath, const std::string & key, const FileFilter & f)
        : oldpath(oldpath), newpath(newpath), key(key), filter(f) {  initialize();   }
    ~Generator() = default;

    bool build(Export & destination); 

    bool check(const std::string & path);

    protected:

    bool isparentdir(const char * path)
    {
        if(path[0] == '.' && (path[1] == 0 || \
            (path[1] == '.' && path[2] == 0))) return true;
        else return false;
    }
    void initialize();    

    bool autobuild(std::string & src, std::string & dest, Export & destination);
    
    virtual bool create_normal_file(std::string & src, std::string & dest,
        struct stat & src_file_state, Export & destination) = 0;

    virtual bool recursion_of_dir(std::string & src, std::string & dest,
        struct stat & src_file_state, Export & destination) = 0;

    virtual bool create_soft_link(std::string & src, std::string & dest,
        struct stat & src_file_state, Export & destination) = 0;
    
    virtual bool create_fifo_pipe(std::string & src, std::string & dest,
        struct stat & src_file_state, Export & destination) = 0;
        
    virtual bool errorProcessing() = 0;

    int add_inodes(const char * src, const char * dest, struct stat & src_file_state);

    protected:

    FileFilter filter;
    HashList hashlist;
    std::string oldpath, newpath, key;
    std::map<ino_t, std::pair<ino_t, std::string>> inodes_map;

};

#endif