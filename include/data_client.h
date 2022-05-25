#pragma once

#include<string>


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