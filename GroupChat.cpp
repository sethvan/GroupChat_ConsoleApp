#include <thread>
#include <algorithm>
#include <iostream>
#include "GroupChat.h"
#include <limits>
#include <fstream>

using namespace std::literals::chrono_literals;

void GroupChat::run()
{
    lineCount = 0;
    firstRead = true;
    _online = true;

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
    std::scoped_lock lock(userList);
    std::ifstream inFile("Users.txt");
    if (inFile)
    {
        users.clear();
        int loops;
        std::string name, password, online_str;
        bool online;

        inFile >> loops;
        inFile.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        for (int i{0}; i < loops; ++i)
        {
            std::getline(inFile, name);
            std::getline(inFile, password);
            std::getline(inFile, online_str);
            if (online_str == "false")
                online = false;
            else
                online = true;

            users[std::move(name)] = UserData{std::move(password), std::move(online)};
        }
    }
}

void GroupChat::updateUserList()
{
    readUserList();
    if (!users.contains(user))
        users[user].password = std::move(_password); //This happens when creating account

    users[user].online = _online; //This is checked every time list updated

    std::scoped_lock lock(userList);
    std::ofstream outFile("Users.txt");
    if (outFile)
    {
        outFile << users.size() << std::endl;

        auto it = users.begin();
        while (it != users.end())
        {
            outFile << it->first << "\n";
            outFile << it->second.password << "\n";
            if (it->second.online == true)
                outFile << "true" << std::endl;
            else
                outFile << "false" << std::endl;
            ++it;
        }
    }
}

void GroupChat::createAccount()
{
    std::cout << "\n\n***************Create an Account***************\n\n";

    std::cout << "Please enter a username: ";
    std::string name;
    std::cin >> name;

    readUserList();

    if (users.contains(name))
    {
        std::cout << "Username already in use!\n";
        run();
    }
    else
    {
        user = name;
        std::cout << "Please enter a password: ";
        std::cin >> _password;
        updateUserList();
        openSession();
    }
}

void GroupChat::logIn()
{
    std::cout << "\n\n******************** Log In *******************\n\n";

    std::cout << "Please enter your username: ";
    std::string name;
    std::cin >> name;

    readUserList();

    if (users.contains(name))
    {
        if (users[name].online)
        {
            std::cout << "\nThat User is already currently in an open session!\n\n";
            run();
        }
        else
        {
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
                    run();
                }
            } while (users[name].password != _password);
        }
    }
    else
    {
        std::cout << "\nUsername does not exist! Please create an account.\n";
        createAccount();
    }
}

void GroupChat::readChat()
{
    int count = 0;
    std::string line;
    std::scoped_lock lock(groupChat);

    std::ifstream inFile("groupChat.txt");
    if (inFile)
    {
        while (std::getline(inFile, line))
        {
            ++count;
            auto index = line.find(user);
            if (count > lineCount)
            {
                if (index != 0 || firstRead)   //Displays previous chat history(listing user's name in their comments)..
                {                              //..and does not display user's current comments as they will already be displayed..
                    std::cout << line << "\n"; //..via their input to app in terminal
                }
                lineCount = count;
            }
        }
        firstRead = false;
    }
}

void GroupChat::writeChat(std::string &str, bool userName)
{
    std::scoped_lock lock(groupChat);
    std::ofstream outFile;
    if (outFile)
    {
        outFile.open("groupChat.txt", std::ios_base::app);
        if (userName)
        {
            outFile << user << ": " << str << "\n\n";
        }
        else
            outFile << str << "\n\n";
    }
}

void GroupChat::openSession()
{
    auto displayChat = [&]
    {
        while (_online)
        {
            readChat();
            std::this_thread::sleep_for(.5s);
        }
    };
    std::thread displayThread(displayChat);

    std::this_thread::sleep_for(1s);

    std::string intro = "User ";
    std::string outro = intro + user + " has left the chat...";
    std::string str{" has entered the chat..."};
    intro = intro + user + str;

    writeChat(intro, false);

    std::this_thread::sleep_for(.5s);
    std::cout << ">>>You are now enterring the group chat.\n"
              << " Please type the tilde '~' and then the 'enter' key when you wish to leave<<<\n"
              << std::endl;

    while (true)
    {
        std::getline(std::cin >> std::ws, str);
        if (str == "~")
        {
            _online = false;
            displayThread.join();
            writeChat(outro, false);
            updateUserList();
            exit(0);
        }
        writeChat(str, true);
    }
}
