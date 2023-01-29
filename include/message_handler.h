#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include <string>

enum class messge_type_t {
    cmd,
    msg
};

struct message {
    messge_type_t type;
    std::string from;
    std::string to;
    std::string payload;
};

class message_handler {
public:
    message_handler(const int &);
    void add_message(const std::string &);

protected:
private:
    void parse_message();
    int m_fd;
};

#endif // MESSAGE_HANDLER_H