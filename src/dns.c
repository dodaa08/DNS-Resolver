#include <stdio.h>
#include "dns.h"
#include <string.h>
#include <arpa/inet.h>   
#include <sys/socket.h>

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


int dns_query(uint8_t *buf, size_t buflen, const char *domain){
    if(buflen < 512) return -1;    

    dns_header header = {0};

    header.id = htons(0xABCD);
    header.flags = htons(0x0100);
    header.qdcount = htons(1);

    int offset = 0;

    memcpy(buf + offset, &header, sizeof(header));
    offset+=sizeof(header);

    int n = dns_encode_name(domain, buf + offset);
    offset += n;

    uint16_t type = htons(1);
    memcpy(buf + offset, &type, 2);
    offset += 2;

    uint16_t class_ = htons(1);
    memcpy(buf + offset, &class_, 2);
    offset += 2;

    return offset;
};


int dns_send(const uint8_t *buf, int len){
    
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) return -1;

    struct sockaddr_in dest;
    dest.sin_family = AF_INET;
    dest.sin_port   = htons(53);
    inet_pton(AF_INET, "8.8.8.8", &dest.sin_addr);

    sendto(sock, buf, len, 0, (struct sockaddr*)&dest, sizeof(dest));

    return sock;
};



int dns_recv(int sock, uint8_t *reply, size_t reply_len){
    int bytes = recvfrom(sock, reply, reply_len, 0, NULL, NULL);
    return bytes;
}



int dns_parse_name(const uint8_t *buf, size_t buflen, int offset, char *out, size_t outlen){
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
int dns_parse_response(const uint8_t *buf, size_t buflen, char *out, size_t outlen){
    dns_header header;
    memcpy(&header, buf, sizeof(dns_header));

    int qdcount = ntohs(header.qdcount);
    int ancount = ntohs(header.ancount);
    int offset = sizeof(dns_header);
    char name[256];

    // Skip questions
    for(int i = 0; i<qdcount; i++){
        offset = dns_parse_name(buf, buflen, offset, name, sizeof(name));
        offset+=4;
    };

    // Parse the answer
    for(int i = 0; i<ancount; i++){
        offset = dns_parse_name(buf, buflen, offset, name, sizeof(name));

        uint16_t type, rdlen;
        memcpy(&type, buf + offset, 2);
        type = ntohs(type);

        memcpy(&rdlen, buf + offset + 8, 2); 
        rdlen = ntohs(rdlen);

        offset += 10;

        if(type == 1 && rdlen == 4){
            sprintf(out, "%d.%d.%d.%d", buf[offset], buf[offset+1], buf[offset+2], buf[offset+3]);
            return 1;
        };
        
        offset += rdlen;
    }

    return -1;
}


// Implement the caching mechanism