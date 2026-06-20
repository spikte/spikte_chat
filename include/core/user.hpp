#ifndef CORE_USER_HPP
#define CORE_USER_HPP

#include <string>
#include <cstdint>

struct User {
    uint32_t id;
    std::string name;
    User(uint32_t id = 0, std::string name = "No username");
};
extern User localUser;


#endif
