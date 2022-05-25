#pragma once

#include<string>
#include<queue>

#include "client.h"
#include "response.h"

#define RECVBUF_SZE 1024



class ControlClient: public Client{

public:
    ControlClient();
    ~ControlClient();
    
    bool                SendReq(const std::string& cmd);
    bool                RecvResponse(int expected_code, std::string* msg = nullptr);
    RESPONSE_TYPE       DecodeResponse(const char* s);
    
    bool                HasMoreResponse();
    RESPONSE_TYPE       PopResponse();
    void                PushResponse(const RESPONSE_TYPE& res);

private:
    char *recv_buf{nullptr};
    int recv_cnt{0};
    std::queue<RESPONSE_TYPE> res_queue;

};