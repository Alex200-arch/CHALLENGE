#include <signal.h>

#include "log.h"
#include "server_application.h"

server_application::server_application(const std::string &application_name)
    : m_pid_file_name(application_name + ".pid")
    , m_log_file_name(application_name + ".log")
    , m_logger_name(application_name)
    , m_server(std::unique_ptr<message_server>(message_server_creator<message_server_socket>().create_message_server())) {
    init_logger(m_logger_name, m_log_file_name);
}

void server_application::run() {
    bool pid_file_state = pid_file_open();
    if (!pid_file_state) {
        return;
    }

    // socket, create listen

    auto sigs = setup_signal_handlers();

    logger->info("application running ...");

    keep_processing_signals(&sigs);

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
    while (m_keep_going) {
        siginfo_t signal_info;
        timespec timeout{1, 0};
        if (sigtimedwait(sigs, &signal_info, &timeout) == -1)
            continue;

        switch (signal_info.si_signo) {
        case SIGINT:
        case SIGQUIT:
        case SIGTERM:
            m_keep_going = false;
            break;
        default:
            logger->info("application does not support signal {}", signal_info.si_signo);
        }
    }
}