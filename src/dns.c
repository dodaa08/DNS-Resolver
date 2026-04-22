#include <stdio.h>
#include "dns.h"
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>  
#include <sys/time.h>
#include "cache.h"

int dns_encode_name(const char *name, uint8_t *out){
    int written = 0;
    const char *p = name;
    while(*p != '\0'){
        const char *dot = strchr(p, '.');
        size_t len = dot ? (size_t)(dot - p) : strlen(p);

        out[written] = (uint8_t)len;
        written++;

        memcpy(out + written, p, len);
        written += len;

        p += len + (dot ? 1 : 0);
    }

    out[written] = 0x00;
    written++;

    return written;
}


int dns_query(const char *domain, const char *dns_server, uint16_t port, char *out_ip){

    char cached_ip[64];

    if (cache_get(domain, cached_ip)) {
        strcpy(out_ip, cached_ip);
        return 1;  
    }
    
    uint8_t buf[512];
    uint8_t reply[512];

    dns_header header = {0};
    header.id      = htons(0xABCD);
    header.flags   = htons(0x0100);
    header.qdcount = htons(1);

    int offset = 0;
    memcpy(buf + offset, &header, sizeof(header));
    offset += sizeof(header);

    int n = dns_encode_name(domain, buf + offset);
    offset += n;

    uint16_t type = htons(1);
    memcpy(buf + offset, &type, 2);
    offset += 2;

    uint16_t class_ = htons(1);
    memcpy(buf + offset, &class_, 2);
    offset += 2;

    int sock = dns_send(buf, offset, dns_server, port);
    if (sock < 0) return -1;

    int received = dns_recv(sock, reply, sizeof(reply));
    close(sock);

    if (received < 0) return -1;

    uint32_t ttl;
    int result = dns_parse_response(reply, received, out_ip, &ttl);
    if (result == 1) cache_set(domain, out_ip, ttl);
    return result;
};


int dns_send(const uint8_t *buf, int len, const char *serverIp, uint16_t port){
    
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) return -1;

    struct sockaddr_in dest;
    dest.sin_family = AF_INET;
    dest.sin_port   = htons(port);
    if (inet_pton(AF_INET, serverIp, &dest.sin_addr) != 1) {
        fprintf(stderr, "Invalid DNS server IP: %s\n", serverIp);
        close(sock);
        return -1;
    }

    if (sendto(sock, buf, len, 0, (struct sockaddr*)&dest, sizeof(dest)) < 0) {
        perror("sendto");
        close(sock);
        return -1;
    }

    return sock;
};



int dns_recv(int sock, uint8_t *reply, size_t reply_len){
    struct timeval timeout = {
        .tv_sec = 5,
        .tv_usec = 0
    };

    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    

    int bytes = recvfrom(sock, reply, reply_len, 0, NULL, NULL);
    if (bytes < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK)
        fprintf(stderr, "DNS query timed out\n");
    else
        perror("recvfrom");
}

    return bytes;
}



int dns_parse_name(const uint8_t *buf, size_t buflen, int offset, char *out){
    int current_offset = offset;
    int next_offset = -1;
    int out_written = 0;
    
    while(current_offset<(int)buflen){

        // End of the word
         if (buf[current_offset] == 0) {
            break; 
         }


        // Pointer 
        if((buf[current_offset] & 0xC0) == 0xC0){
            if (next_offset == -1) next_offset = current_offset + 2;
            current_offset = ((buf[current_offset] & 0x3F) << 8) | buf[current_offset + 1];
            continue;
        }
        
        // Label decode
        else{
            uint8_t len = buf[current_offset];
            current_offset++;
            memcpy(out + out_written, buf + current_offset, len);
            out_written += len;
            current_offset += len;

            if(buf[current_offset] != 0){
                out[out_written] = '.';
                out_written++;
            }
        };
        
    };

    out[out_written] = '\0';
    if (next_offset == -1) next_offset = current_offset + 1;
    return next_offset;

}

// Parse response and extract IP address
int dns_parse_response(const uint8_t *buf, size_t buflen, char *out, uint32_t *out_ttl){
    dns_header header;
    memcpy(&header, buf, sizeof(dns_header));
    
    int rcode = ntohs(header.flags) & 0x000F;
    if (rcode != 0) {
    switch (rcode) {
        case 1: fprintf(stderr, "DNS error: Format error\n");         break;
        case 2: fprintf(stderr, "DNS error: Server failure\n");        break;
        case 3: fprintf(stderr, "DNS error: Domain does not exist\n"); break;
        case 5: fprintf(stderr, "DNS error: Query refused\n");         break;
        default: fprintf(stderr, "DNS error code: %d\n", rcode);      break;
    }
     return -1;
    }
    
    int qdcount = ntohs(header.qdcount);
    int ancount = ntohs(header.ancount);
    if (ancount == 0) {
     fprintf(stderr, "No answers in DNS response\n");
     return -1;
    }
    int offset = sizeof(dns_header);
    char name[256];



    // Skip questions
    for(int i = 0; i<qdcount; i++){
        offset = dns_parse_name(buf, buflen, offset, name);
        offset+=4;
    };

    // Parse the answer
    for(int i = 0; i<ancount; i++){
        offset = dns_parse_name(buf, buflen, offset, name);

        uint16_t type, rdlen;
        memcpy(&type, buf + offset, 2);
        type = ntohs(type);

        memcpy(&rdlen, buf + offset + 8, 2); 
        rdlen = ntohs(rdlen);

        offset += 10;

        if(type == 1 && rdlen == 4){
            uint32_t ttl;
            memcpy(&ttl, buf + offset - 6, 4);  // TTL is 4 bytes before rdlength
            *out_ttl = ntohl(ttl);
            sprintf(out, "%d.%d.%d.%d", buf[offset], buf[offset+1], buf[offset+2], buf[offset+3]);
            return 1;
        };

        offset += rdlen;
    }

    return -1;
}


// Implement the caching mechanism

// Error handling improve, code quality improve before sharing
