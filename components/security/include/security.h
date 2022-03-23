#ifndef __SECURITY_H__
#define __SECURITY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include "mbedtls/aes.h"
#include <mbedtls/sha512.h>

static unsigned char iv[16] = "1234567890123456";

mbedtls_aes_context aes;

static void init_aes(unsigned char enc_key[16], unsigned char dec_key[16], unsigned char _iv[16]);
static void deinit_aes();
static size_t get_output_size(size_t input_size);
static int get_qtd_blocks(size_t input_size);
static void aes_encrypt(unsigned char *input, size_t input_size, unsigned char *output);
static void aes_decrypt(unsigned char *input, size_t input_size, unsigned char *output);
static void print_hex(unsigned char *buf, int len);
static void to_uint8(unsigned char *buf, int len, uint8_t *output);
static void to_uchar(uint8_t *buf, int len, unsigned char *output);


#ifdef __cplusplus
}
#endif

#endif
