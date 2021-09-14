// load/save map from/to a file
#ifndef MAP_FILE_HPP
#define MAP_FILE_HPP

#include <string>
#include <map>
#define STRLEN 64

using namespace std;

class MapFile
{
public:
    MapFile(const char* path) : MapFile(string(path)) {};
    MapFile(const string & path);
    ~MapFile() = default;

    map<string, string> load();
    int save(map<string, string>&);

private:
    string file_path;
};

#endif // MAP_FILE_HPP