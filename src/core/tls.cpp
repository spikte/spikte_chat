#include "../../include/core/tls.hpp"



// Source client: https://github.com/openssl/openssl/blob/master/demos/guide/tls-client-non-block.c
SSL_CTX* create_SSL_ctx_client(int min_tls_version, int max_tls_version) {
    SSL_CTX* ctx;

    // Create the SSL context
    ctx = SSL_CTX_new(TLS_client_method());
    if(ctx == NULL) {
        printf("Failed to create the SSL_CTX\n");
        goto error;
    }
    // If the cert verification fails abort the connection
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
    // !! TEST !! trust local cert
    if(!SSL_CTX_load_verify_locations(ctx, "chain.pem", NULL)) {
        printf("Failed to load CA file\n");
        goto error;
    }
    if(!SSL_CTX_set_default_verify_paths(ctx)) {
        printf("Failed to set the default trusted certificate store\n");
        goto error;
    }
    // Set the min and max TLS version allowed to work with this client
    if(!SSL_CTX_set_min_proto_version(ctx, min_tls_version)) {
        printf("Failed to set the minimum TLS protocol version\n");
        goto error;
    }
    if(!SSL_CTX_set_max_proto_version(ctx, max_tls_version)) {
        printf("Failed to set the minimum TLS protocol version\n");
        goto error;
    }
    return ctx;
error:
    if(ctx)
        SSL_CTX_free(ctx);
    return NULL;
}
int init_socket_TLS(const char *hostname, const char *port, int min_tls_version, int max_tls_version, SSL_CTX **ctx, SSL **ssl) {
    *ctx = NULL;
    *ssl = NULL;

    /* TCP CONNECTION */
    int fd = socket_connect(hostname, port);

    /* TLS CONNECTION */
    // Create the SSL context
    *ctx = create_SSL_ctx_client(min_tls_version, max_tls_version);
    if(*ctx == NULL) {
        printf("Failed to create the SSL_CTX object\n");
        goto error;
    }
    // Create the SSL instance
    *ssl = SSL_new(*ctx);
    if(*ssl == NULL) {
        printf("Failed to create the SSL object\n");
        goto error;
    }
    SSL_set_fd(*ssl, fd);
    // Set which hostname we want the cert for (this only to chose the cert, no hostname verification)
    if(!SSL_set_tlsext_host_name(*ssl, hostname)) {
        printf("Failed to set the SNI hostname\n");
        goto error;
    }
    // Set hostname verification
#if OPENSSL_VERSION_NUMBER < 0x40000000L
    if(!SSL_set1_host(*ssl, hostname)) {
#else
    if(!SSL_set1_dnsname(*ssl, hostname)) {
#endif
        printf("Failed to set the certificate verification hostname");
        goto error;
    }
    return fd;

error:
    ERR_print_errors_fp(stderr);
    if(*ssl)
        SSL_free(*ssl);
    if(*ctx)
        SSL_CTX_free(*ctx);
    return -1;
}

// Source server: https://github.com/openssl/openssl/blob/master/demos/guide/tls-server-block.c
SSL_CTX* create_SSL_ctx_server(int min_tls_version, int max_tls_version) {
    if(min_tls_version > max_tls_version)
        return NULL;
    int res = EXIT_FAILURE;
    SSL_CTX* ctx;
    long opts;

    /* TLS */
    // Create the SSL context
    ctx = SSL_CTX_new(TLS_server_method());
    if(ctx == NULL) {
        errx(res, "Failed to create server SSL_CTX");
        goto error;
    }
    // Set the min TLS version allowed to work with this client
    if(!SSL_CTX_set_min_proto_version(ctx, min_tls_version)) {
        errx(res, "Failed to set the minimum TLS protocol version");
        goto error;
    }
    if(!SSL_CTX_set_max_proto_version(ctx, max_tls_version)) {
        errx(res, "Failed to set the maximum TLS protocol version");
        goto error;
    }

    // Security level
    // TODO: Maybe pass a param to manage this
    //SSL_CTX_set_security_level(ctx, security_level);

    opts = SSL_OP_IGNORE_UNEXPECTED_EOF;
    if(min_tls_version >= TLS1_3_VERSION)
        opts |= SSL_OP_NO_RENEGOTIATION;
    opts |= SSL_OP_SERVER_PREFERENCE;
    SSL_CTX_set_options(ctx, opts);

    if(SSL_CTX_use_certificate_chain_file(ctx, "chain.pem") <= 0) {
        errx(res, "Failed to load the server certificate chain file");
        goto error;
    }
    if(SSL_CTX_use_PrivateKey_file(ctx, "pkey.pem", SSL_FILETYPE_PEM) <= 0) {
        errx(res, "Error loading the server private key file, possible key/cert mismatch???");
        goto error;
    }

    SSL_CTX_set_session_id_context(ctx, (const unsigned char*)"RayChat", 7);
    SSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_SERVER);

    SSL_CTX_sess_set_cache_size(ctx, 1024);

    SSL_CTX_set_timeout(ctx, 3600);

    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);

    // Logging
    SSL_CTX_set_info_callback(ctx, [](const SSL *ssl, int where, int ret) {
        const char *str = SSL_state_string_long(ssl);
        printf("[SSL STATE] %s\n", str);
    });

    return ctx;
error:
    ERR_print_errors_fp(stderr);
    if(ctx)
        SSL_CTX_free(ctx);
    return NULL;
}
