#ifndef SERVER_APPLICATION_H_
#define SERVER_APPLICATION_H_

#include <atomic>
#include <memory>
#include <string>
#include <thread>

#include "message_server_creator.h"

class server_application {
public:
    server_application(const std::string &, const int &);

    void run();

private:
    bool pid_file_open() const;
    void pid_file_close() const;
    sigset_t setup_signal_handlers() const;
    void keep_processing_signals(sigset_t *);
    void running();

    std::atomic_bool m_keep_app_going{true};
    std::string m_pid_file_name;
    std::string m_log_file_name;
    std::string m_logger_name;
    std::unique_ptr<message_server> m_server;
    std::atomic_bool m_keep_runner_going{true};
    int m_epoll_fd{-1};
    std::unique_ptr<std::thread> m_runner{nullptr};
};

#endif // SERVER_APPLICATION_H_