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
    const std::string       ListFile();
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
	boost::algorithm::erase_head(res_str, 27);
	boost::algorithm::erase_tail(res_str, 4);
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

	char recv[1024];
	int res = 0;

	p_cc->SendReq(username_cmd);
	memset(recv, 0, sizeof(recv));
	p_cc->RecvResponse(recv, 1024, res);
	
	std::string pwd_cmd = "PASS ";
	pwd_cmd.append(pwd).append("\r\n");

	p_cc->SendReq(pwd_cmd);
	memset(recv, 0, sizeof(recv));
	p_cc->RecvResponse(recv, 1024, res);

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
		char recv[1024];
		int res = 0;
		memset(recv, 0, sizeof(recv));
		p_cc->RecvResponse(recv, 1024, res);
		return true;
	}
}

const std::string FtpClient::Impl::ListFile()
{
    char recv[1024];
	int res = 0;

	//get  pwd
	p_cc->SendReq("PWD\r\n");
	memset(recv, 0, sizeof(recv));
	p_cc->RecvResponse(recv, 1024, res);


	//get dir file list
	p_cc->SendReq("PASV\r\n");
	memset(recv, 0, sizeof(recv));
	p_cc->RecvResponse(recv, 1024, res);
	int designated_port = GetPort(recv);

	DataClient dc;
	dc.Init();
	dc.Connect(ip, designated_port);


	p_cc->SendReq("LIST\r\n");
	memset(recv, 0, sizeof(recv));
	p_cc->RecvResponse(recv, 1024, res);
	

	char dirs[2048];
	memset(dirs, 0, sizeof(dirs));
	int total_len = 0;
	while(!dc.RecvDir(dirs + total_len, sizeof(dirs) - total_len, total_len));

	memset(recv, 0, sizeof(recv));
	p_cc->RecvResponse(recv, 1024, res);

	printf("\n\n");
	printf("================================== file ================================\n");
	printf(dirs);
	printf("========================================================================\n");
	printf("\n\n");


	dc.Close();

	std::string dirs_str(dirs);
	return dirs_str;

}

bool FtpClient::Impl::UploadFile(const std::string& file_path)
{
    char recv[1024];
	int res = 0;
	
	p_cc->SendReq("TYPE I\r\n");
	memset(recv, 0, sizeof(recv));
	p_cc->RecvResponse(recv, 1024, res);

	p_cc->SendReq("PASV\r\n");
	memset(recv, 0, sizeof(recv));
	p_cc->RecvResponse(recv, 1024, res);
	int designated_port = GetPort(recv);
	DataClient dc;
	dc.Init();
	dc.Connect(ip, designated_port);

	std::string retr = "STOR ";
	retr.append(file_path).append("\r\n");
	p_cc->SendReq(retr);
	memset(recv, 0, sizeof(recv));
	p_cc->RecvResponse(recv, 1024, res);

	std::fstream stream;
	stream.open(file_path, std::ios::in | std::ios::binary);
	
	stream.seekg(0, stream.end);
	int left_to_send = stream.tellg();
	stream.seekg(0, stream.beg);
	LOGMSG("starting to transfer file (%d bytes)", left_to_send);
	while(left_to_send){
		dc.SendData(stream, left_to_send);
	}
	stream.close();
	dc.Close();

	memset(recv, 0, sizeof(recv));
	p_cc->RecvResponse(recv, 1024, res);
	
	LOGMSG("file %s successfully sent.\n", file_path.c_str());

	return true;

}

bool FtpClient::Impl::DownloadFile(const std::string& file_path)
{
    char recv[1024];
	int res = 0;
	
	p_cc->SendReq("TYPE I\r\n");
	memset(recv, 0, sizeof(recv));
	p_cc->RecvResponse(recv, 1024, res);

	p_cc->SendReq("PASV\r\n");
	memset(recv, 0, sizeof(recv));
	p_cc->RecvResponse(recv, 1024, res);
	int designated_port = GetPort(recv);
	DataClient dc;
	dc.Init();
	dc.Connect(ip, designated_port);


	std::string retr = "RETR ";
	retr.append(file_path).append("\r\n");
	p_cc->SendReq(retr);
	memset(recv, 0, sizeof(recv));
	p_cc->RecvResponse(recv, 1024, res);


	std::fstream stream;
	stream.open(file_path, std::ios::out | std::ios::binary);
	int total_len = 0;
	while(!dc.RecvData(stream, total_len));
	
	memset(recv, 0, sizeof(recv));
	p_cc->RecvResponse(recv, 1024, res);
	
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
	p_cc->RecvResponse(recv, 1024, res);

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

const std::string FtpClient::ListFile()
{
    return p_impl->ListFile();
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