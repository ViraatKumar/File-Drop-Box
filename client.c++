#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#define PORTNO 10000
using namespace std;
struct sockaddr_in address;
struct TextFile
{

    // char *clientName;
    // char *fileName;
    // char *pathname;
    char clientName[50];
    char password[50];
    char fileName[50];
    char pathname[100];
};
struct USER
{
    char USERNAME[50];
    char PASSWORD[50];
};
struct USER *createLogIn(char USERNAME[50], char PASSWORD[50])
{
    struct USER *info = (struct USER *)malloc(sizeof(struct USER));
    strcpy(info->USERNAME, USERNAME);
    strcpy(info->PASSWORD, PASSWORD);
    return info;
}
struct TextFile *createTextInfo(char name[50], char fileName[50], char pathname[100], char password[50])
{
    struct TextFile *info = (struct TextFile *)malloc(sizeof(struct TextFile));
    // info->clientName = (char*)malloc(50*sizeof(char));
    // info->fileName = (char*)malloc(50*sizeof(char));
    // info->pathname = (char*)malloc(100*sizeof(char));
    strcpy(info->clientName, name);
    strcpy(info->fileName, fileName);
    strcpy(info->pathname, pathname);
    strcpy(info->password, password);
    return info;
}
int getSocket()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(PORTNO);
    return sockfd;
}
void getConnection(int sockfd)
{
    connect(sockfd, (struct sockaddr *)&address, sizeof(address));
}
struct USER *userAuthentication(int sockfd)
{
    while (1)
    {
        char name[50];
        char password[50];
        printf("USERNAME : ");
        scanf("%s", name);
        printf("PASSWORD : ");
        scanf("%s", password);
        char message[50];
        // send username and password
        write(sockfd, name, sizeof(name));
        write(sockfd, password, sizeof(password));
        // read authentication message from server
        read(sockfd, message, sizeof(message));
        if (!strcmp(message, "****USER-AUTHORIZED****"))
        {
            cout << "USER AUTHORIZED" << endl;
            struct USER *info = createLogIn(name, password);
            return info;
        }
        else
        {
            cout << message << "\tTry again Please..." << endl;
        }
    }
}
int textFileTransfer(int sockfd, char filename[50], struct USER *details)
{
    FILE *input = fopen(filename, "r");
    if (input == NULL)
    {
        printf("ERROR in file\n");
        exit(0);
    }
    char ch = fgetc(input);
    char fileContents[1000];
    int counter = 0;
    // printf("starting....\n");
    while (ch != EOF)
    {
        // printf("ch = %c\n",ch);
        fileContents[counter++] = ch;
        ch = fgetc(input);
    }
    char path[100] = "//home//viraatkumar/.Desktop//OS_PROJECT";
    struct TextFile *info = createTextInfo(details->USERNAME, filename, path, details->PASSWORD);
    // write(sockfd, info->clientName, sizeof(info->clientName));
    // write(sockfd, info->password, sizeof(info->password));
    strcpy(info->clientName, details->USERNAME);
    strcpy(info->password, details->PASSWORD);
    write(sockfd, info->fileName, sizeof(info->fileName));
    write(sockfd, info->pathname, sizeof(info->pathname));
    write(sockfd, fileContents, sizeof(fileContents));
}
void clientFunctions()
{
    // Get connection
    int sockfd = getSocket();
    getConnection(sockfd);
    // Get log-in Details
    struct USER *details = userAuthentication(sockfd);
    // Execute File transfer
    char FILENAME[50];
    cout << "ENTER FILE NAME: ";
    scanf("%s", FILENAME);
    textFileTransfer(sockfd, FILENAME, details);
}
int main()
{
    clientFunctions();
    // int sockfd = getSocket();
    // getConnection(sockfd);
    // // char filename[50] = "input.txt";
    // char filename[50] = "input.txt";
    // //textFileTransfer(sockfd, filename);
    printf("work done...finishing\n");
}