#ifndef _USER_H_
#define _USER_H_
#include <string>

typedef struct User
{
    std::string name = "";
    std::string password = "";
    
    bool operator==(const User& rhs){return name == rhs.name;}
}User;

#endif