#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

#include <err.h>
#include <memory.h>
#include <netdb.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

int getaddrinfo_server(struct addrinfo **res, char *port);
int setup_socket_server(struct addrinfo *res);
int socket_server(const char *port);
int socket_connect(const char *hostname, const char *port);

#endif
