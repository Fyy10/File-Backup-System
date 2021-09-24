#include "hashlist.hpp"

#include <stdio.h>

using namespace std;

string HashList::gethashcode(const string & path)
{
    int n;
    FILE * pfile;
    char hashcode[64];
    string md5sum = "md5sum " + path + " | awk \'{print $1}\'";
    
    if((pfile = popen(md5sum.c_str(), "r")) == nullptr) goto fail;

    if((n = fread(hashcode, 1, 64, pfile)) <= 0) goto fail;

    hashcode[n - 1] = 0;

    pclose(pfile);
    return string(hashcode);

fail:

    pclose(pfile);
    return string();
}

void HashList::add(const string & path, int flag)
{
    switch(flag)
    {
    case File:
        hashlist[path] = gethashcode(path);
        break;
    case Dir:
    default:
        hashlist[path] = string();
        break;
    }
}

bool HashList::equals(const HashList & other) const
{
    if(other.hashlist.size() != hashlist.size()) return false;

    decltype(hashlist.begin()) this_iter = hashlist.begin(),
        other_iter = other.hashlist.begin();
    
    for(; this_iter != hashlist.end(); ++this_iter, ++other_iter)
    {
        if(this_iter->first != other_iter->first ||
            this_iter->second != other_iter->second)
            return false;
    }

    return true;
}

bool HashList::contains(pair<string, string> filecode)
{
    if(hashlist.find(filecode.first) == hashlist.end())
        return false;
    return (hashlist[filecode.first] == filecode.second);

}

void HashList::erase(const string & path)
{
    hashlist.erase(path);
}

bool HashList::contains(const std::string & path, int flag)
{
    if(hashlist.find(path) == hashlist.end()) return false;
    if(flag == Dir) return true;
    else return (hashlist[path] == gethashcode(path));
}