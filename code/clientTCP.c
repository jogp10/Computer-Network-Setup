/**      (C)2000-2021 FEUP
 *       tidy up some includes and parameters
 * */

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "getip.c"

#define MAX_MSG 100
#define SERVER_PORT 21

int connectToServer(char *serverAddr, int serverPort);

int parseURL(char *url, char *user, char *pass, char *host, char *file_path, char *file_name);

int saveFile(char *path, char *fileName, int socket);

int writeCommand(int socket, char *command);

int readResponse(int socket, char *response);

int checkResponse(char *response, int expected);

int readWelcomeResponse(int socket, char *response);

int getCodeResponse(int sockfd, char *response);

int sendCommand(int socketfd, char *command);

int main(int argc, char **argv)
{

    if (argc > 2)
        printf("No more than 2 arguments needed. They will be ignored. Carrying ON.\n");

    int sockfd;
    char *server_addr;
    size_t bytes;

    char user[MAX_MSG], pass[MAX_MSG], host[MAX_MSG], file_path[MAX_MSG], file_name[MAX_MSG];

    // Parse url...
    printf("Parsing URL...\n");

    parseURL(argv[1], user, pass, host, file_path, file_name);

    char user_command[MAX_MSG] = "user ";
    strcat(user_command, user);
    char pass_command[MAX_MSG] = "pass ";
    strcat(pass_command, pass);
    char file_command[MAX_MSG] = "retr ";
    strcat(file_command, file_path);
    char passive_command[MAX_MSG] = "pasv";
    char response[MAX_MSG];

    printf("\nPrinting Commands...\n");
    printf("user_command: %s\n", user_command);
    printf("pass_command: %s\n", pass_command);
    printf("file_command: %s\n", file_command);
    printf("passive_command: %s\n", passive_command);
    printf("file_name: %s\n", file_name);
    printf("file_path: %s\n", file_path);
    printf("host: %s\n", host);

    /*Connect to the server*/
    printf("\nConnecting to server...\n");

    getIP(host, server_addr);

    if (sockfd = connectToServer(server_addr, SERVER_PORT))
    {
        perror("connect()");
        // exit(-1);
    }

    // Read welcome message
    printf("\nReading welcome message...\n");
    readWelcomeResponse(sockfd, response);
    // checkResponse(response, 220);
    printf("Welcome message received without errors!\n");

    /*send user command*/
    printf("\n\nSending user command\n");
    if (writeCommand(sockfd, user_command) != 0)
    {
        exit(-1);
    }
    printf("\n\nReceived response\n");
    readResponse(sockfd, response);
    printf("Response: %s\n", response);
    checkResponse(response, 331);

    /*send pass command*/
    if (writeCommand(sockfd, pass_command) != 0)
    {
        exit(-1);
    }
    printf("\nSent pass command\n");
    readResponse(sockfd, response);
    printf("Response: %s\n", response);
    checkResponse(response, 230);

    /*send passive command*/
    if (writeCommand(sockfd, passive_command) != 0)
    {
        exit(-1);
    }
    printf("\nSent passive command\n");
    readResponse(sockfd, response);
    printf("Response: %s\n", response);
    checkResponse(response, 227);

    if (close(sockfd) < 0)
    {
        perror("close()");
        exit(-1);
    }
    return 0;
}

int connectToServer(char *serverAddr, int serverPort)
{
    int sockfd;
    struct sockaddr_in server_addr;

    /*server address handling*/
    printf("Server address handling\n");
    printf("Server address: %s\n", serverAddr);
    bzero((char *)&server_addr, sizeof(server_addr));
    printf("Server address: %s\n", serverAddr);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(serverAddr); /*32 bit Internet address network byte ordered*/
    server_addr.sin_port = htons(serverPort);            /*server TCP port must be network byte ordered */

    /*open a TCP socket*/
    printf("Opening socket...\n");
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket()");
        exit(-1);
    }

    /*connect to the server*/
    printf("Connecting to server...\n");
    if (connect(sockfd,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        perror("connect()");
        exit(-1);
    }
    printf("Connected to server!\n");
    return sockfd;
}

int parseURL(char *url, char *user, char *pass, char *host, char *file_path, char *file_name)
{

    char *ftp = strtok(url, "/");           // ftp:
    char *urlrest = strtok(NULL, "/");      // <user>:<password>@<host>
    char *new_file_path = strtok(NULL, ""); // <url-path>
    char *new_file_name = strrchr(new_file_path, '/');
    if (new_file_name)
        new_file_name = new_file_name + 1;
    else
    {
        new_file_name = new_file_path;
    }

    char *user_pass = strtok(urlrest, "@"); // [<user>:<password>]
    char *new_host = strtok(NULL, "");      // <host>

    char *new_user;
    char *new_pass;
    if (new_host)
    {
        new_user = strtok(user_pass, ":"); // <user>
        new_pass = strtok(NULL, "");       // <password>
    }
    else
    {
        new_host = user_pass;
        new_user = "anonymous";
        new_pass = "pass";
        printf("No user and password provided. Using default values.\n");
    }

    strncpy(user, new_user, MAX_MSG);
    strncpy(pass, new_pass, MAX_MSG);
    strncpy(host, new_host, MAX_MSG);
    strncpy(file_path, new_file_path, MAX_MSG);
    strncpy(file_name, new_file_name, MAX_MSG);
    return 0;
}

int saveFile(char *path, char *fileName, int socket)
{
    FILE *fp;
    char buffer[MAX_MSG];
    int bytes;

    fp = fopen(fileName, "w");
    if (fp == NULL)
    {
        perror("fopen()");
        return -1;
    }
    while ((bytes = read(socket, buffer, MAX_MSG)) > 0)
    {
        fwrite(buffer, sizeof(char), bytes, fp);
    }
    if (bytes < 0)
    {
        perror("read()");
        return -1;
    }
    fclose(fp);
    return 0;
}

int writeCommand(int socket, char *command)
{
    int sent = send(socket, command, strlen(command), 0);
    if (sent < 0)
    {
        printf("Error writing command\n");
        return -1;
    }
    if (sent == 0)
    {
        perror("????");
        return 1;
    }
    return 0;
}

int readResponse(int socket, char *response)
{
    printf("Reading response...\n");
    int bytes = recv(socket, response, MAX_MSG, 0);
    printf("Bytes: %d\n", bytes);
    if (bytes < 0)
    {
        printf("Error reading response\n");
        return -1;
    }

    printf("Recived response\n");
    return 0;
}

int checkResponse(char *response, int expected)
{
    int code = atoi(response);
    if (code != expected)
    {
        printf("Error: expected code %d, received code %d\n", expected, code);
        return -1;
    }
    return 0;
}

int readWelcomeResponse(int socket, char *response)
{
    int code;

    char *welcomeResponse = (char *)malloc(100 * sizeof(char));

    do
    {

        int bytes = recv(socket, welcomeResponse, 1000, 0);

        if (bytes < 0)
        {
            printf("Error reading response\n");
            return -1;
        }

        code = getCodeResponse(socket, welcomeResponse);
        printf("Response: %s\n", welcomeResponse);
        printf("Response code: %d\n", code);

    } while (welcomeResponse[3] != ' ');

    return 0;
}
int getCodeResponse(int sockfd, char *response)
{
    int responseCode;
    responseCode = (int)response[0] - '0';

    return responseCode;
}

int sendCommand(int socketfd, char *command)
{
    printf(" about to send command: \n> %s", command);
    int sent = send(socketfd, command, strlen(command), 0);
    if (sent == 0)
    {
        printf("sendCommand: Connection closed");
        return 1;
    }
    if (sent == -1)
    {
        printf("sendCommand: error");
        return 2;
    }
    printf("> command sent\n");
    return 0;
}