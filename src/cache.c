#include "cache.h"
#include <string.h>
#include <time.h>
#define CACHE_SIZE 128

static dns_cache_entry cache[CACHE_SIZE];

int cache_get(const char *domain, char *out_ip){
    time_t now = time(NULL);

    for(int i = 0; i < CACHE_SIZE; i++){
        if(cache[i].valid && strcmp(cache[i].domain, domain) == 0 && now < cache[i].expiry){
            fprintf(stderr, "[cache] HIT for %s\n", domain);
            strcpy(out_ip, cache[i].ip);
            return 1;
        }
    }

    return 0;
};

int cache_set(const char *domain, const char *ip, uint32_t ttl){
    
    for(int i = 0; i < CACHE_SIZE; i++){
        if(!cache[i].valid || time(NULL) >= cache[i].expiry){
            strcpy(cache[i].domain, domain);
            strcpy(cache[i].ip, ip);
            cache[i].expiry = time(NULL) + ttl;
            cache[i].valid = 1;
            return 1;
        }
    }


    return 0;
};