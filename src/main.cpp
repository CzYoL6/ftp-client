#include <iostream>
#include <unistd.h>
#include<cstring>
#include<string>
#include<vector>
#include<boost/algorithm/string.hpp>
#include<fstream>

#include "../include/data_client.h"
#include "../include/control_client.h"
#include "../include/logger.h"

int GetPort(const char *s){
	std::string res_str(s);
	boost::algorithm::erase_head(res_str, 27);
	boost::algorithm::erase_tail(res_str, 4);
	std::vector<std::string> res_net_strs;
	boost::algorithm::split(res_net_strs, res_str, boost::is_any_of(","));
	int designated_port = atoi(res_net_strs[4].c_str()) * 256 + atoi(res_net_strs[5].c_str());
	return designated_port;
}

void RefreshDir(ControlClient& cc, const std::string& ip){
	char recv[1024];
	int res = 0;

	//get  pwd
	cc.SendReq("PWD\r\n");
	memset(recv, 0, sizeof(recv));
	cc.RecvResponse(recv, 1024, res);


	//get dir file list
	cc.SendReq("PASV\r\n");
	memset(recv, 0, sizeof(recv));
	cc.RecvResponse(recv, 1024, res);
	int designated_port = GetPort(recv);

	DataClient dc;
	dc.Init();
	dc.Connect(ip, designated_port);


	cc.SendReq("LIST\r\n");
	memset(recv, 0, sizeof(recv));
	cc.RecvResponse(recv, 1024, res);
	

	char dirs[2048];
	memset(dirs, 0, sizeof(dirs));
	int total_len = 0;
	while(!dc.RecvDir(dirs + total_len, sizeof(dirs) - total_len, total_len));

	memset(recv, 0, sizeof(recv));
	cc.RecvResponse(recv, 1024, res);

	printf("\n\n");
	printf("================================== file ================================\n");
	printf(dirs);
	printf("========================================================================\n");
	printf("\n\n");


	dc.Close();

}

void GetFile(ControlClient& cc, const std::string& ip, const std::string& file_name){
	char recv[1024];
	int res = 0;
	
	cc.SendReq("TYPE I\r\n");
	memset(recv, 0, sizeof(recv));
	cc.RecvResponse(recv, 1024, res);

	cc.SendReq("PASV\r\n");
	memset(recv, 0, sizeof(recv));
	cc.RecvResponse(recv, 1024, res);
	int designated_port = GetPort(recv);
	DataClient dc;
	dc.Init();
	dc.Connect(ip, designated_port);


	std::string retr = "RETR ";
	retr.append(file_name).append("\r\n");
	cc.SendReq(retr);
	memset(recv, 0, sizeof(recv));
	cc.RecvResponse(recv, 1024, res);


	std::fstream stream;
	stream.open(file_name, std::ios::out | std::ios::binary);
	int total_len = 0;
	while(!dc.RecvData(stream, total_len));
	
	memset(recv, 0, sizeof(recv));
	cc.RecvResponse(recv, 1024, res);
	
	stream.close();

	LOGMSG("file %s successfully received.\n", file_name.c_str());

}

void DeleteFile(ControlClient& cc, const std::string& file_name){
	std::string del_cmd = "DELE ";
	del_cmd.append(file_name).append("\r\n");

	char recv[1024];
	int res = 0;

	cc.SendReq(del_cmd);
	memset(recv,0 ,sizeof(recv));
	cc.RecvResponse(recv, 1024, res);

	LOGMSG("successfully deleted file %s", file_name.c_str());
}

void Login(ControlClient& cc, const std::string& username, const std::string& password){
	std::string username_cmd = "USER ";
	username_cmd.append(username).append("\r\n");

	char recv[1024];
	int res = 0;

	cc.SendReq(username_cmd);
	memset(recv, 0, sizeof(recv));
	cc.RecvResponse(recv, 1024, res);
	
	std::string pwd_cmd = "PASS ";
	pwd_cmd.append(password).append("\r\n");

	cc.SendReq(pwd_cmd);
	memset(recv, 0, sizeof(recv));
	cc.RecvResponse(recv, 1024, res);
}

void UploadFile(ControlClient& cc, const std::string& ip, const std::string& file_name){
	char recv[1024];
	int res = 0;
	
	cc.SendReq("TYPE I\r\n");
	memset(recv, 0, sizeof(recv));
	cc.RecvResponse(recv, 1024, res);

	cc.SendReq("PASV\r\n");
	memset(recv, 0, sizeof(recv));
	cc.RecvResponse(recv, 1024, res);
	int designated_port = GetPort(recv);
	DataClient dc;
	dc.Init();
	dc.Connect(ip, designated_port);

	std::string retr = "STOR ";
	retr.append(file_name).append("\r\n");
	cc.SendReq(retr);
	memset(recv, 0, sizeof(recv));
	cc.RecvResponse(recv, 1024, res);

	std::fstream stream;
	stream.open(file_name, std::ios::in | std::ios::binary);
	
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
	cc.RecvResponse(recv, 1024, res);
	
	LOGMSG("file %s successfully sent.\n", file_name.c_str());
}

int main(int argc, char *argv[])
{
	
	std::string ip = "192.168.225.128";

	ControlClient control_client;
	control_client.Init();
	control_client.Connect(ip, 21);

	char recv[1024];
	int res = 0;
	memset(recv, 0, sizeof(recv));
	control_client.RecvResponse(recv, 1024, res);

	//login
	Login(control_client,"ftptest", "T0KH3QTRKM");

	control_client.SendReq("PWD\r\n");
	memset(recv, 0, sizeof(recv));
	control_client.RecvResponse(recv, 1024, res);

	//get dir file list
	RefreshDir(control_client, ip);
	
	//get the file
	// {
	// std::string file_name;
	// std::cin >> file_name;
	// GetFile(control_client,ip, file_name);
	// }

	// //delete file
	// {
	// std::string file_name;
	// std::cin >> file_name;
	// DeleteFile(control_client, file_name);
	// RefreshDir(control_client, ip);
	// }

	//upload file
	{
	std::string file_name;
	std::cin >> file_name;
	UploadFile(control_client, ip, file_name);
	RefreshDir(control_client, ip);
	}

	control_client.Close();
}