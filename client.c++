#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#define PORTNO 10000
using namepspace std;
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
int getConnection(int sockfd)
{
    connect(sockfd, (struct sockaddr *)&address, sizeof(address));
}
int textFileTransfer(int sockfd, char filename[50])
{
    // we will transfer line by line
    // printf("hello\n");
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
    char name[50] = "viraat kumar";
    char password[50];
    printf("USERNAME : ");
    scanf("%s", name);
    printf("PASSWORD : ");
    scanf("%s", password);
    char path[100] = "//home//viraatkumar/.Desktop//OS_PROJECT";
    struct TextFile *info = createTextInfo(name, filename, path, password);
    // printf("info of file\nname = %s\nfilename = %s\npathname = %s\n",info->clientName,info->fileName,info->pathname);
    // printf("fileContents = %s\n",fileContents);
    write(sockfd, info->clientName, sizeof(info->clientName));
    write(sockfd, info->password, sizeof(info->password));
    write(sockfd, info->fileName, sizeof(info->fileName));
    write(sockfd, info->pathname, sizeof(info->pathname));
    write(sockfd, fileContents, sizeof(fileContents));
}
int main()
{
    int sockfd = getSocket();
    getConnection(sockfd);
    // char filename[50] = "input.txt";
    char filename[50] = "input.txt";
    textFileTransfer(sockfd, filename);
    printf("work done...finishing\n");
}