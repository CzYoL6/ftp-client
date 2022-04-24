#include "../include/ftp_client.h"



int main(int argc, char *argv[])
{
	
	std::string ip = "192.168.225.128";
	
	FtpClient fc;
	fc.Init();
	fc.Connect(ip, 21);
	fc.Login("ftptest", "T0KH3QTRKM");

	getchar();

	std::string files;
	fc.ListFile(&files);

	getchar();

	fc.UploadFile("./imgui.ini");
	
	getchar();

	fc.DownloadFile("./新建文本文档.txt");

	getchar();

	fc.DeleteFile("./新建文本文档2.txt");

	getchar();

	fc.Close();

}