#ifndef MESSAGE_SERVER_CREATOR_H
#define MESSAGE_SERVER_CREATOR_H

#include "message_server.h"
#include "message_server_socket.h"

template <typename T>
class message_server_creator {
public:
    message_server *create_message_server() {
        return nullptr;
    }
};

template <>
class message_server_creator<message_server_socket> {
public:
    message_server *create_message_server() {
        return new message_server_socket();
    }
};

#endif // MESSAGE_SERVER_CREATOR_H