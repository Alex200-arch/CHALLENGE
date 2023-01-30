#ifndef MESSAGE_CLIENT_H
#define MESSAGE_CLIENT_H

#include <string>

class message_client {
public:
    virtual bool start(const std::string &) = 0;
    virtual void stop() = 0;
    virtual bool working() const = 0;
    virtual int get_fd() const = 0;

protected:
private:
};

#endif // MESSAGE_CLIENT_H