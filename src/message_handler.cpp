#include "message_handler.h"
#include "log.h"

message_handler::message_handler() {
}

void message_handler::receive_message(char *buff, const int &len) {
    parse_network_message(buff, len);
    process_message();
}

void message_handler::send_message(const std::string &from, const std::string &buff, char *out_buff, int &out_len) {
    parse_input_message(from, buff);
}

/*
protocol
package_length  4
type            2
from_length     4
xxx             from_length
to_length       4
xxx             to_length
payload_length  4
xxx             payload_length
*/
void message_handler::parse_network_message(char *buff, const int &len) {
    memcpy(m_buff + m_buff_w_pos, buff, len);
    m_buff_w_pos += len;
    while ((m_buff_w_pos - m_buff_r_pos) >= 4) {
        int32_t *p_package_length = (int32_t *) (m_buff + m_buff_r_pos);
        if (*p_package_length <= (m_buff_w_pos - m_buff_r_pos)) {
            message msg;

            msg.type = static_cast<messge_type_t>(*(int16_t *) (m_buff + m_buff_r_pos + 4));

            int32_t *p_from_length = (int32_t *) (m_buff + m_buff_r_pos + 4 + 2);
            msg.from = std::string(m_buff + m_buff_r_pos + 4 + 2 + 4, *p_from_length);

            int32_t *p_to_length = (int32_t *) (m_buff + m_buff_r_pos + 4 + 2 + 4 + *p_from_length);
            msg.to = std::string(m_buff + m_buff_r_pos + 4 + 2 + 4 + *p_from_length + 4, *p_to_length);

            int32_t *p_payload_length = (int32_t *) (m_buff + m_buff_r_pos + 4 + 2 + 4 + *p_from_length + 4 + *p_to_length);
            msg.payload = std::string(m_buff + m_buff_r_pos + 4 + 2 + 4 + *p_from_length + 4 + *p_to_length + 4, *p_payload_length);

            msg.timestamp = *(long *) (m_buff + m_buff_r_pos + 4 + 2 + 4 + *p_from_length + 4 + *p_to_length + 4 + *p_payload_length);

            m_messages.push_back(msg);

            m_buff_r_pos += *p_package_length;

            if (m_buff_w_pos == m_buff_r_pos) {
                m_buff_w_pos = 0;
                m_buff_r_pos = 0;
            }
        }
        else {
            if (m_buff_w_pos > m_buff_r_pos && m_buff_r_pos > 0) {
                int remain_len = m_buff_w_pos - m_buff_r_pos;
                memcpy(m_buff, m_buff + m_buff_r_pos, remain_len);
                m_buff_w_pos = remain_len;
                m_buff_r_pos = 0;
            }
            break;
        }
    }
}

void message_handler::process_message() {
    while (!m_messages.empty()) {
        auto &msg = m_messages.front();
        logger->info("type: {}, from: {}, to: {}, payload: {}, timestamp: {}", (int16_t) msg.type, msg.from, msg.to, msg.payload, msg.timestamp);
        m_messages.pop_front();
    }
}

void message_handler::make_network_message(const message &msg, char *buff, int32_t &len) {
    int32_t from_length = msg.from.size();
    int32_t to_length = msg.to.size();
    int32_t payload_length = msg.payload.size();
    len = 4 + 2 + 4 + from_length + 4 + to_length + 4 + payload_length;
    memcpy(buff, &len, 4);
    memcpy(buff + 4, &msg.type, 2);
    memcpy(buff + 4 + 2, &from_length, 4);
    memcpy(buff + 4 + 2 + 4, msg.from.data(), from_length);
    memcpy(buff + 4 + 2 + 4 + from_length, &to_length, 4);
    memcpy(buff + 4 + 2 + 4 + from_length + 4, msg.to.data(), to_length);
    memcpy(buff + 4 + 2 + 4 + from_length + 4 + to_length, &payload_length, 4);
    memcpy(buff + 4 + 2 + 4 + from_length + 4 + to_length + 4, msg.payload.data(), payload_length);
    memcpy(buff + 4 + 2 + 4 + from_length + 4 + to_length + 4 + payload_length, &msg.timestamp, sizeof(msg.timestamp));
}

message message_handler::parse_input_message(const std::string &from, const std::string &buff) {
    size_t pos = buff.find(' ');

    message msg;

    msg.type = messge_type_t::MSG;
    msg.from = from;
    msg.to = buff.substr(0, pos);
    msg.payload = buff.substr(pos + 1);
    msg.timestamp = time(NULL);

    return msg;
}