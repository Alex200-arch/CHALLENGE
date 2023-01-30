#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "log.h"
#include "message_client_socket.h"

bool message_client_socket::start(const std::string &identifier) {
    auto pos = identifier.find(':');
    if (pos == std::string::npos) {
        logger->error("server identifier is not proper, {}", identifier);
        return false;
    }
    std::string s_ip = identifier.substr(0, pos);
    std::string s_port = identifier.substr(pos + 1);
    logger->info("server ip {}, port {}", s_ip, s_port);

    struct sockaddr_in serv_addr;
    if ((m_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        logger->error("socket creation error");
        return false;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(s_port.data()));

    if (inet_pton(AF_INET, s_ip.data(), &serv_addr.sin_addr) <= 0) {
        logger->error("invalid address, address not supported");
        stop();
        return false;
    }

    if (connect(m_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        logger->error("connection failed");
        stop();
        return false;
    }

    logger->info("connection succeeded");

    return true;
}

void message_client_socket::stop() {
    if (m_fd != -1) {
        ::close(m_fd);
        m_fd = -1;
        logger->info("disconnected");
    }
}

bool message_client_socket::working() const {
    if (m_fd == -1) {
        return false;
    }
    return true;
}

int message_client_socket::get_fd() const {
    return m_fd;
}