#include "Server.hpp"

void Ftp_User::Parse(const std::string &type, const std::string &msg) {
    std::cout << "XFTPUSER::Parse " << type << " " << msg << "\n";
    ResponseCMD("230 Login successful.\r\n");
}
