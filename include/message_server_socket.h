#ifndef MESSAGE_SERVER_SOCKET_H
#define MESSAGE_SERVER_SOCKET_H

#include "message_server.h"

class message_server_socket : public message_server {
public:
    message_server_socket(const int &);
    bool start() override;
    int get_fd() override;
    int new_connection() override;

protected:
private:
    int m_port;
    int m_socket_fd{-1};
};

#endif // MESSAGE_SERVER_SOCKET_H