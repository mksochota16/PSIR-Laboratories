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

#define MAX_BUF 128

int main() {
    int s, new_s;
    char client_ip_str[INET_ADDRSTRLEN];
    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, "14982", &hints, &res) != 0) {
        printf("ERROR could not return an addrinfo structure");
        return -1;
    }

    s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    if (bind(s, res->ai_addr, res->ai_addrlen) != 0) {
        printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
        return -1;
    }


    if (listen(s, 1) != 0){
        printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
        return -1;
    }

    struct sockaddr_in their_addr;
    socklen_t addr_size = sizeof(their_addr);

    if ((new_s = accept(s, (struct sockaddr *) &their_addr, &addr_size)) == -1)
        printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);

    if(inet_ntop(AF_INET, &(their_addr.sin_addr), client_ip_str, INET_ADDRSTRLEN) != NULL)
        printf("IP: %s, new sock desc.: %d\n", client_ip_str, new_s);

    long conn_res;
    unsigned char rec_mess[MAX_BUF];
    for(;;){
        conn_res=recv(new_s, rec_mess, MAX_BUF, 0);
        if(conn_res==0){
            printf("Peer was disconeted\n");
            break; //kończymy pętle i np.: kończymy działanie programu
        }
        else if(conn_res<0){
            printf("ERROR: %s\n", strerror(errno));
            exit(-4); //napotkano błąd
        }
        rec_mess[conn_res]='\0';  //odbieramy ciąg nie koniecznie zakończny znakiem ’\0’
        printf("Recv(): %s\n", rec_mess);
    }
    freeaddrinfo(res);
    close(new_s);
    close(s);
    return 0;
}


