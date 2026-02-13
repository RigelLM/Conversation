#include "User.hpp"

#include "Database.hpp"

bool User::Login(std::string data)
{
    size_t pos = data.find(":");
    if (pos != std::string::npos)
    {
        std::string username = data.substr(0, pos);
        std::string password = data.substr(pos + 1);

        if (Database::Authenticate(username, password))
        {
            m_Username = username;
            if (!Database::GetUser(username, m_UID, m_Password))
            {
                return false;
            }

            Database::GetFriendsOf(m_UID, m_FriendList);

            return true;
        }
        else
            return false;
    }
    else
        return false;
}