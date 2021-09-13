#ifndef ACCOUNT_HPP
#define ACCOUNT_HPP

#define LENGTH 32

struct account {
    unsigned int user_id;
    char password[LENGTH];
};


#endif