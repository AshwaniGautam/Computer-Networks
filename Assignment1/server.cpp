#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<errno.h>
#include<string>
#include<string.h>
#include<unistd.h>

using namespace std;

int sock, cli                                                       ;
unsigned int len                                                    ;
struct sockaddr_in server, client                                   ;
char msg[] =  "Please Enter the filename\n"                         ;
FILE *fp                                                            ;
char buffer[4096], ack[1], filename[1024]                           ;
char Welcome[] = "Welcome!! Hold on Tight, CS425A is rolling \n"   ;


void Waiting_to_Connect(){                                                /*Server waits till the next Client connect or process termination*/
    if ((cli = accept(sock, (struct sockaddr *)&client, &len)) == -1){
        perror("Failed to accept any client Connection\n")    ;
        exit(-1)    ;
    }
    send(cli, Welcome, sizeof(Welcome), 0)  ;  /*Welcome Message sent to the Client after successful connection*/
    return  ;
}

int main(int argc, char *argv[]){

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){  /*Establishing a Socket for Server Process*/
        perror("Failed to establish Socket :(\n") ;
        exit(-1)    ;
    }

    server.sin_family = AF_INET                 ;
    server.sin_port = htons(atoi(argv[1]))      ; // Server Port
    server.sin_addr.s_addr = INADDR_ANY         ;
    bzero(&server.sin_zero, 8)                  ;
    len = sizeof(struct sockaddr_in)            ;

    if ((bind (sock, (struct sockaddr *)&server, len)) == -1){                 /*Trying to Bind to the Socket*/
        perror("Failed to bind to Socket :(\n" );
        exit(-1)  ;
        }

    if (listen(sock, 1) == -1){                  /*server process listening on the established Socket*/
        perror("Failed to Listen to client(s)\n") ;
        exit(-1)    ;
        }

    Waiting_to_Connect()   ;  /*Waiting for Client to connect*/

    while(1){

        bzero(filename, sizeof(filename));                  /*Clearing filename before use*/
        int n = recv(cli, filename, sizeof(filename), 0);
        if (n == 0 || n == -1)                              /*Client has terminated*/
            Waiting_to_Connect()                        ;

        strtok(filename, "\n")                  ;
        fp = fopen(filename, "rb")              ;         /*fp is file descriptor of the file requested*/
        if (fp == NULL) {
             ack[0] = 'N'                       ;
             send(cli, ack, sizeof (ack), 0)    ;        /*sending acknowledgment to Client about unsuccessful file opening*/
             continue                           ;
        }

        ack[0]  = 'Y'                    ;
        send(cli, ack, sizeof(ack), 0)   ;              /*sending acknowledgment to Client about successful file opening*/

        while(!(feof(fp))){
            ack[0] = 'Y'    ;
            send(cli, ack, sizeof(ack), 0)  ;
            bzero(buffer, sizeof(buffer))       ;       /*Clearing buffer before use*/
            fread(buffer, 1, sizeof(buffer), fp);       /*reading content of the into buffer*/
            send(cli, buffer, sizeof(buffer), 0);       /*sending content of buffer to Client*/
        }
        fclose(fp)                          ; /*Closing file*/
        ack[0] = 'N'                        ;
        send(cli, ack, sizeof (ack), 0)     ; /*Sending acknowledgment to client about file closure*/
    }
}
