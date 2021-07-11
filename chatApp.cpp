#include "GroupChat.h"
#include <cassert>


void GroupChat::run()
{
    readUserList();
    int choice = 0;
    while(true)
    {
        printf("1.)Log in\n2.)Create Account\n3.)Exit\n:");
        std::cin >> choice;
        if(choice < 1 || choice > 3)
           printf("Invalid choice!\n\n");
        else
           break;
    }

    switch(choice)
    {
        case 1 :
            logIn();
            break;
        case 2 :
            createAccount();
            break;
        case 3 :
            exit(0);    
    }
}

void GroupChat::readUserList()
{
    std::lock_guard<std::mutex> lock(ul);
    std::ifstream inFile("Users.txt");
    if(inFile)
    {
        int loops{};
        std::string _name{}, _password{};

        inFile >> loops;
        inFile.ignore(120,'\n');

        for(int i{0}; i < loops; ++i)
        {
            std::getline(inFile, _name);
            std::getline(inFile, _password);
            users.emplace_back(_name, _password);
        }        
        inFile.close();
    }
    if(std::find(users.begin(),users.end(), user) == users.end() && user.name != "")
        users.emplace_back(user);
    
}

void GroupChat::updateUserList()
{
    users.erase(users.begin(), users.end());
    readUserList();
    std::lock_guard<std::mutex> lock(ul);
    std::ofstream outFile("Users.txt");
    assert(outFile);
    outFile << users.size() << std::endl;
    for(size_t i{0}; i < users.size(); ++i)
    {
        outFile << users.at(i).name << std::endl;
        outFile << users.at(i).password << std::endl;
    }
    outFile.close();
}

void GroupChat::createAccount()
{
    std::string _name{}, _password{};

    printf("\n\n***************Create an Account***************\n\n");

    printf("Please enter a username: ");
    std::cin >> _name;

    auto it = std::find(users.begin(), users.end(), User(_name));
    if(it != users.end())
    {   
        printf("Username already in use!\n");
        createAccount();
    }
    else
    {
       printf("Please enter a password: ");
       std::cin >> _password;
       user.name = _name;
       user.password = _password;
       users.emplace_back(user);
       openSession();
    }   
}

void GroupChat::logIn()
{
    std::string _name{}, _password{};

    printf("\n\n******************** Log In *******************\n\n");

    printf("Please enter your username: ");
    std::cin >> _name;

    auto it = std::find(users.begin(), users.end(), User(_name));
    if(it != users.end())
    {
        int wrongGuesses = 0;
        do
        {
            printf("Please enter your password: ");
            std::cin >> _password;

            if(it->password == _password)
            {   
                user.name = _name;
                openSession();
            }
            else
               printf("Password is incorrect!\n");
            
            if(++wrongGuesses == 3)
            {
                printf("Three incorrect guesses! Returning to main menu.\n\n");
                run();
            }
        }while(it->password != _password);
    }
    else
    {
        printf("\nUsername does not exist! Please create an account.\n");
        createAccount();
    }
}

void GroupChat::readChat()
{
    int count {0};
    std::string line{};
    std::lock_guard<std::mutex> lock(gc);

    std::ifstream inFile("groupChat.txt");
    if(inFile)
    {
        while(std::getline(inFile, line))
        {
            ++count;
            auto index = line.find(user.name);
            if(count > lineCount)
            {
                if(index != 0)
                {
                    std::cout << line << std::endl;
                }
                lineCount = count;
            }
        }
        inFile.close();
    }
}

void GroupChat::writeChat(std::string& str, bool userName)
{
     std::lock_guard<std::mutex> lock(gc);
     std::ofstream outFile;
     outFile.open("groupChat.txt", std::ios_base::app);
     if(userName)
     {
         outFile << user.name << ": " << str << std::endl;
     }
     else
         outFile << str << std::endl;
        
}

void GroupChat::openSession()
{
    std::cout << "You are now enterring the group chat.\n" << 
                 " Please type the tilde '~' and then the 'enter' key when you wish to leave" << std::endl;

    std::string str{" has entered the chat..."};

    auto displayChat = [&]{
        while(str != "~")
        {
            readChat();
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        }
    };
    std::thread displayThread(displayChat);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    std::string intro = "User ";
    intro = intro + user.name + str;
    std::string outro = user.name + " has left the chat room";

    writeChat(intro, false);
 
    while(true)
    {
        std::getline(std::cin >> std::ws, str);
        if(str == "~")
        {
            displayThread.join();
            writeChat(outro, false);
            updateUserList(); 
            exit(0);
        }
        writeChat(str, true);
    }
        
}
