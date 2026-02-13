#pragma once

#include <cstdint>
#include <string>

#include "NetCompat.hpp"

enum class MessageType : uint8_t
{
    HEARTBEAT = 0, LOGIN = 1, PULL = 2,
    TEXT = 3
};

class Message
{
public:
    static constexpr int HeaderSize = 4 * sizeof(uint32_t) + sizeof(uint8_t);

    uint32_t    m_TargetUID;
    uint32_t    m_SenderUID;
    MessageType m_Type;
    uint32_t    m_Timestamp;
    uint32_t    m_Size;

    std::string m_Data;

public:
    Message() = default;
    ~Message() = default;

    bool Receive(SocketHandle socket);

    bool Send(SocketHandle socket) const;

private:
    // 将协议数据打包成字节流
    void SerializeHeader(char* buffer) const
    {
        uint32_t t_uid_network = htonl(m_TargetUID);          // 将 m_TargetUID 转换为网络字节序
        uint32_t s_uid_network = htonl(m_SenderUID);
        uint32_t timestamp_network = htonl(m_Timestamp);
        uint32_t msgSize_network = htonl(m_Size);

        // 将数据按顺序写入缓冲区
        memcpy(buffer, &t_uid_network, sizeof(m_TargetUID));    // UID
        memcpy(buffer + sizeof(m_TargetUID), &s_uid_network, sizeof(m_SenderUID));
        memcpy(buffer + sizeof(m_TargetUID) + sizeof(m_SenderUID), &m_Type, sizeof(m_Type));  // MsgType
        memcpy(buffer + sizeof(m_TargetUID) + sizeof(m_SenderUID) + sizeof(m_Type), &timestamp_network, sizeof(m_Timestamp));  // Timestamp
        memcpy(buffer + sizeof(m_TargetUID) + sizeof(m_SenderUID) + sizeof(m_Type) + sizeof(m_Timestamp), &msgSize_network, sizeof(m_Size));  // MsgSize
    }

    // 解析接收到的字节流
    void DeserializeHeader(const char* buffer)
    {
        uint32_t t_uid_network, s_uid_network, timestamp_network, msgSize_network;

        memcpy(&t_uid_network, buffer, sizeof(m_TargetUID)); // UID
        memcpy(&s_uid_network, buffer + sizeof(m_TargetUID), sizeof(m_SenderUID));
        memcpy(&m_Type, buffer + sizeof(m_TargetUID) + sizeof(m_SenderUID), sizeof(m_Type)); // MsgType
        memcpy(&timestamp_network, buffer + sizeof(m_TargetUID) + sizeof(m_SenderUID) + sizeof(m_Type), sizeof(m_Timestamp)); // Timestamp
        memcpy(&msgSize_network, buffer + sizeof(m_TargetUID) + sizeof(m_SenderUID) + sizeof(m_Type) + sizeof(m_Timestamp), sizeof(m_Size)); // MsgSize

        m_TargetUID = ntohl(t_uid_network);
        m_SenderUID = ntohl(s_uid_network);
        m_Timestamp = ntohl(timestamp_network);
        m_Size = ntohl(msgSize_network);
    }
};