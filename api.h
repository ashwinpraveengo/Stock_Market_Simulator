#ifndef API_H
#define API_H

int fetch_stock_price(const char *symbol, double *price);
int fetch_stock_details(const char *exchange);

#endif 
