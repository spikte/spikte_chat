#include "socket_utils.h"

int getaddrinfo_server(struct addrinfo **res, char *port) {
    struct addrinfo hints;
    int status;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    status = getaddrinfo(NULL, port, &hints, res);
    if(status != 0) {
        perror("get_addr_info_list");
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
int setup_socket_server(struct addrinfo *res) {
    struct addrinfo *p;
    int server_fd;
    int status;
    int yes;

    yes = 1;
    server_fd = -1;
    for(p = res; p != NULL; p = p->ai_next) {
        server_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if(server_fd == -1) {
            perror("setup_socket");
            continue;
        }

        status = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        if(status == -1) {
            close(server_fd);
            perror("setup_socket");
            continue;
        }

        status = bind(server_fd, p->ai_addr, p->ai_addrlen);
        if(status == -1) {
            close(server_fd);
            perror("setup_socket");
            continue;
        }

        status = listen(server_fd, SOMAXCONN);
        if(status == -1) {
            close(server_fd);
            perror("setup_socket (listen)");
            continue;
        }
        break;
    }

    return server_fd;
}
int socket_server(const char *port) {
    int server_fd;
    struct addrinfo *res;

    if(getaddrinfo_server(&res, (char *)port) == EXIT_FAILURE)
        return -1;
    server_fd = setup_socket_server(res);
    freeaddrinfo(res);

    return server_fd;
}
int socket_connect(const char *hostname, const char *port) {
    struct addrinfo hints;
    struct addrinfo *res, *p;
    int sockfd;
    int status;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(hostname, port, &hints, &res);
    if(status != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return -1;
    }
    sockfd = -1;
    for(p = res; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if(sockfd == -1) {
            perror("socket");
            continue;
        }
        if(connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            perror("connect");
            close(sockfd);
            sockfd = -1;
            continue;
        }
        break;
    }
    freeaddrinfo(res);
    return sockfd;
}
