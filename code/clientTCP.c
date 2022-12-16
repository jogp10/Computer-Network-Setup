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
#define MAX_RESPONSE 100000
#define SERVER_PORT 21

int connectToServer(char *serverAddr, int serverPort);

int parseURL(char *url, char *user, char *pass, char *host, char *file_path, char *file_name);

int saveFile(char *fileName, int socket);

int writeCommand(int socket, char *command);

int readResponse(int socket, char *response, int *code);

int checkResponse(int code, int expected);



int main(int argc, char **argv)
{

    if (argc > 2)
        printf("No more than 2 arguments needed. They will be ignored. Carrying ON.\n");

    int sockfd, _sockfd, code;
    char server_addr[20];
    size_t bytes;

    char user[MAX_MSG], pass[MAX_MSG], host[MAX_MSG], file_path[MAX_MSG], file_name[MAX_MSG];

    // Parse url...
    printf("Parsing URL...\n");

    parseURL(argv[1], user, pass, host, file_path, file_name);

    char user_command[MAX_MSG] = "user ";
    strcat(user_command, user);
    strcat(user_command, "\n");
    char pass_command[MAX_MSG] = "pass ";
    strcat(pass_command, pass);
    strcat(pass_command, "\n");
    char file_command[MAX_MSG] = "retr ";
    strcat(file_command, file_path);
    strcat(file_command, "\n");
    char passive_command[MAX_MSG] = "pasv\n";
    char response[MAX_RESPONSE];

    printf("\nPrinting Commands...\n");
    printf("user_command: %s\n", user_command);
    printf("pass_command: %s\n", pass_command);
    printf("file_command: %s\n", file_command);
    printf("passive_command: %s\n", passive_command);

    /*Connect to the server*/
    getIP(host, server_addr);

    printf("\nConnecting to server...\n");
    sockfd = connectToServer(server_addr, SERVER_PORT);
    readResponse(sockfd, response, &code);
    checkResponse(code, 220);

    /*send user command*/
    printf("Sending user command\n");
    writeCommand(sockfd, user_command);
    readResponse(sockfd, response, &code);
    checkResponse(code, 331);

    /*send pass command*/
    printf("Sending pass command\n");
    writeCommand(sockfd, pass_command);
    readResponse(sockfd, response, &code);
    checkResponse(code, 230);

    /*send passive command*/
    printf("Sending passive command\n");
    writeCommand(sockfd, passive_command) != 0;
    readResponse(sockfd, response, &code);
    checkResponse(code, 227);


    char *ip_port, *token, _port[20], ip[20];
    char del[2] = ",";
    int port;
    strtok(response, "(");
    ip_port = strtok(NULL, ")");

    token = strtok(ip_port, del);
    for (int i = 0; i < 4; i++)
    {
        strcat(ip, token);
        if(i<3) sprintf(ip, "%s.", ip);
        token = strtok(NULL, del);
    }
    for (int i = 0; i < 2; i++)
    {
        strcat(_port, token);
        if(i<1) sprintf(_port, "%s.", _port);
        token = strtok(NULL, del);
    }
    port = 256 * atoi(strtok(_port, "."));
    port = port + atoi(strtok(NULL, "."));

    /* Connecting client */
    _sockfd = connectToServer(ip, port);

    /*send retr command*/
    printf("Sending retr command\n");
    writeCommand(sockfd, file_command) != 0;
    readResponse(sockfd, response, &code);
    checkResponse(code, 150);

    printf("Writing file\n");
    saveFile(file_name, _sockfd);
    readResponse(sockfd, response, &code);
    checkResponse(code, 226);
    
    if (close(_sockfd) < 0)
    {
        perror("close()");
        exit(-1);
    }
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
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(serverAddr); /*32 bit Internet address network byte ordered*/
    server_addr.sin_port = htons(serverPort);            /*server TCP port must be network byte ordered */

    /*open a TCP socket*/
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket()");
        exit(-1);
    }

    /*connect to the server*/
    if (connect(sockfd,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        perror("connect()");
        exit(-1);
    }
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

    char *user_pass = strtok(urlrest, "@"); // <user>:<password>
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

int saveFile(char *fileName, int socket)
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
        exit(-1);
    }
    else if (sent == 0)
    {
        perror("sendCommand: error");
        exit(1);
    }
    return 0;
}

int readResponse(int socket, char *response, int *code)
{
    char response_code[4];

    FILE *fd;
    fd = fdopen(socket, "r");
    char *msg;
    size_t size;

    while (getline(&response, &size, fd) > 0)
    {
        if (response[3] == ' ')
            break;
    }

    strncpy(response_code, response, 3);
    *code = atoi(response_code);

    printf("Response: %s\n", response);
    return 0;
}

int checkResponse(int code, int expected)
{
    if (code != expected)
    {
        printf("Error: expected code %d, received code %d\n", expected, code);
        return -1;
    }
    return 0;
}
