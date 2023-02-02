#include <netinet/in.h>
#include <signal.h>
#include <sstream>
#include <sys/epoll.h>
#include <sys/socket.h>

#include "log.h"
#include "message_server_socket.h"

message_server_socket::message_server_socket(const int &port)
    : m_port(port) {
}

bool message_server_socket::start() {
    m_socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    sockaddr_in sock_addr{};
    sock_addr.sin_port = htons(m_port);
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
    const size_t BUFF_SIZE = 4096;

    epoll_event events[EVENTS_SIZE];

    char buff[BUFF_SIZE];

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
                        epev.events = EPOLLIN;
                        epev.data.fd = new_fd;
                        int flags = fcntl(new_fd, F_GETFL, 0);
                        if (fcntl(new_fd, F_SETFL, flags | O_NONBLOCK) < 0) {
                            logger->error("new connection set no block error, fd {}", new_fd);
                            continue;
                        }
                        epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, new_fd, &epev);
                        add_handler(new_fd);
                        logger->info("new connection added, fd {}", new_fd);
                    }
                }
            }
            else {
                if (events[i].events & EPOLLIN) {
                    int msg_fd = events[i].data.fd;
                    int len = ::read(msg_fd, buff, BUFF_SIZE);
                    if (len > 0) {
                        if (auto ptr_handler = get_handler_by_fd(msg_fd); ptr_handler) {
                            auto msgs = ptr_handler->receive_message(buff, len);
                            for (const auto &msg : msgs) {
                                logger->info("type: {}, from: {}, to: {}, payload: {}, timestamp: {}", (int16_t) msg.type, msg.from, msg.to, msg.payload, msg.timestamp);
                                if (msg.type == messge_type_t::MSG) {
                                    ptr_handler->make_network_message(msg, buff, len);
                                    if (msg.to == "all") {
                                        auto fds = get_others_fds_by_name(msg.from);
                                        for (const auto &fd : fds) {
                                            ::write(fd, buff, len);
                                        }
                                    }
                                    else {
                                        auto fd = get_fd_by_name(msg.to);
                                        if (fd != -1) {
                                            ::write(fd, buff, len);
                                        }
                                        else {
                                            save_msg_for_off_line_user(msg.to, msg);
                                        }
                                    }
                                }
                                else if (msg.type == messge_type_t::LOGIN) {
                                    logger->info("login message type: {}, from: {}, to: {}, payload: {}, timestamp: {}", (int16_t) msg.type, msg.from, msg.to, msg.payload, msg.timestamp);
                                    auto result = check_password(msg.from, msg.payload);
                                    if (result) {
                                        if (result.value()) {
                                            message msg_response;
                                            msg_response.type = messge_type_t::LOGIN;
                                            msg_response.from = "OK";
                                            msg_response.payload = "welcome back.";
                                            ptr_handler->make_network_message(msg_response, buff, len);
                                            login_handler(msg.from, msg_fd);
                                            ::write(msg_fd, buff, len);

                                            auto msgs = get_off_line_msg(msg.from);
                                            logger->info("resend off line messages, size {}", msgs.size());
                                            for (const auto &msg : msgs) {
                                                ptr_handler->make_network_message(msg, buff, len);
                                                ::write(msg_fd, buff, len);
                                            }
                                        }
                                        else {
                                            message msg_response;
                                            msg_response.type = messge_type_t::LOGIN;
                                            msg_response.from = "Fail";
                                            ptr_handler->make_network_message(msg_response, buff, len);
                                            ::write(msg_fd, buff, len);
                                        }
                                    }
                                    else {
                                        message msg_response;
                                        msg_response.type = messge_type_t::LOGIN;
                                        msg_response.from = "OK";
                                        srand(time(NULL));
                                        int r = rand();
                                        std::stringstream ss;
                                        ss << r;
                                        msg_response.payload = "please save your password: " + ss.str();
                                        ptr_handler->make_network_message(msg_response, buff, len);
                                        login_handler(msg.from, msg_fd);
                                        save_password(msg.from, ss.str());
                                        ::write(msg_fd, buff, len);
                                    }
                                }
                                else if (msg.type == messge_type_t::UNKNOWN) {
                                    logger->error("unknown message type: {}, from: {}, to: {}, payload: {}, timestamp: {}", (int16_t) msg.type, msg.from, msg.to, msg.payload, msg.timestamp);
                                }
                            }
                        }
                    }
                    else if (len == 0) {
                        logger->info("disconnection, fd {}", msg_fd);
                        epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, msg_fd, nullptr);
                        del_handler(msg_fd);
                        ::close(msg_fd);
                    }
                }
            }
        }
    }
}