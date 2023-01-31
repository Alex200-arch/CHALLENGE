#ifndef MESSAGE_CLIENT_H
#define MESSAGE_CLIENT_H

#include <string>

#include "message_handler.h"

class message_client {
public:
    virtual bool start(const std::string &) = 0;
    virtual void stop() = 0;
    virtual bool working() const = 0;
    virtual int get_fd() const = 0;

    message_handler &get_handler() {
        return m_handler;
    }

protected:
private:
    message_handler m_handler;
};

#endif // MESSAGE_CLIENT_H