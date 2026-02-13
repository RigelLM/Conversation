#pragma once

#include <cstdint>
#include <string>
#include <vector>

class User
{
public:
    User() = default;
    ~User() = default;

    bool Login(std::string data);

    uint32_t GetUID() { return m_UID; }
    std::vector<uint32_t> GetFriendList() { return m_FriendList; }

private:
    uint32_t                m_UID;

    std::string             m_Username;
    std::string             m_Password;

    std::vector<uint32_t>   m_FriendList;
};