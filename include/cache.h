#include <stdio.h>
#include <time.h>
#include <stdint.h>

typedef struct {
    char     domain[256];  // "google.com"
    char     ip[64];       // "142.250.182.142"
    time_t   expiry;       // unix timestamp when this entry expires
    int      valid;        // 1 = slot in use, 0 = empty
} dns_cache_entry;

int cache_get(const char *domain, char *out_ip);
int cache_set(const char *domain, const char *ip, uint32_t ttl);