#include "../../include/core/password_hash.hpp"

int hashSHA256_EVP_Q(const char* pwd, size_t pwdLen, unsigned char res[SHA256_DIGEST_LENGTH]) {
    size_t resLen;

    if(EVP_Q_digest(NULL, "SHA256", NULL, pwd, pwdLen, res, &resLen))
        return EXIT_SUCCESS;
    else
        return EXIT_FAILURE;
}
int hashSHA256_SHA256(const char* pwd, size_t pwdLen, unsigned char res[SHA256_DIGEST_LENGTH]) {
    SHA256_CTX sha256;

    if(SHA256_Init(&sha256) == 0)
        return EXIT_FAILURE;
    if(SHA256_Update(&sha256, pwd, pwdLen))
        return EXIT_FAILURE;
    if(SHA256_Final(res, &sha256))
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

int hashSHA256(const char* pwd, size_t pwdLen, unsigned char res[SHA256_DIGEST_LENGTH]) {
    return hashSHA256_EVP_Q(pwd, pwdLen, res);
}

// Copied from: https://docs.openssl.org/3.4/man7/EVP_KDF-ARGON2/#examples
int hashArgon2d(char *pwd, size_t pwdLen, unsigned char* salt, size_t saltLen, unsigned char* res, size_t resLen) {
    int retval = EXIT_FAILURE;

    EVP_KDF *kdf = NULL;
    EVP_KDF_CTX *kctx = NULL;
    OSSL_PARAM params[6], *p = params;

    uint32_t lanes = 2, threads = 1, memcost = 65536;

    /* required if threads > 1 */
    p = params;
    *p++ = OSSL_PARAM_construct_uint32(OSSL_KDF_PARAM_THREADS, &threads);
    *p++ = OSSL_PARAM_construct_uint32(OSSL_KDF_PARAM_ARGON2_LANES, &lanes);
    *p++ = OSSL_PARAM_construct_uint32(OSSL_KDF_PARAM_ARGON2_MEMCOST, &memcost);
    *p++ = OSSL_PARAM_construct_octet_string(OSSL_KDF_PARAM_SALT, salt, saltLen);
    *p++ = OSSL_PARAM_construct_octet_string(OSSL_KDF_PARAM_PASSWORD, pwd, pwdLen);
    *p++ = OSSL_PARAM_construct_end();

    if((kdf = EVP_KDF_fetch(NULL, "ARGON2D", NULL)) == NULL)
        goto fail;
    if((kctx = EVP_KDF_CTX_new(kdf)) == NULL)
        goto fail;
    if(EVP_KDF_derive(kctx, &res[0], resLen, params) == 0)
        goto fail;

    retval = EXIT_SUCCESS;

fail:
    if(kdf)
        EVP_KDF_free(kdf);
    if(kctx)
        EVP_KDF_CTX_free(kctx);

    return retval;
}
