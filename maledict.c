#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include "curses.h"


// Convenience function to find the max of two values
#define MAX(a, b)   ((a) >= (b) ? (a) : (b))

// Store the curses we read into this database
char* CURSES[200];

int readdb() {
    memset(CURSES, 0, sizeof(CURSES));
    FILE* fp = fopen("curses.txt", "r");
    if (fp == NULL) {
        perror("Cannot read database");
        exit(2);
    }

    int numlines = 0;
    for (;;) {
        // Getline will malloc memory for the buffer
        char* line = NULL;
        size_t linecap = 0;
        int len;
        if ((len = getline((char **) &line, &linecap, fp)) == -1) {
            free(line);
            break;
        }
        CURSES[numlines] = line;
        if (CURSES[numlines][len - 1] == '\n') {
            CURSES[numlines][len - 1] = '\0';      // remove trailing newline
        }
        numlines += 1;
    }

    fclose(fp);

    return numlines;
}

// Send an error response
void send_error(int fd, Status status) {
    ResponseHeader response;
    response.protocol_id = htonl(CURSES_PROTOCOL_ID);
    response.status = htonl(status);
    response.length = 0;
    if (send(fd, &response, sizeof(response), 0) == -1) {
        perror("Send error failed");
        exit(3);
    }
}

void send_curses(int fd, int numcurses, int maxcurses) {
    // Form response data
    char buf[8192];
    buf[0] = '\0';
    for (int i = 0; i < numcurses; i++) {
        int curseno = random() % maxcurses;
        strcat(buf, CURSES[curseno]);
        if (i < numcurses - 1) {
            strcat(buf, "\n");
        }
    }

    // Send the response header and then send the data
    ResponseHeader response;
    response.protocol_id = htonl(CURSES_PROTOCOL_ID);
    response.status = htonl(OK);
    response.length = htonl(strlen(buf));
    if (send(fd, &response, sizeof(response), 0) == -1) {
        perror("Send header failed");
        exit(3);
    }

    if (send(fd, buf, strlen(buf), 0) == -1) {
        perror("Send failed");
        exit(3);
    }
}


int main(int argc, char** argv) {
    // Process arguments
    int ch;
    int seed = 1;
    while ((ch = getopt(argc, argv, "p:s:")) != -1) {
        switch (ch) {
        case 's':
            seed = atoi(optarg);
            break;

        default:
            fprintf(stderr, "Usage: maledict [-p port] [-s seed]\n");
            exit(1);
        }
    }

    // Initialize srandom
    srandom(seed);

    // Read curses into memory
    int maxcurses = readdb();

    // Create set of sockets to listen on
    short ports[] = { CURSES_PORT0, CURSES_PORT1, CURSES_PORT2, CURSES_PORT3 };
    fd_set readfds;
    FD_ZERO(&readfds);
    int maxfds = 0;
    int socks[4];
    for (int i = 0; i < 4; i++) {
        socks[i] = socket(AF_INET, SOCK_STREAM, 0);
        if (socks[i] == -1) {
            perror("Cannot create socket");
            exit(3);
        }
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(ports[i]);

        if (bind(socks[i], (struct sockaddr *) &addr, sizeof(addr)) == -1) {
            perror("Cannot bind socket");
            exit(3);
        }
        if (listen(socks[i], 0) == -1) {
            perror("Listen failed");
            exit(3);
        }

        // Capture this descriptor in readfds to use in select later
        FD_SET(socks[i], &readfds);
        maxfds = MAX(maxfds, socks[i]) + 1;
    }

    int nready = select(maxfds, &readfds, NULL, NULL, NULL);
    if (nready <= 0) {
        perror("Select error");
        exit(3);
    }

    int sock = -1;
    for (int i = 0; i < maxfds; i++) {
        if (FD_ISSET(i, &readfds)) {
            sock = i;
            break;
        }
    }
    if (sock == -1) {
        perror("Error in select return");
        exit(3);
    }

    // Accept the connection
    struct sockaddr_in cli_addr;
    socklen_t cli_len = sizeof(cli_addr);
    int fd = accept(sock, (struct sockaddr*) &cli_addr, &cli_len);
    if (fd == -1) {
        perror("Accept failed");
        exit(3);
    }

    // Read request
    RequestHeader req;
    if (read(fd, &req, sizeof(req)) != sizeof(req)) {
        fprintf(stderr, "Not enough data\n");
        send_error(fd, BAD_REQUEST);
        exit(4);
    }

    // Check protocol
    if (ntohl(req.protocol_id) != CURSES_PROTOCOL_ID) {
        fprintf(stderr, "Incorrect protocol id");
        send_error(fd, INVALID_PROTOCOL);
        exit(4);
    }

    // Process request
    int num_to_get = 0;
    switch (ntohl(req.op)) {
    case PING:
        break;

    case GET_SINGLE:
        num_to_get = 1;
        break;

    case GET_MULTI:
        // read argument of number of curses to get
        if (read(fd, &num_to_get, sizeof(num_to_get)) == -1) {
            perror("Could not read num");
            send_error(fd, INSUFFICIENT_ARGS);
            exit(4);
        }
        num_to_get = ntohl(num_to_get);
        printf("%d\n", num_to_get);
        break;

    default:
        send_error(fd, BAD_REQUEST);
        exit(4);
    }

    // send response back to client
    send_curses(fd, num_to_get, maxcurses);

    // Clean up
    for (int i = 0; i < maxcurses; i++) {
        free(CURSES[i]);
    }

    // Success, so exit 0
    exit(0);
}
