#pragma once

#include<string>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<fstream>

#include "client.h"
#include"logger.h"

class DataClient: public Client{

public:
    DataClient();
    ~DataClient();
    
    bool SendData(std::fstream& stream, int& total_len);
    bool RecvData(std::fstream& stream, int& total_len);
    bool RecvDir(char* dirs, int max_len, int& total_len);
};