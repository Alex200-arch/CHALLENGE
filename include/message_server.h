#ifndef MESSAGE_SERVER_H
#define MESSAGE_SERVER_H

#include <unordered_map>

#include "message_handler.h"

class message_server {
public:
    virtual bool start() = 0;
    virtual void stop() = 0;

protected:
    void add_handler(int fd) {
        if (auto it = m_handlers.find(fd); it == m_handlers.end()) {
            m_handlers[fd] = std::make_shared<message_handler>();
        }
    }

    void del_handler(int fd) {
        if (auto it = m_handlers.find(fd); it != m_handlers.end()) {
            m_handlers.erase(it);
        }
    }

    std::shared_ptr<message_handler> get_handler_by_fd(int fd) {
        std::shared_ptr<message_handler> ret;
        if (auto it = m_handlers.find(fd); it != m_handlers.end()) {
            ret = m_handlers[fd];
        }
        return ret;
    }

private:
    std::unordered_map<int, std::shared_ptr<message_handler>> m_handlers;
};

#endif // MESSAGE_SERVER_H