#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT "17170"
#define MAX_BUF 128
#define MAX_CLIENT_COUNT 10
#define SEND_INTERVAL_FLOOR 100000
#define SEND_INTERVAL_CEILING 1570000
#define SERVER_MESSAGE_LEN 18
#define IP_STR_SIZE 15

void gen_random_str(char *s, int len);

void get_time(char *buff);

void *listener(void *param);

void *sender(void *param);

struct conninfo {
    struct addrinfo *c;
    int *s;
    unsigned int *c_len;

    int *clients_cnt;
    struct sockaddr_in *clients;
};

int main() {
    struct addrinfo hints, *res, c;
    unsigned int c_len = sizeof(c);
    int s;

    struct sockaddr_in clients[MAX_CLIENT_COUNT];
    int clients_cnt = 0;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, PORT, &hints, &res) != 0) {
        printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
        exit(1);
    }

    if ((s = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
        printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
        exit(1);
    }

    if (bind(s, res->ai_addr, res->ai_addrlen) != 0) {
        printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
        exit(1);
    }

    struct conninfo info = {
            &c, &s, &c_len, &clients_cnt, clients
    };

    pthread_t listener_id;
    pthread_t sender_id;

    pthread_create(&listener_id, NULL, listener, (void *) &info);
    pthread_create(&sender_id, NULL, sender, (void *) &info);

    pthread_exit(NULL);
}

void *listener(void *param) {
    char received[MAX_BUF];
    char time_str[MAX_BUF];
    char hello_massage[] = "hello_message";

    struct conninfo *info = (struct conninfo *) param;

    int local_s = *info->s;
    short already_con = 0;

    for (;;) {
        ssize_t pos = recvfrom(local_s, received, MAX_BUF, 0, (struct sockaddr *) &info->c, info->c_len);
        if (pos < 0) {
            printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
            exit(1);
        }
        already_con = 0;
        received[pos] = '\0';
        if (memcmp(received, hello_massage, sizeof(hello_massage)) == 0) {
            // check if this client (ip:port) is already connected. if yes, skip this hello message
            for(int i = 0; i < MAX_CLIENT_COUNT; i++){
                if(memcmp(inet_ntoa(info->clients[i].sin_addr), inet_ntoa(((struct sockaddr_in *) &(info->c))->sin_addr), IP_STR_SIZE) == 0 &&
                        ntohs(info->clients[i].sin_port) == ntohs(((struct sockaddr_in *) &info->c)->sin_port)){
                    printf("Client %s:%d - already connected\n", inet_ntoa(((struct sockaddr_in *) &(info->c))->sin_addr),
                           ntohs(((struct sockaddr_in *) &info->c)->sin_port));
                    already_con = 1;
                    break;
                }
            }
            if(already_con == 1)
                continue;

            memcpy(&(info->clients[(*info->clients_cnt) % MAX_CLIENT_COUNT]), (struct sockaddr_in *) &(info->c),
                   sizeof(struct addrinfo));
            (*info->clients_cnt)++;
            printf("New client: (%s:%d)\n", inet_ntoa(((struct sockaddr_in *) &(info->c))->sin_addr),
                   ntohs(((struct sockaddr_in *) &info->c)->sin_port));
        } else {
            get_time(time_str);
            printf("[TIME]:%s | [SRC]:(%s:%d) | [MSG]: %s\n", time_str,
                   inet_ntoa(((struct sockaddr_in *) &(info->c))->sin_addr),
                   ntohs(((struct sockaddr_in *) &info->c)->sin_port), received);
        }
    }
}

void *sender(void *param) {
    char to_send[MAX_BUF];
    struct conninfo *info = (struct conninfo *) param;
    srand(time(NULL));

    int local_s = *info->s;

    for (;;) {
        int time_to_sleep = (rand() % (SEND_INTERVAL_CEILING - SEND_INTERVAL_FLOOR)) + SEND_INTERVAL_FLOOR;
        usleep(time_to_sleep);
        if (*info->clients_cnt == 0)
            continue;

        int random_client = rand() % *info->clients_cnt;
        struct sockaddr *picked_client = (struct sockaddr *) &(info->clients[random_client]);
        gen_random_str(to_send, SERVER_MESSAGE_LEN);

        size_t pos = sendto(local_s, to_send, strlen(to_send), 0, picked_client, *info->c_len);
        if (pos < 0) {
            printf("ERROR: %s (%s:%d)\n", strerror(errno), __FILE__, __LINE__);
            exit(1);
        }
    }
}


void gen_random_str(char *s, int len) {
    static const char alphanum[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

    for (int i = 0; i < len; ++i) {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }
    s[len] = 0;
}

void get_time(char *buff) {
    time_t now;
    time(&now);
    struct tm *local = localtime(&now);
    int hours = local->tm_hour;         // get hours since midnight (0-23)
    int minutes = local->tm_min;        // get minutes passed after the hour (0-59)
    int seconds = local->tm_sec;        // get seconds passed after a minute (0-59)
    int day = local->tm_mday;            // get day of month (1 to 31)
    int month = local->tm_mon + 1;      // get month of year (0 to 11)
    int year = local->tm_year + 1900;   // get year since 1900

    snprintf(buff, MAX_BUF, "%02d/%02d/%d-%02d:%02d:%02d", day, month, year, hours, minutes, seconds);
}
