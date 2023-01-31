#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include <list>
#include <string>

enum class messge_type_t : int16_t {
    CMD = 1,
    MSG = 2,
    UNKNOWN
};

struct message {
    messge_type_t type;
    std::string from;
    std::string to;
    std::string payload;
    long timestamp;
};

class message_handler {
public:
    message_handler();
    void receive_message(char *, const int &);
    void send_message(const std::string &, const std::string &, char *, int &);

protected:
private:
    void parse_network_message(char *, const int &);
    void process_message();
    void make_network_message(const message &, char *, int32_t &);
    message parse_input_message(const std::string &, const std::string &);
    char m_buff[2048];
    int m_buff_w_pos{0};
    int m_buff_r_pos{0};
    std::list<message> m_messages;
};

#endif // MESSAGE_HANDLER_H