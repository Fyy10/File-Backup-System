#ifndef LOCALGENERATOR_HPP
#define LOCALGENERATOR_HPP

#include "generator.hpp"

class LocalGenerator : public Generator
{
    public:
    using Generator::Generator;

    int update(std::string & src, std::string & dest, Export & destination)
    {
        struct stat file_state;
        if(lstat(src.c_str(), &file_state)) return -2;

        if(!filter.ismatch(file_state, src.c_str())) return -1;

        return (!autobuild(src, dest, destination));
    }

    int remove(std::string & path)
    {
        struct stat file_state;
        if(lstat(path.c_str(), &file_state)) return -2;
        if(filter.ismatch(file_state, path.c_str())) return -1;
        return (!Remove(path));
    }

    private:
    virtual bool create_normal_file(std::string & src, std::string & dest,
        struct stat & src_file_state, Export & destination);

    virtual bool recursion_of_dir(std::string & src, std::string & dest,
        struct stat & src_file_state, Export & destination);

    virtual bool create_soft_link(std::string & src, std::string & dest,
        struct stat & src_file_state, Export & destination);
    
    virtual bool create_fifo_pipe(std::string & src, std::string & dest,
        struct stat & src_file_state, Export & destination);
    
    virtual bool errorProcessing();

    bool Remove(std::string & path);
};

#endif