#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <semaphore.h>
#define PORTNO 40001
using namespace std;
struct sockaddr_in address;
sem_t semaphore;

struct TextFile
{
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
    strcpy(info->clientName, name);
    strcpy(info->fileName, fileName);
    strcpy(info->pathname, pathname);
    strcpy(info->password, password);
    return info;
}
int getSocket()
{
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("ERROR::");
        exit(EXIT_FAILURE);
    }
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
        printf("~~USERNAME:->");
        scanf("%s", name);
        printf("~~PASSWORD:->");
        scanf("%s", password);
        char message[50];
        write(sockfd, name, sizeof(name));
        write(sockfd, password, sizeof(password));
        int size = read(sockfd, message, 50);
        message[size] = '\0';
        if (!strcmp(message, "****USER-AUTHORIZED****"))
        {
            cout << "\n****USER-AUTHORIZED****" << endl;
            struct USER *info = createLogIn(name, password);
            return info;
        }
        else
        {
            cout << message << "Try Again" << endl;
        }
    }
}
int textFileTransfer(int sockfd, char filename[50], struct USER *details)
{
    FILE *input = fopen(filename, "r");
    if (input == NULL)
    {
        perror("ERROR :");
        exit(EXIT_FAILURE);
    }
    char ch = fgetc(input);
    char fileContents[1000];
    int counter = 0;
    while (ch != EOF)
    {
        fileContents[counter++] = ch;
        ch = fgetc(input);
    }
    fileContents[counter] = '\0';
    char path[100];
    getcwd(path, 100);
    struct TextFile *info = createTextInfo(details->USERNAME, filename, path, details->PASSWORD);
    strcpy(info->clientName, details->USERNAME);
    strcpy(info->password, details->PASSWORD);
    write(sockfd, info->fileName, sizeof(info->fileName));
    write(sockfd, info->pathname, sizeof(info->pathname));
    write(sockfd, fileContents, sizeof(fileContents));
    return 1;
}
void downloadFile(int sockfd, char filename[1000])
{
    char filecontents[1000];
    write(sockfd, filename, strlen(filename));
    int size = read(sockfd, filecontents, 1000);
    filecontents[size] = '\0';
    FILE *new_file = fopen(filename, "w+");
    if (new_file == NULL)
    {
        printf("***ERROR WITH DOWNLOAD_FILE***\n");
        exit(0);
    }
    fputs(filecontents, new_file);
    fclose(new_file);
    printf("***DOWNLOAD COMPLETED***\n");
    return;
}
void shareFiles(int sockfd)
{
    printf("~~ENTER FILENAME: -> ");
    while (1)
    {
        char FILE[100];
        scanf("%s", FILE);
        if (!strcmp(FILE, "q"))
            break;
        write(sockfd, FILE, sizeof(FILE));
        int file_status = -1;
        read(sockfd, &file_status, sizeof(file_status));

        if (file_status == 1)
        {
            printf("***SHARING COMPLETED SUCESSFULLY***\n");
            cout << "********END OF SHARE PROCESS********\n";
            return;
        }
        else
        {
            printf("###-ERROR-###\n");
            printf("~~RE:ENTER FILENAME:-> ");
        }
    }
}
int countFiles(char files[1000])
{
    int number_of_files = 0;
    for (int i = 0; i < strlen(files); i++)
    {
        if (files[i] == '\n')
            number_of_files++;
    }
    return number_of_files;
}
void displayFiles(int newsockfd, struct USER *details)
{
    int counter = 0;
    while (1)
    {
        char file[100];
        int size = read(newsockfd, file, sizeof(file));
        file[size] = '\0';
        if (!strcmp(file, "q"))
        {
            break;
        }
        cout << ++counter << ") filename : " << file << endl;
    }
    return;
}
string getFileContent(char filename[100])
{
    char path[100];
    getcwd(path, 100);
    strcat(path, "/");
    strcat(path, filename);
    char filecontents[1024];
    int counter = 0;
    FILE *fd = fopen(path, "r");
    char ch = fgetc(fd);
    while (ch != EOF)
    {
        filecontents[counter++] = ch;
        ch = fgetc(fd);
    }
    filecontents[counter++] = '\0';
    fclose(fd);
    string ans = filecontents;
    return ans;
}
void syncFiles(int sockfd, struct USER *details)
{
    int counter = 0;
    char files[100][100];
    while (1)
    {
        char file[100];
        int size = read(sockfd, file, sizeof(file));
        file[size] = '\0';
        if (!strcmp(file, "q"))
        {
            break;
        }
        strcpy(files[counter++], file);
    }
    write(sockfd, &counter, sizeof(counter));
    for (int i = 0; i < counter; i++)
    {
        char hold[100];
        strcpy(hold, files[i]);
        write(sockfd, hold, sizeof(hold));
        char content[1024];
        strcpy(content, getFileContent(hold).c_str());
        write(sockfd, content, sizeof(content));
    }
    return;
}
void clientFunctions()
{
    cout << "\n---------WELCOME TO DROPBOX---------\n";
    int sockfd = getSocket();
    getConnection(sockfd);
    cout << "\n******* LOG-IN *******\n\n";
    struct USER *details = userAuthentication(sockfd);
    int choice = 0;
    while (1)
    {
        cout << "\n************MAIN MENU************\n";
        cout << "1: UPLOAD\n2: DOWNLOAD\n3: DELETE\n4: SHARE\n5: DISPLAY FILES\n6: SYNC FILES\n0: LOG-OFF\n";
        cout << "************END MENU************\n\n";
        cout << "~~ENTER:->";
        cin >> choice;
        int flag = 0;
        write(sockfd, &choice, sizeof(choice));
        switch (choice)
        {
        case 1:
        {
            cout << "*****UPLOAD_PROCESS*****\n";
            char FILENAME[100];
            cout << "~~ENTER FILE NAME: ";
            scanf("%s", FILENAME);
            textFileTransfer(sockfd, FILENAME, details);
            cout << "************************\n";
            break;
        }
        case 2:
        {
            cout << "~~ENTER FILENAME: ";
            char FILENAME[1000];
            scanf("%s", FILENAME);
            downloadFile(sockfd, FILENAME);
            break;
        }
        case 3:
        {
            char FILENAME[100];
            cout << "\n*****DELETING FILE*****\n\n";
            cout << "~~ENTER FILENAME:->";
            scanf("%s", FILENAME);
            fflush(stdout);
            fflush(stdin);
            write(sockfd, FILENAME, sizeof(FILENAME));
            cout << "\n*****PROCESS END*****" << endl;
            break;
        }
        case 4:
        {
            cout << "\n********SHARE PROCESS********\n";
            char USERNAME[100];
            while (1)
            {
                printf("~~ENTER USERNAME:->");
                char U[100];
                scanf("%s", U);
                if (!strcmp(U, details->USERNAME))
                {
                    printf("###-YOUR ENTERED YOUR OWN USERNAME DUMMY-###\n");
                }
                else
                {
                    strcpy(USERNAME, U);
                    break;
                }
            }
            write(sockfd, USERNAME, sizeof(USERNAME));
            int found = 0;
            read(sockfd, &found, sizeof(found));
            if (found != 1)
            {
                printf("*** USER NOT FOUND ***\n");
                printf("abort.\n");
                break;
            }
            else
                shareFiles(sockfd);

            break;
        }
        case 5:
        {
            displayFiles(sockfd, details);
            break;
        }
        case 6:
        {
            cout << "******ALL FILES FOUND HAVE BEEN SYNCED******\n";
            syncFiles(sockfd, details);
            break;
        }
        case 0:
        {
            cout << "---------Thanks For Dropping In!!---------\n";
            flag = 1;
            break;
        }
        default:
        {
            cout << "*** INVALID ****\n";
        }
        }
        if (flag == 1)
            break;
    }
}
int main()
{
    clientFunctions();
}