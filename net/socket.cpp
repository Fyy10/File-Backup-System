#include <unistd.h>
#include <netinet/in.h>

#include "socket.hpp"

Socket::Socket()
{
    socket_id = socket(AF_INET, SOCK_STREAM, 0);
}

Socket::~Socket()
{
    close(socket_id);
}

int Socket::Connect(unsigned int timeout)
{
    return connect(socket_id, &remote_address, sockaddr_length);
}

int Socket::Bindto()
{
    return bind(socket_id, &local_address, sockaddr_length);
}

int Socket::Listen(int backlog)
{
    return listen(socket_id, backlog);
}

int Socket::Accept()
{
    return accept(socket_id, &remote_address, &sockaddr_length);
}

int Socket::setLocalAddress(unsigned int ip_number, unsigned short port)
{
    return setaddress(ip_number, port, (struct sockaddr_in *)&local_address);
}

int Socket::setLocalAddress(const char * ip_string, unsigned short port)
{
    return setaddress(ip_string, port, (struct sockaddr_in *)&local_address);
}

int Socket::setRemoteAddress(unsigned int ip_number, unsigned short port)
{
    return setaddress(ip_number, port, (struct sockaddr_in *)&remote_address);
}

int Socket::setRemoteAddress(const char * ip_string, unsigned short port)
{
    return setaddress(ip_string, port, (struct sockaddr_in *)&remote_address);
}

int Socket::setaddress(unsigned int ip_number, unsigned short port, struct sockaddr_in * address)
{
    address->sin_family = AF_INET;
    address->sin_addr.s_addr = htonl(ip_number);
    address->sin_port = htons(port);
    return 0;
}

int Socket::setaddress(const char * ip_string, unsigned short port, struct sockaddr_in * address)
{
    address->sin_family = AF_INET;
    address->sin_port = htons(port);
    return (inet_pton(AF_INET, ip_string, &(address->sin_addr)) != 1);
}

#include "stdio.h"

void Socket::printRemote()
{
    printf("ip : %s\nport : %d\n", inet_ntoa(((struct sockaddr_in *)(&remote_address))->sin_addr), ntohs(((struct sockaddr_in *)&remote_address)->sin_port));
}

void Socket::printLocal()
{
    printf("ip : %s\nport : %d\n", inet_ntoa(((struct sockaddr_in *)(&local_address))->sin_addr), ntohs(((struct sockaddr_in *)&local_address)->sin_port));
}