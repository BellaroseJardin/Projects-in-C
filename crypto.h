
#ifndef CRYPTO_H
#define CRYPTO_H

#include <stddef.h>
#include <stdint.h>

uint64_t derive_key(const char *master_password, uint32_t salt);
void xor_keystream_bytes(uint8_t *buf, size_t len, uint64_t seed);

#endif // CRYPTO_H
