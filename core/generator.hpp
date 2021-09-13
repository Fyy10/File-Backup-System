#ifndef GENERATOR_HPP
#define GENERATOR_HPP

#include <map>
#include <string>
#include "filter.hpp"
#include "export.hpp"

class Generator
{

    public:
    
    Generator(const char * oldpath, const char * newpath) 
        : Generator(std::string(oldpath), std::string(newpath), FileFilter()) {}
    Generator(const std::string & oldpath, const std::string & newpath)
        : Generator(oldpath, newpath, FileFilter()) {}
    Generator(const std::string & oldpath, const std::string & newpath,const FileFilter & f)
        : oldpath(oldpath), newpath(newpath), filter(f) {  initialize();   }
    ~Generator() = default;

    bool build(Export & destination); 

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
    
    virtual bool errorProcessing() = 0;

    int add_inodes(const char * src, const char * dest, struct stat & src_file_state);

    protected:

    FileFilter filter;
    std::string oldpath, newpath;
    std::map<ino_t, std::pair<ino_t, std::string>> inodes_map;

};

#endif