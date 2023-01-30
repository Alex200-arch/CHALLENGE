#ifndef CLIENT_APPLICATION_H_
#define CLIENT_APPLICATION_H_

#include <string>

enum class input_type_t : int8_t {
    CMD_CONNECT,
    CMD_DISCONNECT,
    CMD_QUIT,
    CMD_SEND,
    CMD_UNKNOWN
};

struct input_t {
    input_type_t type;
    std::string payload;
};

class client_application {
public:
    client_application(const std::string &);

    void run();

private:
    input_t process_input(const std::string &);
    void output_msg(const std::string &);
    void output_error(const std::string &);
    std::string m_user_name;
    std::string m_prompt;
};

#endif // CLIENT_APPLICATION_H_