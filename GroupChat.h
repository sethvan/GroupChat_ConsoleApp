#ifndef GROUPCHAT_H_
#define GROUPCHAT_H_

#include <string>
#include <mutex>
#include <map>
#include <atomic>

class GroupChat
{
private:
    struct UserData
    {
        std::string password;
        bool online;
    };

    std::map<std::string, UserData> users;
    std::string user;
    std::string _password;
    std::atomic<bool> _online;
    std::mutex userList, groupChat;
    int lineCount;
    bool firstRead;

    void readUserList();
    void updateUserList();
    void createAccount();
    void logIn();
    void readChat();
    void writeChat(std::string &str, bool userName);
    void openSession();

public:
    void run();
};

#endif