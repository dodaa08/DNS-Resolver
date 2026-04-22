#include <stdio.h>
#include "dns.h"
#include <stdlib.h>

int main(int argc, char *argv[]){

    if (argc < 2) {
        printf("usage: ./dns-resolver <domain> [dns-server] [port]\n");
        return 1;
    }

    const char *domain   = argv[1];
    const char *serverIp = (argc >= 3) ? argv[2] : "8.8.8.8";
    uint16_t    port     = (argc >= 4) ? (uint16_t)atoi(argv[3]) : 53;

    char ip[64];
    if (dns_query(domain, serverIp, port, ip) == 1) {
        printf("IP Address: %s\n", ip);
    } else {
        printf("Could not resolve %s\n", domain);
        return 1;
    }

    return 0;
}
