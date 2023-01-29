#ifndef CLIENT_APPLICATION_H_
#define CLIENT_APPLICATION_H_

#include <string>

class client_application {
public:
    client_application(const std::string &);

    void run();

private:
    std::string m_user_name;
    std::string m_prompt;
};

#endif // CLIENT_APPLICATION_H_