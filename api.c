#include "api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "cJSON.h"

#define FINNHUB_API_KEY "ctalvipr01qrt5hi060gctalvipr01qrt5hi0610"

struct string {
    char *ptr;
    size_t len;
};

void init_string(struct string *s) {
    s->len = 0;
    s->ptr = malloc(s->len + 1);
    if (s->ptr == NULL) {
        fprintf(stderr, "malloc() failed\n");
        exit(1);
    }
    s->ptr[0] = '\0';
}
size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s) {
    size_t new_len = s->len + size * nmemb;
    s->ptr = realloc(s->ptr, new_len + 1);
    if (s->ptr == NULL) {
        fprintf(stderr, "realloc() failed\n");
        exit(1);
    }
    memcpy(s->ptr + s->len, ptr, size * nmemb);
    s->ptr[new_len] = '\0';
    s->len = new_len;

    return size * nmemb;
}

int fetch_stock_details(const char *exchange) {
    CURL *curl;
    CURLcode res;
    struct string s;

    init_string(&s);

    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize CURL.\n");
        return -1;
    }

    char url[256];
    snprintf(url, sizeof(url), "https://finnhub.io/api/v1/stock/symbol?exchange=%s&token=%s", exchange, FINNHUB_API_KEY);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "CURL Error: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        free(s.ptr);
        return -1;
    }

    curl_easy_cleanup(curl);

    cJSON *json = cJSON_Parse(s.ptr);
    if (!json) {
        fprintf(stderr, "Failed to parse JSON response.\n");
        free(s.ptr);
        return -1;
    }

    if (!cJSON_IsArray(json)) {
        fprintf(stderr, "Unexpected response format.\n");
        cJSON_Delete(json);
        free(s.ptr);
        return -1;
    }

    printf("Available Stocks:\n");
    int count = 0;
    for (cJSON *item = json->child; item != NULL && count < 15; item = item->next) {
        cJSON *symbol = cJSON_GetObjectItem(item, "symbol");
        if (symbol && cJSON_IsString(symbol)) {
            double price;
            if (fetch_stock_price(symbol->valuestring, &price) == 0) {
                if (price != 0.00){

                printf("\nSymbol \tPrice \n%s \t$%.2f\n-----------------------", symbol->valuestring, price);
                }
            }
        count++;
    }
}

    cJSON_Delete(json);
    free(s.ptr);
    return 0;
}


int fetch_stock_price(const char *symbol, double *price) {
    CURL *curl;
    CURLcode res;
    struct string s;

    init_string(&s);

    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize CURL.\n");
        return -1;
    }

    char url[256];
    snprintf(url, sizeof(url), "https://finnhub.io/api/v1/quote?symbol=%s&token=%s", symbol, FINNHUB_API_KEY);

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "CURL Error: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        free(s.ptr);
        return -1;
    }

    curl_easy_cleanup(curl);

    cJSON *json = cJSON_Parse(s.ptr);
    if (!json) {
        fprintf(stderr, ".\n");
        free(s.ptr);
        return -1;
    }

    cJSON *current_price = cJSON_GetObjectItem(json, "c");
    if (current_price && cJSON_IsNumber(current_price)) {
        *price = current_price->valuedouble;
    } else {
        fprintf(stderr, "Invalid response.\n");
        cJSON_Delete(json);
        free(s.ptr);
        return -1;
    }

    cJSON_Delete(json);
    free(s.ptr);
    return 0;
}
