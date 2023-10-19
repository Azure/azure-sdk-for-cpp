
#ifndef TEST_SOCKET_H
#define TEST_SOCKET_H

// This file enables testing of these Linux-oriented unit tests under Windows. It is not
// strictly necessary, but is convenient to have.

#ifdef __cplusplus
extern "C" {
#include <cstdint>
#include <cstddef>
#endif

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define    AF_INET        2
#define    SOCK_STREAM    1
#define IPPROTO_TCP     6


#define htons(x) x


    struct in_addr {
        uint32_t       s_addr;     /* address in network byte order */
    };

    struct sockaddr_in {
        uint8_t         sin_family; /* address family: AF_INET */
        uint16_t        sin_port;   /* port in network byte order */
        struct in_addr  sin_addr;   /* internet address */
    };

    struct sockaddr {
        uint8_t         sin_family; /* address family: AF_INET */
        uint16_t        sin_port;   /* port in network byte order */
        struct in_addr  sin_addr;   /* internet address */
    };

    struct addrinfo {
        int              ai_flags;     // AI_PASSIVE, AI_CANONNAME, etc.
        int              ai_family;    // AF_INET, AF_INET6, AF_UNSPEC
        int              ai_socktype;  // SOCK_STREAM, SOCK_DGRAM
        int              ai_protocol;  // use 0 for "any"
        size_t           ai_addrlen;   // size of ai_addr in bytes
        struct sockaddr *ai_addr;      // struct sockaddr_in or _in6
        char            *ai_canonname; // full canonical hostname

        struct addrinfo *ai_next;      // linked list, next node
    };

    int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res);

    void freeaddrinfo(struct addrinfo* ai);

#ifdef __cplusplus
}
#endif

#endif /* TEST_SOCKET_H */
