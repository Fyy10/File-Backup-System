#include <unistd.h>
#include <fcntl.h>
#include "duplicator.hpp"

int copyfile(int argc, char ** argv)
// int main(int argc, char ** argv)
{
    int src = open(argv[1], O_RDONLY);
    int dest = open(argv[2], O_WRONLY | O_CREAT, 0644);
    Duplicator().exportContent(src, dest);
    close(src);
    close(dest);
    return 0;
}