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

    std::string to_string() {
        // timestamp (broadcast from) from: payload
        // 2021/06/04 15:35:40 tom: hi xx
        // 2021/06/04 15:35:40 broadcast from tom: hi xx
        struct tm timeinfo;
        localtime_r(&timestamp, &timeinfo);
        char time_formator[20] = {0};
        sprintf(time_formator, "%d/%.2d/%.2d %.2d:%.2d:%.2d", timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        std::string ret(time_formator);
        if (to == "all") {
            ret += " broadcast from";
        }
        ret += " " + from + ": ";
        ret += payload;
        return ret;
    }
};

class message_handler {
public:
    message_handler();
    void receive_message(char *, const int &);
    std::string send_message(const std::string &, const std::string &, char *, int &);

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