#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "../include/util.h"

#define PORTNO 9002

int connectSocket(int portno)
{
    int sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == -1)
    {
        perror("Error: Socket ");
        return -1;
    }
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(portno);
    int ret = connect(sd, (struct sockaddr *)&server_address, sizeof(server_address));
    if (ret == -1)
    {
        perror("Error: Connect ");
        return -1;
    }
    return sd;
}

int login(int *session_id, enum AccountType *loginType)
{

    int sd = connectSocket(PORTNO);
    if (sd == -1)
        exit(0);

    char email[100];
    char password[20];
    printf("Enter Email Id: ");
    scanf("%s", email);
    printf("Enter Password: ");
    scanf("%s", password);

    printf("Logging in ...\n");
    struct Header header = {
        .action = Login,
        .session_id = -1,
    };

    write(sd, &header, sizeof(header));

    struct LoginRequest request;

    strcpy(request.email, email);
    strcpy(request.password, password);
    write(sd, &request, sizeof(request));

    struct LoginResponse response;
    read(sd, &response, sizeof(response));

    if (response.status == Unauthorized)
    {
        printf("Unauthorized\n");
    }
    else
    {
        *session_id = response.session_id;
        *loginType = response.loginType;
        printf("Logged in\n");
    }
    return 0;
}

void addUser(int session_id, int sd, bool joint)
{
    struct User user = {
        .id = -1,
        .account_id = -1};

    printf("Enter Username: ");
    scanf(" %[^\n]", user.name);
    printf("Enter Email Id: ");
    scanf(" %s", user.email);
    printf("Enter Password: ");
    scanf(" %s", user.password);
    printf("Enter AccountType (Normal: %d, Admin: %d): ", Normal, Admin);
    scanf("%d", (int *)&user.accountType);
    if (joint)
    {
        printf("Enter Joint account id ");
        scanf("%d", &user.account_id);
    }

    struct Header header = {
        .action = AddAccount,
        .session_id = session_id,
    };

    write(sd, &header, sizeof(header));
    write(sd, &user, sizeof(user));

    struct UserResponse response;
    read(sd, &response, sizeof(response));

    if (response.status == Unauthorized)
    {
        printf("Unauthorized\n");
    }
    else if (response.status == Failure)
    {
        printf("Server Failure\n");
    }
    else
    {
        printf("Success\n");
        printUser(&response.user);
    }
}

void modifyUser(int session_id, int sd)
{

    struct User user = {
        .id = -1,
        .account_id = -1};

    printf("Enter User id ");
    scanf("%d", &user.id);

    printf("Enter Username: ");
    scanf(" %[^\n]", user.name);
    printf("Enter Email Id: ");
    scanf(" %s", user.email);
    printf("Enter Password: ");
    scanf(" %s", user.password);
    printf("Enter AccountType (Normal: %d, Admin: %d): ", Normal, Admin);
    scanf("%d", (int *)&user.accountType);

    struct Header header = {
        .action = ModifyAccount,
        .session_id = session_id,
    };

    write(sd, &header, sizeof(header));
    write(sd, &user, sizeof(user));

    struct UserResponse response;
    read(sd, &response, sizeof(response));

    if (response.status == Unauthorized)
    {
        printf("Unauthorized\n");
    }
    else if (response.status == Failure)
    {
        printf("Server Failure\n");
    }
    else
    {
        printf("Success\n");
        printUser(&response.user);
    }
}

void exitSession(int session_id, int sd)
{
    printf("Exit in process...\n");

    struct Header header = {
        .action = Exit,
        .session_id = session_id,
    };

    write(sd, &header, sizeof(header));
    printf("Exit Success\n");
    exit(0);
}

void viewAllUsers(int session_id, int sd)
{

    printf("View details in process...\n");

    struct Header header = {
        .action = AdminAllUsers,
        .session_id = session_id,
    };

    write(sd, &header, sizeof(header));

    struct ViewAllUsersResponse response;
    read(sd, &response, sizeof(response));

    if (response.status == Unauthorized)
    {
        printf("Unauthorized\n");
    }
    else if (response.status == Failure)
    {
        printf("Server Failure\n");
    }
    else
    {
        printf("Success\n");
        struct User users[response.user_count];
        for (int i = 0; i < response.user_count; i++)
            read(sd, users + i, sizeof(struct User));
        printUsers(users, response.user_count);
    }
}

void delete (int session_id, int sd)
{

    printf("Delete User in process...\n");

    struct Header header = {
        .action = DeleteAccount,
        .session_id = session_id,
    };

    int user_id;
    printf("Enter User Id: ");
    scanf("%d", &user_id);

    write(sd, &header, sizeof(header));
    write(sd, &user_id, sizeof(user_id));

    struct Response response;
    read(sd, &response, sizeof(response));

    if (response.status == Unauthorized)
    {
        printf("Unauthorized\n");
    }
    else if (response.status == Failure)
    {
        printf("Server Failure\n");
    }
    else
    {
        printf("Success\n");
    }
}

void balanceEnquiry(int session_id, int sd)
{

    printf("Balance Enquiry in process...\n");

    struct Header header = {
        .action = BalanceEnquiry,
        .session_id = session_id,
    };

    write(sd, &header, sizeof(header));

    struct BalanceEnquiryResponse response;
    read(sd, &response, sizeof(response));

    if (response.status == Unauthorized)
    {
        printf("Unauthorized\n");
    }
    else if (response.status == Failure)
    {
        printf("Server Failure\n");
    }
    else
    {
        printf("Balance:  %f\n", response.balance);
    }

    close(sd);
}

void transaction(int session_id, int sd, int multiplier)
{

    printf("Transaction in process...\n");

    struct Header header = {
        .action = Transaction,
        .session_id = session_id,
    };

    struct TransactionRequest request;
    request.transactionType = Deposit;

    printf("Enter amount: ");
    scanf("%lf", &request.amount);
    request.amount *= multiplier;

    request.transactionType = (multiplier == 1 ? Deposit : Withdraw);

    write(sd, &header, sizeof(header));
    write(sd, &request, sizeof(request));

    struct TransactionResponse response;
    read(sd, &response, sizeof(response));

    if (response.status == Unauthorized)
    {
        printf("Unauthorized\n");
    }
    else if (response.status == Failure)
    {
        printf("Server Failure\n");
    }
    else
    {
        printf("Success\n");
        printTransaction(&response.transaction);
    }
}

void changePassword(int session_id, int sd)
{

    printf("Change Password in process...\n");

    struct Header header = {
        .action = PasswordChange,
        .session_id = session_id,
    };

    char new_password[100];

    printf("Enter new password: ");
    scanf("%s", new_password);
    write(sd, &header, sizeof(header));
    write(sd, &new_password, sizeof(new_password));

    struct Response response;
    read(sd, &response, sizeof(response));

    if (response.status == Unauthorized)
    {
        printf("Unauthorized\n");
    }
    else if (response.status == Failure)
    {
        printf("Server Failure\n");
    }
    else
    {
        printf("Success\n");
    }
}

void viewDetails(int session_id, int sd)
{

    printf("View details in process...\n");

    struct Header header = {
        .action = Viewdetails,
        .session_id = session_id,
    };

    write(sd, &header, sizeof(header));

    struct ViewDetailsResponse response;
    read(sd, &response, sizeof(response));

    struct User user;
    read(sd, &user, sizeof(user));

    struct Account account;
    read(sd, &account, sizeof(account));

    struct Transaction transactions[response.transaction_details_count];
    for (int i = 0; i < response.transaction_details_count; i++)
        read(sd, transactions + response.transaction_details_count - i - 1, sizeof(struct Transaction));

    printf("\n");
    printUser(&user);

    printf("\n");
    printAccount(&account);

    printf("\n");

    printTransactions(transactions, response.transaction_details_count);

    printf("Success\n");
}

int main()
{
    printf("Client Started\n");
    int session_id = -1;
    enum AccountType loginType;
    login(&session_id, &loginType);
    if (session_id == -1)
        return 0;
    while (1)
    {
        int operation, sd;
        if (loginType == Admin)
        {
            printf("[1]  Add New User\n");
            printf("[2]  Add New Joint User\n");
            printf("[3]  View All Users\n");
            printf("[4]  Delete User\n");
            printf("[5]  Modify User\n");
            printf("[6]  Exit\n");
            printf("Enter Your Choice: ");
            scanf("%d", &operation);
            sd = connectSocket(PORTNO);
            if (sd == -1)
                exit(0);
            switch (operation)
            {
            case 1:
                addUser(session_id, sd, false);
                break;
            case 2:
                addUser(session_id, sd, true);
                break;
            case 3:
                viewAllUsers(session_id, sd);
                break;
            case 4:
                delete (session_id, sd);
                break;
            case 5:
                modifyUser(session_id, sd);
                break;
            case 6:
                exitSession(session_id, sd);
                break;
            }
        }
        else
        {
            printf("[1]  Balance Enquiry\n");
            printf("[2]  Deposit amount\n");
            printf("[3]  Withdraw amount\n");
            printf("[4]  Change Password\n");
            printf("[5]  View Details\n");
            printf("[6]  Exit\n");
            printf("Enter Your Choice: ");
            scanf("%d", &operation);
            sd = connectSocket(PORTNO);
            if (sd == -1)
                exit(0);
            switch (operation)
            {
            case 1:
                balanceEnquiry(session_id, sd);
                break;
            case 2:
                transaction(session_id, sd, 1);
                break;
            case 3:
                transaction(session_id, sd, -1);
                break;
            case 4:
                changePassword(session_id, sd);
                break;
            case 5:
                viewDetails(session_id, sd);
                break;
            case 6:
                exitSession(session_id, sd);
                break;
            }
        }

        close(sd);
    }
}
