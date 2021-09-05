#include "filter.hpp"

#include <iostream>
using namespace std;

bool FileFilter::ismatch(const struct stat & state, const char * name) const
{
    if(!std::regex_match(name, typefilter) || \
            compareTime(state.st_mtim) < 0) return false;
    else return true;
    return true;
}

int FileFilter::compareTime(const timespec & compareto) const
{
    if(timefilter.tv_sec < compareto.tv_sec) return 1;
    if(timefilter.tv_sec > compareto.tv_sec) return -1;
    if(timefilter.tv_nsec < compareto.tv_nsec) return 1;
    if(timefilter.tv_nsec > compareto.tv_nsec) return -1;
    else return 0;
}

