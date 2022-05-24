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
    bool DisplayInTable(FtpClientGUI& ftpClientGUI){
        ImGui::PushID(ID);

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        const bool is_folder = is_dir;
        
        if (is_folder){
            ImGui::BulletText(this->name.c_str());

            ImGui::TableNextColumn();
            ImGui::TextDisabled("--");
            ImGui::TableNextColumn();
            ImGui::TextUnformatted("Folder");

            ImGui::TableNextColumn();

            if(ImGui::Button("open")){
                //change dir and print wd
                ftpClientGUI.ChangeDir(this->name);
                //get files
                ftpClientGUI.GetAllFiles();

                ImGui::PopID();
                return true;
            }
        }
        else{
            ImGui::BulletText(this->name.c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%d", 0);
            ImGui::TableNextColumn();
            ImGui::TextUnformatted("File");
            
            ImGui::TableNextColumn();

            if(ImGui::Button("download")){
                ftpClientGUI.DownloadFile(this->name);
                //show dialog
            }
            ImGui::SameLine();

            if(ImGui::Button("delete")){
                ftpClientGUI.DeleteFile(this->name);
                //get files
                ftpClientGUI.GetAllFiles();
                //show dialog
            }
        }

        ImGui::PopID();
        return false;
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
    void                                GetAllFiles(); 
    void                                SetStyle(); 

    void                                ChangeDir(const std::string& wd);
    void                                DownloadFile(const std::string& file_path);
    void                                DeleteFile(const std::string& file_path);
    //draw UI
    void                                ShowLoginModal();
    void                                ShowUserInfoBar();
    void                                ShowLocalFiles();
    void                                ShowRomoteFiles(FtpClientGUI& ftpClientGUI);
    void                                ShowLog();

private:
    std::vector<FtpClientGUI::File> file_list_of_cur_dir;
    std::unique_ptr<FtpClient> p_fc;
    std::string cwd;

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
            ImGui::Text("Log"); ImGui::SameLine(); 
            HelpMarker(
                "Detailed log of what's happening.");

            // Options menu
            if (ImGui::BeginPopup("Options")){
                ImGui::Checkbox("Auto-scroll", &AutoScroll);
                ImGui::EndPopup();
            }

            // Main window
            if (ImGui::Button("Options"))
                ImGui::OpenPopup("Options");
            ImGui::SameLine();
            bool clear = ImGui::Button("Clear");
            ImGui::SameLine();
            bool copy = ImGui::Button("Copy");
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

void FtpClientGUI::DeleteFile(const std::string& file_path){
    p_impl->DeleteFile(file_path);
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
    p_impl->ShowLocalFiles();
}

FtpClientGUI::Impl::Impl(){
    p_fc = std::make_unique<FtpClient>();
}

FtpClientGUI::Impl::~Impl(){

}

void FtpClientGUI::Impl::ChangeDir(const std::string& wd){
    if(!p_fc->ChangeDir(wd)){
        //show dialog
    }
    if(!p_fc->GetCWD(&this->cwd)){
        //show dialog
    }
}

void FtpClientGUI::Impl::Init(){
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
    ImGui::Text("RemoteFiles"); ImGui::SameLine(); 
    HelpMarker(
        "all files and directories on the remote side.\n"
        "right click to operate on them.");

    ImGui::Text("Current Path: %s", cwd.c_str());
    ImGui::SameLine();
    if(ImGui::Button("refresh")){
        ftpClientGUI.GetAllFiles();
    }

    const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;
    const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();

    static ImGuiTableFlags flags = ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;

    if (ImGui::BeginTable("4ways", 4, flags))
    {
        // The first column will use the default _WidthStretch when ScrollX is Off and _WidthFixed when ScrollX is On
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoHide);
        ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 12.0f);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 18.0f);
        ImGui::TableSetupColumn("Operation", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 18.0f);
        ImGui::TableHeadersRow();


        for(auto file : file_list_of_cur_dir){
            //if file list changes, then break and draw next time.
            if(file.DisplayInTable(ftpClientGUI)) break;
        }
        ImGui::EndTable();
    }
}
 
void FtpClientGUI::Impl::SetStyle(){
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg] = ImColor(30,30,30,255);
    style.Colors[ImGuiCol_TitleBg] = ImColor(255,193,193,255);
    style.Colors[ImGuiCol_TitleBgActive] = ImColor(255,106,106,255);
}

void FtpClientGUI::Impl::ShowLog()
{
    // ImGui::Text("Log"); ImGui::SameLine(); 
    // HelpMarker(
    //     "Detailed log of what's happening.");
    logger->Draw("Log");

}


void FtpClientGUI::Impl::ShowLoginModal()
{
    
}

void FtpClientGUI::Impl::ShowUserInfoBar()
{
    ImGui::Text("host: %s", p_fc->GetIP().c_str());
    ImGui::SameLine();
    ImGui::Text("username: %s", p_fc->GetUsername().c_str());

}

void FtpClientGUI::Impl::ShowLocalFiles()
{
    
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
