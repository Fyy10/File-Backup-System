#include "map_file.hpp"
#include <stdio.h>

MapFile::MapFile(const string & path)
{
    file_path = path;
    FILE* file = fopen(file_path.c_str(), "r");
    // file doesn't exist
    if (file == NULL)
    {
        // create a file with head notification
        FILE* file = fopen(file_path.c_str(), "w");
        fprintf(file, "__JSON__ __HEAD__\n");
        fclose(file);
    }
    else
    {
        fclose(file);
    }
}

map<string, string> MapFile::load()
{
    FILE* file = fopen(file_path.c_str(), "r");
    map<string, string> file_map;
    char s1[STRLEN_MAP_FILE], s2[STRLEN_MAP_FILE];
    while (1)
    {
        fscanf(file, "%s %s\n", s1, s2);
        file_map[s1] = s2;
        if (feof(file)) break;
    }
    fclose(file);
    return file_map;
}

int MapFile::save(map<string, string>& data)
{
    FILE *file = fopen(file_path.c_str(), "w");

    for (map<string, string>::const_iterator iter = data.begin(); iter != data.end(); iter++)
    {
        fprintf(file, "%s %s\n", iter->first.c_str(), iter->second.c_str());
    }

    fclose(file);
    return 0;
}