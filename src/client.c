
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char const* argv[]) {
    if (argc < 3) {
        printf("usage: %s <server_ip> <server_port>\n", argv[0]);
        return -1;
    }

    return 0;
}
