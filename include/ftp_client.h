#pragma once

#include <memory>

#include "control_client.h"
#include "data_client.h"

class FtpClient{
public:
    FtpClient();
    ~FtpClient();
    bool                    Init();
    bool                    Connect(const std::string& ip, int port);
    const std::string       ListFile();
    bool                    UploadFile(const std::string& file_path);
    bool                    DownloadFile(const std::string& file_path);
    bool                    DeleteFile(const std::string& file_path);
    bool                    Login(const std::string& username, const std::string& pwd);
    void                    Close();

private:
    class Impl;
    std::unique_ptr<Impl> p_impl;

};