
#ifndef DB_H
#define DB_H

#include <stddef.h>
#include <stdint.h>

#define MAX_SERVICE  64
#define MAX_USERNAME 64
#define MAX_PASSWORD 128

typedef struct {
    char service[MAX_SERVICE];
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
} Credential;

typedef struct {
    Credential *items;
    size_t count;
    size_t capacity;
    uint32_t salt;
    int dirty; // has unsaved changes
} Database;

int db_init(Database *db);
void db_free(Database *db);
int db_reserve(Database *db, size_t n);
int db_add(Database *db, const Credential *c);
int db_find_index(const Database *db, const char *service);
int db_delete(Database *db, const char *service);

int db_load(Database *db, const char *path, const char *master_password, char *err, size_t errsz);
int db_save(const Database *db, const char *path, const char *master_password, char *err, size_t errsz);

#endif // DB_H
