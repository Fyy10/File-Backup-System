#include <unistd.h>
#include <string.h>
#include "duplicator.hpp"

int Duplicator::exportContent(int src, int dest, const char * key)
{
    int n;
    char buffer[BUFFSIZE];
    
    do
    {
        memset(buffer, 0, BUFFSIZE);
    /*
        从源文件中读取数据，检查读取字节量，出错则进行错误处理  */
        if((n = read(src, buffer, BUFFSIZE)) <= 0)
        {
            if(n == 0) return 0;
            else return n;
        }
    /*
        读取成功，将读到的数据写入到目标文件中
        检查写操作的返回值，若写入字节量小于指定值，则需进行错误处理    */
        if(n != write(dest, buffer, n))
            return n;

    } while (true);
    
    return 0;

}