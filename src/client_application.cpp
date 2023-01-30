#include "client_application.h"
#include "log.h"
#include "message_client_socket.h"

client_application::client_application(const std::string &user_name)
    : m_user_name(user_name)
    , m_prompt(user_name + "> ") {
    init_logger(m_user_name, m_user_name + ".log");
}

void client_application::run() {
    const size_t BUFF_SIZE = 4096;
    char buff[BUFF_SIZE];

    fd_set rfds;
    int nfds;

    message_client_socket client;

    while (true) {
        ::write(STDOUT_FILENO, m_prompt.data(), m_prompt.size());

        FD_ZERO(&rfds);

        FD_SET(STDIN_FILENO, &rfds);
        nfds = 1;

        if (client.working()) {
            FD_SET(client.get_fd(), &rfds);
            nfds = client.get_fd() + 1;
        }

        select(nfds, &rfds, NULL, NULL, NULL);

        if (FD_ISSET(STDIN_FILENO, &rfds)) {
            int len = ::read(STDIN_FILENO, buff, BUFF_SIZE);
            len--; // remove LF; \n(10, LF), \r(13, CR)
            if (len == 0) {
                continue;
            }
            buff[len] = 0;
            logger->info("input {}, len {}", buff, len);
            auto input = process_input(buff);
            if (input.type == input_type_t::CMD_CONNECT) {
                if (!client.working()) {
                    if (client.start(input.payload)) {
                        m_prompt = input.payload + "|" + m_user_name + "> ";
                        output_msg("welcome!!!");
                    }
                    else {
                        output_error("connection failed");
                    }
                }
                else {
                    output_error("already connected");
                }
            }
            else if (input.type == input_type_t::CMD_DISCONNECT) {
                if (client.working()) {
                    client.stop();
                    m_prompt = m_user_name + "> ";
                }
                else {
                    output_error("no connection");
                }
                continue;
            }
            else if (input.type == input_type_t::CMD_QUIT) {
                break;
            }
            else if (input.type == input_type_t::CMD_SEND) {
                if (client.working()) {
                    ::write(client.get_fd(), input.payload.data(), input.payload.size());
                }
                else {
                    output_error("no connection");
                }
            }
            else if (input.type == input_type_t::CMD_UNKNOWN) {
                logger->info("invalid input");
                output_error("invalid input");
            }
        }

        if (FD_ISSET(client.get_fd(), &rfds)) {
            int len = ::read(client.get_fd(), buff, BUFF_SIZE);
            logger->info("data coming, len {}", len);
            if (len > 0) {
                buff[len] = 0;
                output_msg(buff);
            }
            else {
                client.stop();
                logger->error("server shutdown");
                output_error("\nserver shutdown");
                m_prompt = m_user_name + "> ";
            }
        }
    }
}

input_t client_application::process_input(const std::string &input_buff) {
    // HI 1.0.0.1:23235
    // BYE
    // QUIT
    // @bob xxxx
    input_t ret;

    if (input_buff.size() >= 12 && input_buff[0] == 'H' && input_buff[1] == 'I' && input_buff[2] == ' ') {
        ret.type = input_type_t::CMD_CONNECT;
        ret.payload = input_buff.substr(3);
    }
    else if (input_buff == "BYE") {
        ret.type = input_type_t::CMD_DISCONNECT;
    }
    else if (input_buff == "QUIT") {
        ret.type = input_type_t::CMD_QUIT;
    }
    else if (input_buff.size() >= 2 && input_buff[0] == '@') {
        ret.type = input_type_t::CMD_SEND;
        ret.payload = input_buff.substr(1);
    }
    else {
        ret.type = input_type_t::CMD_UNKNOWN;
        ret.payload = input_buff;
    }

    return ret;
}

void client_application::output_msg(const std::string &msg) {
    // green
    ::write(STDOUT_FILENO, "\x1b[0;32;49m", 10);
    ::write(STDOUT_FILENO, msg.data(), msg.size());
    ::write(STDOUT_FILENO, "\n", 1);
    ::write(STDOUT_FILENO, "\x1b[0m", 4);
}

void client_application::output_error(const std::string &output) {
    // red
    ::write(STDOUT_FILENO, "\x1b[0;31;49m", 10);
    ::write(STDOUT_FILENO, output.data(), output.size());
    ::write(STDOUT_FILENO, "\n", 1);
    ::write(STDOUT_FILENO, "\x1b[0m", 4);
}