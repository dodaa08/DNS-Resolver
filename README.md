# DNS Resolver

A DNS client written in C from scratch. No libraries, just raw UDP sockets, manually constructed DNS packets, and a hand-rolled in-memory cache with TTL support.

---

## How it works

<img width="1056" height="729" alt="Screenshot From 2026-04-22 16-32-20" src="https://github.com/user-attachments/assets/a6854816-dc87-44a9-9575-f9c64cb9dd89" />


---

## Demo



---

## What it does

You give it a domain name. It builds a DNS query packet in the DNS wire format, sends it over UDP to a nameserver of your choice, parses the binary response, and hands you back an IP address.

On repeat queries for the same domain within the TTL window, it returns the result from cache without hitting the network.

---

## Core Features

* **Raw DNS over UDP**: No libc resolver. Packets built byte by byte.
* **Configurable nameserver**: Point it at any DNS server (8.8.8.8, 1.1.1.1, 9.9.9.9, etc.)
* **In-memory cache with TTL**: Respects the TTL from the DNS response. Expired entries are evicted automatically.
* **Error handling**: Socket timeouts, RCODE errors (NXDOMAIN, SERVFAIL, etc.), bad server IPs, all handled with clear messages.

---

## How it works

```
dns_query("google.com", "8.8.8.8", 53)
    │
    ├── cache_get()  → hit? return IP immediately
    │
    ├── Build DNS packet
    │     └── Header (ID, flags, question count)
    │     └── Question (encoded domain, type A, class IN)
    │
    ├── dns_send()   → UDP socket → sendto(8.8.8.8:53)
    │
    ├── dns_recv()   → recvfrom() with 5s timeout
    │
    ├── dns_parse_response()
    │     └── Check RCODE (0 = ok, 3 = NXDOMAIN, ...)
    │     └── Skip question section
    │     └── Walk answer records → find type A → extract IPv4 + TTL
    │
    └── cache_set()  → store (domain, ip, now + TTL)
```

---

## Build & Run

```bash
make
./dns-resolver <domain> [dns-server] [port]
```

```bash
./dns-resolver google.com               # uses 8.8.8.8:53 by default
./dns-resolver google.com 1.1.1.1       # Cloudflare
./dns-resolver google.com 9.9.9.9 53    # Quad9
```

## Run Tests

```bash
make test
```

Tests cover cache hit/miss, TTL expiry logic, error paths (invalid IP, NXDOMAIN), and end-to-end resolution.
