#include<iostream>
#include<ctime>
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<errno.h>
#include<string>
#include<string.h>
#include<unistd.h>
#include<unordered_map>

#define SUCCESS 1
#define FAILURE 0
#define conf_file "Initial.conf"                    // Configuration File

using namespace std;

int sock                     ;
unsigned int len                  ;
struct sockaddr_in server, client ;
char file[50]   ;
int status_code ;
char root[200], extra[200]    ;
char Connection_type;
unordered_map <int, const char *>   status_dict ;
unordered_map <string, const char *> media_dict;

void chores(){                                                        // Makes Dictionary having pre decide values

    status_dict[200] = " 200 OK\r\n"                     ;
    status_dict[400] = " 400 Bad Request\r\n"            ;
    status_dict[404] = " 404 Not Found\r\n"              ;
    status_dict[500] = " 500 Internal Server Error\r\n"  ;
    status_dict[501] = " 501 Not Implemeted\r\n"         ;

    media_dict["html"] = "text/html\r\n"     ;
    media_dict["htm"]  = "text/html\r\n"      ;
    media_dict["txt"]  = "text/plain\r\n"     ;
    media_dict["jpeg"] = "image/jpeg\r\n"    ;
    media_dict["jpg"]  = "image/jpeg\r\n"     ;
    media_dict["gif"]  = "image/gif\r\n"      ;
    media_dict["pdf"]  = "Application/pdf\r\n";
    media_dict["other"]= "Application/octet-stream\r\n";

}

void send_file(FILE *fp, int client, int flength){

    char fbuf[8196] ;
    int bytes = 0   ;

    for (int i = 0; i < flength; i++){
        bytes = fread(fbuf, 1, sizeof(fbuf), fp)    ;
        send(client, fbuf, bytes, 0)    ;
    }

}

char * date_time_header(void){
  char buf[1000];
  memset(buf, '\0', sizeof(buf))    ;
  time_t now = time(0);
  struct tm tm = *gmtime(&now);

  strftime(buf, sizeof (buf), "%a, %d %b %Y %H:%M:%S %Z", &tm);
  return buf;

}

int Send_Headers(int status_code, char * type, int length, int client){

    char  http_version[] = "HTTP/1.1"          ;
    char  server[]       = "Server: CS:425\r\n"        ;
    char  cont_length[]  = "Content-Length: "  ;
    char  cont_type[]    = "Content-Type: "    ;
    char  connection[]   = "Connection: "      ;
    char date[]          = "Date: "            ;
    char Whitespace[]    = "\r\n"              ;
    char buf[20], Headers[200] ;

    char * d = date_time_header()   ;
    memset(buf, '\0', sizeof(buf))  ;
    memset(Headers, '\0', sizeof(Headers));
    sprintf(buf, "%d", length)                      ;
    strcat(Headers, http_version)                   ;
    strcat(Headers, (status_dict[status_code]))     ;
    strcat(Headers, server)                         ;
    strcat(Headers, cont_length)                    ;
    strcat(Headers, buf)                            ;
    strcat(Headers, Whitespace)                     ;
    strcat(Headers, date)                           ;
    strcat(Headers, d)                              ;
    strcat(Headers, Whitespace)                     ;
    strcat(Headers, cont_type)                      ;
    strcat(Headers, media_dict[type])               ;
    strcat(Headers, connection)                     ;
    if (Connection_type == 'k')
        strcat(Headers, "keep-alive\r\n")           ;
    else
        strcat(Headers, "close\r\n")                ;
    strcat(Headers, Whitespace)                     ;
    send(client, Headers, strlen(Headers), 0)       ;
}

char * get_file_type(char * filename){

    memset(file, '\0', sizeof(file))    ;
    int i = 0, j = 0   ;
    for (i = 0; i < strlen(filename); i++)
        if(filename[i] == '.')
            break;
    i++ ;

    if (i > strlen(filename)){
        strcat(file, "else")    ;
        return file;
        }

    else{

        memset(file, '\0', sizeof(file))    ;
        while(i < strlen(filename)){
            file[j] = filename[i] ;
            j++    ;
            i++    ;
        }

        if (media_dict.count(file))
            return file;

        strcpy(file, "others")  ;
        return file;
        }
}

int get_file_length(FILE * fp){

    int length = 0;

	fseek(fp, 0, SEEK_END);
	length = ftell(fp);
	rewind(fp);
    return length;

}

void get_one_word(char *out, char *inp, int start_position){

    int position = 0    ;
    for (int i = start_position; i < strlen(inp); i++)
        if (inp[i] != ' '){
            out[position] = inp[i]  ;
            position ++ ;
        }
        else
            break;
}


int process_GET(char * buffer, int client){

    char filename[1024]    ;
    char * file_type    ;
    FILE *fp    ;
    memset(extra, '\0', sizeof(extra)) ;
    memset(filename, '\0', sizeof(filename)) ;

    get_one_word(extra, buffer, 4) ;
    file_type  = get_file_type(extra)   ;

    if (strcmp("else", file_type) == 0)
        sprintf(filename, "%s%s%s", root, extra, "/index.html")  ;
    else
    if ((extra[strlen(extra)-1] == '/'))                                // if the last character is / then append "index.html" to the filename
        sprintf(filename, "%s%s%s", root, extra, "index.html")  ;
    else
        sprintf(filename, "%s%s", root, extra)   ;

    file_type  = get_file_type(filename)   ;

    fp = fopen(filename, "rb")   ;

    if (fp == NULL){
        printf("Could not open File\n") ;
        status_code = 404;
        return SUCCESS;
    }


    int file_length = get_file_length(fp)       ;

    int header_status = Send_Headers(status_code, file_type, file_length, client)  ;
    send_file(fp, client, file_length)   ;
    return SUCCESS  ;

}

void connection_type(char * buffer){
    string buf(buffer);
    int index = buf.find("Connection:")   ;

    if (buf[index+strlen("Connection: ")] == 'k')
        Connection_type = 'k'    ;
    else
        Connection_type = 'c'    ;

}

int query_type(char * buffer){

    char inp[512]   ;
    memset(inp, '\0', sizeof(inp))  ;
    printf("%s", buffer)  ;
    get_one_word(inp, buffer, 0)    ;

    if (strcmp("GET", inp) == 0)
        return 1    ;
    return 4    ;

}

int process_client(int client){

    int type;
    char buffer[512]    ;
    memset(buffer, '\0', sizeof (buffer))   ;

    int query_length = recv(client, buffer, sizeof(buffer), 0) ;  // Recieving request from client

    if (query_length <= 0){

        status_code = 400   ;
        return SUCCESS ;

    }

    type = query_type(buffer) ;
    connection_type(buffer) ;

    if (type == 1)
        return process_GET (buffer, client)   ;

    else
        status_code = 501   ;
        return SUCCESS      ;

    }

void Socket(){
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){  /*Establishing a Socket for Server Process*/
        perror("Failed to establish Socket :(\n") ;
        exit(-1)    ;
    }
}

void Bind(int server_port){

    server.sin_family = AF_INET                 ;
    server.sin_port = htons(server_port)      ; // Server Port
    server.sin_addr.s_addr = INADDR_ANY         ;
    bzero(&server.sin_zero, 8)                  ;
    len = sizeof(struct sockaddr_in)            ;

    if ((bind (sock, (struct sockaddr *)&server, len)) == -1){                 /*Trying to Bind to the Socket*/
        perror("Failed to bind to Socket :(\n" );
        exit(-1)  ;
        }
}

void listen(){
    if (listen(sock, 5) == -1){                                   /*server process listening on the established Socket*/
        perror("Failed to Listen to client(s)\n") ;
        exit(-1)    ;
    }
}

void Connecting(){
    int cli = -1    ;
    int pid ;
    while(1){
        if ((cli = accept(sock, (struct sockaddr *)&client, &len)) == -1){

                        exit(-1)    ;
                    }
        pid = fork()    ;

        if(pid == -1){

            printf("Closing")   ;
            close(cli)  ;
            continue    ;

        }

        if (pid > 0){

            close(cli)  ;
            continue    ;

        }

        if (pid == 0){

            status_code = 200   ;

            int response = process_client(cli)    ;

            if (response == FAILURE)
                close(cli)  ;

            if (Connection_type == 'c')
                close(cli)  ;

            else{

                if (status_code == 400){
                    char msg[] = "HTTP/1.1 400 Bad Request\r\n"   ;
                    send(cli, msg, sizeof(msg), 0)   ;
                    continue    ;
                    }

                if (status_code == 404){
                    char msg[] = "HTTP/1.1 404 Not Found\r\n"   ;
                    send(cli, msg, sizeof(msg), 0)   ;
                    close(cli)  ;
                    close(sock) ;
                    continue    ;
                    }

                if (status_code == 500){
                    char msg[] = "HTTP/1.1 500 Internal Server Error\r\n"   ;
                    send(cli, msg, sizeof(msg), 0)   ;
                    exit(1) ;
                    }

                if (status_code == 501){
                    char msg[] = "HTTP/1.1 501 Not Implemented\r\n"   ;
                    send(cli, msg, sizeof(msg), 0) ;
                    continue    ;
                    }
                }
            }

            close(cli)  ;
            break   ;

        }
        close(sock) ;
    }

void init(int server_port){


    Socket()   ;
    Bind(server_port)     ;
    listen()   ;

    while(1)
        Connecting()  ;

}

int main(int argc, char * argv[]){

    char port[7]    ;
    FILE *fhandle   ;

    fhandle = fopen(conf_file, "r")     ;
    if (fhandle == NULL){
        printf("Error in openining Configuration File\n")   ;
        exit(1) ;
    }

    if (fscanf(fhandle, "%s %s",extra, root )  != 2)
        printf("Error reading Base Directory from configuration file.\n") ;

    /*if (fscanf(fhandle, "%s %s", extra, port ) != 2)
        printf("Error reading port number from configuration file.\n")  ;*/

    int server_port = atoi(argv[1])    ;
    /*cout << server_port << endl;
    cout << root << endl << port << endl;*/
    chores()  ;
    init(server_port);
    return 0    ;

}
