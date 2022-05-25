#pragma once

#include <memory>

#include "control_client.h"
#include "data_client.h"

#define EXPECTED_RES_CODE_CONNECTION_ESTABLISHED 220
#define EXPECTED_RES_CODE_LOGIN_USERNAME         331
#define EXPECTED_RES_CODE_LOGIN_PASSWORD         230
#define EXPECTED_RES_CODE_SET_BINARY             200
#define EXPECTED_RES_CODE_PWD                    257
#define EXPECTED_RES_CODE_TIMEOUT                421
#define EXPECTED_RES_CODE_PASVMODE               227
#define EXPECTED_RES_CODE_OPENCHANNEL            150
#define EXPECTED_RES_CODE_TRANSFERFINISHED       226
#define EXPECTED_RES_CODE_DELETE                 250
#define EXPECTED_RES_CODE_CHANGEDIR              250


class FtpClient{
public:
    FtpClient();
    ~FtpClient();
    bool                    Init();
    bool                    inited();
    bool                    Connect(const std::string& ip, int port);
    void                    DisConnect();
    bool                    connected();
    bool                    ListFile(std::string* files);
    bool                    UploadFile(const std::string& file_path);
    bool                    DownloadFile(const std::string& file_path);
    bool                    _DeleteFile(const std::string& file_path);
    bool                    Login(const std::string& username, const std::string& pwd);
    bool                    ChangeDir(const std::string& dir_name);                      //CWD 改变路径
    bool                    GetCWD(std::string* cwd);                                    //PWD 获取当前的目录路径
    std::string             GetIP();
    std::string             GetUsername();
    void                    Close();

private:
    class Impl;
    std::unique_ptr<Impl> p_impl;


};