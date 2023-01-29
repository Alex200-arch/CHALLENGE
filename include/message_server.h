#ifndef MESSAGE_SERVER_H
#define MESSAGE_SERVER_H

class message_server {
public:
    virtual bool start() = 0;
    virtual void stop() = 0;

protected:
private:
};

#endif // MESSAGE_SERVER_H