#include "Server.hpp"

void Ftp_Task::ReadCB(bufferevent *bev, void *arg) {
    auto task = reinterpret_cast<Ftp_Task *>(arg);
    task->Read(bev);
}

void Ftp_Task::WriteCB(bufferevent *bev, void *arg) {
    auto task = reinterpret_cast<Ftp_Task *>(arg);
    task->Write(bev);
}

void Ftp_Task::EventCB(bufferevent *bev, short what, void *arg) {
    auto task = reinterpret_cast<Ftp_Task *>(arg);
    task->Event(bev, what);
}

void Ftp_Task::SetCallback(bufferevent *bev) {
//    Changes the callbacks for a bufferevent
    bufferevent_setcb(bev, ReadCB, WriteCB, EventCB, this);
    bufferevent_enable(bev, EV_READ | EV_WRITE);
}

bool Ftp_Task::Init() {
    return true;
}

void Ftp_Task::ResponseCMD(const std::string &msg) {
    if (!cmdTask || !cmdTask->cmdbev)
        return;
    std::cout << "ResponseCMD: " << msg << "\n";
    bufferevent_write(cmdTask->cmdbev, msg.c_str(), msg.size());
}

void Ftp_Task::ConnectPORT() {
    if (ip.empty() || port <= 0 || !base)
    {
        std::cout << "ConnectPORT failed\n";
        return;
    }
    Close();
//    Create a new socket bufferevent over an existing socket.
//    Close base when buffer freed
    cmdbev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
    sockaddr_in sock;
    memset(&sock, 0, sizeof(sock));
    sock.sin_family = AF_INET;
    sock.sin_port = htons(port);
    evutil_inet_pton(AF_INET, ip.c_str(), &sock.sin_addr.s_addr);
    SetCallback(cmdbev);
    timeval readtime = {120, 0};
    bufferevent_set_timeouts(cmdbev, &readtime, nullptr);
    bufferevent_socket_connect(cmdbev, reinterpret_cast<sockaddr *>(&sock), sizeof(sock));
}

void Ftp_Task::Send(const std::string &data) {
    Send(data.c_str(), data.size());
}

void Ftp_Task::Send(const char *data, int len) {
    if (!cmdbev)
        return;
    bufferevent_write(cmdbev, data, len);
}

void Ftp_Task::Close() {
    if (cmdbev)
    {
        bufferevent_free(cmdbev);
        this->cmdbev = nullptr;
    }
    if (fp)
    {
        fclose(fp);
        fp = nullptr;
    }
}
