#pragma once

#include <string>
#include <unordered_map>
#include <set>
#include <fstream>
#include <sstream>
#include <cstdint>
#include <mutex>
#include <algorithm>

#include "Message.hpp"

class Database
{
public:

    // 加载用户文件（每次程序启动或文件变更后调用一次）
    // filepath: 文本文件路径
    // 返回 true 表示加载成功，false 则打开文件失败
    static bool LoadUsers(const std::string& filepath) {
        std::lock_guard<std::mutex> lk(m_Mtx);
        m_UserMap.clear();
        m_ReverseUserMap.clear();

        std::ifstream file(filepath);
        if (!file.is_open()) return false;

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;
            std::istringstream iss(line);
            std::string uidStr, username, password;
            // 用逗号分隔三段字段
            if (!std::getline(iss, uidStr, ','))    continue;
            if (!std::getline(iss, username, ','))  continue;
            if (!std::getline(iss, password, ','))  continue;
            uint32_t uid = 0;
            try {
                uid = static_cast<uint32_t>(std::stoull(uidStr));
            } catch (...) {
                continue; // 非法 UID 则跳过
            }
            m_UserMap[username] = { uid, password };
            m_ReverseUserMap[uid] = username;
        }
        return true;
    }

    // 根据 uid 获取对应的 username
    // 返回空字符串表示 uid 不存在
    static std::string GetUsername(uint32_t uid)
    {
        std::lock_guard<std::mutex> lk(m_Mtx);
        auto it = m_ReverseUserMap.find(uid);
        if (it == m_ReverseUserMap.end()) return "";
        return it->second;
    }

    // 根据 username 获取对应的 uid
    // 返回 0 表示 username 不存在
    static uint32_t GetUID(const std::string& username)
    {
        std::lock_guard<std::mutex> lk(m_Mtx);
        auto it = m_UserMap.find(username);
        if (it == m_UserMap.end()) return 0;
        return it->second.first;
    }

    // 根据 username 获取对应的 uid 和 password
    // 返回 true 表示找到了，false 表示用户名不存在
    static bool GetUser(const std::string& username,
                        uint32_t& outUid,
                        std::string& outPassword)
    {
        std::lock_guard<std::mutex> lk(m_Mtx);
        auto it = m_UserMap.find(username);
        if (it == m_UserMap.end()) return false;
        outUid      = it->second.first;
        outPassword = it->second.second;
        return true;
    }

    // 验证用户名和明文密码是否匹配
    // 返回 true 表示用户名存在且密码一致
    static bool Authenticate(const std::string& username,
                             const std::string& password)
    {
        std::lock_guard<std::mutex> lk(m_Mtx);
        auto it = m_UserMap.find(username);
        if (it == m_UserMap.end()) return false;
        return it->second.second == password;
    }


    static bool LoadFriends(const std::string& filename) {
        std::lock_guard<std::mutex> lock(m_Mtx);
        #if 0
        file_ = filename;
        #endif
        m_FriendsPairs.clear();

        std::ifstream ifs(filename);
        if (!ifs.is_open()) {
            // 文件不存在，后面 save 会创建
            return true;
        }

        std::string line;
        while (std::getline(ifs, line)) {
            if (line.empty()) continue;
            std::istringstream iss(line);
            uint32_t a, b;
            char comma;
            if (iss >> a >> comma >> b && comma == ',') {
                EnsureOrder(a, b);
                m_FriendsPairs.insert({a, b});
            }
        }
        return true;
    }

    #if 0
    // 将当前内存中的好友列表写回文件（覆写）
    static bool Save() {
        std::lock_guard<std::mutex> lock(m_Mtx);
        std::ofstream ofs(file_, std::ios::trunc);
        if (!ofs.is_open()) return false;

        for (auto &p : m_FriendsPairs) {
            ofs << p.first << ',' << p.second << '\n';
        }
        return true;
    }
    #endif

    // 添加一对好友（保证 uid1 < uid2），若新插入则自动 save
    static bool AddFriend(uint32_t uid1, uint32_t uid2) {
        EnsureOrder(uid1, uid2);
        std::lock_guard<std::mutex> lock(m_Mtx);
        auto [it, inserted] = m_FriendsPairs.insert({uid1, uid2});

        return inserted;  // 已存在也算成功

        #if 0
        if (inserted) {
            return save();
        }
        return true;  // 已存在也算成功
        #endif
    }

    // 删除一对好友，若存在则删除并自动 save
    static bool RemoveFriend(uint32_t uid1, uint32_t uid2) {
        EnsureOrder(uid1, uid2);
        std::lock_guard<std::mutex> lock(m_Mtx);
        size_t erased = m_FriendsPairs.erase({uid1, uid2});

        return (erased > 0);

        #if 0
        if (erased > 0) {
            return save();
        }
        return true;  // 不存在也算成功
        #endif
    }

    // 检查两个 uid 是否为好友
    static bool IsFriend(uint32_t uid1, uint32_t uid2) {
        EnsureOrder(uid1, uid2);
        std::lock_guard<std::mutex> lock(m_Mtx);
        return m_FriendsPairs.find({uid1, uid2}) != m_FriendsPairs.end();
    }

    // 获取指定 uid 的所有好友
    static void GetFriendsOf(uint32_t uid, std::vector<uint32_t>& friendlist) {
        std::lock_guard<std::mutex> lock(m_Mtx);
        for (const auto &p : m_FriendsPairs) {
            if (p.first == uid) {
                friendlist.push_back(p.second);
            } else if (p.second == uid) {
                friendlist.push_back(p.first);
            }
        }
    }

    // 将一条消息追加到文件末尾
    //TODO: 消息里有逗号/换行会把数据库文件搞坏
    static bool SaveMessage(const Message& msg, const std::string& filename) {
        std::lock_guard<std::mutex> lock(m_Mtx);
        std::ofstream ofs(filename, std::ios::app);
        if (!ofs) return false;
        ofs 
        << msg.m_TargetUID << ','
        << msg.m_SenderUID << ','
        << static_cast<uint32_t>(msg.m_Type) << ','
        << msg.m_Timestamp << ','
        << msg.m_Data
        << '\n';
        return true;
    }

    // 读取并删除给定 targetUID 的所有消息，返回这些消息
    //TODO: 缺少异常保护：遇到脏行会直接 throw
    static std::vector<Message> LoadMessages(uint32_t targetUID, const std::string& filename) {
        std::lock_guard<std::mutex> lock(m_Mtx);
        std::ifstream ifs(filename);
        if (!ifs) return {};

        std::vector<Message> result;
        std::vector<std::string> keepLines;
        std::string line;

        while (std::getline(ifs, line)) {
            std::istringstream iss(line);
            std::string field;
            uint32_t tgt, snd, ts;
            uint32_t typeInt;
            // 解析前四个字段
            if (!std::getline(iss, field, ',')) continue;
            tgt = std::stoul(field);
            if (!std::getline(iss, field, ',')) continue;
            snd = std::stoul(field);
            if (!std::getline(iss, field, ',')) continue;
            typeInt = static_cast<uint32_t>(std::stoul(field));
            if (!std::getline(iss, field, ',')) continue;
            ts = std::stoul(field);
            // 剩余即为 message content（可能含逗号）
            std::string data;
            if (!std::getline(iss, data)) data = "";

            if (tgt == targetUID) {
                Message msg;
                msg.m_TargetUID = tgt;
                msg.m_SenderUID = snd;
                msg.m_Type      = static_cast<MessageType>(typeInt);
                msg.m_Timestamp = ts;
                msg.m_Data      = data;
                msg.m_Size      = static_cast<uint32_t>(data.size());
                result.push_back(std::move(msg));
            } else {
                keepLines.push_back(line);
            }
        }

        // 按照时间戳顺序排序
        std::sort(result.begin(), result.end(),
        [](auto& a, auto& b) {
            return a.m_Timestamp < b.m_Timestamp;
        }
    );

        // 重写文件，只保留未被读取的
        std::ofstream ofs(filename, std::ios::trunc);
        for (auto &ln : keepLines) {
            ofs << ln << '\n';
        }
        return result;
    }

private:
    // 确保 a < b
    static void EnsureOrder(uint32_t &a, uint32_t &b) {
        if (a > b) std::swap(a, b);
    }

private:
    // 内部使用的缓存：username -> (uid, password)
    static std::unordered_map<std::string, std::pair<uint32_t, std::string>> m_UserMap;
    static std::unordered_map<uint32_t, std::string> m_ReverseUserMap;
    // 好友关系表
    static std::set<std::pair<uint32_t, uint32_t>> m_FriendsPairs;
    // 线程安全保护
    static std::mutex m_Mtx;
};