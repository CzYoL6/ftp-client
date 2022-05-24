#pragma once

#include <iostream>
#include <string>
#include <boost/algorithm/string.hpp>
#include <vector>
#include <memory>

#include "../include/imgui.h"
#include "../include/logger.h"

#define FILENAME_INDEX 8
#define FILETYPE_INDEX 0
 
void HelpMarker(const char* desc);

//to specify the logger pointer in "logger.h"
extern ILogger* p_logger;


//to store context information for the gui to use
//like pwd, current file list, etc
class FtpClientGUI{
public:
    struct File;
    
    FtpClientGUI();
    ~FtpClientGUI();

    void                Init();
    void                SetStyle();

    //remote operations
    void                GetAllFiles();
    void                ChangeDir(const std::string& wd);
    void                DownloadFile(const std::string& file_path);
    void                UploadFile(const std::string& file_path);
    void                DeleteFile(const std::string& file_path);

    //local operations
    void                ListLocalFiles();
    void                ChangeLocalDir(const std::string& wd);

    //draw GUI
    void                ShowLoginModal();
    void                ShowUserInfoBar();
    void                ShowLocalFiles();
    void                ShowRomoteFiles();
    void                ShowLog();

private:
    class Impl;
    std::unique_ptr<Impl> p_impl{nullptr};


};

