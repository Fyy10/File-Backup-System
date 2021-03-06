#ifndef EXPORT_HPP
#define EXPORT_HPP

class Export
{
    public:
    virtual int exportContent(int src, int dest, const char * key) = 0;
    virtual ~Export() = default;
};

#endif