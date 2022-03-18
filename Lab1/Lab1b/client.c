#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#define SERVER "127.0.0.1"
#define PORT "17170"
#define MAX_BUF 128

int main() {
    struct addrinfo h, *r, c;
    int s;
    unsigned int c_len = sizeof(c);

    char to_send[MAX_BUF];
    char received[MAX_BUF];

    memset(&h, 0, sizeof(struct addrinfo));
    h.ai_family = PF_INET;
    h.ai_socktype = SOCK_DGRAM;

    if (getaddrinfo(SERVER, PORT, &h, &r) != 0) {
        printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
        exit(1);
    }

    if ((s = socket(r->ai_family, r->ai_socktype, r->ai_protocol)) == -1) {
        printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
        exit(1);
    }

    snprintf(to_send, MAX_BUF, "hello_message");
    size_t pos = sendto(s, to_send, strlen(to_send), 0, r->ai_addr, r->ai_addrlen);
    if (pos < 0) {
        printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
        exit(1);
    }


    for (;;) {
        pos = recvfrom(s, received, MAX_BUF, 0, (struct sockaddr *) &c, &c_len);
        if (pos < 0) {
            printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
            exit(1);
        }

        received[pos] = '\0';
        printf("Recv(%s:%d): %s\n", inet_ntoa(((struct sockaddr_in *) &c)->sin_addr),
               ntohs(((struct sockaddr_in *) &c)->sin_port), received);

        snprintf(to_send, MAX_BUF, "RE:%s", received);
        pos = sendto(s, to_send, strlen(to_send), 0, r->ai_addr, r->ai_addrlen);
        if (pos < 0) {
            printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
            exit(1);
        }
    }
}