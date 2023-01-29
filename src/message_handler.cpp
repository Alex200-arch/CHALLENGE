#include "message_handler.h"
#include "log.h"

message_handler::message_handler(const int &fd)
    : m_fd(fd) {
}

void message_handler::add_message(const std::string &msg) {
    logger->info("msg, fd {}, len {}, msg {}", m_fd, msg.size(), msg);
}

void message_handler::parse_message() {
}