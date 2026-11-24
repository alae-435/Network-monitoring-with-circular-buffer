/* circular_queue.h - Fixed-capacity circular packet queue (FIFO) */
#ifndef CIRCULAR_QUEUE_H
#define CIRCULAR_QUEUE_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define CQ_IP_LEN 16 /* "255.255.255.255" + null terminator */

typedef enum { PROTO_TCP = 0, PROTO_UDP, PROTO_ICMP, PROTO_OTHER, PROTO_COUNT } Protocol;

typedef struct {
    double   timestamp;        /* seconds since the start of the capture */
    char     src[CQ_IP_LEN];
    char     dst[CQ_IP_LEN];
    Protocol protocol;
    uint32_t size;             /* bytes */
} Packet;

typedef enum {
    OVERFLOW_REJECT = 0,       /* queue full: the new packet is dropped        */
    OVERFLOW_OVERWRITE         /* queue full: the oldest packet is overwritten */
} OverflowPolicy;

typedef enum { CQ_OK = 0, CQ_OVERWROTE, CQ_REJECTED, CQ_ERROR } CQStatus;

typedef struct {
    Packet        *data;
    size_t         capacity;
    size_t         head;       /* index of the oldest packet (next dequeue) */
    size_t         tail;       /* index of the next write                   */
    size_t         count;
    OverflowPolicy policy;
    uint64_t       bytes_retained;            /* maintained incrementally */
    size_t         proto_count[PROTO_COUNT];  /* maintained incrementally */
    uint64_t       overwritten;               /* packets overwritten */
    uint64_t       rejected;                  /* packets rejected    */
} CircularQueue;

typedef struct {
    size_t   count;
    size_t   capacity;
    double   occupancy;        /* count / capacity, between 0 and 1 */
    uint64_t bytes_retained;
    double   avg_size;         /* 0 if the queue is empty */
    size_t   per_protocol[PROTO_COUNT];
} CQStats;

/* Lifecycle */
bool     cq_init(CircularQueue *q, size_t capacity, OverflowPolicy policy);
void     cq_destroy(CircularQueue *q);
void     cq_clear(CircularQueue *q);            /* empties the queue, keeps the capacity; resets the overwrite/reject counters to 0 */

/* Basic operations */
CQStatus cq_enqueue(CircularQueue *q, const Packet *p);
bool     cq_dequeue(CircularQueue *q, Packet *out);   /* out may be NULL */
bool     cq_peek(const CircularQueue *q, Packet *out);

/* State */
bool     cq_is_empty(const CircularQueue *q);
bool     cq_is_full(const CircularQueue *q);

/* Copies the packets in FIFO order (oldest to newest).
 * Returns the number of packets copied (<= max). The UI may sort the copy. */
size_t   cq_snapshot(const CircularQueue *q, Packet *out, size_t max);

/* Statistics */
void     cq_stats(const CircularQueue *q, CQStats *s);
const char *cq_protocol_name(Protocol p);

#endif
