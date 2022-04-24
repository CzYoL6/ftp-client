#include <unistd.h>
#include <errno.h>
#include <cstring>
#include<fstream>
#include "../include/data_client.h"


DataClient::DataClient()
{
    //SetSockNonBlocking();
}

DataClient::~DataClient()
{
    
}


void DataClient::SendData(std::fstream& stream, int& left)
{
    char tmp_buf[1024];
    int len_to_send;
    if(left < 1024){
        stream.read(tmp_buf, left);
        len_to_send = left;
    }
    else{
        stream.read(tmp_buf, 1024);
        len_to_send = 1024;
    }

    int res = send(Client::get_sock(), tmp_buf, len_to_send, 0);
    if(res == -1){
        LOGERR("error sending.");
        exit(-1);
    }
    else{
        left -= res;
    }
}

bool DataClient::RecvData(std::fstream& stream, int& total_len)
{
    char tmp_buf[1024];
    // Client::Recv(tmp_buf, 1024, len);
    int res = recv(Client::get_sock(), tmp_buf, 1024, 0);
    if(res == 0){
        LOGMSG("server shut down the connection, file transfer finished.");
        return true;
    }
    else if(res == -1){
        LOGERR("error receiving data.\n");
        exit(-1);
    }
    else {
        total_len += res;
        LOGMSG("received %d bytes, total %d bytes.", res, total_len);
        stream.write(tmp_buf, res);
        return true;
    }
}




bool DataClient::RecvDir(char* dirs, int max_len, int &total_len)
{
    // Client::Recv(tmp_buf, 1024, len);
    int res = recv(Client::get_sock(), dirs, max_len, 0);
    if(res == 0){
        LOGMSG("server shut down the connection, file list transfer finished.");
        //exit(0);
        return true;
    }
    else if(res == -1){
        LOGERR("error receiving data.");
        exit(-1);
    }
    else {
        total_len += res;
        LOGMSG("received %d bytes", res);
        return false;
    
    }
}

