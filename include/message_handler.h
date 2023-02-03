#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include <list>
#include <sstream>
#include <string>
#include <vector>

enum class messge_type_t : int16_t {
    LOGIN = 1,
    MSG = 2,
    UNKNOWN
};

struct message {
    message() = default;
    message(messge_type_t, std::string, std::string, std::string, long);
    std::string to_string() const;

    messge_type_t type;
    std::string from;
    std::string to;
    std::string payload;
    long timestamp;
};

class message_handler {
public:
    message_handler() = default;
    std::vector<message> receive_message(char *, const int &);
    static std::string send_message(const std::string &, const std::string &, char *, int &);
    static void make_network_message(const message &, char *, int32_t &);

protected:
private:
    void parse_network_message(char *, const int &);
    std::vector<message> process_message();
    static message parse_input_message(const std::string &, const std::string &);

    char m_buff[2048];
    int m_buff_w_pos{0};
    int m_buff_r_pos{0};
    std::list<message> m_messages;
};

#endif // MESSAGE_HANDLER_H