#ifndef MESSAGE_CLIENT_CREATOR_H
#define MESSAGE_CLIENT_CREATOR_H

#include "message_client.h"
#include "message_client_socket.h"

template <typename T>
class message_client_creator {
public:
    template <typename... Args>
    message_client *create_message_client(Args &&...args) {
        return nullptr;
    }
};

template <>
class message_client_creator<message_client_socket> {
public:
    template <typename... Args>
    message_client *create_message_client(Args &&...args) {
        return new message_client_socket(std::forward<Args>(args)...);
    }
};

#endif // MESSAGE_CLIENT_CREATOR_H