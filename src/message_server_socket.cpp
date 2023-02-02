#include <netinet/in.h>
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

    return true;
}

int message_server_socket::get_fd() {
    return m_socket_fd;
}

int message_server_socket::new_connection() {
    sockaddr_in cli_addr{};
    socklen_t length = sizeof(cli_addr);
    return accept(m_socket_fd, (sockaddr *) &cli_addr, &length);
}