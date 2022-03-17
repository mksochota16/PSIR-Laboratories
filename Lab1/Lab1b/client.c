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


#define SERWER "127.0.0.1"
#define MAX_BUF 128

int main() {
    struct addrinfo h, *r;
    int s;
    char my_string[MAX_BUF];
    memset(&h, 0, sizeof(struct addrinfo));
    h.ai_family = PF_INET;
    h.ai_socktype = SOCK_DGRAM;

    if (getaddrinfo(SERWER, "12345", &h, &r) != 0) {

    }

    if ((s = socket(r->ai_family, r->ai_socktype, r->ai_protocol)) == -1) {

    }

    snprintf(my_string, MAX_BUF, "Test message.");
    size_t pos = sendto(s, my_string, strlen(my_string), 0, r->ai_addr, r->ai_addrlen);

    if (pos < 0) {
        printf("ERROR: %s. in %s:%d", strerror(errno), __FILE__, __LINE__);
    }
    freeaddrinfo(r);
    close(s);


}