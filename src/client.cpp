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

bool Client::Init()
{
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sock == -1){
        LOGERR("error creating socket");
        return false;
    }
    return true;
}

bool Client::Connect(const std::string& ip, const int& port)
{
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if(inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr) == -1){
        LOGERR("error inet_pton");
        return false;

    }
    socklen_t len = sizeof(server_addr);

    if(connect(sock, (struct sockaddr*)(&server_addr), len) == -1){
        LOGERR("error establishing connection to %s:%d, error: %d\n ", ip.c_str(), port, errno);
        return false;
    }

    LOGMSG("successfully established connection.\n");
    return true;
}

bool Client::Send(const char *send_buf, int len)
{
    int ret = send(sock, send_buf, len, 0);
    if(ret < 0){
        LOGERR("error sending control msg.\n");
        return false;
    }
    return true;

}

bool Client::Recv(char* recv_buf, int max_len, int& len)
{
    int res = recv(sock, recv_buf,  max_len, 0);
    if(res == -1){
        LOGERR("error receiving.\n");
        return false;
    }
    else if(res == 0){
        LOGERR("server shut down.\n");
        return true;
    }
    len = res;
    return true;
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