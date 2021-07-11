#ifndef GROUPCHAT_H_
#define GROUPCHAT_H_

#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <algorithm>
#include <mutex>
#include <fstream>


class GroupChat
{
    typedef struct User
{
    std::string name = "";
    std::string password = "";
    bool online = false;
    bool operator==(const User& rhs){return name == rhs.name;}
}User;

private:
    std::vector<User> users;
    User user;
    std::mutex ul, gc;
    int lineCount {0};
    bool firstRead = true;

public:
    void run();
    void readUserList();
    void updateUserList();
    void createAccount();
    void logIn();
    void readChat();
    void writeChat(std::string& str, bool userName);
    void openSession();
};

#endif