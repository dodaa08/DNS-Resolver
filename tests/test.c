#include <stdio.h>
#include <string.h>
#include "dns.h"
#include "cache.h"

// ---- simple test helpers ----
static int passed = 0;
static int failed = 0;

#define ASSERT(desc, expr) do { \
    if (expr) { printf("  [PASS] %s\n", desc); passed++; } \
    else       { printf("  [FAIL] %s\n", desc); failed++; } \
} while(0)


// ---- cache tests ----
void test_cache() {
    printf("\n=== Cache Tests ===\n");

    char out[64];

    // Miss on empty cache
    ASSERT("cache miss on empty cache", cache_get("google.com", out) == 0);

    // Store an entry with a 300s TTL and retrieve it
    cache_set("google.com", "1.2.3.4", 300);
    ASSERT("cache hit after set",      cache_get("google.com", out) == 1);
    ASSERT("cache returns correct ip", strcmp(out, "1.2.3.4") == 0);

    // Different domain should still miss
    ASSERT("cache miss for different domain", cache_get("github.com", out) == 0);
}


// ---- DNS resolution tests ----
void test_dns_resolution() {
    printf("\n=== DNS Resolution Tests ===\n");

    char ip[64];

    // Valid domain — default server 8.8.8.8
    int result = dns_query("google.com", "8.8.8.8", 53, ip);
    ASSERT("resolves google.com", result == 1);
    ASSERT("ip is non-empty",     strlen(ip) > 0);

    // Same domain second time — should hit cache
    char ip2[64];
    result = dns_query("google.com", "8.8.8.8", 53, ip2);
    ASSERT("second query succeeds (cache hit)", result == 1);
    ASSERT("cache returns same ip", strcmp(ip, ip2) == 0);
}


// ---- error handling tests ----
void test_error_handling() {
    printf("\n=== Error Handling Tests ===\n");

    char ip[64];

    // Invalid server IP — use a domain not previously cached
    int result = dns_query("example.org", "notAnIp", 53, ip);
    ASSERT("invalid server ip returns -1", result == -1);

    // Non-existent domain
    result = dns_query("thisdoesnotexist99999abc.com", "8.8.8.8", 53, ip);
    ASSERT("non-existent domain returns -1", result == -1);
}


int main() {
    printf("Running DNS Resolver Tests\n");
    printf("==========================\n");

    test_cache();
    test_dns_resolution();
    test_error_handling();

    printf("\n==========================\n");
    printf("Results: %d passed, %d failed\n", passed, failed);

    return failed > 0 ? 1 : 0;
}
