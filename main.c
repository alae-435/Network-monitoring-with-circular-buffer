#include <stdio.h>

#define BUFFER_SIZE 5

typedef struct {
    int data[BUFFER_SIZE];
    int head;   // next write position
    int count;  // number of stored items
} CircularBuffer;

void cb_init(CircularBuffer *cb) {
    cb->head = 0;
    cb->count = 0;
}

void cb_add(CircularBuffer *cb, int value) {
    cb->data[cb->head] = value;
    cb->head = (cb->head + 1) % BUFFER_SIZE;
    if (cb->count < BUFFER_SIZE) {
        cb->count++;
    }
}

// Copies items oldest to newest into out[], returns how many
int cb_get_all(const CircularBuffer *cb, int out[]) {
    int start = (cb->head - cb->count + BUFFER_SIZE) % BUFFER_SIZE;
    for (int i = 0; i < cb->count; i++) {
        out[i] = cb->data[(start + i) % BUFFER_SIZE];
    }
    return cb->count;
}

int main(void) {
    CircularBuffer buf;
    cb_init(&buf);

    int latencies[] = {12, 15, 11, 30, 14, 13, 55};
    int n = sizeof(latencies) / sizeof(latencies[0]);

    for (int i = 0; i < n; i++) {
        cb_add(&buf, latencies[i]);
    }

    int out[BUFFER_SIZE];
    int count = cb_get_all(&buf, out);

    int sum = 0;
    printf("Last %d readings:", count);
    for (int i = 0; i < count; i++) {
        printf(" %d", out[i]);
        sum += out[i];
    }
    printf("\nAverage: %.2f ms\n", (double)sum / count);

    return 0;
}