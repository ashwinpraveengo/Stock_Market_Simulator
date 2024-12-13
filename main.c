// main.c
#include "database.h"
#include "auth.h"
#include "api.h"
#include <stdio.h>
#include <string.h>
#include <termios.h>
#define RESET_COLOR "\033[0m"
#define BOLD "\033[1m"
#define UNDERLINE "\033[4m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"

void disable_echo() {
    struct termios t;
    tcgetattr(fileno(stdin), &t);
    t.c_lflag &= ~ECHO;
    tcsetattr(fileno(stdin), TCSANOW, &t);
}

void enable_echo() {
    struct termios t;
    tcgetattr(fileno(stdin), &t);
    t.c_lflag |= ECHO;
    tcsetattr(fileno(stdin), TCSANOW, &t);
}

void get_password(char *password, size_t size) {
    disable_echo();
    fgets(password, size, stdin);
    password[strcspn(password, "\n")] = 0; 
    enable_echo();
    printf("\n"); 
}

void show_main_menu() {
    printf("\n%s=== Stock Market Simulator ===%s\n", CYAN, RESET_COLOR);
    printf("%s1. Signup%s\n", GREEN, RESET_COLOR);
    printf("%s2. Login%s\n", GREEN, RESET_COLOR);
    printf("%s3. View Leaderboard%s\n", GREEN, RESET_COLOR);
    printf("%s4. Exit%s\n", GREEN, RESET_COLOR);
    printf("%sChoose an option: %s", YELLOW, RESET_COLOR);
}

void show_user_menu(int user_id) {
    printf("\n%s=== User Menu ===%s\n", CYAN, RESET_COLOR);
    printf("%s1. View Portfolio%s\n", GREEN, RESET_COLOR);
    printf("%s2. Display Stocks%s\n", GREEN, RESET_COLOR);
    printf("%s3. Buy Stocks%s\n", GREEN, RESET_COLOR);
    printf("%s4. Sell Stocks%s\n", GREEN, RESET_COLOR);
    printf("%s5. View Transaction History%s\n", GREEN, RESET_COLOR);
    printf("%s6. Fetch Stock Price%s\n", GREEN, RESET_COLOR);
    printf("%s7. View Leaderboard%s\n", GREEN, RESET_COLOR);
    printf("%s8. View Profile%s\n", GREEN, RESET_COLOR);
    printf("%s9. Logout%s\n", RED, RESET_COLOR);
    printf("%sChoose an option: %s", YELLOW, RESET_COLOR);
}

int main() {
    initialize_database();

    int choice;
    char username[50];
    char password[50];
    char confirm_password[50];
    int user_id;

    while (1) {
        show_main_menu();
        scanf("%d", &choice);
        getchar(); 

        switch (choice) {
            case 1:
                printf("=== Signup ===\n");
                printf("Enter username: ");
                fgets(username, sizeof(username), stdin);
                username[strcspn(username, "\n")] = 0; 

                printf("Enter password: ");
                get_password(password, sizeof(password));

                printf("Confirm password: ");
                get_password(confirm_password, sizeof(confirm_password));

                if (strcmp(password, confirm_password) != 0) {
                    printf("Passwords do not match. Please try again.\n");
                    break;
                }

                if (signup(username, password) != 0) {
                    printf("%sSignup failed: Username '%s' is already taken. Please choose a different username.%s\n", RED, username, RESET_COLOR);
                } 
                else{
                    printf("%sSignup successful! Now you can login with your username and password.%s\n", GREEN, RESET_COLOR);
                }
                break;

            case 2:
                printf("=== Login ===\n");
                printf("Enter username: ");
                fgets(username, sizeof(username), stdin);
                username[strcspn(username, "\n")] = 0;

                printf("Enter password: ");
                get_password(password, sizeof(password));

                if (login(username, password, &user_id) == 0) {
                    printf("Welcome, %s! (User ID: %d)\n", username, user_id);
                    int user_choice;
                    while (1) {
                        show_user_menu(user_id);
                        scanf("%d", &user_choice);
                        getchar();
                        if (user_choice == 9) {
                            printf("%sLogging out...%s\n", RED, RESET_COLOR);
                            break;
                        }
                        switch (user_choice) {
                            case 1:
                                view_portfolio(user_id);
                                break;
                            case 2:
                                fetch_stock_details("US");
                                break;
                            case 3: {
                                char symbol[10];
                                int quantity;
                                printf("Enter stock symbol to buy (e.g., AAPL): ");
                                fgets(symbol, sizeof(symbol), stdin);
                                symbol[strcspn(symbol, "\n")] = 0;

                                printf("Enter quantity to buy: ");
                                scanf("%d", &quantity);
                                getchar(); 

                                double price;
                                if (fetch_stock_price(symbol, &price) == 0) {
                                    buy_stocks(user_id, symbol, quantity, price);
                                } else {
                                    printf("Failed to fetch stock price. Cannot proceed with purchase.\n");
                                }
                                break;
                            }
                            case 4: {
                                char symbol[10];
                                int quantity;
                                printf("Enter stock symbol to sell (e.g., AAPL): ");
                                fgets(symbol, sizeof(symbol), stdin);
                                symbol[strcspn(symbol, "\n")] = 0;

                                printf("Enter quantity to sell: ");
                                scanf("%d", &quantity);
                                getchar(); 

                                double price;
                                if (fetch_stock_price(symbol, &price) == 0) {
                                    sell_stocks(user_id, symbol, quantity, price);
                                } else {
                                    printf("Failed to fetch stock price. Cannot proceed with sale.\n");
                                }
                                break;
                            }
                            case 5:
                                view_transactions(user_id);
                                break;
                            case 6: {
                                char symbol[10];
                                printf("Enter stock symbol (e.g., AAPL): ");
                                fgets(symbol, sizeof(symbol), stdin);
                                symbol[strcspn(symbol, "\n")] = 0;

                                double price;
                                if (fetch_stock_price(symbol, &price) == 0) {
                                    printf("Current price of %s: $%.2f\n", symbol, price);
                                } else {
                                    printf("Failed to fetch stock price.\n");
                                }
                                break;
                            }
                            case 7:
                                view_leaderboard();
                                break;
                            case 8:
                                view_user_details(user_id);
                                // if (view_user_details(user_id) == 0) {
                                //     // printf("Failed to fetch user details.\n");
                                // }
                                break;
                            default:
                                printf("Invalid option. Please try again.\n");
                        }
                    }
                }
                break;

            case 3:
                view_leaderboard();
                break;

            case 4:
                printf("Exiting...\n");
                return 0;

            default:
                printf("Invalid option. Please try again.\n");
        }
    }

    return 0;
}
