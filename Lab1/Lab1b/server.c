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
#include <pthread.h>

#define MAX_BUF 128

char received[MAX_BUF];
char to_send[MAX_BUF];

struct conninfo {
    struct addrinfo *c;
    int *s;
    unsigned int *c_len;
    char *string;
};

void *listen_for_new_clients(void *param) {
    struct conninfo *info = (struct conninfo *) param;
    printf("%s\n",info->string);

    for (;;) {
        ssize_t pos = recvfrom(*info->s, received, MAX_BUF, 0, (struct sockaddr *) &info->c, info->c_len);
        if (pos == 0) {
            continue;
        }

        if (pos < 0) {
            printf("ERROR: %s\n", strerror(errno));
            exit(-4);
        }

        received[pos] = '\0';
        printf("Recv(%s:%d): %s\n", inet_ntoa(((struct sockaddr_in *) &(info->c))->sin_addr),
               ntohs(((struct sockaddr_in *) &info->c)->sin_port), received);
    }
}

void *send_to_random_client() {
    /*for (;;) {
        printf("bede wysylal zaraz");
        sleep(1);
        strcpy(to_send, "siema");
        size_t pos = sendto(s, to_send, strlen(to_send), 0, (const struct sockaddr *) &c, c_len);
        if (pos < 0) {
            printf("ERROR: %s\n", strerror(errno));
            exit(-4);
        }
    }*/
}


int main() {
    struct addrinfo h, *r, c;
    int s;
    unsigned int c_len = sizeof(c);
    char string[10] = "elooo";


    memset(&h, 0, sizeof(struct addrinfo));
    h.ai_family = PF_INET;
    h.ai_socktype = SOCK_DGRAM;
    h.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, "12345", &h, &r) != 0) {
        printf("addrinfo error\n");
    }

    if ((s = socket(r->ai_family, r->ai_socktype, r->ai_protocol)) == -1) {
        printf("socket error\n");
    }

    if (bind(s, r->ai_addr, r->ai_addrlen) != 0) {
        printf("bind error\n");
    }

    struct conninfo info = {
            &c, &s, &c_len, string
    };

    pthread_t listen_for_new_clients_id;
    pthread_t send_to_random_client_id;

    pthread_create(&listen_for_new_clients_id, NULL, listen_for_new_clients, (void *) &info);
    pthread_create(&send_to_random_client_id, NULL, send_to_random_client, (void *) &info);

    freeaddrinfo(r);
    close(s);
    pthread_exit(NULL);
}
