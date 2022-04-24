#include <iostream>
#include <unistd.h>
#include<cstring>
#include<string>
#include<vector>
#include<boost/algorithm/string.hpp>
#include<fstream>

#include "../include/ftp_client.h"

class FtpClient::Impl{
private:
    std::string ip;
    std::unique_ptr<ControlClient> p_cc;
    int GetPort(const char* s);

public:
    Impl();
    ~Impl();

    bool                    Init();
    bool                    Connect(const std::string& ip, int port);
    bool  			        ListFile(std::string* files);
    bool                    UploadFile(const std::string& file_path);
    bool                    DownloadFile(const std::string& file_path);
    bool                    DeleteFile(const std::string& file_path);
	bool 					Login(const std::string& username, const std::string& pwd);
	void 					Close();
};

FtpClient::Impl::Impl()
{
    p_cc = std::make_unique<ControlClient>();
}

FtpClient::Impl::~Impl()
{
}

int FtpClient::Impl::GetPort(const char* s){
    std::string res_str(s);
	boost::algorithm::erase_head(res_str, 23);
	//boost::algorithm::erase_tail(res_str, 4);
	std::vector<std::string> res_net_strs;
	boost::algorithm::split(res_net_strs, res_str, boost::is_any_of(","));
	int designated_port = atoi(res_net_strs[4].c_str()) * 256 + atoi(res_net_strs[5].c_str());
	return designated_port;
}

bool FtpClient::Impl::Init()
{
    if(!p_cc->Init()) return false;
    return true;
}

void FtpClient::Impl::Close(){
	p_cc->Close();
}

bool FtpClient::Impl::Login(const std::string& username, const std::string& pwd){
	std::string username_cmd = "USER ";
	username_cmd.append(username).append("\r\n");

	if(!p_cc->SendReq(username_cmd)) return false;
	if(!p_cc->RecvResponse(EXPECTED_RES_CODE_LOGIN_USERNAME)) return false;
	
	std::string pwd_cmd = "PASS ";
	pwd_cmd.append(pwd).append("\r\n");

	if(!p_cc->SendReq(pwd_cmd)) return false;
	if(!p_cc->RecvResponse(EXPECTED_RES_CODE_LOGIN_PASSWORD)) return false;

	return true;
}

bool FtpClient::Impl::Connect(const std::string& ip, int port)
{
    if(!p_cc->Connect(ip,port)){
		this->ip = "";
		return false;
	}
	else{
		this->ip = ip;
		
		RESPONSE_TYPE res;
		p_cc->RecvResponse(EXPECTED_RES_CODE_CONNECTION_ESTABLISHED);
	}
}

bool FtpClient::Impl::ListFile(std::string* files)
{
	//get  pwd
	if(!p_cc->SendReq("PWD\r\n")) return false;
	
	if(!p_cc->RecvResponse(EXPECTED_RES_CODE_PWD)) return false;


	//get dir file list
	if(!p_cc->SendReq("PASV\r\n")) return false;
	
	std::string msg;
	if(!p_cc->RecvResponse(EXPECTED_RES_CODE_PASVMODE, &msg)) return false;
	int designated_port = GetPort(msg.c_str());

	DataClient dc;
	if(!dc.Init()) return false;
	if(!dc.Connect(ip, designated_port)) return false;


	if(!p_cc->SendReq("LIST\r\n")) return false;
	
	if(!p_cc->RecvResponse(EXPECTED_RES_CODE_OPENCHANNEL)) return false;
	

	char dirs[2048];
	memset(dirs, 0, sizeof(dirs));
	int total_len = 0;
	while(!dc.RecvDir(dirs + total_len, sizeof(dirs) - total_len, total_len));

	
	if(!p_cc->RecvResponse(EXPECTED_RES_CODE_TRANSFERFINISHED)) return false;

	LOGMSG("File list: ");
	printf("================================== file ================================\n");
	printf(dirs);
	printf("========================================================================\n");
	printf("\n");

	dc.Close();

	std::string dirs_str(dirs);

	if(files) *files = dirs_str;

	return true;
}

bool FtpClient::Impl::UploadFile(const std::string& file_path)
{
	
	if(!p_cc->SendReq("TYPE I\r\n")) return false;
	
	if(!p_cc->RecvResponse(EXPECTED_RES_CODE_SET_BINARY)) return false;

	if(!p_cc->SendReq("PASV\r\n")) return false;
	
	std::string msg;
	if(!p_cc->RecvResponse(EXPECTED_RES_CODE_PASVMODE, &msg)) return false;
	int designated_port = GetPort(msg.c_str());
	DataClient dc;
	if(!dc.Init()) return false;
	if(!dc.Connect(ip, designated_port)) return false;

	std::string retr = "STOR ";
	retr.append(file_path).append("\r\n");
	if(!p_cc->SendReq(retr)) return false;
	
	if(!p_cc->RecvResponse(EXPECTED_RES_CODE_OPENCHANNEL)) return false;

	std::fstream stream;
	stream.open(file_path, std::ios::in | std::ios::binary);
	
	stream.seekg(0, stream.end);
	int left_to_send = stream.tellg();
	stream.seekg(0, stream.beg);
	LOGMSG("starting to transfer file (%d bytes)", left_to_send);
	while(left_to_send){
		if(!dc.SendData(stream, left_to_send)) return false;
	}
	stream.close();
	dc.Close();

	
	if(!p_cc->RecvResponse(EXPECTED_RES_CODE_TRANSFERFINISHED)) return false;
	
	LOGMSG("file %s successfully sent.\n", file_path.c_str());

	return true;

}

bool FtpClient::Impl::DownloadFile(const std::string& file_path)
{
	
	if(!p_cc->SendReq("TYPE I\r\n")) return false;
	
	if(!p_cc->RecvResponse(EXPECTED_RES_CODE_SET_BINARY)) return false;

	p_cc->SendReq("PASV\r\n");
	
	std::string msg;
	if(!p_cc->RecvResponse(EXPECTED_RES_CODE_PASVMODE, &msg)) return false;

	int designated_port = GetPort(msg.c_str());
	DataClient dc;
	if(!dc.Init()) return false;
	if(!dc.Connect(ip, designated_port)) return false;
	 
	std::string retr = "RETR ";
	retr.append(file_path).append("\r\n");
	if(!p_cc->SendReq(retr)) return false;
	
	if(!p_cc->RecvResponse(EXPECTED_RES_CODE_OPENCHANNEL)) return false;


	std::fstream stream;
	stream.open(file_path, std::ios::out | std::ios::binary);
	int total_len = 0;
	while(!dc.RecvData(stream, total_len));
	
	
	if(!p_cc->RecvResponse(EXPECTED_RES_CODE_TRANSFERFINISHED)) return false;
	
	stream.close();

	LOGMSG("file %s successfully received.\n", file_path.c_str());

	return true;
}

bool FtpClient::Impl::DeleteFile(const std::string& file_path)
{
    std::string del_cmd = "DELE ";
	del_cmd.append(file_path).append("\r\n");

	char recv[1024];
	int res = 0;

	p_cc->SendReq(del_cmd);
	memset(recv,0 ,sizeof(recv));
	p_cc->RecvResponse(EXPECTED_RES_CODE_DELETE);

	LOGMSG("successfully deleted file %s", file_path.c_str());
}


FtpClient::FtpClient(){
    p_impl = std::make_unique<Impl>();
}

FtpClient::~FtpClient()
{
    
}

bool FtpClient::Init()
{
    if(!p_impl->Init()) return false;
    return true;
}

bool FtpClient::Connect(const std::string& ip, int port)
{
    if(!p_impl->Connect(ip, port)) return false;
    return true;
}

bool FtpClient::ListFile(std::string* files)
{
    return p_impl->ListFile(files);
}

bool FtpClient::UploadFile(const std::string& file_path)
{
    return p_impl->UploadFile(file_path);
}

bool FtpClient::DownloadFile(const std::string& file_path)
{
    return p_impl->DownloadFile(file_path);
}

bool FtpClient::DeleteFile(const std::string& file_path)
{
    return p_impl->DeleteFile(file_path);
}

bool FtpClient::Login(const std::string& username, const std::string& pwd)
{
	return p_impl->Login(username, pwd);
}

void FtpClient::Close()
{
	p_impl->Close();
}