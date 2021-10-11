#include "Server.hpp"

Ftp_Factory *Ftp_Factory::GetInstance() {
    static Ftp_Factory factory;
    return &factory;
}

Task *Ftp_Factory::CreateTask() {
    auto cmd = new Ftp_Server_CMD();
    cmd->Reg("USER", new Ftp_User());
    auto list = new Ftp_List();
    cmd->Reg("PWD", list);
    cmd->Reg("LIST", list);
    cmd->Reg("CWD", list);
    cmd->Reg("CDUP", list);
    cmd->Reg("PORT", new Ftp_Port());
    cmd->Reg("RETR", new Ftp_Retr());
    cmd->Reg("STOR", new Ftp_Stor());

    return cmd;
}
