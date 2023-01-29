#ifndef SERVER_APPLICATION_H_
#define SERVER_APPLICATION_H_

#include <atomic>
#include <memory>
#include <string>

#include "message_server_creator.h"

class server_application {
public:
    server_application(const std::string &);

    void run();

private:
    bool pid_file_open() const;
    void pid_file_close() const;
    sigset_t setup_signal_handlers() const;
    void keep_processing_signals(sigset_t *);

    std::atomic_bool m_keep_going{true};
    std::string m_pid_file_name;
    std::string m_log_file_name;
    std::string m_logger_name;
    std::unique_ptr<message_server> m_server;
};

#endif // SERVER_APPLICATION_H_