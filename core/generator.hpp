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
        : Generator(std::string(oldpath), std::string(newpath)) {}
    Generator(const std::string & oldpath, const std::string & newpath)
        : oldpath(oldpath), newpath(newpath) {
            initialize();
        }
    ~Generator() = default;

    bool build(const FileFilter & filter, Export & destination); 

    protected:

    virtual bool errorProcessing() = 0;

    bool isparentdir(const char * path)
    {
        if(path[0] == '.' && (path[1] == 0 || \
            (path[1] == '.' && path[2] == 0))) return true;
        else return false;
    }
    void initialize();    

    bool autobuild(std::string & src, std::string & dest,
        const FileFilter & filter, Export & destination);
    
    virtual bool create_normal_file(std::string & src, std::string & dest,
        struct stat & src_file_state, const FileFilter & filter, Export & destination) = 0;

    virtual bool recursion_of_dir(std::string & src, std::string & dest,
        struct stat & src_file_state, const FileFilter & filter, Export & destination) = 0;

    virtual bool create_soft_link(std::string & src, std::string & dest,
        struct stat & src_file_state, const FileFilter & filter, Export & destination) = 0;
    
    int add_inodes(const char * src, const char * dest, struct stat & src_file_state);

    protected:
    std::string oldpath, newpath;
    std::map<ino_t, std::pair<ino_t, std::string>> inodes_map;

};

#endif