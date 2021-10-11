#include <iostream>
#include <csignal>
#include <cstring>
#include <event.h>
#include <event2/listener.h>
#include "Server.hpp"

#define PORT 5001

void listen_cb(evconnlistener *e, evutil_socket_t s, sockaddr *sock, int socklen, void *arg) {
    std::cout << "listen_cb\n";
    Task *task = Ftp_Factory::GetInstance()->CreateTask();
    task->sock = s;
    ThreadPool::GetInstance()->Dispatch(task);
}

int main(int argc, char *argv[]) {
    if (signal(SIGPIPE, SIG_IGN) == SIG_IGN) //ignore all signals exapt SIGKILL or SIGSTOP
        return 1;

//    int s(*argv[0]);
    ThreadPool::GetInstance()->Init(10); //initiation of communication flows

    auto base = event_base_new();
    if (base)
    {
        std::cout << "event_base_new success\n";
    }
    sockaddr_in sock;
    memset(&sock, 0, sizeof(sock));
    sock.sin_family = AF_INET;
    sock.sin_port = htons(PORT);

    //allocate a new evconnlistener object to listen for incoming TCP connections on a given address
    auto ev = evconnlistener_new_bind(
            base,
            listen_cb,
            base,
            LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE,
            10,/**argv[0],*/
            reinterpret_cast<sockaddr *>(&sock),
            sizeof(sock)
    );

    if (base)
    {
        //loop will run the event base until there are no more pending or
        //active, or until something calls event_base_loopbreak() or
        //event_base_loopexit().
        event_base_dispatch(base);
    }

    if (ev)
    {
        evconnlistener_free(ev); //disable and deallocate an evconnlistener
    }
    if (base)
    {
        event_base_free(base); //deallocate all memory associated with an event_base, and free the base
    }
    return 0;
}

