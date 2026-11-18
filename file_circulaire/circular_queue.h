/* circular_queue.h - File circulaire de paquets à capacité fixe (FIFO) */
#ifndef CIRCULAR_QUEUE_H
#define CIRCULAR_QUEUE_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define CQ_IP_LEN 16 /* "255.255.255.255" + fin de chaîne */

typedef enum { PROTO_TCP = 0, PROTO_UDP, PROTO_ICMP, PROTO_OTHER, PROTO_COUNT } Protocol;

typedef struct {
    double   timestamp;        /* secondes depuis le début de la capture */
    char     src[CQ_IP_LEN];
    char     dst[CQ_IP_LEN];
    Protocol protocol;
    uint32_t size;             /* octets */
} Packet;

typedef enum {
    OVERFLOW_REJECT = 0,       /* file pleine : le nouveau paquet est écarté */
    OVERFLOW_OVERWRITE         /* file pleine : le plus ancien est écrasé    */
} OverflowPolicy;

typedef enum { CQ_OK = 0, CQ_OVERWROTE, CQ_REJECTED, CQ_ERROR } CQStatus;

typedef struct {
    Packet        *data;
    size_t         capacity;
    size_t         head;       /* indice du plus ancien paquet (prochain dequeue) */
    size_t         tail;       /* indice de la prochaine écriture                 */
    size_t         count;
    OverflowPolicy policy;
    uint64_t       bytes_retained;            /* maintenu incrémentalement */
    size_t         proto_count[PROTO_COUNT];  /* maintenu incrémentalement */
    uint64_t       overwritten;               /* paquets écrasés  */
    uint64_t       rejected;                  /* paquets refusés  */
} CircularQueue;

typedef struct {
    size_t   count;
    size_t   capacity;
    double   occupancy;        /* count / capacity, entre 0 et 1 */
    uint64_t bytes_retained;
    double   avg_size;         /* 0 si la file est vide */
    size_t   per_protocol[PROTO_COUNT];
} CQStats;

/* Cycle de vie */
bool     cq_init(CircularQueue *q, size_t capacity, OverflowPolicy policy);
void     cq_destroy(CircularQueue *q);
void     cq_clear(CircularQueue *q);            /* vide la file, garde la capacité ; remet les compteurs d'écrasement/rejet à 0 */

/* Opérations de base */
CQStatus cq_enqueue(CircularQueue *q, const Packet *p);
bool     cq_dequeue(CircularQueue *q, Packet *out);   /* out peut être NULL */
bool     cq_peek(const CircularQueue *q, Packet *out);

/* État */
bool     cq_is_empty(const CircularQueue *q);
bool     cq_is_full(const CircularQueue *q);

/* Copie des paquets dans l'ordre FIFO (du plus ancien au plus récent).
 * Retourne le nombre de paquets copiés (<= max). L'interface peut trier la copie. */
size_t   cq_snapshot(const CircularQueue *q, Packet *out, size_t max);

/* Statistiques */
void     cq_stats(const CircularQueue *q, CQStats *s);
const char *cq_protocol_name(Protocol p);

#endif