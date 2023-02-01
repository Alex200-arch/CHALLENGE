#ifndef MESSAGE_SERVER_SOCKET_H
#define MESSAGE_SERVER_SOCKET_H

#include <atomic>
#include <memory>
#include <thread>

#include "message_server.h"

class message_server_socket : public message_server {
public:
    message_server_socket(const int &);
    bool start() override;
    void stop() override;

protected:
private:
    void running();
    int m_port;
    std::atomic_bool m_keep_going{true};
    int m_socket_fd{-1};
    int m_epoll_fd{-1};
    std::unique_ptr<std::thread> m_runner{nullptr};
};

#endif // MESSAGE_SERVER_SOCKET_H