#include "circular_queue.h"
#include <stdlib.h>
#include <string.h>

static bool valid_protocol(Protocol p) { return (unsigned)p < (unsigned)PROTO_COUNT; }

static void remove_accounting(CircularQueue *q, const Packet *p) {
    q->bytes_retained -= p->size;
    q->proto_count[p->protocol]--;
}

bool cq_init(CircularQueue *q, size_t capacity, OverflowPolicy policy) {
    if (!q || capacity == 0) return false;
    memset(q, 0, sizeof *q);
    q->data = calloc(capacity, sizeof(Packet));
    if (!q->data) return false;
    q->capacity = capacity;
    q->policy = policy;
    return true;
}

void cq_destroy(CircularQueue *q) {
    if (!q) return;
    free(q->data);
    memset(q, 0, sizeof *q);
}

void cq_clear(CircularQueue *q) {
    if (!q) return;
    q->head = q->tail = q->count = 0;
    q->bytes_retained = 0;
    memset(q->proto_count, 0, sizeof q->proto_count);
    q->overwritten = q->rejected = 0;
}

bool cq_is_empty(const CircularQueue *q) { return q->count == 0; }
bool cq_is_full(const CircularQueue *q)  { return q->count == q->capacity; }

CQStatus cq_enqueue(CircularQueue *q, const Packet *p) {
    if (!q || !q->data || !p || !valid_protocol(p->protocol)) return CQ_ERROR;

    CQStatus status = CQ_OK;
    if (cq_is_full(q)) {
        if (q->policy == OVERFLOW_REJECT) {
            q->rejected++;
            return CQ_REJECTED;
        }
        /* OVERWRITE : evict the oldest packet */
        remove_accounting(q, &q->data[q->head]);
        q->head = (q->head + 1) % q->capacity;
        q->count--;
        q->overwritten++;
        status = CQ_OVERWROTE;
    }
    q->data[q->tail] = *p;
    q->tail = (q->tail + 1) % q->capacity;
    q->count++;
    q->bytes_retained += p->size;
    q->proto_count[p->protocol]++;
    return status;
}

bool cq_dequeue(CircularQueue *q, Packet *out) {
    if (!q || cq_is_empty(q)) return false;
    if (out) *out = q->data[q->head];
    remove_accounting(q, &q->data[q->head]);
    q->head = (q->head + 1) % q->capacity;
    q->count--;
    return true;
}

bool cq_peek(const CircularQueue *q, Packet *out) {
    if (!q || cq_is_empty(q) || !out) return false;
    *out = q->data[q->head];
    return true;
}

size_t cq_snapshot(const CircularQueue *q, Packet *out, size_t max) {
    if (!q || !out) return 0;
    size_t n = q->count < max ? q->count : max;
    for (size_t i = 0; i < n; i++)
        out[i] = q->data[(q->head + i) % q->capacity];
    return n;
}

void cq_stats(const CircularQueue *q, CQStats *s) {
    memset(s, 0, sizeof *s);
    s->count = q->count;
    s->capacity = q->capacity;
    s->occupancy = q->capacity ? (double)q->count / (double)q->capacity : 0.0;
    s->bytes_retained = q->bytes_retained;
    s->avg_size = q->count ? (double)q->bytes_retained / (double)q->count : 0.0;
    memcpy(s->per_protocol, q->proto_count, sizeof s->per_protocol);
}

const char *cq_protocol_name(Protocol p) {
    switch (p) {
        case PROTO_TCP:  return "TCP";
        case PROTO_UDP:  return "UDP";
        case PROTO_ICMP: return "ICMP";
        default:         return "OTHER";
    }
}