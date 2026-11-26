#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define CQ_IP_LEN 16

int main(void)
{   
    srand((unsigned)time(NULL));
    double timestamp = 0.0;
    char src[CQ_IP_LEN];
    char dst[CQ_IP_LEN];
    int choice = rand() % 4;
    const char *protocols[] = {"TCP", "UDP", "ICMP", "OTHER"};
    const char *protocol = protocols[choice];
    uint32_t size = 64 + rand() % (1500 - 64 + 1);
    FILE *fp;

fp = fopen("packets_5000.csv", "w");
 if (fp == NULL) {
        perror("packets_5000.csv");
        return EXIT_FAILURE;
    }
fprintf(fp, "timestamp,src,dst,protocol,size\n");
    for (int i=0;i<5000;i++){
        timestamp= timestamp+(double)rand() / RAND_MAX;
        snprintf(src, sizeof(src), "%d.%d.%d.%d",
             rand() % 256,
             rand() % 256,
             rand() % 256,
             rand() % 256);
        snprintf(dst, sizeof(dst), "%d.%d.%d.%d",
             rand() % 256,
             rand() % 256,
             rand() % 256,
             rand() % 256);
        choice = rand() % 4;
        protocol = protocols[choice];
        size = 64 + rand() % (1500 - 64 + 1);
        fprintf(fp, "%.6f,%s,%s,%s,%u\n",
        timestamp, src, dst, protocol, size);
    }
fclose(fp);
timestamp = 0.0;
fp = fopen("packets_20000.csv", "w");
 if (fp == NULL) {
        perror("packets_20000.csv");
        return EXIT_FAILURE;
    }
fprintf(fp, "timestamp,src,dst,protocol,size\n");
    for (int i=0;i<20000;i++){
        timestamp= timestamp+(double)rand() / RAND_MAX;
        snprintf(src, sizeof(src), "%d.%d.%d.%d",
             rand() % 256,
             rand() % 256,
             rand() % 256,
             rand() % 256);
        snprintf(dst, sizeof(dst), "%d.%d.%d.%d",
             rand() % 256,
             rand() % 256,
             rand() % 256,
             rand() % 256);
        choice = rand() % 4;
        protocol = protocols[choice];
        size = 64 + rand() % (1500 - 64 + 1);
        fprintf(fp, "%.6f,%s,%s,%s,%u\n",
        timestamp, src, dst, protocol, size);
    }
fclose(fp);
timestamp = 0.0;
fp = fopen("packets_100000.csv", "w");
 if (fp == NULL) {
        perror("packets_100000.csv");
        return EXIT_FAILURE;
    }

fprintf(fp, "timestamp,src,dst,protocol,size\n");
    for (int i=0;i<100000;i++){
        timestamp= timestamp+(double)rand() / RAND_MAX;
        snprintf(src, sizeof(src), "%d.%d.%d.%d",
             rand() % 256,
             rand() % 256,
             rand() % 256,
             rand() % 256);
        snprintf(dst, sizeof(dst), "%d.%d.%d.%d",
             rand() % 256,
             rand() % 256,
             rand() % 256,
             rand() % 256);
        choice = rand() % 4;
        protocol = protocols[choice];
        size = 64 + rand() % (1500 - 64 + 1);
        fprintf(fp, "%.6f,%s,%s,%s,%u\n",
        timestamp, src, dst, protocol, size);
    }
    fclose(fp);
    return EXIT_SUCCESS;
}
