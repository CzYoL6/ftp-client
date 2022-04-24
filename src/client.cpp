#include <unistd.h>
#include <errno.h>
#include <cstring>
#include<fcntl.h>
#include "../include/client.h"


Client::Client()
{
    
}
Client::~Client()
{
    
}

void Client::Init()
{
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sock == -1){
        LOGERR("error creating socket");
        exit(-1);
    }
}

void Client::Connect(const std::string& ip, const int& port)
{
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if(inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr) == -1){
        LOGERR("error inet_pton");
        exit(-1);

    }
    socklen_t len = sizeof(server_addr);

    if(connect(sock, (struct sockaddr*)(&server_addr), len) == -1){
        LOGERR("error establishing connection to %s:%d, error: %d\n ", ip.c_str(), port, errno);
        exit(-1);
    }
    LOGMSG("successfully established connection.\n");
}

void Client::Send(const char *send_buf, int len)
{
    int ret = send(sock, send_buf, len, 0);
    if(ret < 0){
        LOGERR("error sending control msg.\n");
        exit(-1);
    }

}

void Client::Recv(char* recv_buf, int max_len, int& len)
{
    int res = recv(sock, recv_buf,  max_len, 0);
    if(res == -1){
        LOGERR("error receiving.\n");
        exit(-1);
    }
    else if(res == 0){
        LOGERR("server shut down.\n");
        exit(0);
    }
    len = res;
}

int Client::SetSockNonBlocking()
{
    int flags = fcntl(sock, F_GETFL);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
    return flags;
}

void Client::SetSockBlocking(int pre_flags)
{
    fcntl(sock, F_SETFL, pre_flags);
}

void Client::Close()
{
    close(sock);
}