#include <gtest/gtest.h>

#include "message_handler.h"

TEST(message, serialization) {
    char buff[4096];
    int32_t len;

    message send_msg;
    send_msg.type = messge_type_t::MSG;
    send_msg.from = "sender";
    send_msg.to = "receiver";
    send_msg.payload = "hello world";
    send_msg.timestamp = 123456;

    message_handler handler;
    message_handler::make_network_message(send_msg, buff, len);
    auto recv_msgs = handler.receive_message(buff, len);

    EXPECT_EQ(1, recv_msgs.size());
    EXPECT_EQ(send_msg.type, recv_msgs[0].type);
    EXPECT_EQ(send_msg.from, recv_msgs[0].from);
    EXPECT_EQ(send_msg.to, recv_msgs[0].to);
    EXPECT_EQ(send_msg.payload, recv_msgs[0].payload);
    EXPECT_EQ(send_msg.timestamp, recv_msgs[0].timestamp);
}
