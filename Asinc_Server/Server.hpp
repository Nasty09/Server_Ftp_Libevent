#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include <cstring>
#include <event.h>
#include <thread>
#include <list>
#include <csignal>
#include <unistd.h>
#include <mutex>
#include <vector>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <map>

class Task {
public:
    event_base *base = nullptr;
    int sock = 0;
    int thread_id = 0;
    virtual bool Init() = 0;
};


class Thread {
    int notify_send_fd = 0;
    event_base *base = nullptr;
    std::list<Task *> tasks;
    std::mutex tasks_mutex;
public:
    Thread() = default;
    ~Thread() = default;
    void Start();
    void Main();
    bool Setup();
    void Notify(evutil_socket_t fd, short which);
    void Activate();
    void AddTask(Task *task);
    int id = 0;
};


class ThreadPool {
    ThreadPool() = default;
    int threadCount = 0;
    int lastThread = -1;
    std::vector<Thread *> threads;
public:
    ~ThreadPool() = default;
    static ThreadPool *GetInstance() {
        static ThreadPool threadPool;
        return &threadPool;
    }
    void Init(int num);
    void Dispatch(Task *task);
};


class Ftp_Factory {
    Ftp_Factory() = default;
public:
    static Ftp_Factory *GetInstance();
    static Task *CreateTask();
};


class Ftp_Task : public Task {
public:
    std::string curDir = "/";
    std::string rootDir = "..";
    Ftp_Task *cmdTask = nullptr;
    std::string ip = "";
    int port = 0;
    virtual void Read(bufferevent *bev) {}
    virtual void Write(bufferevent *bev) {}
    virtual void Event(bufferevent *bev, short what) {}
    void SetCallback(bufferevent *bev);
    bool Init() override;
    virtual void Parse(const std::string &type, const std::string &msg) {}
    void ResponseCMD(const std::string &msg);
    void ConnectPORT();
    void Send(const std::string &data);
    void Send(const char *data, int len);
    void Close();
protected:
    static void EventCB(bufferevent *bev, short what, void *arg);
    static void ReadCB(bufferevent *bev, void *arg);
    static void WriteCB(bufferevent *bev, void *arg);
    bufferevent *cmdbev = nullptr;
    FILE *fp = nullptr;
};


class Ftp_List : public Ftp_Task {
    static std::string getDirData(const std::string &path);
    static std::string getFilePermissions(const std::filesystem::directory_entry &f);
    static std::string getFileTime(const std::filesystem::directory_entry &f);
    static int getDirectoryCount(const std::filesystem::path &fp);
public:
    void Parse(const std::string &type, const std::string &msg) override;
    void Write(bufferevent *bev) override;
    void Event(bufferevent *bev, short what) override;
};


class Ftp_Port : public Ftp_Task {
    void Parse(const std::string &type, const std::string &msg) override;
};


class Ftp_Retr : public Ftp_Task {
    FILE *fp = nullptr;
    char buf[1024] = {0};
public:
    void Parse(const std::string &type, const std::string &msg) override;
    void Write(bufferevent *bev) override;
    void Event(bufferevent *bev, short what) override;
};


class Ftp_Server_CMD : public Ftp_Task {
    std::map<std::string, Ftp_Task *> calls;
    std::map<Ftp_Task *, int> calls_del;
public:
    Ftp_Server_CMD() = default;
    ~Ftp_Server_CMD();
    bool Init() override;
    void Read(bufferevent *bev) override;
    void Event(bufferevent *bev, short what) override;
    void Reg(const std::string &cmd, Ftp_Task *call);
};


class Ftp_Stor : public Ftp_Task {
    FILE *fp = nullptr;
    char buf[1024] = {0};
public:
    void Parse(const std::string &type, const std::string &msg) override;
    void Read(bufferevent *bev) override;
    void Event(bufferevent *bev, short what) override;
};

#endif
