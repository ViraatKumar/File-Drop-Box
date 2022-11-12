#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <thread>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <vector>
#include <sys/stat.h>
#include <dirent.h>
#include <chrono>
#include <filesystem>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
using namespace std;
#define MAX_USERS 4
#define PORTNO 40001
sem_t semaphore;
struct sockaddr_in server, client;
struct TextFile
{
    int sockfd;
    char clientName[50];
    char fileName[50];
    char pathname[100];
    char fileContents[10000];
    char password[50];
};
struct USER
{
    char USERNAME[50];
    char PASSWORD[50];
};
struct USER *createLOGIN()
{
    struct USER *info = (struct USER *)malloc(sizeof(USER));
    return info;
}
struct TextFile *createTextInfo()
{
    struct TextFile *info = (struct TextFile *)malloc(sizeof(struct TextFile));
    return info;
}
map<string, string> database;
void poplateHashMap()
{
    FILE *username_file = fopen("usernames.txt", "r");
    FILE *password_file = fopen("password.txt", "r");
    char ch = fgetc(username_file);
    string username = "";
    string password = "";
    vector<string> username_vector;
    vector<string> password_vector;
    // FOR USERNAMES
    while (ch != EOF)
    {
        if (ch != '\n')
            username = username + ch;
        else
        {
            username_vector.push_back(username);
            username = "";
        }
        ch = fgetc(username_file);
    }
    // FOR PASSWORDS
    ch = fgetc(password_file);
    while (ch != EOF)
    {
        if (ch != '\n')
            password = password + ch;
        else
        {
            password_vector.push_back(password);
            password = "";
        }
        ch = fgetc(password_file);
    }
    map<string, string>::iterator it;
    for (int i = 0; i < username_vector.size(); i++)
    {
        database.insert({username_vector[i], password_vector[i]});
    }
}
int checkExistenceOfUser(char USERNAME[100])
{
    string user = USERNAME;
    if (database.find(user) != database.end())
        return 1;
    return 0;
}
struct USER *clientAuthentication(int newsockfd)
{
    while (1)
    {
        struct USER *details = createLOGIN();
        char message[50];
        read(newsockfd, details->USERNAME, sizeof(details->USERNAME));
        read(newsockfd, details->PASSWORD, sizeof(details->PASSWORD));
        string username = details->USERNAME;
        string password = details->PASSWORD;
        if (database.find(username) != database.end())
        {
            if (!database[username].compare(password))
            {
                strcpy(message, "****USER-AUTHORIZED****");
                write(newsockfd, message, sizeof(message));
                return details;
            }
            else
            {
                strcpy(message, "****INNCORRECT-PASSWORD****\n");
            }
        }
        else
            strcpy(message, "****INNCORRECT-USERNAME****\n");

        write(newsockfd, message, sizeof(message));
    }
}
void display(struct TextFile *info);
void insertFiles(struct TextFile *info);
struct TextFile *recieveFile(int newsockfd, struct USER *details)
{
    sem_wait(&semaphore);
    struct TextFile *info = createTextInfo();
    strcpy(info->clientName, details->USERNAME);
    strcpy(info->password, details->PASSWORD);
    int size;
    // cout << "reading filename\n";
    size = read(newsockfd, info->fileName, sizeof(info->fileName));
    info->fileName[size] = '\0';
    // cout << "reading pathname\n";
    size = read(newsockfd, info->pathname, sizeof(info->pathname));
    info->pathname[size] = '\0';
    // cout << "reading filecontents\n";
    size = read(newsockfd, info->fileContents, sizeof(info->fileContents));
    info->fileContents[size] = '\0'; // cout << "trying to recieve " << info->fileName << "..." << endl;
    //  info->sockfd = newsockfd;
    insertFiles(info);
    // cout << "exiting semaphore\n";
    sem_post(&semaphore);
    return info;
}
void display(struct TextFile *info)
{
    // not really used.
    printf("Info recieved\n");
    printf("~client name -> %s\n", info->clientName);
    printf("~password -> %s\n", info->password);
    printf("~fileName -> %s\n", info->fileName);
    printf("~pathnameee -> %s\n", info->pathname);
    printf("file contents = %s\n", info->fileContents);
}
void createDirectories()
{
    // USERS directory should exist already.
    map<string, string>::iterator it;
    for (it = database.begin(); it != database.end(); it++)
    {

        char hold[100];
        char USERS[100] = "USERS/";
        string check = it->first;
        int counter = 0;
        for (int i = 0; i < check.length(); i++)
        {
            char ch = check[i];
            hold[counter++] = ch;
        }
        hold[counter] = '\0';
        strcat(USERS, hold);
        // printf("file name in create_directories = %s\n",USERS);
        mkdir(USERS, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }
}
char *getPath(char FILENAME[100], char USERNAME[100])
{
    char path[256];
    char curr[256];
    getcwd(curr, 256);
    strcpy(path, curr);
    int p = strlen(path);
    path[p++] = '/';
    path[p] = '\0';
    char users[100] = "USERS/";
    strcat(path, users);
    strcat(path, USERNAME);
    char slash[10] = "/";
    strcat(path, slash);
    strcat(path, FILENAME);
    char ans[50];
    strcpy(ans, path);
    // printf("file path chosen is = %s",ans);
    char *final_ans = ans;
    return final_ans;
}
void insertFiles(struct TextFile *info)
{
    char path[50];
    strcpy(path, getPath(info->fileName, info->clientName));
    FILE *fd = fopen(path, "w+");
    for (int i = 0; i < strlen(info->fileContents); i++)
    {
        putc(info->fileContents[i], fd);
    }
    fclose(fd);
}
void deleteFile(int newsockfd, struct USER *details)
{
    char FILENAME[100];
    int size = read(newsockfd, FILENAME, 100);
    FILENAME[size] = '\0';
    remove(getPath(FILENAME, details->USERNAME));
    remove(getPath(FILENAME, details->USERNAME));
}
void displayFiles(int newsockfd, struct USER *details)
{
    struct dirent *d;
    DIR *dr;
    char path[100];
    getcwd(path, 100);
    strcat(path, "/USERS/");
    strcat(path, details->USERNAME);
    dr = opendir(path);
    if (dr != NULL)
    {
        for (d = readdir(dr); d != NULL; d = readdir(dr))
        {
            if (strcmp(".", d->d_name) == 0 || strcmp("..", d->d_name) == 0)
                continue;
            char file[100];
            strcpy(file, d->d_name);
            write(newsockfd, file, sizeof(file));
        }
        closedir(dr);
    }
    else
        cout << "\nError Occurred!";
    char quit[10] = "q";
    write(newsockfd, quit, sizeof(quit));
}
bool fileExists(char path[100])
{
    FILE *fp = fopen(path, "r");
    if (fp != NULL)
    {
        fclose(fp);
        return true;
    }
    return false;
}
void shareFiles(int newsockfd, char user[100], struct USER *details)
{
    while (1)
    {
        char FILE[100];
        int size = read(newsockfd, FILE, sizeof(FILE));
        FILE[size] = '\0';
        char new_path[256] = "";
        char old_path[256] = "";
        strcat(old_path, getPath(FILE, details->USERNAME));
        strcat(new_path, getPath(FILE, user));
        int file_status = 0;
        if (!fileExists(old_path) || fileExists(new_path))
        {
            write(newsockfd, &file_status, sizeof(file_status));
        }
        else
        {
            file_status = 1;
            write(newsockfd, &file_status, sizeof(file_status));
            std::filesystem::create_symlink(old_path, new_path);
            break;
        }
    }
}
void downloadFile(int newsockfd, struct USER *details)
{
    char filex[1000];
    int hold = read(newsockfd, filex, 1000);
    filex[hold] = '\0';
    char *path = getPath(filex, details->USERNAME);
    // printf("path = %s\n", path);
    FILE *fd = fopen(path, "r");
    char filecontents[1000];
    int counter = 0;
    char ch = fgetc(fd);
    while (ch != EOF)
    {
        filecontents[counter++] = ch;
        ch = fgetc(fd);
    }
    fclose(fd);
    filecontents[counter] = '\0';
    write(newsockfd, filecontents, strlen(filecontents));
}
void syncFiles(int newsockfd, struct USER *details)
{
    sem_wait(&semaphore);
    displayFiles(newsockfd, details);
    int counter = 0;
    read(newsockfd, &counter, sizeof(counter));
    for (int i = 0; i < counter; i++)
    {
        char file[100];
        int size = read(newsockfd, file, sizeof(file));
        file[size] = '\0';
        char filecontents[1024];
        size = read(newsockfd, filecontents, sizeof(filecontents));
        filecontents[size] = '\0';
        FILE *fd = fopen(getPath(file, details->USERNAME), "w+");
        fputs(filecontents, fd);
        fclose(fd);
        filecontents[0] = '\0';
        file[0] = '\0';
    }
    sem_post(&semaphore);
    return;
}
void clientFunctions(int newsockfd)
{
    createDirectories();
    struct USER *details = clientAuthentication(newsockfd);
    printf("+ %s Has Logged In.\n", details->USERNAME);
    int MENU_OPTION = 0;
    while (1)
    {
        read(newsockfd, &MENU_OPTION, sizeof(MENU_OPTION));
        switch (MENU_OPTION)
        {
        case 0:
        {
            MENU_OPTION = -1;
            break;
        }
        case 1:
        {
            recieveFile(newsockfd, details);
            break;
        }
        case 2:
        {
            downloadFile(newsockfd, details);
            break;
        }
        case 5:
        {
            displayFiles(newsockfd, details);

            break;
        }
        case 3:
        {
            deleteFile(newsockfd, details);
            break;
        }
        case 4:
        {
            char USERNAME_TO_SHARE[100];
            int size = read(newsockfd, USERNAME_TO_SHARE, 100);
            USERNAME_TO_SHARE[size] = '\0';
            int a = -1;
            if ((a = checkExistenceOfUser(USERNAME_TO_SHARE)))
            {
                write(newsockfd, &a, sizeof(a));
                shareFiles(newsockfd, USERNAME_TO_SHARE, details);
            }
            else
            {
                write(newsockfd, &a, sizeof(a));
                printf("###-USER_NOT_FOUND-###\n");
            }
            break;
        }

        case 6:
        {
            syncFiles(newsockfd, details);
            break;
        }
        }
        if (MENU_OPTION == -1)
        {
            printf("- %s Has Logged Off \n", details->USERNAME);
            break;
        }
    }
}

int getSocket()
{
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1"); // INADDR_ANY
    server.sin_port = htons(PORTNO);
    bind(socketfd, (struct sockaddr *)&server, sizeof(server));
    listen(socketfd, 5);
    return socketfd;
}
int main()
{
    sem_init(&semaphore, 0, 1);
    poplateHashMap();
    createDirectories();
    int sockfd = getSocket();
    thread t[MAX_USERS];
    int counter = 0;
    int clilen = sizeof(clilen);
    while (1)
    {
        int newsockfd = -1;
        if ((newsockfd = accept(sockfd, (struct sockaddr *)&client, (socklen_t *)&clilen)) != -1)
        {
            t[counter] = thread(&clientFunctions, newsockfd);
            t[counter].detach();
            counter++;
        }
    }
    sem_destroy(&semaphore);
}
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <thread>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <bits/stdc++.h>
#include <vector>
#include <sys/stat.h>
#include <dirent.h>
#include <chrono>
#include <filesystem>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
using namespace std;
#define MAX_USERS 4
#define PORTNO 40001
sem_t semaphore;
struct sockaddr_in server, client;
struct TextFile
{
    int sockfd;
    char clientName[50];
    char fileName[50];
    char pathname[100];
    char fileContents[10000];
    char password[50];
};
struct USER
{
    char USERNAME[50];
    char PASSWORD[50];
};
struct USER *createLOGIN()
{
    struct USER *info = (struct USER *)malloc(sizeof(USER));
    return info;
}
struct TextFile *createTextInfo()
{
    struct TextFile *info = (struct TextFile *)malloc(sizeof(struct TextFile));
    return info;
}
map<string, string> database;
void poplateHashMap()
{
    FILE *username_file = fopen("usernames.txt", "r");
    FILE *password_file = fopen("password.txt", "r");
    char ch = fgetc(username_file);
    string username = "";
    string password = "";
    vector<string> username_vector;
    vector<string> password_vector;
    // FOR USERNAMES
    while (ch != EOF)
    {
        if (ch != '\n')
            username = username + ch;
        else
        {
            username_vector.push_back(username);
            username = "";
        }
        ch = fgetc(username_file);
    }
    // FOR PASSWORDS
    ch = fgetc(password_file);
    while (ch != EOF)
    {
        if (ch != '\n')
            password = password + ch;
        else
        {
            password_vector.push_back(password);
            password = "";
        }
        ch = fgetc(password_file);
    }
    map<string, string>::iterator it;
    for (int i = 0; i < username_vector.size(); i++)
    {
        database.insert({username_vector[i], password_vector[i]});
    }
}
int checkExistenceOfUser(char USERNAME[100])
{
    string user = USERNAME;
    if (database.find(user) != database.end())
        return 1;
    return 0;
}
struct USER *clientAuthentication(int newsockfd)
{
    while (1)
    {
        struct USER *details = createLOGIN();
        char message[50];
        read(newsockfd, details->USERNAME, sizeof(details->USERNAME));
        read(newsockfd, details->PASSWORD, sizeof(details->PASSWORD));
        string username = details->USERNAME;
        string password = details->PASSWORD;
        if (database.find(username) != database.end())
        {
            if (!database[username].compare(password))
            {
                strcpy(message, "****USER-AUTHORIZED****");
                write(newsockfd, message, sizeof(message));
                return details;
            }
            else
            {
                strcpy(message, "****INNCORRECT-PASSWORD****\n");
            }
        }
        else
            strcpy(message, "****INNCORRECT-USERNAME****\n");

        write(newsockfd, message, sizeof(message));
    }
}
void display(struct TextFile *info);
void insertFiles(struct TextFile *info);
struct TextFile *recieveFile(int newsockfd, struct USER *details)
{
    sem_wait(&semaphore);
    struct TextFile *info = createTextInfo();
    strcpy(info->clientName, details->USERNAME);
    strcpy(info->password, details->PASSWORD);
    int size;
    // cout << "reading filename\n";
    size = read(newsockfd, info->fileName, sizeof(info->fileName));
    info->fileName[size] = '\0';
    // cout << "reading pathname\n";
    size = read(newsockfd, info->pathname, sizeof(info->pathname));
    info->pathname[size] = '\0';
    // cout << "reading filecontents\n";
    size = read(newsockfd, info->fileContents, sizeof(info->fileContents));
    info->fileContents[size] = '\0'; // cout << "trying to recieve " << info->fileName << "..." << endl;
    //  info->sockfd = newsockfd;
    insertFiles(info);
    // cout << "exiting semaphore\n";
    sem_post(&semaphore);
    return info;
}
void display(struct TextFile *info)
{
    // not really used.
    printf("Info recieved\n");
    printf("~client name -> %s\n", info->clientName);
    printf("~password -> %s\n", info->password);
    printf("~fileName -> %s\n", info->fileName);
    printf("~pathnameee -> %s\n", info->pathname);
    printf("file contents = %s\n", info->fileContents);
}
void createDirectories()
{
    // USERS directory should exist already.
    map<string, string>::iterator it;
    for (it = database.begin(); it != database.end(); it++)
    {

        char hold[100];
        char USERS[100] = "USERS/";
        string check = it->first;
        int counter = 0;
        for (int i = 0; i < check.length(); i++)
        {
            char ch = check[i];
            hold[counter++] = ch;
        }
        hold[counter] = '\0';
        strcat(USERS, hold);
        // printf("file name in create_directories = %s\n",USERS);
        mkdir(USERS, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    }
}
char *getPath(char FILENAME[100], char USERNAME[100])
{
    char path[256];
    char curr[256];
    getcwd(curr, 256);
    strcpy(path, curr);
    int p = strlen(path);
    path[p++] = '/';
    path[p] = '\0';
    char users[100] = "USERS/";
    strcat(path, users);
    strcat(path, USERNAME);
    char slash[10] = "/";
    strcat(path, slash);
    strcat(path, FILENAME);
    char ans[50];
    strcpy(ans, path);
    // printf("file path chosen is = %s",ans);
    char *final_ans = ans;
    return final_ans;
}
void insertFiles(struct TextFile *info)
{
    char path[50];
    strcpy(path, getPath(info->fileName, info->clientName));
    FILE *fd = fopen(path, "w+");
    for (int i = 0; i < strlen(info->fileContents); i++)
    {
        putc(info->fileContents[i], fd);
    }
    fclose(fd);
}
void deleteFile(int newsockfd, struct USER *details)
{
    char FILENAME[100];
    int size = read(newsockfd, FILENAME, 100);
    FILENAME[size] = '\0';
    remove(getPath(FILENAME, details->USERNAME));
    remove(getPath(FILENAME, details->USERNAME));
}
void displayFiles(int newsockfd, struct USER *details)
{
    struct dirent *d;
    DIR *dr;
    char path[100];
    getcwd(path, 100);
    strcat(path, "/USERS/");
    strcat(path, details->USERNAME);
    dr = opendir(path);
    if (dr != NULL)
    {
        for (d = readdir(dr); d != NULL; d = readdir(dr))
        {
            if (strcmp(".", d->d_name) == 0 || strcmp("..", d->d_name) == 0)
                continue;
            char file[100];
            strcpy(file, d->d_name);
            write(newsockfd, file, sizeof(file));
        }
        closedir(dr);
    }
    else
        cout << "\nError Occurred!";
    char quit[10] = "q";
    write(newsockfd, quit, sizeof(quit));
}
bool fileExists(char path[100])
{
    FILE *fp = fopen(path, "r");
    if (fp != NULL)
    {
        fclose(fp);
        return true;
    }
    return false;
}
void shareFiles(int newsockfd, char user[100], struct USER *details)
{
    while (1)
    {
        char FILE[100];
        int size = read(newsockfd, FILE, sizeof(FILE));
        FILE[size] = '\0';
        char new_path[256] = "";
        char old_path[256] = "";
        strcat(old_path, getPath(FILE, details->USERNAME));
        strcat(new_path, getPath(FILE, user));
        int file_status = 0;
        if (!fileExists(old_path) || fileExists(new_path))
        {
            write(newsockfd, &file_status, sizeof(file_status));
        }
        else
        {
            file_status = 1;
            write(newsockfd, &file_status, sizeof(file_status));
            std::filesystem::create_symlink(old_path, new_path);
            break;
        }
    }
}
void downloadFile(int newsockfd, struct USER *details)
{
    char filex[1000];
    int hold = read(newsockfd, filex, 1000);
    filex[hold] = '\0';
    char *path = getPath(filex, details->USERNAME);
    // printf("path = %s\n", path);
    FILE *fd = fopen(path, "r");
    char filecontents[1000];
    int counter = 0;
    char ch = fgetc(fd);
    while (ch != EOF)
    {
        filecontents[counter++] = ch;
        ch = fgetc(fd);
    }
    fclose(fd);
    filecontents[counter] = '\0';
    write(newsockfd, filecontents, strlen(filecontents));
}
void syncFiles(int newsockfd, struct USER *details)
{
    sem_wait(&semaphore);
    displayFiles(newsockfd, details);
    int counter = 0;
    read(newsockfd, &counter, sizeof(counter));
    for (int i = 0; i < counter; i++)
    {
        char file[100];
        int size = read(newsockfd, file, sizeof(file));
        file[size] = '\0';
        char filecontents[1024];
        size = read(newsockfd, filecontents, sizeof(filecontents));
        filecontents[size] = '\0';
        FILE *fd = fopen(getPath(file, details->USERNAME), "w+");
        fputs(filecontents, fd);
        fclose(fd);
        filecontents[0] = '\0';
        file[0] = '\0';
    }
    sem_post(&semaphore);
    return;
}
void clientFunctions(int newsockfd)
{
    createDirectories();
    struct USER *details = clientAuthentication(newsockfd);
    printf("+ %s Has Logged In.\n", details->USERNAME);
    int MENU_OPTION = 0;
    while (1)
    {
        read(newsockfd, &MENU_OPTION, sizeof(MENU_OPTION));
        switch (MENU_OPTION)
        {
        case 0:
        {
            MENU_OPTION = -1;
            break;
        }
        case 1:
        {
            recieveFile(newsockfd, details);
            break;
        }
        case 2:
        {
            downloadFile(newsockfd, details);
            break;
        }
        case 5:
        {
            displayFiles(newsockfd, details);

            break;
        }
        case 3:
        {
            deleteFile(newsockfd, details);
            break;
        }
        case 4:
        {
            char USERNAME_TO_SHARE[100];
            int size = read(newsockfd, USERNAME_TO_SHARE, 100);
            USERNAME_TO_SHARE[size] = '\0';
            int a = -1;
            if ((a = checkExistenceOfUser(USERNAME_TO_SHARE)))
            {
                write(newsockfd, &a, sizeof(a));
                shareFiles(newsockfd, USERNAME_TO_SHARE, details);
            }
            else
            {
                write(newsockfd, &a, sizeof(a));
                printf("###-USER_NOT_FOUND-###\n");
            }
            break;
        }

        case 6:
        {
            syncFiles(newsockfd, details);
            break;
        }
        }
        if (MENU_OPTION == -1)
        {
            printf("- %s Has Logged Off \n", details->USERNAME);
            break;
        }
    }
}

int getSocket()
{
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1"); // INADDR_ANY
    server.sin_port = htons(PORTNO);
    bind(socketfd, (struct sockaddr *)&server, sizeof(server));
    listen(socketfd, 5);
    return socketfd;
}
int main()
{
    sem_init(&semaphore, 0, 1);
    poplateHashMap();
    createDirectories();
    int sockfd = getSocket();
    thread t[MAX_USERS];
    int counter = 0;
    int clilen = sizeof(clilen);
    while (1)
    {
        int newsockfd = -1;
        if ((newsockfd = accept(sockfd, (struct sockaddr *)&client, (socklen_t *)&clilen)) != -1)
        {
            t[counter] = thread(&clientFunctions, newsockfd);
            t[counter].detach();
            counter++;
        }
    }
    sem_destroy(&semaphore);
}
