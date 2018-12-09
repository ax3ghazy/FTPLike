#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define CHUNKSIZE 1000

#define handle_error(msg) \
    do { fprintf(stderr, msg); exit(EXIT_FAILURE); } while (0)

void read_file_from_socket(int sockfd, const char *read_filename, const char *write_filename){

	char read_filename_length = strlen(read_filename);
	size_t write_bytes_socket = write (sockfd, &read_filename_length, 1);
	write_bytes_socket = write(sockfd, read_filename, (unsigned int) read_filename_length);

	fprintf(stderr, "Reading %s of %ld (%ld) bytes\n", read_filename, strlen(read_filename), write_bytes_socket);

	unsigned char buffer_size[4], buffer_chunk[CHUNKSIZE];
        char filename[100];

       FILE *wfile;
       unsigned int file_size = 0;
       unsigned int filename_length = 0;
       ssize_t read_size = 0;
       int total_size = 0;
       int j = 0;


       remove(filename);
       wfile = fopen(write_filename, "a");

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






void write_file_to_socket(int sockfd, const char *read_filename, const char *write_filename){
    //write filename
    //easy exploit here:
    char write_filename_length = strlen(write_filename);
    size_t write_bytes_socket = write(sockfd, &write_filename_length, 1);
    write_bytes_socket = write(sockfd, write_filename, (unsigned int)write_filename_length);

    fprintf(stderr, "Writing %s of %ld (%ld) bytes\n", write_filename, strlen(write_filename), write_bytes_socket);


    FILE *f;
    f=fopen(read_filename,"r");

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

int main(int argc, char const* argv[]) {
#if 0 //make this 1 to disable debugging messages
    freopen("/dev/null", "w", stderr);
#endif
    if (argc < 3) {
        printf("usage: %s <server_ip> <server_port>\n", argv[0]);
        return -1;
    }

    int server_port = atoi(argv[2]);

    int sockfd;
    struct sockaddr_in server, client;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        handle_error("unable to create socket.\n");
    }

    bzero(&server, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_port = htons(server_port);

    if (connect(sockfd, (struct sockaddr *)& server, sizeof(server)) != 0){
        handle_error("unable to connect to server \n");
    }

    printf("Welcome to our bad version of FTP! \n");

    printf("For reading a file: press 0 \n");

    printf("For writing a file: press 1 \n");


    int answer;
    scanf("%d", &answer);
    unsigned int k = (unsigned int) answer;
    size_t write_bytes_socket = write(sockfd, &answer, 1);

    char r[80], w[80];


    switch (k){
	case 0:
	    read_file_from_socket(sockfd, "b.jpg", "x.jpg");
	    break;
	case 1:
	    write_file_to_socket(sockfd, "a.jpg", "b.jpg");
	    break;
	default:
	    printf("Sorry Boy! Not today\n");
	    break;

    }

    close(sockfd);

    return 0;
}
