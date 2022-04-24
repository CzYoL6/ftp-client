#pragma once

#include<string>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "client.h"

#define RECVBUF_SZE 1024

#define RESPONSE_TYPE std::pair<int,std::string>

class ControlClient: public Client{

public:
    ControlClient();
    ~ControlClient();
    
    bool SendReq(const std::string& cmd);
    bool RecvResponse(int expected_code, std::string* msg = nullptr);
    RESPONSE_TYPE DecodeResponse(const char* s);

private:
    char *recv_buf{nullptr};
    int recv_cnt{0};
};