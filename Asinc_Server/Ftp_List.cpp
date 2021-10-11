#include "Server.hpp"

std::string Ftp_List::GetDirData(const std::string &path) {
    std::cout << path << " " << std::filesystem::absolute(path) << "\n";
    std::string data;
    for (auto &f:std::filesystem::directory_iterator(path))
    {
        auto &fp = f.path();
        data += getFilePermissions(f) + " ";
        if (std::filesystem::is_directory(f))
        {
            data += std::to_string(getDirectoryCount(fp)) + " " +
                    std::to_string(4096) + " ";
        }
        else
        {
            data += std::to_string(1) + " " +
                    std::to_string(f.file_size()) + " ";
        }
        data += getFileTime(f) + " ";
        data += fp.filename();
        data += "\r\n";
    }
    return data;
}

std::string Ftp_List::getFilePermissions(const std::filesystem::directory_entry &f) {
    std::filesystem::perms permission = f.status().permissions();
    std::string permissions;
    ///directory
    permissions += f.is_directory() ? "d" : "-";
    ///owner
    permissions += (permission & std::filesystem::perms::owner_read) != std::filesystem::perms::none ? "r" : "-";
    permissions += (permission & std::filesystem::perms::owner_write) != std::filesystem::perms::none ? "w" : "-";
    permissions += (permission & std::filesystem::perms::owner_exec) != std::filesystem::perms::none ? "x" : "-";
    ///group
    permissions += (permission & std::filesystem::perms::group_read) != std::filesystem::perms::none ? "r" : "-";
    permissions += (permission & std::filesystem::perms::group_write) != std::filesystem::perms::none ? "w" : "-";
    permissions += (permission & std::filesystem::perms::group_exec) != std::filesystem::perms::none ? "x" : "-";
    ///others
    permissions += (permission & std::filesystem::perms::others_read) != std::filesystem::perms::none ? "r" : "-";
    permissions += (permission & std::filesystem::perms::others_write) != std::filesystem::perms::none ? "w" : "-";
    permissions += (permission & std::filesystem::perms::others_exec) != std::filesystem::perms::none ? "x" : "-";

    return permissions;
}

std::string Ftp_List::getFileTime(const std::filesystem::directory_entry &f) {
    auto ftime = f.last_write_time();
    std::time_t fileTime = std::chrono::system_clock::to_time_t(std::chrono::file_clock::to_sys(ftime));
    std::stringstream ss;
    ss << std::put_time(std::localtime(&fileTime), "%b %d %H:%M");
    return ss.str();
}

int Ftp_List::getDirectoryCount(const std::filesystem::path &fp) {
    int ans = 2;
    for (auto &f:std::filesystem::directory_iterator(fp))
    {
        if (std::filesystem::is_directory(f))
        {
            ans++;
        }
    }
    return ans;
}

void Ftp_List::Parse(const std::string &type, const std::string &msg)
{
    if (type == "PWD") {
        std::string responseMsg = "257 \"";
        responseMsg += cmdTask->curDir;
        responseMsg += "\" is current dir.\r\n";
        ResponseCMD(responseMsg);
    }
    else if (type == "LIST") {
        //std::string dirData = "-rwxrwxrwx 1 root root 64463 Mar 14 09:53 101.jpg\r\n";
        std::string dirData = GetDirData(cmdTask->rootDir + cmdTask->curDir);
        ConnectPORT();
        ResponseCMD("150 Here comes the directory listing.\r\n");
        Send(dirData);
    }
    else if (type == "CWD") {
        int pos = msg.rfind(' ') + 1;
        std::string path = msg.substr(pos, msg.size() - pos - 2);
        if (path[0] == '/')
        {
            cmdTask->curDir = path;
        }
        else
        {
            if (cmdTask->curDir[cmdTask->curDir.size() - 1] != '/')
            {
                cmdTask += '/';
            }
            cmdTask->curDir += path + "/";
        }
        ResponseCMD("250 Directory success changed.\r\n");
    }
    else if (type == "CDUP")
    {
        std::string path = cmdTask->curDir;
        if (path[path.size() - 1] == '/')
        {
            path = path.substr(0, path.size() - 1);
        }
        int pos = path.rfind('/');
        path = path.substr(0, pos);
        cmdTask->curDir = path;
        ResponseCMD("250 Directory success changed.\r\n");
    }
}

void Ftp_List::Write(bufferevent *bev) {
    ResponseCMD("226 Transfer complete\r\n");
    Close();
}

void Ftp_List::Event(bufferevent *bev, short what) {
    if (what & (BEV_EVENT_EOF | BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT))
    {
        std::cout << "BEV_EVENT_EOF | BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT\n";
        Close();
    }
    else if (what & BEV_EVENT_CONNECTED)
    {
        std::cout << "BEV_EVENT_CONNECTED\n";
    }
}
