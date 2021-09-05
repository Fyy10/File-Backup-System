#include "localgenerator.hpp"
#include "duplicator.hpp"
#include "filter.hpp"

#include <iostream>
using namespace std;

void build(Generator && g, FileFilter && f, Export && e)
{
    g.build(f, e);
}
int main(int argc, char ** argv)
{  
    build(LocalGenerator(argv[1], argv[2]),FileFilter(),Duplicator());
}