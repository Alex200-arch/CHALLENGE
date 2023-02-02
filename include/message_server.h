#ifndef MESSAGE_SERVER_H
#define MESSAGE_SERVER_H

#include <unordered_map>

#include "message_handler.h"

class message_server {
public:
    message_server();
    virtual ~message_server();
    virtual bool start() = 0;
    virtual int get_fd() = 0;
    virtual int new_connection() = 0;

    void add_handler(int);
    void login_handler(const std::string &, int);
    void del_handler(int);
    std::shared_ptr<message_handler> get_handler_by_fd(int);
    int get_fd_by_name(const std::string &) const;
    std::vector<int> get_others_fds_by_name(const std::string &) const;
    std::optional<bool> check_password(const std::string &, const std::string &) const;
    void save_password(const std::string &, const std::string &);
    void save_msg_for_off_line_user(const std::string &, const message &);
    std::vector<message> get_off_line_msg(const std::string &);

protected:
private:
    std::unordered_map<int, std::shared_ptr<message_handler>> m_handlers;
    std::unordered_map<std::string, int> m_name_to_fd;
    std::unordered_map<int, std::string> m_fd_to_name;
    std::unordered_map<std::string, std::string> m_name_to_password;
    std::unordered_map<std::string, std::vector<message>> m_storage_name_to_msg;
};

#endif // MESSAGE_SERVER_H