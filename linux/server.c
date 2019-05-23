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
#include <my_global.h>
#include <mysql.h>

#define USERNAME "user"    //MySql details
#define PASSWORD "lolololol"

#define PORT "42069"  // the port users will be connecting to

#define BACKLOG 10     // how many pending connections queue will hold

#define MAXMSGLENGTH 500 //maximum message length to be recv() by server

#define SENDMESSAGE(x,y) if(send(new_fd, (x), (y), 0) == -1) \
                            break; \
                         else \
                            printf("SENDING %s\n",(x))

#define SENDCODE(x) SENDMESSAGE((x),3)

/*struct circularBuffer
{
    int start; //points to first element
    char string[2*MAXMSGLENGTH];
    int end; //points to next to last element
};
*/
bool prefix( const char *str, char *pre)
{
    return strncmp(pre, str, strlen(str)) == 0;
}
char* sanitize_email(char mail[]){
    int len=strlen(mail);
    char* lol=malloc(len+7);
    int y=0;
    for(int i=0;i<len;i++,y++){
        if(mail[i]=='.'){
            strncpy(lol+y,"_dot_",5);
            y+=4;
        }
        else if(mail[i]=='@'){
            strncpy(lol+y,"_at_",4);
            y+=3;
        }
        else{
            lol[y]=mail[i];
        }
    }
    lol[y]=0;
    return lol;
}
bool maddrprocess(char *buff, int pre, char* dest) 
{
    int at=0,period=0;
    int len=strlen(buff);
    if(buff[pre]!='<'&&buff[len-3]!='>')
        return 0;
    if(buff[len-4]=='.')
        return 0;
    int y=pre+1;
    int i=0;
    for(;i<(len-pre-2);i++,y++){
        if((buff[y]>='a'&&buff[y]<='z')||(buff[y]>='A'&&buff[y]<='Z')||(buff[y]>='0'&&buff[y]<='9')){
            dest[i]=buff[y];
            continue;
        }
        else if(buff[y]=='@'){
            at++;
            dest[i]=buff[y];
        }
        else if(at>0&&buff[y]=='.'){
            period++;
            dest[i]=buff[y];
        }
        else
            return 0;
    }
    dest[i]=0;
    buff[y]=0;
    if(at==1&&period==1)
        return 1; 
    else
        return 0; 
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
MYSQL* connect_to_mysql(){
    MYSQL* temp;
    temp = mysql_init(NULL);
      if (temp == NULL) 
  {
      fprintf(stderr, "%s\n", mysql_error(temp));
      exit(1);
  }

if (mysql_real_connect(temp, "localhost", USERNAME, PASSWORD, 
        "mails", 0, NULL, 0) == NULL) 
{
    fprintf(stderr, "%s\n", mysql_error(temp));
    mysql_close(temp);
    printf("mysql fatal error, exiting\n");
    exit(1);
}   
return temp;
}

int main(void)
{
    /** IMPORTANT CREATE A DATABSE BY THE NAME OF mails **/


    printf("MySQL client version: %s\n", mysql_get_client_info());
      MYSQL *con = mysql_init(NULL);

    mysql_close(con);
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
        int lol=fork();
        if (lol==0) { // this is the child process  TODO: add a function to delete pointers on conn failure/quit
            int nbytes=0;
            bool helo=0;
            char buff[MAXMSGLENGTH*2];
            char mailFrom[51];
            char **recipients=NULL;
            int nrecipients=0;
            char mailData[MAXMSGLENGTH];
            memset(mailData,0,sizeof(mailData));
            int count=0;
            bool quit=1;
            while ((nbytes>0||(helo==0&&nbytes==0))&&quit){
                if ((nbytes = recv(new_fd, buff, MAXMSGLENGTH*2, 0)) <= 0) {
                    // got error or connection closed by client
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
                    printf("RECEIVED %s\n",buff);
                    if(helo==0){
                       if(!prefix("HELO",buff)){ 
                           perror("server: client didn't identify using HELO");
                           quit=0;
                        }
                        else{
                            helo=1;
                            SENDCODE("250");
                        }
                    }
                    else{
                        if(prefix("MAIL FROM",buff)){
                            if(maddrprocess(buff, 10, mailFrom)){//maddrprocess checks if a mail address has valid format and copies it to mailFrom array(in this case)
                                SENDCODE("250");
                            }
                            else{
                                mailFrom[0]=0;
                                SENDCODE("501");
                            }
                        }
                        else if(prefix("RCPT TO",buff)){
                            //dynamically allocating memory to recipients
                            if(nrecipients==0){
                                if(!((recipients = malloc(sizeof(char*)))&&(recipients[0] = malloc((51) * sizeof(char))))){
                                    SENDCODE("452");
                                }
                                else {
                                    if(maddrprocess(buff, 8, recipients[0])){ 
                                        SENDCODE("250");
                                        nrecipients++;
                                    }
                                    else{
                                        SENDCODE("501");
                                        free (recipients[nrecipients]);
                                        free (recipients);
                                    }                                    
                                }
                            }
                            else if(!((recipients = realloc(recipients, (nrecipients+1) * sizeof(char*)))&&(recipients[nrecipients] = malloc((51) * sizeof(char))))){
                                SENDCODE("452");
                            }
                            else{
                                if(maddrprocess(buff, 8, recipients[nrecipients])){
                                    SENDCODE("250");
                                    nrecipients++;
                                }
                                else{
                                    SENDCODE("501");
                                    free (recipients[nrecipients]);
                                }  
                            }
                        }
                        else if(prefix("DATA",buff)){
                            SENDCODE("354");
                            count=0;
                            char t2buff[200];
                            int tbytes;
                            bool loop=1;
                            while(loop){    //new input handler so that as the only command accepted inside DATA is RSET
                                if ((tbytes = recv(new_fd, t2buff, MAXMSGLENGTH*2, 0)) <= 0) {
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
                                }
                                else{
                                    //add received data to previous mail data
                                    printf("RECEIVED- %s\n",t2buff);
                                    int temp=10;
                                    if(t2buff[0]=='.'&&t2buff[1]=='\r'&&t2buff[2]=='\n'){ //checking for .<CRLF>
                                        mailData[count]=0;
                                        t2buff[0]=0;
                                        SENDCODE("250");
                                        loop=0;
                                    }
                                    else if(!(strncmp("RSET",t2buff,4))){
                                        count=0;
                                        for(int t=0;t<nrecipients;t++){
                                            free(recipients[t]);
                                        }
                                        free(recipients);
                                        nrecipients=0;
                                        mailFrom[0]=0;
                                    }
                                    else{
                                        count+=tbytes;
                                    }
                                    strcat(mailData,t2buff);
                                    memset(t2buff,0,sizeof(t2buff));
                                }
                            }
                        }
                        else if(prefix("RSET",buff)){ //freeing pointers and clearing other stuff
                            for(int t=0;t<nrecipients;t++)
                                free(recipients[t]);
                            if(recipients)
                                free(recipients);
                            nrecipients=0;
                            mailFrom[0]=0;
                            mailData[0]=0;
                            SENDCODE("250");
                        }
                        else if(prefix("SEND",buff)){
                            if(mailFrom[0]!=0&&nrecipients!=0&&mailData[0]!=0){
                                MYSQL *con1=connect_to_mysql();
                                int z=0;
                                /*This is my super inefficient database management
                                  Creates table for every receiver(even if the table exists
                                  no one cares, it'll only throw an error saying table 
                                  exists lol) then add an entry of the sender and the mailData
                                  to that table. All these strcat()s are a result of lack of
                                  + operator overloading in C  >:(
                                */
                                for(;z<nrecipients;z++){
                                    char query[100];
                                    memset(query,0,sizeof(query));
                                    strcpy(query,"CREATE TABLE ");
                                    strcat(query, sanitize_email(recipients[z]));
                                    strcat(query, " (sender varchar(50), mail varchar(255));");
                                    mysql_query(con1, query);
                                    fprintf(stderr, "%s\n", mysql_error(con));
                                    strcpy(query,"INSERT INTO ");
                                    strcat(query, sanitize_email(recipients[z]));
                                    strcat(query, " values('");
                                    strcat(query, mailFrom);
                                    strcat(query, "','");
                                    strcat(query, mailData);
                                    strcat(query, "')");
                                    if(mysql_query(con1, query)){
                                        fprintf(stderr, "%s\n", mysql_error(con));
                                        SENDCODE("451");
                                        for(int k=0;k<=z;k++){//in case of an error in any one of the mails being send DELETE ALL MAILS THAT HAVE BEEN SENT
                                            strcpy(query,"DELETE FROM ");
                                            strcat(query, recipients[z]);
                                            strcat(query, " WHERE text='");
                                            strcat(query, mailData);
                                            strcat(query, "'");
                                            if(mysql_query(con1, query)){
                                                fprintf(stderr, "%s\n", mysql_error(con));
                                                mysql_close(con);
                                                printf("mysql fatal error, exiting\n");
                                                exit(1);
                                            }
                                        }
                                        break;
                                    }
                                }
                                if(z==nrecipients){//ALL MAILS SENT=SUCCESS
                                    SENDCODE("250");
                                }
                            }
                            else{
                                SENDCODE("501");
                            }
                        }
                        else if(prefix("VRFY",buff)){//VRFY is dangerous therefore not implemented :p
                            SENDCODE("502");
                        }
                        else if(prefix("NOOP",buff)){
                            SENDCODE("250");
                            printf("BOOP!\n");
                        }
                        else if(prefix("QUIT",  buff)){
                            SENDCODE("251");
                            quit=0;
                        }
                        else{
                            SENDCODE("500");
                        }
                    }
                }
                memset(buff,0,sizeof(buff));
            }
            for(int t=0;t<nrecipients;t++){ //prevent orphan blocks
                free(recipients[t]);
            }
            free(recipients);            
        }


        close(new_fd);  // parent doesn't need this
    }
    return 0;
}