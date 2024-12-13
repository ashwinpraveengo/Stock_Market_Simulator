#include "database.h"
#include <stdio.h>
#include <stdlib.h>
#define RESET_COLOR "\033[0m"
#define BOLD "\033[1m"
#define CYAN "\033[36m"
#include "api.h"

int execute_sql(sqlite3 *db, const char *sql) {
    char *err_msg = 0;
    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK ) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        return rc;
    }
    return SQLITE_OK;
}

int initialize_database() {
    sqlite3 *db;
    int rc = sqlite3_open("stock_simulator.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    const char *sql = 
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT NOT NULL UNIQUE,"
        "password TEXT NOT NULL,"
        "cash_balance REAL DEFAULT 10000.0,"
        "total_portfolio_value REAL DEFAULT 0.0" 
        ");"

        "CREATE TABLE IF NOT EXISTS portfolio ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "user_id INTEGER NOT NULL,"
        "stock_symbol TEXT NOT NULL,"
        "quantity INTEGER NOT NULL,"
        "purchase_price REAL NOT NULL,"
        "FOREIGN KEY (user_id) REFERENCES users(id)"
        ");"

        "CREATE TABLE IF NOT EXISTS transactions ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "user_id INTEGER NOT NULL,"
        "stock_symbol TEXT NOT NULL,"
        "transaction_type TEXT CHECK(transaction_type IN ('buy', 'sell')),"
        "quantity INTEGER NOT NULL,"
        "price REAL NOT NULL,"
        "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "FOREIGN KEY (user_id) REFERENCES users(id)"
        ");"

        "CREATE TABLE IF NOT EXISTS leaderboard ("
        "user_id INTEGER PRIMARY KEY,"
        "rank INTEGER,"
        "FOREIGN KEY (user_id) REFERENCES users(id)"
        ");";

    rc = execute_sql(db, sql);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to create tables.\n");
        sqlite3_close(db);
        return rc;
    }

    printf("Database initialized successfully.\n");
    sqlite3_close(db);
    return SQLITE_OK;
}

sqlite3* open_database() {
    sqlite3 *db;
    int rc = sqlite3_open("stock_simulator.db", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return NULL;
    }
    return db;
}

void close_database(sqlite3 *db) {
    if (db) {
        sqlite3_close(db);
    }
}

int buy_stocks(int user_id, const char *symbol, int quantity, double price) {
    sqlite3 *db = open_database();
    if (!db) return -1;

    double total_cost = quantity * price;

    const char *cash_sql = "SELECT cash_balance FROM users WHERE id = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, cash_sql, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare cash query: %s\n", sqlite3_errmsg(db));
        close_database(db);
        return -1;
    }
    sqlite3_bind_int(stmt, 1, user_id);
    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        fprintf(stderr, "User not found.\n");
        sqlite3_finalize(stmt);
        close_database(db);
        return -1;
    }
    double cash_balance = sqlite3_column_double(stmt, 0);
    sqlite3_finalize(stmt);

    if (cash_balance < total_cost) {
        printf("Insufficient funds. You have $%.2f but need $%.2f.\n", cash_balance, total_cost);
        close_database(db);
        return -1;
    }

    execute_sql(db, "BEGIN TRANSACTION;");

    const char *update_cash_sql = "UPDATE users SET cash_balance = cash_balance - ? WHERE id = ?;";
    if (sqlite3_prepare_v2(db, update_cash_sql, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare cash update: %s\n", sqlite3_errmsg(db));
        execute_sql(db, "ROLLBACK;");
        close_database(db);
        return -1;
    }
    sqlite3_bind_double(stmt, 1, total_cost);
    sqlite3_bind_int(stmt, 2, user_id);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        fprintf(stderr, "Failed to update cash balance: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        execute_sql(db, "ROLLBACK;");
        close_database(db);
        return -1;
    }
    sqlite3_finalize(stmt);

    const char *check_portfolio_sql = "SELECT quantity, purchase_price FROM portfolio WHERE user_id = ? AND stock_symbol = ?;";
    if (sqlite3_prepare_v2(db, check_portfolio_sql, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare portfolio check: %s\n", sqlite3_errmsg(db));
        execute_sql(db, "ROLLBACK;");
        close_database(db);
        return -1;
    }
    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_bind_text(stmt, 2, symbol, -1, SQLITE_TRANSIENT);
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        int existing_quantity = sqlite3_column_int(stmt, 0);
        double existing_price = sqlite3_column_double(stmt, 1);
        sqlite3_finalize(stmt);

        double new_average_price = ((existing_quantity * existing_price) + (quantity * price)) / (existing_quantity + quantity);

        const char *update_portfolio_sql = "UPDATE portfolio SET quantity = ?, purchase_price = ? WHERE user_id = ? AND stock_symbol = ?;";
        if (sqlite3_prepare_v2(db, update_portfolio_sql, -1, &stmt, 0) != SQLITE_OK) {
            fprintf(stderr, "Failed to prepare portfolio update: %s\n", sqlite3_errmsg(db));
            execute_sql(db, "ROLLBACK;");
            close_database(db);
            return -1;
        }
        sqlite3_bind_int(stmt, 1, existing_quantity + quantity);
        sqlite3_bind_double(stmt, 2, new_average_price);
        sqlite3_bind_int(stmt, 3, user_id);
        sqlite3_bind_text(stmt, 4, symbol, -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            fprintf(stderr, "Failed to update portfolio: %s\n", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            execute_sql(db, "ROLLBACK;");
            close_database(db);
            return -1;
        }
        sqlite3_finalize(stmt);
    } else {
        sqlite3_finalize(stmt);

        const char *insert_portfolio_sql = "INSERT INTO portfolio (user_id, stock_symbol, quantity, purchase_price) VALUES (?, ?, ?, ?);";
        if (sqlite3_prepare_v2(db, insert_portfolio_sql, -1, &stmt, 0) != SQLITE_OK) {
            fprintf(stderr, "Failed to prepare portfolio insert: %s\n", sqlite3_errmsg(db));
            execute_sql(db, "ROLLBACK;");
            close_database(db);
            return -1;
        }
        sqlite3_bind_int(stmt, 1, user_id);
        sqlite3_bind_text(stmt, 2, symbol, -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 3, quantity);
        sqlite3_bind_double(stmt, 4, price);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            fprintf(stderr, "Failed to insert into portfolio: %s\n", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            execute_sql(db, "ROLLBACK;");
            close_database(db);
            return -1;
        }
        sqlite3_finalize(stmt);
    }

    const char *insert_transaction_sql = "INSERT INTO transactions (user_id, stock_symbol, transaction_type, quantity, price) VALUES (?, ?, ?, ?, ?);";
    if (sqlite3_prepare_v2(db, insert_transaction_sql, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare transaction insert: %s\n", sqlite3_errmsg(db));
        execute_sql(db, "ROLLBACK;");
        close_database(db);
        return -1;
    }
    sqlite3_bind_int(stmt, 1, user_id);        
    sqlite3_bind_text(stmt, 2, symbol, -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 3, "buy", -1, SQLITE_STATIC);  
    sqlite3_bind_int(stmt, 4, quantity);          
    sqlite3_bind_double(stmt, 5, price);         

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        fprintf(stderr, "Failed to insert transaction: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        execute_sql(db, "ROLLBACK;");
        close_database(db);
        return -1;
    }
    sqlite3_finalize(stmt);

    const char *update_total_sql = 
        "UPDATE users SET total_portfolio_value = "
        "(SELECT SUM(quantity * purchase_price) FROM portfolio WHERE user_id = ?) WHERE id = ?;";
    if (sqlite3_prepare_v2(db, update_total_sql, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare total portfolio update: %s\n", sqlite3_errmsg(db));
        execute_sql(db, "ROLLBACK;");
        close_database(db);
        return -1;
    }
    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_bind_int(stmt, 2, user_id);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        fprintf(stderr, "Failed to update total portfolio value: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        execute_sql(db, "ROLLBACK;");
        close_database(db);
        return -1;
    }
    sqlite3_finalize(stmt);

    execute_sql(db, "COMMIT;");

    printf("Bought %d shares of %s at $%.2f each. Total cost: $%.2f\n", quantity, symbol, price, total_cost);
    close_database(db);
    return 0;
}

int sell_stocks(int user_id, const char *symbol, int quantity, double price) {
    sqlite3 *db = open_database();
    if (!db) return -1;

    const char *portfolio_sql = "SELECT quantity FROM portfolio WHERE user_id = ? AND stock_symbol = ?;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, portfolio_sql, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare portfolio query: %s\n", sqlite3_errmsg(db));
        close_database(db);
        return -1;
    }
    sqlite3_bind_int(stmt, 1, user_id);
    sqlite3_bind_text(stmt, 2, symbol, -1, SQLITE_TRANSIENT);
    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_ROW) {
        printf("You do not own any shares of %s.\n", symbol);
        sqlite3_finalize(stmt);
        close_database(db);
        return -1;
    }
    int owned_quantity = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);

    if (owned_quantity < quantity) {
        printf("Insufficient shares. You own %d shares of %s.\n", owned_quantity, symbol);
        close_database(db);
        return -1;
    }

    double total_revenue = quantity * price;

    execute_sql(db, "BEGIN TRANSACTION;");

    const char *update_cash_sql = "UPDATE users SET cash_balance = cash_balance + ? WHERE id = ?;";
    if (sqlite3_prepare_v2(db, update_cash_sql, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare cash update: %s\n", sqlite3_errmsg(db));
        execute_sql(db, "ROLLBACK;");
        close_database(db);
        return -1;
    }
    sqlite3_bind_double(stmt, 1, total_revenue);
    sqlite3_bind_int(stmt, 2, user_id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        fprintf(stderr, "Failed to update cash balance: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        execute_sql(db, "ROLLBACK;");
        close_database(db);
        return -1;
    }
    sqlite3_finalize(stmt);

    if (owned_quantity == quantity) {

        const char *delete_portfolio_sql = "DELETE FROM portfolio WHERE user_id = ? AND stock_symbol = ?;";
        if (sqlite3_prepare_v2(db, delete_portfolio_sql, -1, &stmt, 0) != SQLITE_OK) {
            fprintf(stderr, "Failed to prepare portfolio delete: %s\n", sqlite3_errmsg(db));
            execute_sql(db, "ROLLBACK;");
            close_database(db);
            return -1;
        }
        sqlite3_bind_int(stmt, 1, user_id);
        sqlite3_bind_text(stmt, 2, symbol, -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            fprintf(stderr, "Failed to delete portfolio entry: %s\n", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            execute_sql(db, "ROLLBACK;");
            close_database(db);
            return -1;
        }
        sqlite3_finalize(stmt);
    } else {

        const char *update_portfolio_sql = "UPDATE portfolio SET quantity = quantity - ? WHERE user_id = ? AND stock_symbol = ?;";
        if (sqlite3_prepare_v2(db, update_portfolio_sql, -1, &stmt, 0) != SQLITE_OK) {
            fprintf(stderr, "Failed to prepare portfolio update: %s\n", sqlite3_errmsg(db));
            execute_sql(db, "ROLLBACK;");
            close_database(db);
            return -1;
        }
        sqlite3_bind_int(stmt, 1, quantity);
        sqlite3_bind_int(stmt, 2, user_id);
        sqlite3_bind_text(stmt, 3, symbol, -1, SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            fprintf(stderr, "Failed to update portfolio: %s\n", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            execute_sql(db, "ROLLBACK;");
            close_database(db);
            return -1;
        }
        sqlite3_finalize(stmt);
    }

    const char *insert_transaction_sql = "INSERT INTO transactions (user_id, stock_symbol, transaction_type, quantity, price) VALUES (?, ?, ?, ?, ?);";
    if (sqlite3_prepare_v2(db, insert_transaction_sql, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare transaction insert: %s\n", sqlite3_errmsg(db));
        execute_sql(db, "ROLLBACK;");
        close_database(db);
        return -1;
    }
    sqlite3_bind_int(stmt, 1, user_id);           
    sqlite3_bind_text(stmt, 2, symbol, -1, SQLITE_TRANSIENT); 
    sqlite3_bind_text(stmt, 3, "sell", -1, SQLITE_STATIC);   
    sqlite3_bind_int(stmt, 4, quantity);          
    sqlite3_bind_double(stmt, 5, price);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        fprintf(stderr, "Failed to insert transaction: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        execute_sql(db, "ROLLBACK;");
        close_database(db);
        return -1;
    }
    sqlite3_finalize(stmt);

    const char *recalculate_portfolio_sql = "SELECT SUM(quantity * purchase_price) FROM portfolio WHERE user_id = ?;";
    if (sqlite3_prepare_v2(db, recalculate_portfolio_sql, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare portfolio recalculation: %s\n", sqlite3_errmsg(db));
        execute_sql(db, "ROLLBACK;");
        close_database(db);
        return -1;
    }
    sqlite3_bind_int(stmt, 1, user_id);
    rc = sqlite3_step(stmt);
    double total_portfolio_value = (rc == SQLITE_ROW) ? sqlite3_column_double(stmt, 0) : 0.0;
    sqlite3_finalize(stmt);

    const char *update_total_value_sql = "UPDATE users SET total_portfolio_value = ? WHERE id = ?;";
    if (sqlite3_prepare_v2(db, update_total_value_sql, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare total portfolio value update: %s\n", sqlite3_errmsg(db));
        execute_sql(db, "ROLLBACK;");
        close_database(db);
        return -1;
    }
    sqlite3_bind_double(stmt, 1, total_portfolio_value);
    sqlite3_bind_int(stmt, 2, user_id);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        fprintf(stderr, "Failed to update total portfolio value: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        execute_sql(db, "ROLLBACK;");
        close_database(db);
        return -1;
    }
    sqlite3_finalize(stmt);

    execute_sql(db, "COMMIT;");

    printf("Sold %d shares of %s at $%.2f each. Total revenue: $%.2f\n", quantity, symbol, price, total_revenue);
    close_database(db);
    return 0;
}

int view_portfolio(int user_id) {
    sqlite3 *db = open_database();
    if (!db) return -1;

    const char *sql = "SELECT stock_symbol, quantity, purchase_price FROM portfolio WHERE user_id = ?;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare portfolio query: %s\n", sqlite3_errmsg(db));
        close_database(db);
        return -1;
    }

    sqlite3_bind_int(stmt, 1, user_id);

    printf("\n=== Your Portfolio ===\n");
    printf("%-10s %-10s %-15s %-15s %-15s\n", "Symbol", "Quantity", "Purchase Price", "Current Price", "P/L");

    double total_cost = 0.0;
    double total_current = 0.0;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *symbol = sqlite3_column_text(stmt, 0);
        int quantity = sqlite3_column_int(stmt, 1);
        double purchase_price = sqlite3_column_double(stmt, 2);

        double current_price;
        if (fetch_stock_price((const char*)symbol, &current_price) != 0) {
            current_price = 0.0;
        }

        double pl = (current_price - purchase_price) * quantity;
        total_cost += purchase_price * quantity;
        total_current += current_price * quantity;

        printf("%-10s %-10d $%-14.2f $%-14.2f $%-14.2f\n", symbol, quantity, purchase_price, current_price, pl);
    }

    double total_pl = total_current - total_cost;

    printf("\nTotal Portfolio Value: $%.2f\n", total_current);
    printf("Total Profit/Loss: $%.2f\n", total_pl);

    sqlite3_finalize(stmt);
    close_database(db);
    return 0;
}

int view_transactions(int user_id) {
    sqlite3 *db = open_database();
    if (!db) return -1;

    const char *sql = "SELECT transaction_type, stock_symbol, quantity, price, timestamp FROM transactions WHERE user_id = ? ORDER BY timestamp DESC;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare transactions query: %s\n", sqlite3_errmsg(db));
        close_database(db);
        return -1;
    }

    sqlite3_bind_int(stmt, 1, user_id);

    printf("\n=== Transaction History ===\n");
    printf("%-10s %-10s %-10s %-10s %-20s\n", "Type", "Symbol", "Quantity", "Price", "Timestamp");

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *type = sqlite3_column_text(stmt, 0);
        const unsigned char *symbol = sqlite3_column_text(stmt, 1);
        int quantity = sqlite3_column_int(stmt, 2);
        double price = sqlite3_column_double(stmt, 3);
        const unsigned char *timestamp = sqlite3_column_text(stmt, 4);

        printf("%-10s %-10s %-10d $%-9.2f %-20s\n", type, symbol, quantity, price, timestamp);
    }

    sqlite3_finalize(stmt);
    close_database(db);
    return 0;
}

int view_leaderboard() {
    sqlite3 *db = open_database();
    if (!db) return 0;

    const char *sql = 
        "SELECT users.username, users.cash_balance, users.total_portfolio_value, "
        "(users.cash_balance + users.total_portfolio_value) AS net_worth "
        "FROM users "
        "ORDER BY net_worth DESC "
        "LIMIT 10;";

    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare leaderboard query: %s\n", sqlite3_errmsg(db));
        close_database(db);
        return 0;
    }

    printf("\n%s=== Leaderboard ===%s\n", CYAN, RESET_COLOR);
    printf("%s%-4s   %-12s %-10s  %-12s%s\n", BOLD, "RANK", "USERNAME", "BALANCE", "NET WORTH", RESET_COLOR);

    int rank = 1;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char *username = sqlite3_column_text(stmt, 0);
        double cash_balance = sqlite3_column_double(stmt, 1);
        double total_portfolio_value = sqlite3_column_double(stmt, 2);

        printf("%-4d   %-12s $%-10.2f $%-12.2f\n", rank++, username, cash_balance, total_portfolio_value);
    }

    sqlite3_finalize(stmt);
    close_database(db);
    return 0;
}
