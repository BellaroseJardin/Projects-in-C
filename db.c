
#include "db.h"
#include "crypto.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAGIC "PWDDB1\0"
#define VERSION 1

typedef struct {
    char magic[8];
    uint32_t version;
    uint32_t salt;
    uint32_t count;
} DBHeader;

int db_init(Database *db) {
    if (!db) return 0;
    db->items = NULL;
    db->count = 0;
    db->capacity = 0;
    db->salt = (uint32_t)time(NULL) ^ (uint32_t)(uintptr_t)db;
    db->dirty = 0;
    return 1;
}

void db_free(Database *db) {
    if (!db) return;
    free(db->items);
    db->items = NULL;
    db->count = 0;
    db->capacity = 0;
}

int db_reserve(Database *db, size_t n) {
    if (n <= db->capacity) return 1;
    size_t newcap = db->capacity ? db->capacity : 8;
    while (newcap < n) newcap *= 2;
    Credential *p = (Credential *)realloc(db->items, newcap * sizeof(Credential));
    if (!p) return 0;
    db->items = p;
    db->capacity = newcap;
    return 1;
}

int db_add(Database *db, const Credential *c) {
    if (!db || !c) return 0;
    if (!db_reserve(db, db->count + 1)) return 0;
    db->items[db->count++] = *c;
    db->dirty = 1;
    return 1;
}

int db_find_index(const Database *db, const char *service) {
    if (!db || !service) return -1;
    for (size_t i = 0; i < db->count; ++i) {
        if (strncmp(db->items[i].service, service, MAX_SERVICE) == 0) {
            return (int)i;
        }
    }
    return -1;
}

int db_delete(Database *db, const char *service) {
    int idx = db_find_index(db, service);
    if (idx < 0) return 0;
    db->items[idx] = db->items[db->count - 1];
    db->count--;
    db->dirty = 1;
    return 1;
}

int db_load(Database *db, const char *path, const char *master_password, char *err, size_t errsz) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        if (err && errsz) snprintf(err, errsz, "Cannot open '%s' for reading", path);
        return 0;
    }
    DBHeader h;
    size_t r = fread(&h, 1, sizeof(h), f);
    if (r != sizeof(h) || strncmp(h.magic, MAGIC, 7) != 0 || h.version != VERSION) {
        if (err && errsz) snprintf(err, errsz, "Invalid vault format");
        fclose(f);
        return 0;
    }
    if (!db_reserve(db, h.count)) {
        if (err && errsz) snprintf(err, errsz, "Out of memory");
        fclose(f);
        return 0;
    }
    size_t payload_len = (size_t)h.count * sizeof(Credential);
    uint8_t *payload = (uint8_t *)malloc(payload_len);
    if (!payload) {
        if (err && errsz) snprintf(err, errsz, "Out of memory");
        fclose(f);
        return 0;
    }
    size_t got = fread(payload, 1, payload_len, f);
    fclose(f);
    if (got != payload_len) {
        if (err && errsz) snprintf(err, errsz, "Vault truncated");
        free(payload);
        return 0;
    }
    uint64_t key = derive_key(master_password, h.salt);
    xor_keystream_bytes(payload, payload_len, key);
    memcpy(db->items, payload, payload_len);
    db->count = h.count;
    db->salt = h.salt;
    db->dirty = 0;
    free(payload);
    return 1;
}

int db_save(const Database *db, const char *path, const char *master_password, char *err, size_t errsz) {
    FILE *f = fopen(path, "wb");
    if (!f) {
        if (err && errsz) snprintf(err, errsz, "Cannot open '%s' for writing", path);
        return 0;
    }
    DBHeader h;
    memset(&h, 0, sizeof(h));
    memcpy(h.magic, MAGIC, 8);
    h.version = VERSION;
    h.salt = db->salt;
    h.count = (uint32_t)db->count;

    if (fwrite(&h, 1, sizeof(h), f) != sizeof(h)) {
        if (err && errsz) snprintf(err, errsz, "Failed to write header");
        fclose(f);
        return 0;
    }

    size_t payload_len = db->count * sizeof(Credential);
    uint8_t *payload = (uint8_t *)malloc(payload_len);
    if (!payload) {
        if (err && errsz) snprintf(err, errsz, "Out of memory");
        fclose(f);
        return 0;
    }
    memcpy(payload, db->items, payload_len);
    uint64_t key = derive_key(master_password, db->salt);
    xor_keystream_bytes(payload, payload_len, key);

    size_t wrote = fwrite(payload, 1, payload_len, f);
    free(payload);
    if (wrote != payload_len) {
        if (err && errsz) snprintf(err, errsz, "Failed to write payload");
        fclose(f);
        return 0;
    }
    fclose(f);
    return 1;
}
