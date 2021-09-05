#ifndef DUPLICATOR_HPP
#define DUPLICATOR_HPP

#include "export.hpp"

class Duplicator : public Export
{

    public:
    virtual int exportContent(int src, int dest);
    private:
    enum {BUFFSIZE = 1 << 12};
};

#endif