#include <iostream>
#include <thread>
#include <unistd.h>

#include "Message.hpp"
#include "User.hpp"
#include "Database.hpp"

#define SERVER_ID 0

class OnlineUsers
{
public:
    bool IsOnline(uint32_t uid)
    {
        std::lock_guard<std::mutex> lk(m_Mtx);
        return UID2Socket.find(uid) != UID2Socket.end();
    }

    int GetSocket(uint32_t uid)
    {
        std::lock_guard<std::mutex> lk(m_Mtx);
        auto it = UID2Socket.find(uid);
        return it != UID2Socket.end() ? it->second : -1;
    }

    bool SetOnline(uint32_t uid, int socket)
    {
        std::lock_guard<std::mutex> lk(m_Mtx);
        auto [it, inserted] = UID2Socket.emplace(uid, socket);
        return inserted;
    }

    bool SetOffline(uint32_t uid)
    {
        std::lock_guard<std::mutex> lk(m_Mtx);
        return UID2Socket.erase(uid) > 0;
    }

private:
    std::unordered_map<uint32_t, int> UID2Socket;
    std::mutex                       m_Mtx;
};

static OnlineUsers s_OnlineUsers;

// 处理每个客户端连接
void handle_client(int client_socket)
{
    User user;

    try
    {
        while (true)
        {
            Message msg;

            if(!msg.Receive(client_socket))
            {
                // 客户端断开时，移除该客户端
                if (s_OnlineUsers.SetOffline(user.GetUID()))
                    std::cout << user.GetUID() << " disconnected" << std::endl;
                break;
            }

            if (msg.m_Type == MessageType::HEARTBEAT)
            {
                std::cout << msg.m_SenderUID << " sent Heartbeat pkg" << std::endl;
            }
            else if (msg.m_Type == MessageType::LOGIN)
            {
                Message replyMsg;
                replyMsg.m_SenderUID = SERVER_ID;
                replyMsg.m_Type = MessageType::LOGIN;
                replyMsg.m_Timestamp = std::time(0);

                if (user.Login(msg.m_Data))
                {
                    if (s_OnlineUsers.IsOnline(user.GetUID()))
                    {
                        // TODO: login already online account
                        replyMsg.m_TargetUID = msg.m_SenderUID;
                        replyMsg.m_Data = "Conflict";
                        replyMsg.m_Size = replyMsg.m_Data.length();

                        replyMsg.Send(client_socket);

                        break;
                    }
                    else
                    {
                        if(s_OnlineUsers.SetOnline(user.GetUID(), client_socket))
                        {
                            std::cout << user.GetUID() << " connected" << std::endl;

                            replyMsg.m_TargetUID = user.GetUID();
                            replyMsg.m_Data = "Success";
                            replyMsg.m_Size = replyMsg.m_Data.length();

                            replyMsg.Send(client_socket);
                        }
                    }
                }
                else
                {
                    replyMsg.m_TargetUID = msg.m_SenderUID;
                    replyMsg.m_Data = "Failed";
                    replyMsg.m_Size = replyMsg.m_Data.length();

                    replyMsg.Send(client_socket);

                    break;
                }
            }
            else if (msg.m_Type == MessageType::PULL)
            {
                if (msg.m_Data == "FriendList")
                {
                    Message replyMsg;
                    replyMsg.m_TargetUID = msg.m_SenderUID;
                    replyMsg.m_SenderUID = SERVER_ID;
                    replyMsg.m_Type = MessageType::TEXT;
                    replyMsg.m_Timestamp = std::time(0);

                    std::vector<uint32_t> friendlist = user.GetFriendList();
                    std::ostringstream oss;
                    for (size_t i = 0; i < friendlist.size(); ++i)
                    {
                        // TODO: implement {username,uid;username2,uid2}
                        oss << Database::GetUsername(friendlist[i]) << ',' << friendlist[i];
                        if (i + 1 < friendlist.size()) 
                            oss << ';';
                    }

                    replyMsg.m_Data = oss.str();
                    replyMsg.m_Size = replyMsg.m_Data.length();

                    replyMsg.Send(client_socket);

                }
                else if (msg.m_Data == "OfflineMessage")
                {
                    std::vector<Message> offlinemsg = Database::LoadMessages(msg.m_SenderUID, "db/offlinemsg.txt");
                    for (auto message : offlinemsg)
                    {
                        message.Send(client_socket);
                    }
                }
            }
            else if (msg.m_Type == MessageType::TEXT)
            {
                // if online
                if (s_OnlineUsers.IsOnline(msg.m_TargetUID))
                {
                    int target_socket = s_OnlineUsers.GetSocket(msg.m_TargetUID);
                    msg.Send(target_socket);
                }
                else
                {
                    Database::SaveMessage(msg, "db/offlinemsg.txt");

                    // TODO: 离线消息不能有换行
                }
            }
        }
    }
    catch (...)
    {
        std::cerr << "Connection error" << std::endl;
    }

    close(client_socket);
}

int main()
{
    if (!Database::LoadUsers("db/users.txt"))
    {
        std::cerr << "Unable to open user database" << std::endl;
        return 1;
    }

    if (!Database::LoadFriends("db/friends.txt"))
    {
        std::cerr << "Unable to open friends database" << std::endl;
        return 1;
    }

    const std::string host("0.0.0.0");
    const uint32_t port = 25565;

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        std::cerr << "Socket creation failed." << std::endl;
        return -1;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(host.c_str());
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        std::cerr << "Bind failed." << std::endl;
        return -1;
    }

    if (listen(server_socket, 5) < 0)
    {
        std::cerr << "Listen failed." << std::endl;
        return -1;
    }

    std::cout << "Waiting for connection... " << host << ":" << port << std::endl;

    while (true)
    {
        int client_socket = accept(server_socket, NULL, NULL);
        std::thread(handle_client, client_socket).detach(); // 为每个客户端启动一个新线程
    }

    close(server_socket);

    return 0;
}
