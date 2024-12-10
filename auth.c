#include "auth.h"
#include "database.h"
#include <stdio.h>
#include <string.h>
#include <openssl/sha.h>

void hash_password(const char *password, char *hashed_password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*)password, strlen(password), hash);
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(hashed_password + (i * 2), "%02x", hash[i]);
    }
    hashed_password[64] = 0; 
}

int signup(const char *username, const char *password) {
    sqlite3 *db = open_database();
    if (!db) return -1;

    char hashed_password[65];
    hash_password(password, hashed_password);

    const char *sql = "INSERT INTO users (username, password) VALUES (?, ?);";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        close_database(db);
        return -1;
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hashed_password, -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        close_database(db);
        return -1;
    }

    sqlite3_finalize(stmt);
    close_database(db);
    printf("Signup successful! Nw you can login with username and password\n");
    return 0;
}

int login(const char *username, const char *password, int *user_id) {
    sqlite3 *db = open_database();
    if (!db) return -1;

    char hashed_password[65];
    hash_password(password, hashed_password);

    const char *sql = "SELECT id FROM users WHERE username = ? AND password = ?;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        close_database(db);
        return -1;
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hashed_password, -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        *user_id = sqlite3_column_int(stmt, 0);
        printf("Login successful!\n");
        sqlite3_finalize(stmt);
        close_database(db);
        return 0;
    } else {
        printf("Invalid username or password.\n");
        sqlite3_finalize(stmt);
        close_database(db);
        return -1;
    }
}
