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

#define MAX_MSG 100
#define SERVER_PORT 21

int connectToServer(char *serverAddr, int serverPort);

int parseURL(char *url, char *user, char *pass, char *host, char *file_path, char *file_name);

int saveFile(char *path, char *fileName, int socket);

int writeCommand(int socket, char *command);

int readResponse(int socket, char *response);

int checkResponse(char *response, int expected);

int main(int argc, char **argv)
{

    if (argc > 2)
        printf("No more than 2 arguments needed. They will be ignored. Carrying ON.\n");

    int sockfd;
    char *server_addr;
    size_t bytes;

    char user[MAX_MSG], pass[MAX_MSG], host[MAX_MSG], file_path[MAX_MSG], file_name[MAX_MSG];

    // parse url

    char user_command[MAX_MSG] = "user ";
    strcat(user_command, user);
    char pass_command[MAX_MSG] = "pass ";
    strcat(pass_command, pass);
    char file_command[MAX_MSG] = "retr ";
    strcat(file_command, file_path);
    char passive_command[MAX_MSG] = "pasv";

    char response[MAX_MSG];




    /*connect to the server*/
    if (sockfd = connectToServer(server_addr, SERVER_PORT))
    {
        perror("connect()");
        exit(-1);
    }

    /*send user command*/
    if(writeCommand(sockfd, user_command) != 0)
    {
        exit(-1);
    }
    getResponse(sockfd, response);
    checkResponse(response, 331);


    /*send pass command*/	
    if(writeCommand(sockfd, pass_command) != 0)
    {
        exit(-1);
    }
    getResponse(sockfd, response);
    checkResponse(response, 230);


    /*send passive command*/
    if(writeCommand(sockfd, passive_command) != 0)
    {
        exit(-1);
    }
    getResponse(sockfd, response);
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

    char *ftp = strtok(url, "/");      // ftp:
    char *urlrest = strtok(NULL, "/"); // [<user>:<password>@]<host>
    char *file_path = strtok(NULL, "");     // <url-path>
    char *file_name = strrchr(file_path, '/');
    if (file_name != NULL)file_name = file_name + 1;
    else file_name = file_path;

    char *user_pass = strtok(urlrest, "@"); // [<user>:<password>]
    char *host = strtok(NULL, "");          // <host>
    host = host + 1; // remove the first character (the ])

    char *user = strtok(user_pass, ":"); // <user>
    user = user + 1;
    char *pass = strtok(NULL, "");       // <password>
    
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
    if(sent == 0)
    {
        perror("????");
        return 1;
    }
    printf("Sent command\n");
    return 0;
}


int readResponse(int socket, char *response)
{
    int bytes = recv(socket, response, MAX_MSG, 0);
    if (bytes < 0)
    {
        printf("Error reading response\n");
        return -1;
    }

    printf("Received response\n");
    return 0;
}


int checkResponse(char *response, int expected)
{
    //TODO

}