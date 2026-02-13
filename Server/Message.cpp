#include "Message.hpp"

bool Message::Receive(uint32_t socket)
{
    char buffer[HeaderSize];
    int bytes_received = recv(socket, buffer, HeaderSize, 0);
    if (bytes_received <= 0)
    {
        return false;
    }

    DeserializeHeader(buffer);

    char dataBuffer[m_Size + 1];
    bytes_received = recv(socket, dataBuffer, m_Size, 0);
    if (bytes_received <= 0)
    {
        return false;
    }
    dataBuffer[bytes_received] = '\0';  // 确保字符串结束

    m_Data = dataBuffer;
    return true;
}

bool Message::Send(uint32_t socket) const
{
    char buffer[HeaderSize + m_Size];
    
    SerializeHeader(buffer);

    memcpy(buffer + HeaderSize, m_Data.c_str(), m_Size);

    int bytes_sent = send(socket, buffer, HeaderSize + m_Size, 0);
    if (bytes_sent <= 0)
    {
        return false;
    }

    return true;
}