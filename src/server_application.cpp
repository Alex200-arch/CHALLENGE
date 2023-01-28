#include <signal.h>

#include "log.h"
#include "server_application.h"

ServerApplication::ServerApplication(const std::string &applicationName)
    : mPidFilename(applicationName + ".pid")
    , mLogFilename(applicationName + ".log")
    , mLoggername(applicationName) {
    initLogger(mLoggername, mLogFilename);
}

void ServerApplication::run() {
    bool pidFileState = pidFileOpen();
    if (!pidFileState) {
        return;
    }

    // socket, create listen

    auto sigs = setupSignalHandlers();

    logger->info("application running ...");

    keepProcessingSignals(&sigs);

    logger->info("application shutdown");

    pidFileClose();
}

bool ServerApplication::pidFileOpen() const {
    const auto fd = ::open(mPidFilename.c_str(), O_RDWR | O_CREAT, 0644);

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

void ServerApplication::pidFileClose() const {
    unlink(mPidFilename.c_str());
}

sigset_t ServerApplication::setupSignalHandlers() const {
    sigset_t sigs;

    sigemptyset(&sigs);
    sigaddset(&sigs, SIGINT);
    sigaddset(&sigs, SIGQUIT);
    sigaddset(&sigs, SIGTERM);
    pthread_sigmask(SIG_BLOCK, &sigs, 0);

    return sigs;
}

void ServerApplication::keepProcessingSignals(sigset_t *sigs) {
    while (mKeepGoing) {
        siginfo_t signal_info;
        timespec timeout{1, 0};
        if (sigtimedwait(sigs, &signal_info, &timeout) == -1)
            continue;

        switch (signal_info.si_signo) {
        case SIGINT:
        case SIGQUIT:
        case SIGTERM:
            mKeepGoing = false;
            break;
        default:
            logger->info("application does not support signal {}", signal_info.si_signo);
        }
    }
}