#include "circular_queue.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

static int tests = 0, failures = 0;
#define CHECK(cond) do { tests++; if (!(cond)) { failures++; \
    printf("  ECHEC ligne %d : %s\n", __LINE__, #cond); } } while (0)

static Packet mk(double ts, Protocol pr, uint32_t size) {
    Packet p; memset(&p, 0, sizeof p);
    p.timestamp = ts; p.protocol = pr; p.size = size;
    strcpy(p.src, "10.0.0.1"); strcpy(p.dst, "10.0.0.2");
    return p;
}

static void test_init_invalid(void) {
    puts("init invalide");
    CircularQueue q;
    CHECK(!cq_init(&q, 0, OVERFLOW_REJECT));
    CHECK(!cq_init(NULL, 4, OVERFLOW_REJECT));
}

static void test_empty(void) {
    puts("file vide");
    CircularQueue q; cq_init(&q, 4, OVERFLOW_REJECT);
    Packet out; CQStats s;
    CHECK(cq_is_empty(&q) && !cq_is_full(&q));
    CHECK(!cq_dequeue(&q, &out));
    CHECK(!cq_peek(&q, &out));
    CHECK(cq_snapshot(&q, &out, 1) == 0);
    cq_stats(&q, &s);
    CHECK(s.count == 0 && s.bytes_retained == 0 && s.avg_size == 0.0 && s.occupancy == 0.0);
    cq_destroy(&q);
}

static void test_fifo_basic(void) {
    puts("ordre FIFO, peek");
    CircularQueue q; cq_init(&q, 4, OVERFLOW_REJECT);
    for (int i = 0; i < 3; i++) { Packet p = mk(i, PROTO_TCP, 100); CHECK(cq_enqueue(&q, &p) == CQ_OK); }
    Packet out;
    CHECK(cq_peek(&q, &out) && out.timestamp == 0);
    CHECK(q.count == 3);                       /* peek ne retire rien */
    for (int i = 0; i < 3; i++) { CHECK(cq_dequeue(&q, &out) && out.timestamp == i); }
    CHECK(cq_is_empty(&q));
    cq_destroy(&q);
}

static void test_full_reject(void) {
    puts("file pleine, politique REJECT");
    CircularQueue q; cq_init(&q, 3, OVERFLOW_REJECT);
    for (int i = 0; i < 3; i++) { Packet p = mk(i, PROTO_UDP, 10); cq_enqueue(&q, &p); }
    CHECK(cq_is_full(&q));
    Packet extra = mk(99, PROTO_UDP, 10);
    CHECK(cq_enqueue(&q, &extra) == CQ_REJECTED);
    CHECK(q.rejected == 1 && q.overwritten == 0 && q.count == 3);
    Packet out; cq_peek(&q, &out);
    CHECK(out.timestamp == 0);
    cq_destroy(&q);
}

static void test_full_overwrite(void) {
    puts("file pleine, politique OVERWRITE");
    CircularQueue q; cq_init(&q, 3, OVERFLOW_OVERWRITE);
    for (int i = 0; i < 3; i++) { Packet p = mk(i, PROTO_TCP, 100); cq_enqueue(&q, &p); }
    Packet p3 = mk(3, PROTO_UDP, 50);
    CHECK(cq_enqueue(&q, &p3) == CQ_OVERWROTE);
    CHECK(q.overwritten == 1 && q.count == 3);
    Packet snap[3]; CHECK(cq_snapshot(&q, snap, 3) == 3);
    CHECK(snap[0].timestamp == 1 && snap[1].timestamp == 2 && snap[2].timestamp == 3);
    CQStats s; cq_stats(&q, &s);
    CHECK(s.bytes_retained == 250);
    CHECK(s.per_protocol[PROTO_TCP] == 2 && s.per_protocol[PROTO_UDP] == 1);
    cq_destroy(&q);
}

static void test_capacity_one(void) {
    puts("capacité 1");
    CircularQueue q; cq_init(&q, 1, OVERFLOW_REJECT);
    Packet a = mk(1, PROTO_TCP, 10), b = mk(2, PROTO_UDP, 20), out;
    CHECK(cq_enqueue(&q, &a) == CQ_OK);
    CHECK(cq_is_full(&q));
    CHECK(cq_enqueue(&q, &b) == CQ_REJECTED);
    CHECK(cq_dequeue(&q, &out) && out.timestamp == 1);
    CHECK(cq_is_empty(&q));
    CHECK(cq_enqueue(&q, &b) == CQ_OK);
    cq_destroy(&q);

    cq_init(&q, 1, OVERFLOW_OVERWRITE);
    cq_enqueue(&q, &a);
    CHECK(cq_enqueue(&q, &b) == CQ_OVERWROTE);
    CHECK(cq_peek(&q, &out) && out.timestamp == 2);
    CHECK(q.bytes_retained == 20 && q.proto_count[PROTO_TCP] == 0 && q.proto_count[PROTO_UDP] == 1);
    cq_destroy(&q);
}

static void test_wraparound(void) {
    puts("retour au début du tableau");
    CircularQueue q; cq_init(&q, 4, OVERFLOW_REJECT);
    Packet out;
    for (int i = 0; i < 4; i++) { Packet p = mk(i, PROTO_TCP, 10); cq_enqueue(&q, &p); }
    cq_dequeue(&q, &out); cq_dequeue(&q, &out);          /* head = 2 */
    for (int i = 4; i < 6; i++) { Packet p = mk(i, PROTO_ICMP, 20); cq_enqueue(&q, &p); }
    CHECK(q.tail == 2 && q.head == 2 && cq_is_full(&q)); /* tail a bouclé */
    Packet snap[4]; CHECK(cq_snapshot(&q, snap, 4) == 4);
    for (int i = 0; i < 4; i++) CHECK(snap[i].timestamp == i + 2);
    /* nombreux tours complets */
    for (int i = 6; i < 1000; i++) {
        cq_dequeue(&q, &out); CHECK(out.timestamp == i - 4);
        Packet p = mk(i, PROTO_UDP, 5); CHECK(cq_enqueue(&q, &p) == CQ_OK);
    }
    cq_destroy(&q);
}

static void test_wraparound_overwrite(void) {
    puts("retour au début avec écrasements");
    CircularQueue q; cq_init(&q, 5, OVERFLOW_OVERWRITE);
    for (int i = 0; i < 23; i++) { Packet p = mk(i, (Protocol)(i % 3), 10 + i); cq_enqueue(&q, &p); }
    CHECK(q.overwritten == 18 && q.count == 5);
    Packet snap[5]; cq_snapshot(&q, snap, 5);
    uint64_t bytes = 0;
    for (int i = 0; i < 5; i++) { CHECK(snap[i].timestamp == 18 + i); bytes += snap[i].size; }
    CHECK(q.bytes_retained == bytes);
    size_t total = 0; for (int k = 0; k < PROTO_COUNT; k++) total += q.proto_count[k];
    CHECK(total == 5);
    cq_destroy(&q);
}

static void test_stats(void) {
    puts("statistiques");
    CircularQueue q; cq_init(&q, 10, OVERFLOW_REJECT);
    Packet a = mk(0, PROTO_TCP, 100), b = mk(1, PROTO_UDP, 200), c = mk(2, PROTO_TCP, 300), d = mk(3, PROTO_OTHER, 400);
    cq_enqueue(&q, &a); cq_enqueue(&q, &b); cq_enqueue(&q, &c); cq_enqueue(&q, &d);
    CQStats s; cq_stats(&q, &s);
    CHECK(s.count == 4 && s.capacity == 10);
    CHECK(fabs(s.occupancy - 0.4) < 1e-9);
    CHECK(s.bytes_retained == 1000 && fabs(s.avg_size - 250.0) < 1e-9);
    CHECK(s.per_protocol[PROTO_TCP] == 2 && s.per_protocol[PROTO_UDP] == 1 &&
          s.per_protocol[PROTO_ICMP] == 0 && s.per_protocol[PROTO_OTHER] == 1);
    Packet out; cq_dequeue(&q, &out);                    /* retire a (TCP, 100) */
    cq_stats(&q, &s);
    CHECK(s.bytes_retained == 900 && s.per_protocol[PROTO_TCP] == 1);
    CHECK(strcmp(cq_protocol_name(PROTO_ICMP), "ICMP") == 0);
    cq_destroy(&q);
}

static void test_snapshot_independent(void) {
    puts("snapshot : copie indépendante et limite max");
    CircularQueue q; cq_init(&q, 4, OVERFLOW_REJECT);
    for (int i = 0; i < 4; i++) { Packet p = mk(i, PROTO_TCP, 10 * (4 - i)); cq_enqueue(&q, &p); }
    Packet snap[4];
    CHECK(cq_snapshot(&q, snap, 2) == 2);
    CHECK(cq_snapshot(&q, snap, 4) == 4);
    snap[0].size = 9999; snap[0].timestamp = -1;          /* l'UI modifie / trie sa copie */
    Packet front; cq_peek(&q, &front);
    CHECK(front.timestamp == 0 && front.size == 40);
    cq_destroy(&q);
}

static void test_clear_and_errors(void) {
    puts("clear et paramètres invalides");
    CircularQueue q; cq_init(&q, 3, OVERFLOW_OVERWRITE);
    for (int i = 0; i < 5; i++) { Packet p = mk(i, PROTO_TCP, 10); cq_enqueue(&q, &p); }
    cq_clear(&q);
    CQStats s; cq_stats(&q, &s);
    CHECK(cq_is_empty(&q) && s.bytes_retained == 0 && s.per_protocol[PROTO_TCP] == 0);
    CHECK(q.overwritten == 0 && s.capacity == 3);
    Packet p = mk(1, PROTO_TCP, 10);
    CHECK(cq_enqueue(&q, &p) == CQ_OK);                   /* réutilisable */
    Packet bad = mk(1, (Protocol)42, 10);
    CHECK(cq_enqueue(&q, &bad) == CQ_ERROR);
    CHECK(cq_enqueue(NULL, &p) == CQ_ERROR);
    cq_destroy(&q);
}

int main(void) {
    test_init_invalid(); test_empty(); test_fifo_basic(); test_full_reject();
    test_full_overwrite(); test_capacity_one(); test_wraparound();
    test_wraparound_overwrite(); test_stats(); test_snapshot_independent();
    test_clear_and_errors();
    printf("\n%d vérifications, %d échec(s)\n", tests, failures);
    return failures ? 1 : 0;
}