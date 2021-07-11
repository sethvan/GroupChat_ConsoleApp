#ifndef CHATAPP_H_
#define CHATAPP_H_

#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <algorithm>
#include <mutex>
#include <fstream>
#include "User.h"

class GroupChat
{
    private:
    std::vector<User> users;
    User user;
    std::mutex ul, gc;
    int lineCount {0};

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