#include <netinet/in.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#include "log.h"
#include "message_server_socket.h"

bool message_server_socket::start() {
    m_socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    sockaddr_in sock_addr{};
    sock_addr.sin_port = htons(23235);
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = htons(INADDR_ANY);

    if (bind(m_socket_fd, (sockaddr *) &sock_addr, sizeof(sock_addr)) == -1) {
        logger->error("socket bind error");
        return false;
    }

    if (listen(m_socket_fd, 10) == -1) {
        logger->error("socket listen error");
        return false;
    }

    m_epoll_fd = epoll_create(1);

    epoll_event epev{};
    epev.events = EPOLLIN;
    epev.data.fd = m_socket_fd;
    epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_socket_fd, &epev);

    m_runner = std::make_unique<std::thread>([this]() { running(); });

    return true;
}

void message_server_socket::stop() {
    if (m_keep_going) {
        m_keep_going = false;
    }
    m_runner->join();
    logger->info("stop listening");
}

void message_server_socket::running() {
    logger->info("start listening ...");
    usleep(500000);

    const size_t EVENTS_SIZE = 20;

    epoll_event events[EVENTS_SIZE];

    while (m_keep_going) {
        int num = epoll_wait(m_epoll_fd, events, EVENTS_SIZE, 500);

        if (num == -1) {
            logger->error("epoll wait return -1, errno {}", errno);
            kill(getpid(), SIGINT);
            break;
        }

        for (int i = 0; i < num; i++) {
            if (events[i].data.fd == m_socket_fd) {
                if (events[i].events & EPOLLIN) {
                    sockaddr_in cli_addr{};
                    socklen_t length = sizeof(cli_addr);
                    int new_fd = accept(m_socket_fd, (sockaddr *) &cli_addr, &length);
                    if (new_fd > 0) {
                        logger->info("new connection, fd {}", new_fd);
                        epoll_event epev{};
                        epev.events = EPOLLIN | EPOLLET;
                        epev.data.fd = new_fd;
                        int flags = fcntl(new_fd, F_GETFL, 0);
                        if (fcntl(new_fd, F_SETFL, flags | O_NONBLOCK) < 0) {
                            logger->error("new connection set no block error, fd {}", new_fd);
                            continue;
                        }
                        epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, new_fd, &epev);
                        logger->info("new connection added, fd {}", new_fd);
                    }
                }
            }
            else {
            }
        }
    }
}