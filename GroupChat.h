#ifndef GROUPCHAT_H_
#define GROUPCHAT_H_

#include <string>
#include <map>
#include <atomic>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>

class GroupChat
{
private:
    struct UserData
    {
        std::string password;
        bool online;
    };

    std::map<std::string, UserData> users; // Key will be users' names
    std::string user;
    std::string _password;
    std::atomic<bool> _online;

    sem_t *userSem;
    sem_t *chatSem;
    int lineCount;
    bool othersOnline;
    bool firstRead;

    void mainMenu();
    void readUserList();   // Updates the map "users" from userList.txt so this process is aprised
    void updateUserList(); // Updates userList.txt so that it reflects the "user" status from this process
    void createAccount();
    void logIn();
    void readChat();
    void writeChat(const std::string &str, bool includeUserName);
    void openSession();

public:
    void run();
};

#endif
