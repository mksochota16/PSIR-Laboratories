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
#define MAX_CLIENT_COUNT 10

struct conninfo {
    struct addrinfo *c;
    int *s;
    unsigned int *c_len;

    int *clients_cnt;
    struct sockaddr_in *clients;
};

void gen_random_str(char *s, int len);
void get_time(char *buff);

void *listen_for_new_clients(void *param) {
    char received[MAX_BUF];
    char time_str[MAX_BUF];
    char hello_massage[] = "hello_message";

    struct conninfo *info = (struct conninfo *) param;

    //jesli do recvfrom() prekazujemy dereferencjie "recvfrom(*info->s)", wartosc pointera *s otrzymuje wartosc NULL
    int local_s = *info->s;

    for (;;) {
        ssize_t pos = recvfrom(local_s, received, MAX_BUF, 0, (struct sockaddr *) &info->c, info->c_len);
        if (pos < 0) {
            printf("ERROR: %s\n", strerror(errno));
            exit(-4);
        }

        received[pos] = '\0';
        if (memcmp(received, hello_massage, sizeof(hello_massage)) == 0) {
            memcpy(&(info->clients[(*info->clients_cnt) % MAX_CLIENT_COUNT]), (struct sockaddr_in *) &(info->c), sizeof(struct addrinfo));
            (*info->clients_cnt)++;
            printf("New client: (%s:%d)\n", inet_ntoa(((struct sockaddr_in *) &(info->c))->sin_addr),
                   ntohs(((struct sockaddr_in *) &info->c)->sin_port));
        }
        else{

            get_time(time_str);
            printf("[TIME]:%s, [SRC]:(%s:%d), [MSG]: ''%s''\n", time_str, inet_ntoa(((struct sockaddr_in *) &(info->c))->sin_addr),
                   ntohs(((struct sockaddr_in *) &info->c)->sin_port), received);
        }
    }
}

void *send_to_random_client(void *param) {
    char to_send[MAX_BUF];
    struct conninfo *info = (struct conninfo *) param;
    srand(time(NULL));

    //jesli do recvfrom() prekazujemy dereferencjie "recvfrom(*info->s)", wartosc pointera *s otrzymuje wartosc NULL
    int local_s = *info->s;

    for (;;) {
        usleep(500000);
        if(*info->clients_cnt == 0)
            continue;

        int random_client = rand() % *info->clients_cnt;
        struct sockaddr *picked_client = (struct sockaddr *) &(info->clients[random_client]);
        gen_random_str(to_send,20);

        size_t pos = sendto(local_s, to_send, strlen(to_send), 0,picked_client, *info->c_len);
        if (pos < 0) {
            printf("ERROR: %s\n", strerror(errno));
            exit(-4);
        }
    }
}


int main() {
    struct addrinfo h, *r, c;
    int s;
    unsigned int c_len = sizeof(c);

    struct sockaddr_in clients[MAX_CLIENT_COUNT];
    int clients_cnt = 0;

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

    pthread_t listen_for_new_clients_id;
    pthread_t send_to_random_client_id;

    struct conninfo info = {
            &c, &s, &c_len, &clients_cnt, clients
    };

    pthread_create(&listen_for_new_clients_id, NULL, listen_for_new_clients, (void *) &info);
    pthread_create(&send_to_random_client_id, NULL, send_to_random_client, (void *) &info);

    pthread_exit(NULL);
}

void gen_random_str(char *s, int len) {
    static const char alphanum[] =     "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < len; ++i) {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    s[len] = 0;
}

void get_time(char *buff){
    time_t now;
    time(&now);
    struct tm *local = localtime(&now);
    int hours = local->tm_hour;         // get hours since midnight (0-23)
    int minutes = local->tm_min;        // get minutes passed after the hour (0-59)
    int seconds = local->tm_sec;        // get seconds passed after a minute (0-59)

    int day = local->tm_mday;            // get day of month (1 to 31)
    int month = local->tm_mon + 1;      // get month of year (0 to 11)
    int year = local->tm_year + 1900;   // get year since 1900

    snprintf(buff, MAX_BUF, "%02d/%02d/%d %02d:%02d:%02d", day, month, year, hours, minutes, seconds);

}