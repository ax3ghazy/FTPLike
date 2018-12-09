#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BACKLOG 5
#define CHUNKSIZE 1000

#define handle_error(msg) \
    do { fprintf(stderr, msg); exit(EXIT_FAILURE); } while (0)


void write_file_to_socket(int sockfd){

    unsigned char buffer_size[4], buffer_chunk[CHUNKSIZE];
    char filename[100];

    //very unsafe:
    unsigned int file_size = 0;
    unsigned int filename_length = 0;
    ssize_t read_size = 0;
    int total_size = 0;
    int j = 0;

    //write filename length
    read_size = read(sockfd, buffer_chunk, 1);  //filename length
    filename_length = buffer_chunk[0];

    //read filename
    read_size = read(sockfd, buffer_chunk, filename_length);
    memcpy(filename, buffer_chunk, read_size);
    filename[read_size] = '\0';

    //remove(filename);

    // write the file

    FILE *f;
    f=fopen(filename,"r");

    fseek(f, 0L, SEEK_END);
    int sz = ftell(f);
    rewind(f);
    fprintf(stderr, "File of size %d opened for writing to socket.\n", sz);

    char size [4];
    int temp = sz;

    for (int i = 0; i < 4; i++){
        char x = temp&0xFF;
        temp = temp >> 8;
        size[i] = x;
    }

    write(sockfd, size, 4);

    unsigned char buffer[CHUNKSIZE];
    int readData = sz;
    do {
        size_t read_bytes_file = fread((void *)buffer, 1, (size_t)CHUNKSIZE, f);
        fprintf(stderr, "Read %ld from file\n", read_bytes_file);
        //int x;
        //scanf("%d",&x);
        ssize_t write_bytes_socket = write(sockfd, buffer, read_bytes_file);
        fprintf(stderr, "Writing %ld bytes\n", write_bytes_socket);
        readData -= write_bytes_socket;
    } while (readData > 0);

    fclose(f);
}

void read_socket_to_file (int sockfd) {
    unsigned char buffer_size[4], buffer_chunk[CHUNKSIZE];
    char filename[100];
    //very unsafe:
    FILE *wfile;
    unsigned int file_size = 0;
    unsigned int filename_length = 0;
    ssize_t read_size = 0;
    int total_size = 0;
    int j = 0;

    //read filename length
    read_size = read(sockfd, buffer_chunk, 1);  //filename length
    filename_length = buffer_chunk[0];

    //read filename
    read_size = read(sockfd, buffer_chunk, filename_length);
    memcpy(filename, buffer_chunk, read_size);
    filename[read_size] = '\0';

    remove(filename);
     wfile = fopen(filename, "a");

    //read file size
    if (read(sockfd, &buffer_size, 4) != 4)
        handle_error("sock.server.buffer_size.read.error");

    for (int i = 0; i < 4; i++) {
        file_size |= (unsigned char)buffer_size[i] << (i*8);
    }


    fprintf(stderr, "Read filename of %ld bytes\n", read_size);

    fprintf(stderr, "Trying to read %d bytes from the socket...\n", file_size);

    for (int i = 0; i < file_size; i += read_size){
        read_size = read(sockfd, buffer_chunk, CHUNKSIZE);
        total_size += read_size;
        fprintf(stderr, "Reading chunk %d... of %ld bytes\n", j++, read_size);
        if (read_size == -1) {
            handle_error("sock.server.buffer_chunk.read.error");
        }
        size_t write_bytes_file = fwrite(buffer_chunk, 1, read_size, wfile);
        fprintf(stderr, "Wrote %ld to file\n", write_bytes_file);
    }
    fclose(wfile);
}

int bindSocket (int sockfd, const char *ip, int port) {
    struct sockaddr_in my_addr;
    socklen_t peer_addr_size;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    inet_aton(ip, &my_addr.sin_addr);

    return bind(sockfd, (struct sockaddr*)&my_addr, sizeof(my_addr)) != -1;
}

int main(int argc, char const* argv[]) {
#if 0 //make this 1 to disable debugging messages
    freopen("/dev/null", "w", stderr);
#endif
    if (argc < 2) {
        printf("usage: %s <listening_port>\n", argv[0]);
        handle_error("usage.error");
    }


    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        handle_error("sock.error");
    }

    if (!bindSocket(sockfd, "0.0.0.0", atoi(argv[1]))){
        handle_error("bind.error");
    }
    listen(sockfd, BACKLOG);

    while (1) {
        struct sockaddr peer_addr; socklen_t addrlen;
        char operation; char buffer_size[4];
        printf("Waiting for a connection...\n");
        int sockfd_server = accept(sockfd, &peer_addr, &addrlen);
        printf("Connection Received from %s:%d\n", inet_ntoa(((struct sockaddr_in *)&peer_addr)->sin_addr), ntohs(((struct sockaddr_in *)&peer_addr)->sin_port));
        if (sockfd_server == -1) {
            handle_error("sock.server.error");
        }

        unsigned char buffer_chunk[CHUNKSIZE];
        unsigned int filename_length = 0;
        ssize_t read_size = 0;
        read_size = read(sockfd_server, buffer_chunk, 1);  //filename length
        filename_length = buffer_chunk[0];

        if (!filename_length){
          write_file_to_socket(sockfd_server);
        }

        else{
          read_socket_to_file(sockfd_server);
        }

        close(sockfd_server);
    }

    close(sockfd);

    return 0;
}
