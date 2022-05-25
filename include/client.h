#pragma once

#include<string>

#include <memory>

#include "net.h"
#include "logger.h"

class Client{

public:
    Client();
    ~Client();
    bool Init();
    bool Connect(const std::string& ip, const int& port);
    bool Send(const char* send_buf, int len);
    bool Recv(char* recv_buf, int max_len, int& len);
    // int SetSockNonBlocking();
    // void SetSockBlocking(int pre_flags);
    void Close();

    int get_sock(){return sock;}
private:
    _socket sock{_invalid_socket};
    sockaddr_in server_addr;
};