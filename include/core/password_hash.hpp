#ifndef CORE_PASSWORD_HASH_HPP
#define CORE_PASSWORD_HASH_HPP

#include <string.h>
#include <openssl/core_names.h>
#include <openssl/params.h>
#include <openssl/thread.h>
#include <openssl/kdf.h> 
#include <openssl/rand.h>
#include <openssl/sha.h>

int hashSHA256_EVP_Q(const char* pwd, size_t pwdLen, unsigned char res[SHA256_DIGEST_LENGTH]);
int hashSHA256_SHA256(const char* pwd, size_t pwdLen, unsigned char res[SHA256_DIGEST_LENGTH]);
int hashSHA256(const char* pwd, size_t pwdLen, unsigned char res[SHA256_DIGEST_LENGTH]);
int hashArgon2d(char *pwd, size_t pwdLen, unsigned char* salt, size_t saltLen, unsigned char* res, size_t resLen);

#endif
