#ifndef FILTER_HPP
#define FILTER_HPP

#include <regex>
#include <string>
#include <sys/stat.h>

class FileFilter
{
    public:    
    FileFilter() : typefilter(".+"), timefilter({0, 0}) {}
    FileFilter(const std::string & pattern, const struct timespec & timefilter)
        :typefilter(pattern), timefilter(timefilter) {}
    FileFilter(const std::regex & re, const struct timespec & timefilter)
        :typefilter(re), timefilter(timefilter) {}

    ~FileFilter() = default;

    bool ismatch(const struct stat & state,const char * name) const;
    void settime(const struct timespec & timefilter);
    void settype(const std::regex & typefilter);
    void reset(const std::regex &,const struct timespec & timefilter);


    private:
    int compareTime(const timespec & compareto) const;

    private:
    std::regex typefilter;
    struct timespec timefilter;
};

#endif