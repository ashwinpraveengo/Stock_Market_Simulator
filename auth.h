#ifndef AUTH_H
#define AUTH_H

int signup(const char *username, const char *password);
int login(const char *username, const char *password, int *user_id);

#endif 
