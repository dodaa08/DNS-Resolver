#include <stdio.h>
#include "dns.h"
#include <unistd.h>

int main(int argc, char *argv[]){

    if (argc < 2) {
        printf("usage: ./dns-resolver <domain>\n");
        return 1;
    }

    const char *domain = argv[1];

    uint8_t buf[512];
    uint8_t reply[512];

    int len = dns_query(buf, sizeof(buf), domain);
    int sock = dns_send(buf, len);
    int received = dns_recv(sock, reply, sizeof(reply));
    close(sock);

    printf("received %d bytes:\n", received);
    for(int i = 0; i < received; i++){
        printf("%02x", reply[i]);
        if((i+1)%8==0){
            printf("\n");
        }
    };

    // use decode these bytes and get a real human readable domain ip address for the answer
    char ip[64];
    if(dns_parse_response(reply, received, ip, sizeof(ip)) == 1){
        printf("\n IP Address: %s\n", ip);
    }
    else{
        printf("\nCould not find IP address.\n");
    };

    return 0;
}
