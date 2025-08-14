
#include "db.h"
#include "ui.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void print_menu(void) {
    ui_title("Tiny Password Manager");
    printf("1) Add credential\n");
    printf("2) List credentials\n");
    printf("3) Find credential by service\n");
    printf("4) Delete credential by service\n");
    printf("5) Save vault\n");
    printf("6) Change master password\n");
    printf("7) Exit\n");
    ui_prompt("> ");
}

static void add_credential(Database *db) {
    Credential c;
    memset(&c, 0, sizeof(c));
    ui_prompt("Service: ");
    ui_read_line(c.service, sizeof(c.service));
    ui_prompt("Username: ");
    ui_read_line(c.username, sizeof(c.username));
    ui_prompt("Password: ");
    ui_read_line(c.password, sizeof(c.password));

    if (strlen(c.service) == 0) {
        ui_err("Service cannot be empty.");
        return;
    }
    if (!db_add(db, &c)) {
        ui_err("Failed to add credential (OOM).");
    } else {
        ui_ok("Credential added (unsaved).");
    }
}

static void list_credentials(const Database *db) {
    if (db->count == 0) {
        ui_info("No credentials.");
        return;
    }
    printf("\nIndex  Service                          Username\n");
    printf("-----  --------------------------------  --------------------------------\n");
    for (size_t i = 0; i < db->count; ++i) {
        printf("%5zu  %-32.32s  %-32.32s\n", i, db->items[i].service, db->items[i].username);
    }
}

static void find_credential(const Database *db) {
    char svc[128];
    ui_prompt("Service to find: ");
    ui_read_line(svc, sizeof(svc));
    int idx = db_find_index(db, svc);
    if (idx < 0) {
        ui_warn("Not found.");
        return;
    }
    const Credential *c = &db->items[idx];
    printf("Service : %s\n", c->service);
    printf("Username: %s\n", c->username);
    printf("Password: %s\n", c->password);
}

static void delete_credential(Database *db) {
    char svc[128];
    ui_prompt("Service to delete: ");
    ui_read_line(svc, sizeof(svc));
    if (db_delete(db, svc)) {
        ui_ok("Deleted (unsaved).");
    } else {
        ui_warn("Service not found.");
    }
}

static void change_master_and_save(Database *db, const char *path, char *master, size_t msz) {
    char newpass[256];
    ui_prompt("New master password: ");
    ui_read_password(newpass, sizeof(newpass));
    if (strlen(newpass) == 0) {
        ui_warn("Master password unchanged.");
        return;
    }
    strncpy(master, newpass, msz - 1);
    master[msz - 1] = '\0';
    char err[256];
    if (db_save(db, path, master, err, sizeof(err))) {
        ui_ok("Master password changed and vault saved.");
        db->dirty = 0;
    } else {
        ui_err(err);
    }
}

int main(int argc, char **argv) {
    const char *path = "vault.dat";
    if (argc >= 3 && strcmp(argv[1], "-f") == 0) {
        path = argv[2];
    } else {
        char input[512];
        ui_prompt("Enter path to vault (default: vault.dat): ");
        ui_read_line(input, sizeof(input));
        if (strlen(input) > 0) path = strdup(input);
    }

    FILE *f = fopen(path, "rb");
    int exists = (f != NULL);
    if (f) fclose(f);

    char master[256];
    Database db;
    db_init(&db);

    if (!exists) {
        ui_prompt("Vault not found. Create new vault? (y/n): ");
        char ans[8]; ui_read_line(ans, sizeof(ans));
        if (ans[0] != 'y' && ans[0] != 'Y') {
            ui_info("Aborted.");
            db_free(&db);
            return 0;
        }
        ui_prompt("Set master password: ");
        ui_read_password(master, sizeof(master));
        if (strlen(master) == 0) {
            ui_err("Master password cannot be empty.");
            db_free(&db);
            return 1;
        }
        char err[256];
        if (db_save(&db, path, master, err, sizeof(err))) {
            ui_ok("Vault created.");
        } else {
            ui_err(err);
            db_free(&db);
            return 1;
        }
    } else {
        ui_prompt("Master password: ");
        ui_read_password(master, sizeof(master));
        char err[256];
        if (!db_load(&db, path, master, err, sizeof(err))) {
            ui_err(err);
            db_free(&db);
            return 1;
        } else {
            ui_ok("Vault unlocked.");
        }
    }

    for (;;) {
        print_menu();
        char choice[8]; ui_read_line(choice, sizeof(choice));
        switch (choice[0]) {
            case '1': add_credential(&db); break;
            case '2': list_credentials(&db); break;
            case '3': find_credential(&db); break;
            case '4': delete_credential(&db); break;
            case '5': {
                char err[256];
                if (db_save(&db, path, master, err, sizeof(err))) {
                    ui_ok("Vault saved.");
                    db.dirty = 0;
                } else ui_err(err);
                break;
            }
            case '6': change_master_and_save(&db, path, master, sizeof(master)); break;
            case '7':
                if (db.dirty) {
                    ui_prompt("Unsaved changes. Save before exit? (y/n): ");
                    char a[8]; ui_read_line(a, sizeof(a));
                    if (a[0] == 'y' || a[0] == 'Y') {
                        char err[256];
                        if (db_save(&db, path, master, err, sizeof(err))) {
                            ui_ok("Saved.");
                        } else ui_err(err);
                    }
                }
                db_free(&db);
                ui_info("Goodbye.");
                return 0;
            default:
                ui_warn("Invalid choice.");
        }
    }
}
