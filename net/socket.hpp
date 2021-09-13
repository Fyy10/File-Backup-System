#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <sys/socket.h>

#define Default 10u
#define Timeout 30u
#define Port 12800u

class Socket
{

    public:
    
    Socket();
    bool Success() const { return (socket_id > 0);}

    int Connect(unsigned int timeout = Timeout);
    
    int Listen(int backlog = Default);
    
    int Accept();

    int Bindto();

    int getfd()
    {
        return socket_id;
    }

    ~Socket();
    
    int setLocalAddress(unsigned int ip_number,unsigned short port = Port);
    
    int setLocalAddress(const char * ip_string, unsigned short port = Port);

    int setRemoteAddress(unsigned int ip_number, unsigned short port = Port);

    int setRemoteAddress(const char * ip_string, unsigned short port = Port);

    void printRemote();
    void printLocal();

    private:
    int setaddress(unsigned int ip_number,unsigned short port, struct sockaddr_in * address);
    int setaddress(const char * ip_string, unsigned short port, struct sockaddr_in * address);

    private:

    int socket_id;

    struct sockaddr remote_address, local_address;
    
    socklen_t sockaddr_length = sizeof(struct sockaddr);

};

#endif