#ifndef ACCOUNT_HPP
#define ACCOUNT_HPP

#define LENGTH 32

struct account {
    union 
    {
        unsigned int user_id;
        char request[4];
    };
    char password[LENGTH];
};

#endif