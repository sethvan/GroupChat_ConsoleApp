#include <thread>
#include <algorithm>
#include <iostream>
#include "GroupChat.h"
#include <limits>
#include <fstream>
#include <iomanip>

using namespace std::literals::chrono_literals;

void GroupChat::run()
{
    lineCount = 0;
    firstRead = true;
    _online = true;
    readUserList();

    if (!othersOnline)
    {
        sem_unlink("userListSem");
        sem_unlink("groupChatSem");
    }
    if ((userSem = sem_open("userListSem", othersOnline ? 0 : O_CREAT, 0644, 1)) == SEM_FAILED)
    {
        std::cout << "semaphore initilization failed";
        exit(1);
    }
    if ((chatSem = sem_open("groupChatSem", othersOnline ? 0 : O_CREAT, 0644, 1)) == SEM_FAILED)
    {
        std::cout << "semaphore initilization failed";
        exit(1);
    }

    mainMenu();
}

void GroupChat::mainMenu()
{
    while (true)
    {
        std::cout << "1.)Log in\n2.)Create Account\n3.)Exit\n:";
        char choice;
        std::cin >> choice;
        switch (choice)
        {
        case '1':
            logIn();
            break;
        case '2':
            createAccount();
            break;
        case '3':
            exit(0);
        default:
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "\nInvalid choice!\n\n";
        }
    }
}

void GroupChat::readUserList()
{
    if (!firstRead && users.size())
        sem_wait(userSem);

    std::ifstream inFile("Users.txt");
    if (!inFile)
        return;

    users.clear();

    inFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        othersOnline = false;
        int loops;
        inFile >> loops;
        inFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        std::string name, password, online_str;
        for (int i{0}; i < loops; ++i)
        {
            std::getline(inFile, name);
            std::getline(inFile, password);
            std::getline(inFile, online_str);

            users[std::move(name)] = UserData{std::move(password), online_str != "false"};
            if (online_str == "true")
                othersOnline = true;
        }
    }
    catch (const std::ifstream::failure &e)
    {
        std::cerr << "Exception opening/reading/closing \"Users.txt\"\n";
    }
    if (!firstRead && users.size())
        sem_post(userSem);
}

void GroupChat::updateUserList()
{
    readUserList();
    sem_wait(userSem);

    std::ofstream outFile("Users.txt");
    outFile.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    try
    {
        if (!users.contains(user))
            users[user].password = std::move(_password); //This happens when creating account

        users[user].online = _online; //This is checked every time list updated

        outFile << users.size() << std::endl;
        outFile << std::boolalpha;

        auto it = users.begin();
        while (it != users.end())
        {
            outFile << it->first << "\n";
            outFile << it->second.password << "\n";
            outFile << it->second.online << '\n';
            ++it;
        }
    }
    catch (const std::ofstream::failure &e)
    {
        std::cerr << "Exception opening/writing/appending/closing \"Users.txt\"\n";
    }
    sem_post(userSem);
}

void GroupChat::createAccount()
{
    std::cout << "\n\n***************Create an Account***************\n\n";
    std::cout << "Please enter a username: ";
    std::string name;
    std::cin >> name;

    for (auto c : name)
    {
        if (c == ':')
        {
            std::cout << user << "\n";
            std::cout << "Username cannot contain \':\' character \n";
            mainMenu();
        }
    }

    readUserList();
    if (users.contains(name))
    {
        std::cout << "Username already in use!\n";
        mainMenu();
    }

    user = name;
    std::cout << "Please enter a password: ";
    std::cin >> _password;
    updateUserList();
    openSession();
}

void GroupChat::logIn()
{
    std::cout << "\n\n******************** Log In *******************\n\n";
    std::cout << "Please enter your username: ";
    std::string name;
    std::cin >> name;

    readUserList();
    if (!users.contains(name))
    {
        std::cout << "\nUsername does not exist! Please create an account.\n";
        createAccount();
    }

    if (users[name].online)
    {
        std::cout << "\nThat User is already currently in an open session!\n\n";
        mainMenu();
    }

    int wrongGuesses = 0;
    do
    {
        std::cout << "Please enter your password: ";
        std::cin >> _password;

        if (users[name].password == _password)
        {
            user = name;
            updateUserList();
            openSession();
        }
        else
            std::cout << "Password is incorrect!\n";

        if (++wrongGuesses == 3)
        {
            std::cout << "Three incorrect guesses! Returning to main menu.\n\n";
            mainMenu();
        }
    } while (users[name].password != _password);
}

void GroupChat::readChat()
{
    sem_wait(chatSem);

    std::ifstream inFile("groupChat.txt");
    if (!inFile)
        return;

    inFile.exceptions(std::ifstream::badbit);
    try
    {
        int count = 0;
        std::string line;
        std::string screenName = user + ":";

        while (std::getline(inFile, line))
        {
            if (++count <= lineCount)
                continue;

            if (line.find(screenName) != 0 || firstRead) //Displays previous chat history(listing user's name in their comments)..
                std::cout << line << "\n";               //..and does not display user's current comments as they will already be displayed..
                                                         //..via their input to app in terminal
            lineCount = count;
        }
        firstRead = false;
    }
    catch (const std::ifstream::failure &e)
    {
        std::cerr << "Exception opening/reading/closing \"groupChat.txt\"\n";
    }
    sem_post(chatSem);
}

void GroupChat::writeChat(const std::string &str, bool includeUserName)
{
    sem_wait(chatSem);

    std::ofstream outFile;
    outFile.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    try
    {
        outFile.open("groupChat.txt", std::ios_base::app);
        if (includeUserName)
            outFile << user << ": " << str << "\n\n";
        else
            outFile << str << "\n\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception opening/writing/closing \"groupChat.txt\"\n";
    }
    sem_post(chatSem);
}

void GroupChat::openSession()
{
    const std::string prefix = "User ";
    const std::string outro = prefix + user + " has left the chat...";
    const std::string intro = prefix + user + " has entered the chat...";

    auto displayChat = [&]
    {
        while (_online)
        {
            readChat();
            std::this_thread::sleep_for(.1s);
        }
    };
    std::thread displayThread(displayChat);

    writeChat(intro, false);
    std::this_thread::sleep_for(.5s);
    std::cout << ">>>You are now enterring the group chat.\n"
              << " Please type the tilde '~' and then the 'enter' key when you wish to leave<<<\n"
              << std::endl;

    std::string str;
    while (true)
    {
        std::getline(std::cin >> std::ws, str);
        if (str == "~")
        {
            _online = false;
            writeChat(outro, false);
            updateUserList();

            if (!othersOnline)
            {
                sem_close(userSem);
                sem_close(chatSem);
            }

            displayThread.join();
            exit(0);
        }
        writeChat(str, true);
    }
}
