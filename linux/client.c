/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define CLEAR memset(msg,0,sizeof(msg))

#define PORT "42069" // the port client will be connecting to 

#define MAXDATASIZE 100 // max number of bytes we can get at once 


int sendall(int s, char *buf, int *len)
{
    printf("Sending %s\n",buf);
    //sleep(1);
    *len=strlen(buf);
    int total = 0;        // how many bytes we've sent
    int bytesleft = *len; // how many we have left to send
    int n;

    while(total < *len) {
        n = send(s, buf+total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }

    *len = total; // return number actually sent here

    return n==-1?-1:0; // return -1 on failure, 0 on success
} 

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    int sockfd, numbytes;  
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    if (argc != 2) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);
    printf("client: connecting to %s\n", s);
    freeaddrinfo(servinfo); // all done with this structure


    int size;  

    //TRY EDITING THE COMMANDS BELLOW

    char msg[200];
    strcpy(msg,"HELO");
    sendall(sockfd, msg, &size );

    CLEAR;

    recv(sockfd, msg, 5, 0);
    printf("Received %s\n", msg);

    CLEAR;

    sleep(1);

    strcpy(msg,"MAIL FROM:<a@a.a>");
    sendall(sockfd, msg, &size );

    CLEAR;

    recv(sockfd, msg, 5, 0);
    printf("Received %s\n", msg);

    sleep(1);

    CLEAR;

    strcpy(msg,"RCPT TO:<b@b.b>");
    sendall(sockfd, msg, &size );

    CLEAR;

    recv(sockfd, msg, 5, 0);
    printf("Received %s\n", msg);

    sleep(1);

    CLEAR;

    strcpy(msg,"RCPT TO:<c@c.c>");
    sendall(sockfd, msg, &size );

    CLEAR;

    recv(sockfd, msg, 5, 0);
    printf("Received %s\n", msg);

    sleep(1);

    CLEAR;

    strcpy(msg,"DATA");
    sendall(sockfd, msg, &size );

    CLEAR;

    recv(sockfd, msg, 5, 0);
    printf("Received %s\n", msg);

    sleep(1);

    CLEAR;

    strcpy(msg,"dslfnslfbekfb  fjds fdsk fkfds \r\n");
    sendall(sockfd, msg, &size );

    sleep(1);

    CLEAR;

    strcpy(msg,"fsdfdsfdsfsf  fdsfdsf \r\n");
    sendall(sockfd, msg, &size );

    sleep(1);

    CLEAR;

    strcpy(msg,"dsafnldsfn fkds vkdsj vkds f\r\n");
    sendall(sockfd, msg, &size );

    sleep(1);

    CLEAR;

    strcpy(msg,".\r\n");
    sendall(sockfd, msg, &size );

    CLEAR;

    recv(sockfd, msg, 5, 0);
    printf("Received %s\n", msg);

    sleep(1);

    CLEAR;

    strcpy(msg,"SEND");
    sendall(sockfd, msg, &size );

    CLEAR;

    recv(sockfd, msg, 5, 0);

    sleep(1);

    strcpy(msg,"QUIT");
    sendall(sockfd, msg, &size );

    CLEAR;

    recv(sockfd, msg, 5, 0);

    sleep(1);

    close(sockfd);

    return 0;
}