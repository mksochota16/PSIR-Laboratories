#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define SERWER "localhost"
#define MAX_LINE 1024

int main(){
    int s;
    long pos;
    struct addrinfo hints, *res;
    time_t t;
    char my_string[MAX_LINE];
    struct tm tm;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family=AF_INET;
    hints.ai_socktype=SOCK_STREAM;
    if(getaddrinfo(SERWER, "14982", &hints, &res) != 0){
        printf("ERROR could not return an addrinfo structure");
        return -1;
    }
    if((s=socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1){
        printf("ERROR could not open socket");
        return -1;
    }

    if(connect(s, res->ai_addr, res->ai_addrlen)!=0){
            printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
            return -1;
    }
    for(int i=0;i<10;i++){
        usleep(1040000);
        t = time(NULL);
        tm = *localtime(&t);
        snprintf(my_string, MAX_LINE, "now: %d-%02d-%02d %02d:%02d:%02d\r\n",
                 tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        if(send(s, my_string, strlen(my_string), 0)>0){
            printf("Wyslalem hihi\n");
        }
        else{
            printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
            break;
        }

    }


    freeaddrinfo(res);
    close(s);
    return 0;
}

