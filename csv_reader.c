#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    FILE *fp;
    double timestamp;
char src[16];
char dst[16];
char protocol[10];
unsigned int size;



    fp = fopen("packets_5000.csv", "r");

    if (fp == NULL) {
        perror("packets_5000.csv");
        return EXIT_FAILURE;
    }

    char line[256];

    while (fgets(line, sizeof(line), fp) != NULL) {
        sscanf(line, "%lf,%15[^,],%15[^,],%9[^,],%u",
       &timestamp, src, dst, protocol, &size);
        printf("Timestamp: %f\n", timestamp);
        printf("Source: %s\n", src);
        printf("Destination: %s\n", dst);
        printf("Protocol: %s\n", protocol);
        printf("Size: %u\n", size);
    }
Protocol proto;

if (strcmp(protocol, "TCP") == 0) {
    proto = PROTO_TCP;
}
else if (strcmp(protocol, "UDP") == 0) {
    proto = PROTO_UDP;
}
else if (strcmp(protocol, "ICMP") == 0) {
    proto = PROTO_ICMP;
}
else if (strcmp(protocol, "OTHER") == 0) {
    proto = PROTO_OTHER;
}
    fclose(fp);

    return EXIT_SUCCESS;
}