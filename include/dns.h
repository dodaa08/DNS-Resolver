#include <stdint.h>
#include <string.h>

// header struct
typedef struct {
    uint16_t id;
    uint16_t flags;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
} dns_header;

// questions struct
typedef struct {
    char name[256];
    uint16_t type;
    uint16_t class_;
} dns_question;

// Answer struct
typedef struct {
    char name[256];
    uint16_t type;
    uint16_t class_;
    uint32_t ttl;
    uint16_t rdlength;
    uint8_t  rdata[256];
} dns_answer;


// Complete reply struct
typedef struct {
  dns_header header;
  dns_question question[10];
  dns_answer answer[10];
} dns_message;


// Declaration for all the helpers

// Send the raw DNS packet to the given server. Returns the open socket fd.
int dns_send(const uint8_t *buf, int len, const char *server_ip, uint16_t port);

// Receive Raw bytes as the answer containing the response, resolved ip other details etc...
int dns_recv(int sock, uint8_t *reply, size_t reply_len);

// Build the query, send to dns_server:port, receive the response and parse the IP into out_ip.
// Returns 1 on success, -1 on failure.
int dns_query(const char *domain, const char *dns_server, uint16_t port, char *out_ip);

// Encode the domain name to wire format to be send via the udp
int dns_encode_name(const char *name,uint8_t *out);

// Parse the name decode the name back to humar readable form
int dns_parse_name(const uint8_t *buf, size_t buflen, int offset, char *out);

// Parse the entire response, extract the ip and the answer details and print them
int dns_parse_response(const uint8_t *buf, size_t buflen, char *out, uint32_t *out_ttl);

// caching the resolved ip with a ttl so everytime a user sends a query if a cache hits the ip will be picked and sent directly from here avoiding the requests
