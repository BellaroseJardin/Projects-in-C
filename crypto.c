
#include "crypto.h"
#include <string.h>

// 64-bit FNV-1a hash for simple key derivation.
static uint64_t fnv1a64(const void *data, size_t len) {
    const uint8_t *p = (const uint8_t *)data;
    uint64_t h = 0xcbf29ce484222325ULL;
    for (size_t i = 0; i < len; ++i) {
        h ^= p[i];
        h *= 0x100000001b3ULL;
    }
    return h;
}

uint64_t derive_key(const char *master_password, uint32_t salt) {
    // Combine salt and password; simple but portable.
    uint64_t h = fnv1a64(&salt, sizeof(salt));
    if (master_password) {
        h ^= fnv1a64(master_password, strlen(master_password));
        h *= 0x100000001b3ULL;
    }
    // Avoid zero seed
    if (h == 0) h = 0x9e3779b97f4a7c15ULL;
    return h;
}

// xorshift64* PRNG for keystream
static uint64_t xorshift64star(uint64_t x) {
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    return x * 0x2545F4914F6CDD1DULL;
}

void xor_keystream_bytes(uint8_t *buf, size_t len, uint64_t seed) {
    uint64_t s = seed;
    size_t i = 0;
    while (i < len) {
        s = xorshift64star(s);
        uint8_t block[8];
        for (int j = 0; j < 8; ++j) block[j] = (uint8_t)(s >> (j * 8));
        size_t n = (len - i) < 8 ? (len - i) : 8;
        for (size_t k = 0; k < n; ++k) {
            buf[i + k] ^= block[k];
        }
        i += n;
    }
}
