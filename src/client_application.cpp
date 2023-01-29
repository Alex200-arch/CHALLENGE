#include "client_application.h"
#include "log.h"

client_application::client_application(const std::string &user_name)
    : m_user_name(user_name)
    , m_prompt(user_name + "> ") {
    init_logger(m_user_name, m_user_name + ".log");
}

void client_application::run() {
    const size_t BUFF_SIZE = 1024;
    char buff[BUFF_SIZE];

    while (true) {
        ::write(STDOUT_FILENO, m_prompt.data(), m_prompt.size());
        int len = ::read(STDIN_FILENO, buff, BUFF_SIZE);
        len--; // remove \r
        buff[len] = 0;

        logger->info("input {}, len {}", buff, len);

        // new message_client
        // connect
        // keep receving socket and stdin
        // callback then send
    }
}
