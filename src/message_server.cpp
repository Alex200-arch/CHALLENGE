#include <unordered_map>

#include "csv.hpp"
#include "message_server.h"

message_server::message_server() {
    if (std::ifstream f("usr_info.csv"); f.good()) {
        csv::CSVReader reader_user_info("usr_info.csv");
        for (csv::CSVRow &row : reader_user_info) {
            m_name_to_password[row[0].get<>()] = row[1].get<>();
        }
    }

    if (std::ifstream f("off_line_message.csv"); f.good()) {
        csv::CSVReader reader_offline("off_line_message.csv");
        for (csv::CSVRow &row : reader_offline) {
            // name,type,from,to,payload,timestamp
            m_storage_name_to_msg[row[0].get<>()].emplace_back(static_cast<messge_type_t>(row[1].get<int16_t>()), row[2].get<>(), row[3].get<>(), row[4].get<>(), row[5].get<long>());
        }
    }
}

message_server::~message_server() {
    std::ofstream outfile_user_info;
    outfile_user_info.open("usr_info.csv", std::ofstream::out | std::ofstream::trunc);
    auto writer_user_info = csv::make_csv_writer(outfile_user_info);
    writer_user_info << std::vector<std::string>({"name", "password"});
    for (const auto &[name, password] : m_name_to_password) {
        writer_user_info << std::vector<std::string>({name, password});
    }

    std::ofstream outfile_offline;
    outfile_offline.open("off_line_message.csv", std::ofstream::out | std::ofstream::trunc);
    auto writer_off_line = csv::make_csv_writer(outfile_offline);
    writer_off_line << std::vector<std::string>({"name", "type", "from", "to", "payload", "timestamp"});
    for (const auto &[name, msgs] : m_storage_name_to_msg) {
        for (const auto &msg : msgs) {
            writer_off_line << std::make_tuple(name, static_cast<int16_t>(msg.type), msg.from, msg.to, msg.payload, msg.timestamp);
        }
    }
}

void message_server::add_handler(int fd) {
    if (auto it = m_handlers.find(fd); it == m_handlers.end()) {
        m_handlers[fd] = std::make_shared<message_handler>();
    }
}

void message_server::login_handler(const std::string &user_name, int fd) {
    m_name_to_fd[user_name] = fd;
    m_fd_to_name[fd] = user_name;
}

void message_server::del_handler(int fd) {
    if (auto it = m_handlers.find(fd); it != m_handlers.end()) {
        m_handlers.erase(it);
    }

    if (auto it = m_fd_to_name.find(fd); it != m_fd_to_name.end()) {
        if (auto it_n = m_name_to_fd.find(it->second); it_n != m_name_to_fd.end()) {
            m_name_to_fd.erase(it_n);
        }
        m_fd_to_name.erase(it);
    }
}

std::shared_ptr<message_handler> message_server::get_handler_by_fd(int fd) {
    std::shared_ptr<message_handler> ret;
    if (auto it = m_handlers.find(fd); it != m_handlers.end()) {
        ret = m_handlers[fd];
    }
    return ret;
}

int message_server::get_fd_by_name(const std::string &user_name) const {
    if (auto it = m_name_to_fd.find(user_name); it != m_name_to_fd.end()) {
        return it->second;
    }
    return -1;
}

std::vector<int> message_server::get_others_fds_by_name(const std::string &user_name) const {
    std::vector<int> ret;
    for (const auto &[name, fd] : m_name_to_fd) {
        if (name != user_name) {
            ret.push_back(fd);
        }
    }
    return ret;
}

std::optional<bool> message_server::check_password(const std::string &user_name, const std::string &password) const {
    if (auto it = m_name_to_password.find(user_name); it != m_name_to_password.end()) {
        if (it->second == password) {
            return true;
        }
        return false;
    }
    return {};
}

void message_server::save_password(const std::string &user_name, const std::string &password) {
    m_name_to_password[user_name] = password;
}

void message_server::save_msg_for_off_line_user(const std::string &user_name, const message &msg) {
    if (auto it = m_name_to_password.find(user_name); it != m_name_to_password.end()) {
        m_storage_name_to_msg[user_name].push_back(msg);
    }
}

std::vector<message> message_server::get_off_line_msg(const std::string &user_name) {
    if (auto it = m_storage_name_to_msg.find(user_name); it != m_storage_name_to_msg.end()) {
        std::vector<message> msgs(it->second);
        m_storage_name_to_msg.erase(it);
        return msgs;
    }
    return {};
}
