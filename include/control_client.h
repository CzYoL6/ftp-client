#pragma once

#include<string>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "client.h"

class ControlClient: public Client{

public:
    ControlClient();
    ~ControlClient();
    
    void SendReq(const std::string& cmd);
    void RecvResponse(char* recv_buf, int max_len, int& len);

    bool RecvResponseAsync(char* recv_buf, int max_len, int& len);
};