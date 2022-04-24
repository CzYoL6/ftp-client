#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <vector>
#include <boost/algorithm/string.hpp>

#include "../include/control_client.h"


ControlClient::ControlClient()
{
    recv_buf = new char[RECVBUF_SZE];

}

ControlClient::~ControlClient()
{
    delete[] recv_buf;
}

bool ControlClient::SendReq(const std::string& cmd)
{
    if(!Client::Send(cmd.c_str(), cmd.size())) return false;
    LOGMSG("successfully sent control msg %s\n", cmd.c_str());
    return true;

}

bool ControlClient::RecvResponse(int expected_code, std::string *msg)
{
    memset(recv_buf, 0,RECVBUF_SZE);
    if(!Client::Recv(recv_buf, RECVBUF_SZE, recv_cnt)) return false;

    LOGMSG("received %d bytes: %s\n", recv_cnt, recv_buf);

    RESPONSE_TYPE res = DecodeResponse(recv_buf);

    LOGMSG("response code: %d, res msg: %s \n", res.first, res.second.c_str());

    if(msg) *msg = res.second;

    if(expected_code != res.first) return false;

    else return true;
}

RESPONSE_TYPE ControlClient::DecodeResponse(const char *s)
{
    std::string s_str(s);
    std::vector<std::string> splt;
    boost::algorithm::split(splt, s_str, boost::is_any_of(" "));

    int res_code = atoi(splt[0].c_str());

    boost::algorithm::erase_tail(s_str, 2);
    boost::algorithm::erase_head(s_str, splt[0].size() + 1);

    //std::cout << "split res: " << res_code << " " << res_msg.c_str() << std::endl;

    return std::make_pair(res_code, s_str);
}