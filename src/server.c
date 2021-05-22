#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "../include/server.h"

#define SESSION_COUNT 100
#define PORTNO 9002
int sd;

struct Session sessions[SESSION_COUNT];
pthread_mutex_t sessions_lock;

void sig_handler(int sig)
{
    if (sig == SIGINT)
    {
        printf("Server Closed\n");
        close(sd);
        exit(0);
    }
}

int createSocket(int portno)
{
    int sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == -1)
    {
        perror("Error :Socket ");
        return sd;
    }

    int reuse = 1;
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (const char *)&reuse, sizeof(reuse)) < 0)
        perror("Error :setsockopt(SO_REUSEADDR) failed");

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(portno);
    int status = bind(sd, (struct sockaddr *)&server_address, sizeof(server_address));
    if (status == -1)
    {
        perror("Error: Bind ");
        return status;
    }
    status = listen(sd, 5);
    if (status == -1)
    {
        perror("Error: Listen ");
        return status;
    }
    return sd;
}

void *request_handler(void *socket_fd)
{
    int nsd = *(int *)socket_fd;
    struct Header header;

    read(nsd, &header, sizeof(header));

    switch (header.action)
    {
    case Login:
        loginController(nsd);
        break;
    case AddAccount:
        addAccountController(nsd, header.session_id);
        break;
    case ModifyAccount:
        modifyAccountController(nsd, header.session_id);
        break;
    case AdminAllUsers:
        getAllUsersController(nsd, header.session_id);
        break;
    case DeleteAccount:
        deleteAccountController(nsd, header.session_id);
        break;
    case BalanceEnquiry:
        balanceEnquiryController(nsd, header.session_id);
        break;
    case Transaction:
        transactionController(nsd, header.session_id);
        break;
    case PasswordChange:
        changePasswordController(nsd, header.session_id);
        break;
    case Viewdetails:
        viewDetailsController(nsd, header.session_id);
        break;
    case Exit:
        exitSessionController(nsd, header.session_id);
        break;
    }

    close(nsd);
    return NULL;
}

int validatePassword(struct LoginRequest request)
{

    struct User user;
    if (getUser(request.email, &user) == -1)
        return 0;

    if (strcmp(user.password, request.password) == 0)
        return 1;

    return 0;
}

void loginController(int nsd)
{
    struct LoginRequest request;
    read(nsd, &request, sizeof(request));

    int isValidPassword = validatePassword(request);

    struct LoginResponse response = {
        .session_id = -1};

    if (!isValidPassword)
    {
        response.status = Unauthorized;
    }
    else
    {
        struct User user;
        getUser(request.email, &user);
        int session_id;

        pthread_mutex_lock(&sessions_lock);
        for (session_id = 0; session_id < SESSION_COUNT; session_id++)
        {
            if (!sessions[session_id].isActive)
                break;
        }

        if (session_id == SESSION_COUNT)
        {
            response.status = Failure;
        }
        else
        {
            sessions[session_id].isActive = 1;
            sessions[session_id].user = user;
            response.session_id = session_id;
            response.loginType = user.accountType;
        }
        pthread_mutex_unlock(&sessions_lock);
    }

    write(nsd, &response, sizeof(response));
}

void addAccountController(int nsd, int session_id)
{
    struct User user;
    read(nsd, &user, sizeof(user));

    struct UserResponse response;

    if (sessions[session_id].user.accountType == Admin)
    {
        int status = createUser(&user);
        if (status == 0)
        {
            response.status = Success;
            printf("User created: \n");
            printf("User Id: %d \n", user.id);
            printf("Account Id: %d \n", user.account_id);
            response.user = user;
        }
        else
            response.status = Failure;
    }
    else
    {
        response.status = Unauthorized;
    }

    write(nsd, &response, sizeof(response));
}

void modifyAccountController(int nsd, int session_id)
{
    struct User user;
    read(nsd, &user, sizeof(user));

    struct UserResponse response;

    if (sessions[session_id].user.accountType == Admin)
    {
        struct User temp;
        getUserById(user.id, &temp);
        user.account_id = temp.account_id;

        int status = saveUser(&user);
        if (status == 0)
        {
            response.status = Success;
            printf("User modified: \n");
            printf("User Id: %d \n", user.id);
            printf("Account Id: %d \n", user.account_id);
            response.user = user;
        }
        else
            response.status = Failure;
    }
    else
    {
        response.status = Unauthorized;
    }

    write(nsd, &response, sizeof(response));
}

void exitSessionController(int nsd, int session_id)
{
    pthread_mutex_lock(&sessions_lock);
    sessions[session_id].isActive = 0;
    pthread_mutex_unlock(&sessions_lock);
}

void getAllUsersController(int nsd, int session_id)
{

    struct ViewAllUsersResponse response;
    struct User users[100];

    if (sessions[session_id].user.accountType == Admin)
    {
        // struct User user;
        // getUser(sessions[session_id].user.email, &user);
        // struct Account account;
        // getAccount(user.account_id, &account);

        int count = getUsers(users, 100);

        response.status = Success;
        response.user_count = count;
    }
    else
    {
        response.status = Unauthorized;
    }

    write(nsd, &response, sizeof(response));
    if (response.status == Success)
    {
        for (int i = 0; i < response.user_count; i++)
            write(nsd, users + i, sizeof(struct User));
    }
}

void deleteAccountController(int nsd, int session_id)
{

    int user_id;
    read(nsd, &user_id, sizeof(user_id));

    struct Response response;

    if (sessions[session_id].user.accountType == Admin)
    {
        if (!deleteUser(user_id))
        {
            response.status = Success;
        }
        else
        {
            response.status = Failure;
        }
    }
    else
    {
        response.status = Unauthorized;
    }

    write(nsd, &response, sizeof(response));
}

void balanceEnquiryController(int nsd, int session_id)
{
    struct Account account;
    struct BalanceEnquiryResponse response;
    if (getAccount(sessions[session_id].user.account_id, &account) == -1)
        response.status = Failure;
    else
    {
        response.status = Success;
        response.balance = account.balance;
    }
    write(nsd, &response, sizeof(response));
}

void transactionController(int nsd, int session_id)
{
    struct TransactionRequest request;
    read(nsd, &request, sizeof(request));

    struct TransactionResponse response;

    int status;
    int account_id = sessions[session_id].user.account_id;
    double amount = request.amount;
    double opening_balance, closing_balance;
    struct Account account;
    getAccount(account_id, &account);
    opening_balance = account.balance;

    status = changeAccountBalance(account_id, amount);

    getAccount(account_id, &account);
    closing_balance = account.balance;

    if (status == 0)
    {
        response.status = Success;
        struct Transaction transaction = {
            .account_id = account_id,
            .user_id = sessions[session_id].user.id,
            .opening_balance = opening_balance,
            .closing_balance = closing_balance,
            .transactionType = request.transactionType,
            .id = -1,
            .date = time(NULL)};
        strcpy(transaction.name, sessions[session_id].user.name);
        createTransaction(&transaction);
        response.transaction = transaction;
    }
    else
    {
        response.status = Failure;
    }

    write(nsd, &response, sizeof(response));
}

void changePasswordController(int nsd, int session_id)
{

    char new_password[100];

    read(nsd, &new_password, sizeof(new_password));

    struct Response response;

    strcpy(sessions[session_id].user.password, new_password);

    int status = saveUser(&sessions[session_id].user);

    if (status == 0)
    {
        response.status = Success;
    }
    else
    {
        response.status = Failure;
    }
    write(nsd, &response, sizeof(response));
}

void viewDetailsController(int nsd, int session_id)
{
    struct User user;
    getUser(sessions[session_id].user.email, &user);
    struct Account account;
    getAccount(user.account_id, &account);

    struct Transaction transactions[100];
    int count = getTransactions(user.account_id, transactions, 100);

    struct ViewDetailsResponse response = {
        .status = Success,
        .transaction_details_count = count};

    write(nsd, &response, sizeof(response));

    write(nsd, &user, sizeof(user));
    write(nsd, &account, sizeof(account));

    for (int i = 0; i < count; i++)
        write(nsd, &transactions[i], sizeof(transactions[i]));
}

int main()
{
    printf("Starting Server\n");
    sd = createSocket(PORTNO);
    if (sd != -1)
        printf("Server started on port number %d\n", PORTNO);
    else
        exit(0);

    for (int i = 0; i < SESSION_COUNT; i++)
        sessions[i].isActive = 0;

    signal(SIGINT, sig_handler);

    pthread_mutex_init(&sessions_lock, NULL);

    while (1)
    {
        int nsd = accept(sd, NULL, NULL);
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, &request_handler, &nsd);
    }

    close(sd);
}