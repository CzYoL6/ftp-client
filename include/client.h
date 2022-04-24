#pragma once

#include<string>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory>

#include "../include/logger.h"

class Client{

public:
    Client();
    ~Client();
    void Init();
    void Connect(const std::string& ip, const int& port);
    void Send(const char* send_buf, int len);
    void Recv(char* recv_buf, int max_len, int& len);
    int SetSockNonBlocking();
    void SetSockBlocking(int pre_flags);
    void Close();

    int get_sock(){return sock;}
private:
    int sock{-1};
    sockaddr_in server_addr;
};