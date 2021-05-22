
#include "../include/db.h"

#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int createAccount(struct Account *account)
{

    struct flock fl;

    if (account->id != -1)
        return -1;

    int metadata_fd = open("../database/metadata.dat", O_RDWR);

    struct Metadata metadata;

    fl.l_type = F_WRLCK, fl.l_whence = SEEK_SET, fl.l_start = 0, fl.l_len = 0, fl.l_pid = getpid();
    fcntl(metadata_fd, F_SETLKW, &fl);
    read(metadata_fd, &metadata, sizeof(metadata));
    account->id = metadata.account_id;
    metadata.account_id++;
    lseek(metadata_fd, 0, SEEK_SET);
    write(metadata_fd, &metadata, sizeof(metadata));
    fl.l_type = F_UNLCK, fl.l_whence = SEEK_SET, fl.l_start = 0, fl.l_len = 0, fl.l_pid = getpid();
    fcntl(metadata_fd, F_SETLKW, &fl);

    int account_fd = open("../database/account.dat", O_APPEND | O_WRONLY);
    write(account_fd, account, sizeof(*account));

    close(metadata_fd);
    close(account_fd);
    return 0;
}

int createTransaction(struct Transaction *transaction)
{
    struct flock fl;

    if (transaction->id != -1)
        return -1;

    int metadata_fd = open("../database/metadata.dat", O_RDWR);

    struct Metadata metadata;

    fl.l_type = F_WRLCK, fl.l_whence = SEEK_SET, fl.l_start = 0, fl.l_len = 0, fl.l_pid = getpid();
    fcntl(metadata_fd, F_SETLKW, &fl);
    read(metadata_fd, &metadata, sizeof(metadata));
    transaction->id = metadata.transaction_id;
    metadata.transaction_id++;
    lseek(metadata_fd, 0, SEEK_SET);
    write(metadata_fd, &metadata, sizeof(metadata));
    fl.l_type = F_UNLCK, fl.l_whence = SEEK_SET, fl.l_start = 0, fl.l_len = 0, fl.l_pid = getpid();
    fcntl(metadata_fd, F_SETLKW, &fl);

    int transaction_fd = open("../database/transaction.dat", O_APPEND | O_WRONLY);
    write(transaction_fd, transaction, sizeof(*transaction));

    close(metadata_fd);
    close(transaction_fd);
    return 0;
}

int getTransactions(int account_id, struct Transaction transactions[], int maxcount)
{
    struct flock fl;

    int fd = open("../database/transaction.dat", O_RDONLY);

    int len;
    int count = 0;
    fl.l_type = F_RDLCK, fl.l_whence = SEEK_SET, fl.l_start = 0, fl.l_len = 0, fl.l_pid = getpid();
    fcntl(fd, F_SETLKW, &fl);
    struct Transaction transaction;
    while (len = read(fd, &transaction, sizeof(transaction)))
    {
        if (transaction.account_id == account_id)
        {
            transactions[count++] = transaction;
        }
        if (count == maxcount)
            break;
    }
    fl.l_type = F_UNLCK, fl.l_whence = SEEK_SET, fl.l_start = 0, fl.l_len = 0, fl.l_pid = getpid();
    fcntl(fd, F_SETLKW, &fl);
    close(fd);
    return count;
}

int getUsers(struct User users[], int maxcount)
{
    struct flock fl;
    int fd = open("../database/user.dat", O_RDONLY);

    int len;
    int count = 0;
    fl.l_type = F_RDLCK, fl.l_whence = SEEK_SET, fl.l_start = 0, fl.l_len = 0, fl.l_pid = getpid();
    fcntl(fd, F_SETLKW, &fl);
    struct User user;
    while (len = read(fd, &user, sizeof(user)))
    {
        if (~user.id)
        {
            users[count++] = user;
        }
        if (count == maxcount)
            break;
    }
    fl.l_type = F_UNLCK, fl.l_whence = SEEK_SET, fl.l_start = 0, fl.l_len = 0, fl.l_pid = getpid();
    fcntl(fd, F_SETLKW, &fl);
    close(fd);
    return count;
}

int getAccounts(struct Account accounts[], int maxcount)
{

    struct flock fl;
    int fd = open("../database/account.dat", O_RDONLY);

    int len;
    int count = 0;
    fl.l_type = F_RDLCK, fl.l_whence = SEEK_SET, fl.l_start = 0, fl.l_len = 0, fl.l_pid = getpid();
    fcntl(fd, F_SETLKW, &fl);
    struct Account account;
    while (len = read(fd, &account, sizeof(account)))
    {
        if (~account.id)
        {
            accounts[count++] = account;
        }
        if (count == maxcount)
            break;
    }
    fl.l_type = F_UNLCK, fl.l_whence = SEEK_SET, fl.l_start = 0, fl.l_len = 0, fl.l_pid = getpid();
    fcntl(fd, F_SETLKW, &fl);
    close(fd);
    return count;
}

int createUser(struct User *user)
{
    struct flock fl;

    if (user->id != -1)
        return -1;

    int metadata_fd = open("../database/metadata.dat", O_RDWR);

    struct Metadata metadata;

    fl.l_type = F_WRLCK, fl.l_whence = SEEK_SET, fl.l_start = 0, fl.l_len = 0, fl.l_pid = getpid();
    fcntl(metadata_fd, F_SETLKW, &fl);
    read(metadata_fd, &metadata, sizeof(metadata));
    user->id = metadata.user_id;
    metadata.user_id++;
    lseek(metadata_fd, 0, SEEK_SET);
    write(metadata_fd, &metadata, sizeof(metadata));
    fl.l_type = F_UNLCK, fl.l_whence = SEEK_SET, fl.l_start = 0, fl.l_len = 0, fl.l_pid = getpid();
    fcntl(metadata_fd, F_SETLKW, &fl);

    if (user->account_id == -1)
    {
        struct Account account = {
            .balance = 1000,
            .id = -1};
        createAccount(&account);
        user->account_id = account.id;
    }

    int user_fd = open("../database/user.dat", O_APPEND | O_WRONLY);

    write(user_fd, user, sizeof(*user));

    close(metadata_fd);
    close(user_fd);
    return 0;
}

int getUser(char email[], struct User *user)
{
    struct flock fl;
    int fd = open("../database/user.dat", O_RDONLY);

    int len;

    fl.l_type = F_RDLCK, fl.l_whence = SEEK_SET, fl.l_start = 0, fl.l_len = 0, fl.l_pid = getpid();
    fcntl(fd, F_SETLKW, &fl);
    while (len = read(fd, user, sizeof(*user)))
    {
        if (strcmp(user->email, email) == 0 && ~user->id)
        {
            close(fd);
            return 0;
        }
    }
    fl.l_type = F_UNLCK, fl.l_whence = SEEK_SET, fl.l_start = 0, fl.l_len = 0, fl.l_pid = getpid();
    fcntl(fd, F_SETLKW, &fl);
    close(fd);
    return -1;
}

int getUserById(int user_id, struct User *user)
{

    struct flock fl;
    int fd = open("../database/user.dat", O_RDONLY);

    int len;

    fl.l_type = F_RDLCK, fl.l_whence = SEEK_SET, fl.l_start = 0, fl.l_len = 0, fl.l_pid = getpid();
    fcntl(fd, F_SETLKW, &fl);
    while (len = read(fd, user, sizeof(*user)))
    {
        if (user->id == user_id && ~user->id)
        {
            close(fd);
            return 0;
        }
    }
    fl.l_type = F_UNLCK, fl.l_whence = SEEK_SET, fl.l_start = 0, fl.l_len = 0, fl.l_pid = getpid();
    fcntl(fd, F_SETLKW, &fl);
    close(fd);
    return -1;
}

int getAccount(int account_id, struct Account *account)
{
    struct flock fl;
    int fd = open("../database/account.dat", O_RDONLY);

    int len;

    fl.l_type = F_RDLCK, fl.l_whence = SEEK_SET, fl.l_start = 0, fl.l_len = 0, fl.l_pid = getpid();
    fcntl(fd, F_SETLKW, &fl);
    while (len = read(fd, account, sizeof(*account)))
    {
        if (account->id == account_id)
        {
            close(fd);
            return 0;
        }
    }
    fl.l_type = F_UNLCK, fl.l_whence = SEEK_SET, fl.l_start = 0, fl.l_len = 0, fl.l_pid = getpid();
    fcntl(fd, F_SETLKW, &fl);
    close(fd);
    return -1;
}

int changeAccountBalance(int account_id, float amount)
{

    struct flock fl;
    struct Account account;

    int fd = open("../database/account.dat", O_RDWR);

    int len;

    while (len = read(fd, &account, sizeof(account)))
    {
        if (account.id == account_id)
            break;
    }

    if (len != 0)
    {

        int pos = lseek(fd, -sizeof(account), SEEK_CUR);
        int recordNo = pos / sizeof(account);
        fl.l_type = F_WRLCK, fl.l_whence = SEEK_SET, fl.l_start = pos, fl.l_len = sizeof(account), fl.l_pid = getpid();
        fcntl(fd, F_SETLKW, &fl);
        read(fd, &account, sizeof(account));
        lseek(fd, -sizeof(account), SEEK_CUR);
        account.balance += amount;
        if (account.balance < 0)
        {
            close(fd);
            return -1;
        }
        write(fd, &account, sizeof(account));
        fl.l_type = F_UNLCK, fl.l_whence = SEEK_SET, fl.l_start = pos, fl.l_len = sizeof(account), fl.l_pid = getpid();
        fcntl(fd, F_SETLKW, &fl);

        close(fd);
        return 0;
    }

    close(fd);
    return -1;
}

int saveUser(struct User *user)
{

    struct flock fl;
    struct User temp;

    int fd = open("../database/user.dat", O_RDWR);

    int len;

    while (len = read(fd, &temp, sizeof(temp)))
    {
        if (temp.id == user->id)
            break;
    }

    if (len != 0)
    {
        int pos = lseek(fd, -sizeof(temp), SEEK_CUR);
        int recordNo = pos / sizeof(temp);
        fl.l_type = F_WRLCK, fl.l_whence = SEEK_SET, fl.l_start = pos, fl.l_len = sizeof(temp), fl.l_pid = getpid();
        fcntl(fd, F_SETLKW, &fl);
        write(fd, user, sizeof(*user));
        fl.l_type = F_UNLCK, fl.l_whence = SEEK_SET, fl.l_start = pos, fl.l_len = sizeof(temp), fl.l_pid = getpid();
        fcntl(fd, F_SETLKW, &fl);
        close(fd);
        return 0;
    }

    close(fd);
    return -1;
}

int deleteUser(int user_id)
{

    struct flock fl;
    struct User temp;

    int fd = open("../database/user.dat", O_RDWR);

    int len;

    while (len = read(fd, &temp, sizeof(temp)))
    {
        if (temp.id == user_id)
            break;
    }

    if (len != 0)
    {
        temp.id = -1;
        int pos = lseek(fd, -sizeof(temp), SEEK_CUR);
        int recordNo = pos / sizeof(temp);
        fl.l_type = F_WRLCK, fl.l_whence = SEEK_SET, fl.l_start = pos, fl.l_len = sizeof(temp), fl.l_pid = getpid();
        fcntl(fd, F_SETLKW, &fl);
        write(fd, &temp, sizeof(temp));
        fl.l_type = F_UNLCK, fl.l_whence = SEEK_SET, fl.l_start = pos, fl.l_len = sizeof(temp), fl.l_pid = getpid();
        fcntl(fd, F_SETLKW, &fl);
        close(fd);
        return 0;
    }

    close(fd);
    return -1;
}