#ifndef MESSAGE_SERVER_H
#define MESSAGE_SERVER_H

#include <unordered_map>

#include "message_handler.h"

class message_server {
public:
    virtual bool start() = 0;
    virtual void stop() = 0;

protected:
    void add_handler(int fd) {
        if (auto it = m_handlers.find(fd); it == m_handlers.end()) {
            m_handlers[fd] = std::make_shared<message_handler>();
        }
    }

    void login_handler(const std::string &user_name, int fd) {
        m_name_to_fd[user_name] = fd;
        m_fd_to_name[fd] = user_name;
    }

    void del_handler(int fd) {
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

    std::shared_ptr<message_handler> get_handler_by_fd(int fd) {
        std::shared_ptr<message_handler> ret;
        if (auto it = m_handlers.find(fd); it != m_handlers.end()) {
            ret = m_handlers[fd];
        }
        return ret;
    }

    int get_fd_by_name(const std::string &user_name) {
        if (auto it = m_name_to_fd.find(user_name); it != m_name_to_fd.end()) {
            return it->second;
        }
        return -1;
    }

    std::vector<int> get_others_fds_by_name(const std::string &user_name) {
        std::vector<int> ret;
        for (const auto &[name, fd] : m_name_to_fd) {
            if (name != user_name) {
                ret.push_back(fd);
            }
        }
        return ret;
    }

private:
    std::unordered_map<int, std::shared_ptr<message_handler>> m_handlers;
    std::unordered_map<std::string, int> m_name_to_fd;
    std::unordered_map<int, std::string> m_fd_to_name;
};

#endif // MESSAGE_SERVER_H