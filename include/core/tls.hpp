#ifndef CORE_TLS_HPP
#define CORE_TLS_HPP

#include "../../lib/socket_utils.h"

// Client
SSL_CTX* create_SSL_ctx_client(int min_tls_version, int max_tls_version); 
int init_socket_TLS(const char *hostname, const char *port, int min_tls_version, int max_tls_version, SSL_CTX **ctx, SSL **ssl);

// Server
SSL_CTX* create_SSL_ctx_server(int min_tls_version, int max_tls_version);

#endif
