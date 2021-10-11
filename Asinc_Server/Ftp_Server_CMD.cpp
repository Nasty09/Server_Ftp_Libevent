#include "Server.hpp"

Ftp_Server_CMD::~Ftp_Server_CMD()
{
    Close();
    for (auto i : calls_del)
    {
        i.first->Close();
        delete i.first;
    }
}

bool Ftp_Server_CMD::Init() {
    std::cout << "Ftp_Server_CMD::Init()\n";
    auto bev = bufferevent_socket_new(base, sock, BEV_OPT_CLOSE_ON_FREE);
    if (bev == nullptr)
    {
        delete this;
        return false;
    }
    this->cmdbev = bev;
    this->SetCallback(bev);

    timeval readtime = {120, 0};
    bufferevent_set_timeouts(bev, &readtime, nullptr);

    std::string msg = "220 Welcome to libevent XFTPServer\r\n";
    bufferevent_write(bev, msg.c_str(), msg.size());

    return true;
}

void Ftp_Server_CMD::Read(bufferevent *bev) {
    char data[1024] = {0};
    while (true)
    {
        int len = bufferevent_read(bev, data, sizeof(data) - 1);
        if (len <= 0)
            break;
        data[len] = '\0';
        std::cout << "Recv CMD: " << data << "\n";

        std::string type;
        for (int i = 0; i < len; ++i)
        {
            if (data[i] == ' ' || data[i] == '\r')
                break;
            type += data[i];
        }
        std::cout << "Type: [" << type << "]\n";
        if (calls.find(type) != calls.end())
        {
            auto task = calls[type];
            task->cmdTask = this;
            task->ip = ip;
            task->port = port;
            task->base = base;
            task->Parse(type, data);
            if (type == "PORT")
            {
                ip = task->ip;
                port = task->port;
            }
        }
        else
        {
            std::string msg = "200 OK\r\n";
            bufferevent_write(bev, msg.c_str(), msg.size());
        }
    }
}

void Ftp_Server_CMD::Event(bufferevent *bev, short what) {
    if (what & (BEV_EVENT_EOF | BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT))
    {
        std::cout << "BEV_EVENT_EOF | BEV_EVENT_ERROR | BEV_EVENT_TIMEOUT\n";
        delete this;
    }
}

void Ftp_Server_CMD::Reg(const std::string &cmd, Ftp_Task *call) {
    if (cmd.empty())
    {
        std::cout << "Ftp_Server_CMD::Reg cmd is empty\n";
        return;
    }
    if (!call)
    {
        std::cout << "Ftp_Server_CMD::Reg call is empty\n";
        return;
    }
    ///已经注册的不覆盖，提示错误
    if (calls.find(cmd) != calls.end())
    {
        std::cout << cmd << " is already register \n";
        return;
    }
    calls[cmd] = call;
    calls_del[call] = 0;
}
