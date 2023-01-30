#ifndef MESSAGE_CLIENT_SOCKET_H
#define MESSAGE_CLIENT_SOCKET_H

#include "message_client.h"

class message_client_socket : public message_client {
public:
    bool start(const std::string &) override;
    void stop() override;
    bool working() const override;
    int get_fd() const override;

protected:
private:
    int m_fd{-1};
};

#endif // MESSAGE_CLIENT_SOCKET_H