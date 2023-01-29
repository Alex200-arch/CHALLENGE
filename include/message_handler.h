#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include <list>
#include <string>

enum class messge_type_t : int16_t {
    cmd = 1,
    msg = 2,
    unknown
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
    void add_message(char *, const int &);

protected:
private:
    void parse_message(char *, const int &);
    void make_message(const message &, char *, int32_t &);
    int m_fd;
    char m_buff[2048];
    int m_buff_w_pos{0};
    int m_buff_r_pos{0};
    std::list<message> m_messages;
};

#endif // MESSAGE_HANDLER_H