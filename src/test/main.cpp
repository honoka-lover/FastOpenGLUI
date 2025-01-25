//
// Created by m1393 on 2025/1/17.
//
#include <curl/curl.h>
#include <cstdio>

int main() {
    CURL *curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://www.google.com");
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    } else {
        fprintf(stderr, "Failed to initialize libcurl\n");
    }
    return 0;
}
