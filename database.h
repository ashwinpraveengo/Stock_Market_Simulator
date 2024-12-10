#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>

int initialize_database();

sqlite3* open_database();

void close_database(sqlite3 *db);

int signup(const char *username, const char *password);
int login(const char *username, const char *password, int *user_id);

int buy_stocks(int user_id, const char *symbol, int quantity, double price);
int sell_stocks(int user_id, const char *symbol, int quantity, double price);

int view_portfolio(int user_id);

int view_transactions(int user_id);


int update_leaderboard(int user_id);
int view_leaderboard();

#endif 