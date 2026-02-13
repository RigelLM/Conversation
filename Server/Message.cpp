#include "Message.hpp"

#include <vector>

bool Message::Receive(SocketHandle socket)
{
    char buffer[HeaderSize];
    int bytes_received = recv(socket, buffer, HeaderSize, 0);
    if (bytes_received <= 0)
    {
        return false;
    }

    DeserializeHeader(buffer);

    m_Data.clear();
    if (m_Size == 0) return true;

    std::vector<char> dataBuffer(m_Size + 1);
    bytes_received = recv(socket, dataBuffer.data(), static_cast<int>(m_Size), 0);
    if (bytes_received <= 0) return false;
    dataBuffer[bytes_received] = '\0';  // 确保字符串结束

    m_Data.assign(dataBuffer.data(), m_Size);
    return true;
}

bool Message::Send(SocketHandle socket) const
{
    std::vector<char> buffer(HeaderSize + m_Size);
    
    SerializeHeader(buffer.data());

    if (m_Size > 0) {
        std::memcpy(buffer.data() + HeaderSize, m_Data.data(), m_Size);
    }

    int bytes_sent = send(socket, buffer.data(), static_cast<int>(buffer.size()), 0);

    return bytes_sent > 0;
}