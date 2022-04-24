#include <unistd.h>
#include <errno.h>
#include <cstring>

#include "../include/control_client.h"


ControlClient::ControlClient()
{
    
}

ControlClient::~ControlClient()
{
    
}

void ControlClient::SendReq(const std::string& cmd)
{
    Client::Send(cmd.c_str(), cmd.size());
    LOGMSG("successfully sent control msg %s\n", cmd.c_str());

}

void ControlClient::RecvResponse(char* recv_buf, int max_len, int& len)
{
    Client::Recv(recv_buf, max_len, len);
    LOGMSG("received %d bytes: %s\n", len, recv_buf);
}

bool ControlClient::RecvResponseAsync(char* recv_buf, int max_len, int& len)
{
    int res = recv(Client::get_sock(), recv_buf, max_len, 0);
    if(res == -1){
        if(errno == EWOULDBLOCK || errno == EAGAIN){
            return false;
        }
        else{
            LOGERR("error receiving. \n");
            exit(-1);
        }
    }
    else if(res == 0){
        LOGERR("server shutdown.\n");
        exit(0);
    }
    else{
        len = res;
        return true;
    }
}
