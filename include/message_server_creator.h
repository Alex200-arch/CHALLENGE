#ifndef MESSAGE_SERVER_CREATOR_H
#define MESSAGE_SERVER_CREATOR_H

#include "message_server.h"
#include "message_server_socket.h"

template <typename T>
class message_server_creator {
public:
    template <typename... Args>
    message_server *create_message_server(Args &&...args) {
        return nullptr;
    }
};

template <>
class message_server_creator<message_server_socket> {
public:
    template <typename... Args>
    message_server *create_message_server(Args &&...args) {
        return new message_server_socket(std::forward<Args>(args)...);
    }
};

#endif // MESSAGE_SERVER_CREATOR_H