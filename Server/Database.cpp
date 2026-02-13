#include "Database.hpp"

// 在 .cpp 文件中定义静态成员
std::unordered_map<std::string, std::pair<uint32_t, std::string>> Database::m_UserMap;
std::unordered_map<uint32_t, std::string> Database::m_ReverseUserMap;
std::set<std::pair<uint32_t, uint32_t>> Database::m_FriendsPairs;
std::mutex Database::m_Mtx;