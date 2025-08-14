
#include "crypto.h"
#include "ui.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

int main(void) {
    const char *pw = "secret";
    uint32_t salt = 12345;
    uint64_t key = derive_key(pw, salt);
    assert(key != 0);

    uint8_t msg[32];
    for (int i = 0; i < 32; ++i) msg[i] = (uint8_t)i;
    uint8_t copy[32];
    memcpy(copy, msg, sizeof(msg));

    xor_keystream_bytes(copy, sizeof(copy), key);
    // decrypt (XOR again) should restore original
    xor_keystream_bytes(copy, sizeof(copy), key);

    assert(memcmp(copy, msg, sizeof(msg)) == 0);
    ui_ok("crypto tests passed");
    return 0;
}
