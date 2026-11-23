#include <sys/epoll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>


#define PORT 8000
#define MAX_EVENTS 16


int main(void) {
    // Create a non-blocking/standard TCP socket (IPv4, stream-oriented)
    int serverFd = socket(AF_INET, SOCK_STREAM, 0);

    // Allow immediate reuse of the local address/port after server restart (avoids TIME_WAIT bind errors)
    int opt = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Configure listening address: 0.0.0.0 (all interfaces) on PORT
    struct sockaddr_in addr = 
    {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(PORT)
    };

    // Bind socket to the IP/port and start listening for incoming connections
    bind(serverFd, (struct sockaddr *)&addr, sizeof(addr));
    listen(serverFd, 128);
    
    // Create an epoll instance in the kernel; returns an epoll file descriptor
    int epfd = epoll_create1(0);
    struct epoll_event event;
    
    // Register server_fd for Level-Triggered read events (EPOLLIN)
    event.events = EPOLLIN;
    event.data.fd = serverFd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, serverFd, &event);

    struct epoll_event events[MAX_EVENTS];
    char buf[1024];
    
    printf("Listening on: %d\n", PORT);

    while(1) {
        // Block (-1) until at least one monitored fd has an event; returns count of ready fds
        int n = epoll_wait(epfd, events, MAX_EVENTS, -1);

        for (int i = 0; i<n; i++) {
            int fd = events[i].data.fd;

            if (fd == serverFd) {
                // Listening socket is readable -> new client connection is waiting
                int clientFd = accept(serverFd, NULL, NULL);
                // Register the new client socket with epoll so we get notified when it sends data
                struct epoll_event clientEvent = 
                {
                    .events = EPOLLIN,
                    .data.fd = clientFd
                };
                epoll_ctl(epfd, EPOLL_CTL_ADD, clientFd, &clientEvent);
                
            } else {
                // Existing client socket has incoming data (or EOF/disconnect)
                ssize_t len = read(fd, buf, sizeof(buf));

                if (len <= 0){
                    // len == 0 means client closed connection cleanly (EOF); len < 0 means read error
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                    close(fd);
                } else {
                    // Echo received bytes back to the client
                    write(fd, buf, (size_t)len);
                } 
            }
        }
    }
}