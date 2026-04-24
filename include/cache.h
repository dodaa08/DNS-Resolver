#include <stdio.h>
#include <time.h>
#include <stdint.h>

typedef struct {
    char     domain[256];
    char     ip[64];      
    time_t   expiry;       
    int      valid;       
} dns_cache_entry;

int cache_get(const char *domain, char *out_ip);
int cache_set(const char *domain, const char *ip, uint32_t ttl);