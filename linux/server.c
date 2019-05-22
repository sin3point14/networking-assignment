/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdbool.h>

#define PORT "42067"  // the port users will be connecting to

#define BACKLOG 10     // how many pending connections queue will hold

#define MAXMSGLENGTH 500 //maximum message length to be recv() by server

#define SENDMESSAGE(x,y) if(send(new_fd, (x), (y), 0) == -1) break

#define SENDCODE(x) SENDMESSAGE((x),sizeof(int))

struct circularBuffer
{
    int start; //points to first element
    char string[2*MAXMSGLENGTH];
    int end; //points to next to last element
};

bool prefix( const char *str, struct circularBuffer *pre)
{
    return strncmp(pre->string+pre->start, str, strlen(pre)) == 0;
}
bool maddrprocess(struct circularBuffer *buff, int pre, char* dest) //such that buff->add+pre gives first char of mail addr
{
    int at=0,period=0;
    if(buff->string[buff->start+pre]!='<'&&buff->string[buff->end]!='>')
        return 0;
    if(buff->string[buff->end-1]=='.')
    return;
    int size = ((buff->start<buff->end)?(buff->end-buff->start+1):(buff->end+2*MAXMSGLENGTH-buff->start));
    for(int i=0,y=buff->start+pre+1;i<(size-pre-1);i++,y=(y+1)%(2*MAXMSGLENGTH)){
        if((buff->string[y]>='a'&&buff->string[y]<='z')||(buff->string[y]>='A'&&buff->string[y]<='Z')||(buff->string[y]>='0'&&buff->string[y]<='9')){
            dest[i]=buff->string[y];
            continue;
        }
        else if(buff->string[y]=='@'){
            at++;
            dest[i]=buff->string[y];
        }
        else if(at>0&&buff->string[y]=='.'){
            period++;
            dest[i]=buff->string[y];
        }
        else 
            return 0;
    }
    if(at==1&&period==1)
        return 1; 
}

void push_to_buffer(struct circularBuffer *buff, char tempbuff[], int nbytes){
    strncpy(buff->string+buff->end, tempbuff, 2*MAXMSGLENGTH-buff->start-1);
    buff->end= (buff->end+nbytes)%(2*MAXMSGLENGTH);
}
void pop_from_buffer(struct circularBuffer *buff, int nbytes){
    buff->start= (buff->start+nbytes)%(2*MAXMSGLENGTH);
}

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");

    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("server: got connection from %s\n", s);

        if (!fork()) { // this is the child process  TODO: add a function to delete pointers on conn failure/quit
            printf("fork ke andar"); //CHECK KARNE KE LIYE
            int nbytes=0;
            bool helo=0;
            struct circularBuffer buff[MAXMSGLENGTH*2];
            char tbuff[MAXMSGLENGTH];
            char mailFrom[51];
            char **recipients;
            int nrecipients=0;
            char mailData[MAXMSGLENGTH];
            int count=0;
            bool quit=1;
            while ((nbytes>0||(helo==0&&nbytes==0))&&quit){
               if ((nbytes = recv(new_fd, tbuff, MAXMSGLENGTH*2, 0)) <= 0) {
                   // got error or connection closed by client
                   printf("sent msg- %d",nbytes); //CHECK KARNE KE LIYE
                   if (nbytes == 0) {
                       // connection closed
                        printf("server: socket %d hung up\n", new_fd);
                        quit=0;
                    }
                    else {
                        perror("recv");
                    }
               }
               else{
                   printf("sent msg- %d",nbytes); //CHECK KARNE KE LIYE
                   push_to_buffer(buff, tbuff, nbytes);
                   if(helo==0){
                       if(!prefix("HELO",buff)){
                           perror("server: client didn't identify using HELO");
                           quit=0;
                        }
                        else
                            helo=1;
                    }
                    else{
                        if(prefix("MAIL FROM",buff)){
                            if(maddrprocess(buff, 10, mailFrom))
                                SENDCODE(250);
                            else
                                SENDCODE(501);
                        }
                        else if(prefix("RCPT TO",buff)){
                            
                            if(nrecipients==0){
                                if(!((recipients = malloc(nrecipients * sizeof(char*)))&&(recipients[0] = malloc((51) * sizeof(char)))))
                                    SENDCODE(452);
                                else {
                                    if(maddrprocess(buff, 10, recipients[0])){
                                        SENDCODE(250);
                                        nrecipients++;
                                    }
                                    else{
                                        SENDCODE(501);
                                    }                                    
                                }
                            }
                            if(!((recipients = realloc(recipients, nrecipients * sizeof(char*)))&&(recipients[0] = malloc((51) * sizeof(char)))))
                                SENDCODE(452);
                            else{
                                SENDCODE(250);
                            }
                        }
                        else if(prefix("DATA",buff)){
                            SENDCODE(354);
                            char tbuff[MAXMSGLENGTH];
                            
                            int tbytes;
                            while(mailData[0]!='a'&&mailData[1]!='\r'&&mailData[2]!='\n'&&mailData[3]!='\0'){
                                if ((tbytes = recv(new_fd, tbuff+count, MAXMSGLENGTH*2, 0)) <= 0) {
                                    // got error or connection closed by client
                                    if (tbytes == 0) {
                                        // connection closed
                                        printf("server: socket %d hung up\n", new_fd);
                                        quit=0;
                                    } 
                                    else {
                                        perror("recv");
                                    }
                                    break;
                                    //close(i); // bye!
                                }
                                else{
                                    if(strncmp("RSET",tbuff,4)){
                                        count=0;
                                        for(int t=0;t<nrecipients;t++){
                                            free(recipients[t]);
                                        }
                                        free(recipients);
                                        nrecipients=0;
                                        mailFrom[0]=0;
                                    }
                                    else{
                                        count+=nbytes+2;
                                        strncpy(mailData+count-1,tbuff,tbytes);
                                    }
                                }
                            }
                            /**
                             * 
                             * DATABASE ENTRY CODE
                             * 
                            **/
                        }
                        else if(prefix("RSET",buff)){
                            for(int t=0;t<nrecipients;t++){
                                free(recipients[t]);
                            }
                            free(recipients);
                            nrecipients=0;
                            mailFrom[0]=0;
                        }
                        else if(prefix("VRFY",buff)){
                            SENDCODE(502);
                        }
                        else if(prefix("NOOP",buff)){
                            SENDCODE(250);
                        }
                        else if(prefix("QUIT",buff)){
                            SENDCODE(251);
                            quit=0;
                        }
                        else{
                            SENDCODE(500);
                        }
                    }
                    pop_from_buffer(buff, nbytes);
                }
            }
            for(int t=0;t<nrecipients;t++){
                free(recipients[t]);
            }
            free(recipients);            
        }


        close(new_fd);  // parent doesn't need this
    }
    return 0;
}