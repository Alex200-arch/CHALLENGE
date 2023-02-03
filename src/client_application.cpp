#include "client_application.h"
#include "log.h"
#include "message_client_creator.h"

client_application::client_application(const std::string &user_name, const std::string &user_password)
    : m_user_name(user_name)
    , m_user_password(user_password)
    , m_prompt(user_name + "> ") {
    init_logger(m_user_name, m_user_name + ".log");
}

void client_application::run() {
    const size_t BUFF_SIZE = 4096;
    char buff[BUFF_SIZE];
    int len;

    fd_set rfds;
    struct timeval tv;
    int nfds;

    std::unique_ptr<message_client> client(message_client_creator<message_client_socket>().create_message_client());

    while (true) {
        if (m_prompt_write) {
            ::write(STDOUT_FILENO, m_prompt.data(), m_prompt.size());
            m_prompt_write = false;
        }

        FD_ZERO(&rfds);
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        FD_SET(STDIN_FILENO, &rfds);
        nfds = 1;

        if (client->working()) {
            FD_SET(client->get_fd(), &rfds);
            nfds = client->get_fd() + 1;
        }

        select(nfds, &rfds, NULL, NULL, &tv);

        if (FD_ISSET(STDIN_FILENO, &rfds)) {
            m_prompt_write = true;
            len = ::read(STDIN_FILENO, buff, BUFF_SIZE);
            len--; // remove LF; \n(10, LF), \r(13, CR)
            if (len == 0) {
                continue;
            }
            buff[len] = 0;
            logger->info("input {}, len {}", buff, len);
            auto input = process_input(buff);
            if (input.type == input_type_t::CMD_CONNECT) {
                if (!client->working()) {
                    if (client->start(input.payload)) {
                        message msg;
                        msg.type = messge_type_t::LOGIN;
                        msg.from = m_user_name;
                        msg.payload = m_user_password;
                        msg.timestamp = time(NULL);
                        message_handler::make_network_message(msg, buff, len);
                        ::write(client->get_fd(), buff, len);
                        // recv
                        std::vector<message> msgs;
                        while (msgs.empty()) {
                            len = ::read(client->get_fd(), buff, BUFF_SIZE);
                            msgs = client->get_handler().receive_message(buff, len);
                        }

                        if (msgs.size() >= 1 && msgs[0].type == messge_type_t::LOGIN && msgs[0].from == "OK") {
                            m_prompt = input.payload + "|" + m_user_name + "> ";
                            output_msg_recv("hello " + m_user_name + "!!! " + msgs[0].payload);
                            for (int i = 1; i < msgs.size(); i++) {
                                if (i == 1) {
                                    ::write(STDOUT_FILENO, m_prompt.data(), m_prompt.size());
                                    output_msg_recv("\n" + msgs[i].to_string());
                                }
                                else {
                                    output_msg_recv(msgs[i].to_string());
                                }
                            }
                            client->set_working();
                        }
                        else {
                            output_msg_recv("hello " + m_user_name + "!!! " + msgs[0].payload);
                            client->stop();
                        }
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
                if (client->working()) {
                    client->stop();
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
                if (client->working()) {
                    std::string msg = message_handler::send_message(m_user_name, input.payload, buff, len);
                    ::write(client->get_fd(), buff, len);
                    output_msg_echo(msg);
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

        if (FD_ISSET(client->get_fd(), &rfds)) {
            m_prompt_write = true;
            len = ::read(client->get_fd(), buff, BUFF_SIZE);
            logger->info("data coming, len {}", len);
            if (len > 0) {
                auto msgs = client->get_handler().receive_message(buff, len);
                for (const auto &msg : msgs) {
                    logger->info("type: {}, from: {}, to: {}, payload: {}, timestamp: {}", (int16_t) msg.type, msg.from, msg.to, msg.payload, msg.timestamp);
                    if (msg.type == messge_type_t::MSG) {
                        output_msg_recv("\n" + msg.to_string());
                    }
                    else if (msg.type == messge_type_t::HEARTBEAT) {
                        m_prompt_write = false;
                        logger->info("heartbeat mesage type: {}, from: {}, to: {}, payload: {}, timestamp: {}", (int16_t) msg.type, msg.from, msg.to, msg.payload, msg.timestamp);
                    }
                    else if (msg.type == messge_type_t::UNKNOWN) {
                        m_prompt_write = false;
                        logger->error("unknown message type: {}, from: {}, to: {}, payload: {}, timestamp: {}", (int16_t) msg.type, msg.from, msg.to, msg.payload, msg.timestamp);
                    }
                }
            }
            else {
                client->stop();
                logger->error("server shutdown");
                output_error("\nserver shutdown");
                m_prompt = m_user_name + "> ";
            }
        }

        ////// heartbeat
        {
            static time_t t = time(NULL);
            message msg_heartbeat;
            msg_heartbeat.type = messge_type_t::HEARTBEAT;
            msg_heartbeat.from = m_user_name;
            message_handler::make_network_message(msg_heartbeat, buff, len);
            if (difftime(time(NULL), t) > 10.0f) {
                t = time(NULL);
                ::write(client->get_fd(), buff, len);
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

void client_application::output_msg_echo(const std::string &msg) {
    // green
    ::write(STDOUT_FILENO, "\x1b[0;32;49m", 10);
    ::write(STDOUT_FILENO, msg.data(), msg.size());
    ::write(STDOUT_FILENO, "\n", 1);
    ::write(STDOUT_FILENO, "\x1b[0m", 4);
}

void client_application::output_msg_recv(const std::string &msg) {
    // green
    ::write(STDOUT_FILENO, "\x1b[0;33;49m", 10);
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