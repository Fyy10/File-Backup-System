#ifndef HASHLIST_HPP
#define HASHLIST_HPP

#include <map>
#include <string>

class HashList
{

    public:

    enum { Dir, File };
    
    bool equals(const HashList & other) const;

    void add(const std::string & path, int flag);

    void erase(const std::string & path);

    static std::string gethashcode(const std::string & path);

    bool contains(std::pair<std::string, std::string> filecode);
    bool contains(const std::string & path, int flag);

    private:
    
    std::map<std::string, std::string> hashlist;

};

#endif