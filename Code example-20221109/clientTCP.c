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

#define SERVER_PORT 21
#define SERVER_ADDR "ftp.up.pt"

#define PASSIVE "pasv"
#define ACTIVE "actv"

int main(int argc, char **argv)
{

    if (argc > 1)
        printf("**** No arguments needed. They will be ignored. Carrying ON.\n");
    int sockfd;
    char buf[] = "Mensagem de teste na travessia da pilha TCP/IP\n";
    size_t bytes;


    sockfd = initSocket();

    // send message to server
    sendCommand(sockfd, buf, sizeof(buf));

    
    if (close(sockfd) < 0)
    {
        perror("close()");
        exit(-1);
    }

    return 0;
}

int initSocket()
{
    int socketfd;
    struct sockaddr_in server_addr;
    

    /*server address handling*/
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR); /*32 bit Internet address network byte ordered*/
    server_addr.sin_port = htons(SERVER_PORT);          /*server TCP port must be network byte ordered */

    /*open a TCP socket*/
    if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket()");
        exit(-1);
    }
    /*connect to the server*/
    if (connect(socketfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect()");
        exit(-1);
    }

    return socketfd;
}

int sendCommand(int sockfd, char *command, int size)
{
    size_t bytes;

    /*send a string to the server*/
    bytes = write(sockfd, command, size);
    if (bytes > 0)
        printf("Bytes escritos %ld", bytes);
    else
    {
        perror("write()");
        exit(-1);
    }
}

int receiveResponse(int sockfd, char *response, int size)
{
    size_t bytes;

    /*read a string from the server*/
    bytes = read(sockfd, response, size);
    if (bytes > 0)
        printf("Bytes lidos %ld", bytes);
    else
    {
        perror("read()");
        exit(-1);
    }
}


