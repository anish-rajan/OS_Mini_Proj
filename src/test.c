#include "../include/util.h"

#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int main()
{

    mkdir("../database", 0744);

    int account_fd = creat("../database/account.dat", 0744);
    int user_fd = creat("../database/user.dat", 0744);
    int transaction_fd = creat("../database/transaction.dat", 0744);
    int metadata_fd = creat("../database/metadata.dat", 0744);

    if (metadata_fd == -1 || user_fd == -1 || account_fd == -1 || transaction_fd == -1)
    {
        close(account_fd);
        close(transaction_fd);
        close(metadata_fd);
        close(metadata_fd);
        return 0;
    }

    struct Metadata metadata = {
        .user_id = 0,
        .account_id = 0,
        .transaction_id = 0,
    };

    write(metadata_fd, &metadata, sizeof(metadata));

    struct User u1 = {
        .accountType = Admin,
        .name = "Administrator",
        .email = "admin@gmail.com",
        .password = "admin@123",
        .account_id = -1,
        .id = -1};

    struct User u2 = {
        .accountType = Normal,
        .name = "Anish Rajan",
        .email = "anish@gmail.com",
        .password = "anish123",
        .account_id = -1,
        .id = -1};

    int user_id;
    createUser(&u1);
    createUser(&u2);

    struct User users[100];
    int count = getUsers(users, 100);

    printUsers(users, count);

    close(account_fd);
    close(transaction_fd);
    close(metadata_fd);
    close(metadata_fd);
}
