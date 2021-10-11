#include "Server.hpp"

static void NotifyCB(evutil_socket_t fd, short events, void *arg) {
    auto t = reinterpret_cast<Thread *>(arg);
    t->Notify(fd, events);
}

void Thread::Start() {
    Setup();
    std::thread th(&Thread::Main, this);
    th.detach();
}

void Thread::Main() {
    std::cout << id << " Thread::Main() Begin\n";

    event_base_dispatch(base);
    event_base_free(base);

    std::cout << id << " Thread::Main() End\n";
}

bool Thread::Setup() {
    int fds[2];
//    Create a one-way communication channel (pipe).
//    If successful, two file descriptors are stored in PIPEDES;
//    bytes written on PIPEDES[1] can be read from PIPEDES[0].
    if (pipe(fds))
    {
        std::cerr << "pipe failed\n";
        return false;
    }
    notify_send_fd = fds[1];

    auto ev_config = event_config_new(); //allocates a new event configuration object
    event_config_set_flag(ev_config, EVENT_BASE_FLAG_NOLOCK);
    this->base = event_base_new_with_config(ev_config);
    event_config_free(ev_config); //free configuration after event base created
    if (!base)
    {
        std::cout << "event_base_new_with_config failed in thread " << id << "\n";
        return false;
    }

    auto *ev = event_new(base, fds[0], EV_READ | EV_PERSIST, NotifyCB, this);
    event_add(ev, nullptr);
    return true;
}

void Thread::Notify(evutil_socket_t fd, short which) {
    char buf[2] = {0};
    int res = read(fd, buf, 1);
    if (res <= 0)
        return;
    std::cout << id << " thread " << buf << "\n";

    Task *task = nullptr;

    tasks_mutex.lock();
    if (tasks.empty())
    {
        tasks_mutex.unlock();
        return;
    }
//    Returns a read-only (constant) reference to
//    the data at the first element of the %list
    task = tasks.front();
    tasks.pop_front(); //removes first element
    tasks_mutex.unlock();
    task->Init();
}

void Thread::Activate() {
    int re = write(this->notify_send_fd, "c", 1);
    if (re <= 0)
    {
        std::cerr << "Thread::Activate() failed\n";
    }
}

void Thread::AddTask(Task *task) {
    if (!task)
        return;
    task->base = this->base;
    tasks_mutex.lock();
    tasks.emplace_back(task);
    tasks_mutex.unlock();
}
