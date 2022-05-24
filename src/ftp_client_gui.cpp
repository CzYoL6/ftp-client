#include <boost/filesystem.hpp>

#include "../include/ftp_client_gui.h"
#include "../include/ftp_client.h"
#include "../include/logger.h"

struct FtpClientGUI::File{
    bool        is_dir;
    std::string name;
    int         ID;

    File(){
        DesignateID();
    }

    void DesignateID(){
        static int id = 0;
        ID = id++;
    }

    //show GUI
    //return true if a folder is open, the file list needs refreshing.
    //return false if no folder is open, the file list stay unchanged.
    bool DisplayInTable(FtpClientGUI& ftpClientGUI, bool remote = false){

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        const bool is_folder = is_dir;
        
        bool need_refresh = false;

        if (is_folder){
            ImGui::BulletText(this->name.c_str());

            ImGui::TableNextColumn();
            ImGui::TextDisabled("--");
            ImGui::TableNextColumn();
            ImGui::TextUnformatted("Folder");

            ImGui::TableNextColumn();

            if(remote){

                char Id[50];
                memset(Id, 0, sizeof(Id));
                const char* id = "remote open";
                sprintf(Id, "%s %d", id, ID);

                ImGui::PushID(Id);
                if(ImGui::Button("open")){
                    //change dir and print wd
                    ftpClientGUI.ChangeDir(this->name);
                    //get files
                    ftpClientGUI.GetAllFiles();

                    need_refresh = true;
                }
                ImGui::PopID();

            }
            else{
                char Id[50];
                memset(Id, 0, sizeof(Id));
                const char* id = "local open";
                sprintf(Id, "%s %d", id, ID);

                ImGui::PushID(Id);
                if(ImGui::Button("open")){
                    //change dir and print wd
                    ftpClientGUI.ChangeLocalDir(this->name);
                    //get files
                    ftpClientGUI.ListLocalFiles();

                    need_refresh = true;
                }
                ImGui::PopID();
            } 
        }
        else{
            ImGui::BulletText(this->name.c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%d", 0);
            ImGui::TableNextColumn();
            ImGui::TextUnformatted("File");
            
            ImGui::TableNextColumn();

            if(remote){
                
                {
                    char Id[50];
                    memset(Id, 0, sizeof(Id));
                    const char* id = "remote download";
                    sprintf(Id, "%s %d", id, ID);

                    ImGui::PushID(Id);

                    if(ImGui::Button("download")){
                        ftpClientGUI.DownloadFile(this->name);
                        //show dialog
                    }

                    ImGui::PopID();
                }

                ImGui::SameLine();

                {
                    char Id[50];
                    memset(Id, 0, sizeof(Id));
                    const char* id = "remote delete";
                    sprintf(Id, "%s %d", id, ID);

                    ImGui::PushID(Id);
                    if(ImGui::Button("delete")){
                        ftpClientGUI.DeleteFile(this->name);
                        //get files
                        ftpClientGUI.GetAllFiles();
                        //show dialog
                    }

                    ImGui::PopID();
                }

            }
            else{

                char Id[50];
                memset(Id, 0, sizeof(Id));
                const char* id = "local upload";
                sprintf(Id, "%s %d", id, ID);

                ImGui::PushID(Id);

                if(ImGui::Button("upload")){
                    ftpClientGUI.UploadFile(this->name);
                    //show dialog
                }

                ImGui::PopID();
            }
        }
        return need_refresh;
    }
};

class FtpClientGUI::Impl{
public:
    class Logger;
    std::unique_ptr<Logger> logger;
    

    Impl();
    ~Impl();

    void                                Init();
    //parse the file string got from the client, and store them in vector 'file_list_of_cur_dir'
    void                                SetStyle(); 

    //remote operations
    void                                GetAllFiles(); 
    void                                ChangeDir(const std::string& wd);
    void                                DownloadFile(const std::string& file_path);
    void                                UploadFile(const std::string& file_path);
    void                                DeleteFile(const std::string& file_path);

    //local operations
    void                                ListLocalFiles();
    void                                ChangeLocalDir(const std::string& wd);

    //draw UI
    void                                ShowLoginModal();
    void                                ShowUserInfoBar();
    void                                ShowLocalFiles(FtpClientGUI& ftpClientGUI);
    void                                ShowRomoteFiles(FtpClientGUI& ftpClientGUI);
    void                                ShowLog();

private:
    std::vector<FtpClientGUI::File> file_list_of_cur_dir;
    std::unique_ptr<FtpClient> p_fc;
    std::string cwd;

    std::vector<FtpClientGUI::File> local_file_list_of_cur_dir;
    std::string local_cwd;

    void                                ListTopLevelFiles(const boost::filesystem::path& directoryPath, 
                                                            std::vector<File>* topLevelFiles) ;
    void                                ShowModal(const std::string&& msg);

public:
    class Logger : public ILogger{
    public:
        ImGuiTextBuffer     Buf;
        ImGuiTextFilter     Filter;
        ImVector<int>       LineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
        bool                AutoScroll;  // Keep scrolling if already at the bottom.

        Logger(){
            AutoScroll = true;
            Clear();
        }

        void    Clear(){
            Buf.clear();
            LineOffsets.clear();
            LineOffsets.push_back(0);
        }

        void    AddLog(const char* fmt, ...) override IM_FMTARGS(2){
            int old_size = Buf.size();
            va_list args;
            va_start(args, fmt);
            Buf.appendfv(fmt, args);
            va_end(args);
            for (int new_size = Buf.size(); old_size < new_size; old_size++)
                if (Buf[old_size] == '\n')
                    LineOffsets.push_back(old_size + 1);
        }

        void    Draw(const char* title, bool* p_open = NULL){
            ImGui::Text(title); ImGui::SameLine(); 
            HelpMarker(
                "操作执行的细节.");

            // Options menu
            if (ImGui::BeginPopup("Options")){
                ImGui::Checkbox("Auto-scroll", &AutoScroll);
                ImGui::EndPopup();
            }

            // Main window
            if (ImGui::Button("Options"))
                ImGui::OpenPopup("Options");
            ImGui::SameLine();
            bool clear = ImGui::Button("清空");
            ImGui::SameLine();
            bool copy = ImGui::Button("复制");
            ImGui::SameLine();
            Filter.Draw("Filter", -100.0f);

            ImGui::Separator();
            ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

            if (clear)
                Clear();
            if (copy)
                ImGui::LogToClipboard();

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            const char* buf = Buf.begin();
            const char* buf_end = Buf.end();
            if (Filter.IsActive()){
                for (int line_no = 0; line_no < LineOffsets.Size; line_no++)
                {
                    const char* line_start = buf + LineOffsets[line_no];
                    const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                    if (Filter.PassFilter(line_start, line_end))
                        ImGui::TextUnformatted(line_start, line_end);
                }
            }
            else{
                ImGuiListClipper clipper;
                clipper.Begin(LineOffsets.Size);
                while (clipper.Step())
                {
                    for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
                    {
                        const char* line_start = buf + LineOffsets[line_no];
                        const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                        ImGui::TextUnformatted(line_start, line_end);
                    }
                }
                clipper.End();
            }
            ImGui::PopStyleVar();

            if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                ImGui::SetScrollHereY(1.0f);

            ImGui::EndChild();
        }
    };
};


FtpClientGUI::FtpClientGUI(){
    p_impl = std::make_unique<FtpClientGUI::Impl>();
}

FtpClientGUI::~FtpClientGUI(){

}

void FtpClientGUI::Init(){
    p_impl->Init();
}

void FtpClientGUI::GetAllFiles(){
    p_impl->GetAllFiles();
}

void FtpClientGUI::SetStyle(){
    p_impl->SetStyle();
}

void FtpClientGUI::ChangeDir(const std::string& wd){
    p_impl->ChangeDir(wd);
}

void FtpClientGUI::DownloadFile(const std::string& file_path){
    p_impl->DownloadFile(file_path);
}

void FtpClientGUI::UploadFile(const std::string& file_path){
    p_impl->UploadFile(file_path);
}

void FtpClientGUI::DeleteFile(const std::string& file_path){
    p_impl->DeleteFile(file_path);
}

void FtpClientGUI::ChangeLocalDir(const std::string& wd){
    p_impl->ChangeLocalDir(wd);
}

void FtpClientGUI::ListLocalFiles(){
    p_impl->ListLocalFiles();
} 

void FtpClientGUI::ShowRomoteFiles(){
    p_impl->ShowRomoteFiles(*this);
}

void FtpClientGUI::ShowLog()
{
    p_impl->ShowLog();
}


void FtpClientGUI::ShowLoginModal()
{
    p_impl->ShowLoginModal();
}

void FtpClientGUI::ShowUserInfoBar()
{
    p_impl->ShowUserInfoBar();
}

void FtpClientGUI::ShowLocalFiles()
{
    p_impl->ShowLocalFiles(*this);
}

FtpClientGUI::Impl::Impl(){
    p_fc = std::make_unique<FtpClient>();
}

FtpClientGUI::Impl::~Impl(){

}

void FtpClientGUI::Impl::ShowModal(const std::string&& msg){
    ImGui::OpenPopup("modal");

    bool unused_open = true;
    if (ImGui::BeginPopupModal("modal", &unused_open))
    {
        ImGui::Text(msg.c_str());
        if (ImGui::Button("Close"))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

void FtpClientGUI::Impl::ChangeDir(const std::string& wd){
    if(!p_fc->ChangeDir(wd)){
        //show dialog
    }
    if(!p_fc->GetCWD(&this->cwd)){
        //show dialog
    }
        ShowModal("进入目录成功！");
}

void FtpClientGUI::Impl::Init(){
    local_cwd = boost::filesystem::current_path().string();
    ListLocalFiles();

    //set the gui logger pointer
    logger = std::make_unique<Logger>();
    p_logger = logger.get();

    std::string ip = "192.168.225.128";
	
	if(!p_fc->Init()){
        //show dialog
    }
    if(!p_fc->Connect(ip, 21)){
        //show dialog
    }
    if(!p_fc->Login("ftptest", "T0KH3QTRKM")){
        //show dialog
    }
    if(!p_fc->GetCWD(&(this->cwd))){
        //show dialog
    }

    GetAllFiles();


}

void FtpClientGUI::Impl::DownloadFile(const std::string& file_path){
    if(!p_fc->DownloadFile(file_path)){
        //show dialog
    }
}

void FtpClientGUI::Impl::UploadFile(const std::string& file_path){
    if(!p_fc->UploadFile(file_path)){
        //show dialog
    }
}

void FtpClientGUI::Impl::DeleteFile(const std::string& file_path){
    if(!p_fc->DeleteFile(file_path)){
        //show dialog
    }
}

void FtpClientGUI::Impl::GetAllFiles(){
    std::string files;
    if(!p_fc->ListFile(&files)){
        //show dialog
    }

    std::vector<std::string> file_list_strs;
    std::vector<std::string> meta_list;
    std::vector<File> &file_list = file_list_of_cur_dir;
    file_list_strs.clear();
    meta_list.clear();

    file_list.clear();
    //add .. to the file list

    File DotDotFolder;
    DotDotFolder.name = "..";
    DotDotFolder.is_dir = true;
    file_list.push_back(DotDotFolder);

    if(files.size()) {

        boost::algorithm::trim(files);
        boost::algorithm::split(file_list_strs, files, boost::is_any_of("\r\n"), boost::token_compress_on);

        for(auto file : file_list_strs){
            boost::algorithm::split(meta_list, file, boost::is_any_of(" "), boost::token_compress_on);
            const char* type = meta_list[FILETYPE_INDEX].c_str();
            bool is_folder = type[0] == 'd' ;

            File new_file;
            new_file.is_dir = is_folder;
            new_file.name = meta_list[FILENAME_INDEX];
            file_list.push_back(new_file);

            meta_list.clear();
        }
    }
    
}

void FtpClientGUI::Impl::ShowRomoteFiles(FtpClientGUI& ftpClientGUI){
    ImGui::Text("远端文件"); ImGui::SameLine(); 
    HelpMarker(
        "服务器的文件，从本地上传的文件会保存在这里.\n");

    ImGui::Text("当前目录: %s", cwd.c_str());
    ImGui::SameLine();

    ImGui::PushID("refresh_button_remote");
    if(ImGui::Button("刷新")){
        ftpClientGUI.GetAllFiles();
    }
    ImGui::PopID();

    const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;
    const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();

    static ImGuiTableFlags flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;

    if (ImGui::BeginTable("4ways", 4, flags))
    {
        // The first column will use the default _WidthStretch when ScrollX is Off and _WidthFixed when ScrollX is On
        ImGui::TableSetupColumn("文件名", ImGuiTableColumnFlags_NoHide);
        ImGui::TableSetupColumn("类型", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 18.0f);
        ImGui::TableSetupColumn("操作", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 18.0f);
        ImGui::TableHeadersRow();


        for(auto file : file_list_of_cur_dir){
            //if file list changes, then break and draw next time.
            if(file.DisplayInTable(ftpClientGUI, true)) break;
        }
        ImGui::EndTable();
    }
}
 
void FtpClientGUI::Impl::ListTopLevelFiles(const boost::filesystem::path& directoryPath, std::vector<File>* topLevelFiles) {
    topLevelFiles->clear();
    if (!exists(directoryPath))
        return;

    File DotDotFolder;
    DotDotFolder.name = "..";
    DotDotFolder.is_dir = true;
    topLevelFiles->push_back(DotDotFolder);

    //non-recursive directory iterator
    boost::filesystem::directory_iterator end_itr;
    for (boost::filesystem::directory_iterator itr(directoryPath); itr != end_itr; ++itr) {
        File file;
        file.name = itr->path().filename().string();
        file.is_dir = boost::filesystem::is_directory(itr->status());
        topLevelFiles->push_back(file);
    }
}

void FtpClientGUI::Impl::SetStyle(){
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg] = ImColor(30,30,30,255);
    style.Colors[ImGuiCol_TitleBg] = ImColor(255,193,193,255);
    style.Colors[ImGuiCol_TitleBgActive] = ImColor(255,106,106,255);
}

void FtpClientGUI::Impl::ChangeLocalDir(const std::string& wd){
    boost::filesystem::current_path(boost::filesystem::current_path() / wd);
    local_cwd = boost::filesystem::current_path().string();
}

void FtpClientGUI::Impl::ListLocalFiles(){
    local_file_list_of_cur_dir.clear();
    ListTopLevelFiles(local_cwd, &local_file_list_of_cur_dir);
} 

void FtpClientGUI::Impl::ShowLog()
{
    // ImGui::Text("Log"); ImGui::SameLine(); 
    // HelpMarker(
    //     "Detailed log of what's happening.");
    logger->Draw("日志");

}


void FtpClientGUI::Impl::ShowLoginModal()
{
    //TODO
}

void FtpClientGUI::Impl::ShowUserInfoBar()
{
    ImGui::Text("主机: %s", p_fc->GetIP().c_str());
    ImGui::SameLine();
    ImGui::Text("用户名: %s", p_fc->GetUsername().c_str());

}

void FtpClientGUI::Impl::ShowLocalFiles(FtpClientGUI& ftpClientGUI){
    ImGui::Text("本地文件"); ImGui::SameLine(); 
    HelpMarker(
        "本地的所有文件，从远端下载的文件会保存在这里.\n");

    ImGui::Text("当前目录: %s", local_cwd.c_str());
    ImGui::SameLine();

    ImGui::PushID("refresh_button_local");
    if(ImGui::Button("刷新")){
        ftpClientGUI.ListLocalFiles();
    }
    ImGui::PopID();

    const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;
    const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();

    static ImGuiTableFlags flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;

    if (ImGui::BeginTable("4ways", 4, flags))
    {
        // The first column will use the default _WidthStretch when ScrollX is Off and _WidthFixed when ScrollX is On
        ImGui::TableSetupColumn("文件名", ImGuiTableColumnFlags_NoHide);
        ImGui::TableSetupColumn("类型", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 18.0f);
        ImGui::TableSetupColumn("操作", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 18.0f);
        ImGui::TableHeadersRow();


        for(auto file : local_file_list_of_cur_dir){
            //if file list changes, then break and draw next time.
            if(file.DisplayInTable(ftpClientGUI, false)) break;
        }
        ImGui::EndTable();
    }
}


void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}
