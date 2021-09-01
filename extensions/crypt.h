//
// Created by jefffu on 2021/9/1.
//

#ifndef FILE_BACKUP_SYSTEM_CRYPT_H
#define FILE_BACKUP_SYSTEM_CRYPT_H

#include "openssl/evp.h"


class Crypt {
public:
    Crypt();
    ~Crypt();

    void encrypt();
    void decrypt();

private:
};


#endif //FILE_BACKUP_SYSTEM_CRYPT_H
