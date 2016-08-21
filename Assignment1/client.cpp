#include<iostream>
#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<errno.h>
#include<string>
#include<string.h>
#include<unistd.h>
#include<iostream>
#include<netdb.h>
#include<cstdlib>
#include<signal.h>

using namespace std;

int main(int argc, char *argv[]){

    int sock, cli                                               ;
    struct sockaddr_in server_addr                              ;
    struct hostent *Server_name                                 ;
    FILE *fp                                                    ;
    char buffer[4096], ack[1], filename[1024], Welcome[100]     ;

    if (argc != 3){
        printf("Exactly 2 arguments are reqired, Server-IP and Port Number\n")    ;
        exit(-1)    ;
    }

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){  /*Establishig a Socket for Client Process*/
        perror("Failed to establish Socket :(") ;
        exit(-1)    ;
    }

    Server_name = gethostbyname(argv[1])    ;  /*gets the server address by name*/
    if (Server_name == NULL){
        printf("No such HOST\n")            ;
        exit(-1)                            ;
    }

    int server_port = atoi(argv[2])             ; /*server port*/
    server_addr.sin_family = AF_INET            ;
    server_addr.sin_port = htons(server_port)   ;
    bcopy((char *)Server_name -> h_addr, (char *)&server_addr.sin_addr.s_addr, Server_name-> h_length)     ; /*Copying host address*/

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){    /*trying to connect to server*/
        printf("Can't Connect to server");
        exit(-1);
    }

    recv(sock, Welcome, sizeof(Welcome), 0); /*Welcome message recieved from server*/
    printf("%s", Welcome)   ;

    while(1){
        printf("Enter the Filename: ")              ;

        bzero(filename, sizeof(filename))   ;                     /*Clearing filename and ack for further use*/
        bzero(ack, sizeof(ack)) ;

        scanf("%s", filename)                       ; /*taking filename from stdin*/
        send(sock, filename, sizeof (filename), 0)  ; /*sending filename to server*/

        int n = recv(sock, ack, sizeof (ack), 0)    ;               /*recieves acknowledgment whether file is valid or not*/
        if (n == 0 || n == -1){                                     /*server process is terminated, as there is no acknowledgment recieved*/
            printf("Server has stopped responding.\n")    ;
            exit(0);
        }

        if (ack[0] == 'N'){                              /*acknowledgment about unsuccessful file opening*/
            printf("No such file exists\n");
            continue;
        }

        while(1){
            bzero(ack, sizeof(ack));
            recv(sock, ack, sizeof(ack), 0) ;           /*Acknowledgment whether file has ended or not*/
            if (ack[0] != 'Y')
                break;

            bzero(buffer, sizeof(buffer))          ;
            recv(sock, buffer, sizeof(buffer), 0)  ;    /*Recieving messages from server into buffer*/
            printf("%s", buffer);                       /*Printing buffer to standard output*/
        }
    }
}
