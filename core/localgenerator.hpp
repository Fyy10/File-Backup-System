#ifndef LOCALGENERATOR_HPP
#define LOCALGENERATOR_HPP

#include "generator.hpp"

class LocalGenerator : public Generator
{
    public:
    using Generator::Generator;
    private:
    virtual bool create_normal_file(std::string & src, std::string & dest,
        struct stat & src_file_state, Export & destination);

    virtual bool recursion_of_dir(std::string & src, std::string & dest,
        struct stat & src_file_state, Export & destination);

    virtual bool create_soft_link(std::string & src, std::string & dest,
        struct stat & src_file_state, Export & destination);
    
    virtual bool errorProcessing();

    bool Remove(std::string & path);
};

#endif