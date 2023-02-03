#include <signal.h>
#include <sys/epoll.h>

#include "log.h"
#include "server_application.h"

server_application::server_application(const std::string &application_name, const int &port)
    : m_application_name(application_name)
    , m_pid_file_name(application_name + ".pid")
    , m_log_file_name(application_name + ".log")
    , m_logger_name(application_name)
    , m_server(std::unique_ptr<message_server>(message_server_creator<message_server_socket>().create_message_server(port))) {
    init_logger(m_logger_name, m_log_file_name);
}

void server_application::run() {
    bool pid_file_state = pid_file_open();
    if (!pid_file_state) {
        return;
    }

    auto sigs = setup_signal_handlers();

    logger->info("application running ...");

    if (m_server->start()) {
        m_epoll_fd = epoll_create(1);
        epoll_event epev{};
        epev.events = EPOLLIN;
        epev.data.fd = m_server->get_fd();
        epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, m_server->get_fd(), &epev);
        m_runner = std::make_unique<std::thread>([this]() { running(); });

        keep_processing_signals(&sigs);

        m_keep_runner_going = false;
        m_runner->join();
        logger->info("stop listening");
    }

    pid_file_close();

    logger->info("application shutdown");
}

bool server_application::pid_file_open() const {
    const auto fd = ::open(m_pid_file_name.c_str(), O_RDWR | O_CREAT, 0644);

    if (fd == -1) {
        logger->error("unable to open pid file");
        return false;
    }

    char buf[32];
    auto n = ::read(fd, buf, sizeof(buf) - 1);
    if (n > 0) {
        ::close(fd);
        logger->error("application already running");
        return false;
    }

    n = snprintf(buf, sizeof(buf), "%d", getpid());
    if (pwrite(fd, buf, n, 0) == -1) {
        ::close(fd);
        logger->error("unable to write pid file, error code {}", std::strerror(errno));
        return false;
    }

    ::close(fd);

    return true;
}

void server_application::pid_file_close() const {
    unlink(m_pid_file_name.c_str());
}

sigset_t server_application::setup_signal_handlers() const {
    sigset_t sigs;

    sigemptyset(&sigs);
    sigaddset(&sigs, SIGINT);
    sigaddset(&sigs, SIGQUIT);
    sigaddset(&sigs, SIGTERM);
    pthread_sigmask(SIG_BLOCK, &sigs, 0);

    return sigs;
}

void server_application::keep_processing_signals(sigset_t *sigs) {
    while (m_keep_app_going) {
        siginfo_t signal_info;
        timespec timeout{1, 0};
        if (sigtimedwait(sigs, &signal_info, &timeout) == -1)
            continue;

        switch (signal_info.si_signo) {
        case SIGINT:
        case SIGQUIT:
        case SIGTERM:
            m_keep_app_going = false;
            break;
        default:
            logger->info("application does not support signal {}", signal_info.si_signo);
        }
    }
}

void server_application::running() {
    logger->info("start listening ...");
    usleep(500000);

    const size_t EVENTS_SIZE = 20;
    const size_t BUFF_SIZE = 4096;

    epoll_event events[EVENTS_SIZE];

    char buff[BUFF_SIZE];
    int len;

    while (m_keep_runner_going) {
        int num = epoll_wait(m_epoll_fd, events, EVENTS_SIZE, 500);

        if (num == -1) {
            logger->error("epoll wait return -1, errno {}", errno);
            kill(getpid(), SIGINT);
            break;
        }

        for (int i = 0; i < num; i++) {
            if (events[i].data.fd == m_server->get_fd()) {
                if (events[i].events & EPOLLIN) {
                    int new_fd = m_server->new_connection();
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
                        m_server->add_handler(new_fd);
                        logger->info("new connection added, fd {}", new_fd);
                    }
                }
            }
            else {
                if (events[i].events & EPOLLIN) {
                    int msg_fd = events[i].data.fd;
                    len = ::read(msg_fd, buff, BUFF_SIZE);
                    if (len > 0) {
                        if (auto ptr_handler = m_server->get_handler_by_fd(msg_fd); ptr_handler) {
                            auto msgs = ptr_handler->receive_message(buff, len);
                            for (const auto &msg : msgs) {
                                logger->info("fd{}, type: {}, from: {}, to: {}, payload: {}, timestamp: {}", msg_fd, (int16_t) msg.type, msg.from, msg.to, msg.payload, msg.timestamp);
                                if (msg.type == messge_type_t::MSG) {
                                    message_handler::make_network_message(msg, buff, len);
                                    if (msg.to == "all") {
                                        auto fds = m_server->get_others_fds_by_name(msg.from);
                                        for (const auto &fd : fds) {
                                            ::write(fd, buff, len);
                                        }
                                    }
                                    else {
                                        auto fd = m_server->get_fd_by_name(msg.to);
                                        if (fd != -1) {
                                            ::write(fd, buff, len);
                                        }
                                        else {
                                            m_server->save_msg_for_off_line_user(msg.to, msg);
                                        }
                                    }
                                }
                                else if (msg.type == messge_type_t::LOGIN) {
                                    logger->info("fd {}, login message type: {}, from: {}, to: {}, payload: {}, timestamp: {}", msg_fd, (int16_t) msg.type, msg.from, msg.to, msg.payload, msg.timestamp);
                                    if (m_server->logined(msg.from)) {
                                        message msg_response;
                                        msg_response.type = messge_type_t::LOGIN;
                                        msg_response.from = "Fail";
                                        msg_response.payload = "user have been logined";
                                        message_handler::make_network_message(msg_response, buff, len);
                                        ::write(msg_fd, buff, len);
                                        logger->info("fd {}, user have been logined", msg_fd);
                                    }
                                    else {
                                        auto result = m_server->check_password(msg.from, msg.payload);
                                        if (result) {
                                            if (result.value()) {
                                                message msg_response;
                                                msg_response.type = messge_type_t::LOGIN;
                                                msg_response.from = "OK";
                                                msg_response.payload = "welcome back.";
                                                message_handler::make_network_message(msg_response, buff, len);
                                                m_server->login_handler(msg.from, msg_fd);
                                                ::write(msg_fd, buff, len);

                                                auto msgs = m_server->get_off_line_msg(msg.from);
                                                logger->info("fd {}, resend off line messages, size {}", msg_fd, msgs.size());
                                                for (const auto &msg : msgs) {
                                                    message_handler::make_network_message(msg, buff, len);
                                                    ::write(msg_fd, buff, len);
                                                }
                                            }
                                            else {
                                                message msg_response;
                                                msg_response.type = messge_type_t::LOGIN;
                                                msg_response.from = "Fail";
                                                msg_response.payload = "password is not proper.";
                                                message_handler::make_network_message(msg_response, buff, len);
                                                ::write(msg_fd, buff, len);
                                                logger->info("fd {}, password is wrong", msg_fd);
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
                                            message_handler::make_network_message(msg_response, buff, len);
                                            m_server->login_handler(msg.from, msg_fd);
                                            m_server->save_password(msg.from, ss.str());
                                            ::write(msg_fd, buff, len);
                                        }
                                    }
                                }
                                else if (msg.type == messge_type_t::HEARTBEAT) {
                                    logger->info("fd {}, heartbeat message type: {}, from: {}, to: {}, payload: {}, timestamp: {}", msg_fd, (int16_t) msg.type, msg.from, msg.to, msg.payload, msg.timestamp);
                                }
                                else if (msg.type == messge_type_t::UNKNOWN) {
                                    logger->error("fd {}, unknown message type: {}, from: {}, to: {}, payload: {}, timestamp: {}", msg_fd, (int16_t) msg.type, msg.from, msg.to, msg.payload, msg.timestamp);
                                }
                            }
                        }
                    }
                    else if (len == 0) {
                        logger->info("disconnection, fd {}", msg_fd);
                        epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, msg_fd, nullptr);
                        m_server->del_handler(msg_fd);
                        ::close(msg_fd);
                    }
                }
            }
        }

        ////// heartbeat
        {
            static time_t t = time(NULL);
            message msg_heartbeat;
            msg_heartbeat.type = messge_type_t::HEARTBEAT;
            msg_heartbeat.from = m_application_name;
            message_handler::make_network_message(msg_heartbeat, buff, len);
            if (difftime(time(NULL), t) > 10.0f) {
                t = time(NULL);
                auto fds = m_server->get_all_fds();
                for (const auto &fd : fds) {
                    ::write(fd, buff, len);
                }
            }
        }
    }
}