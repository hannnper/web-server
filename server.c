// File:    server.c
// Author:  Hannah Jean Perry <hperry1@student.unimelb.edu.au>
// Subject: COMP30023 Computer Systems
// Project: Serving the Web (project 2)
// Purpose: Contains the main program of server, which is a basic
//          multithreaded HTTP server that responds to a limited set of 
//          GET requests


#define _POSIX_C_SOURCE 200112L
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char** argv) {
    int protocol;
    char* port;
    char* path;

    // print usage information if incorrect number command line arguments given
    if (argc != 4) {
        fprintf(stderr, "./server usage:\n\t./server <protocol> <port> <path>"\
                        "\n\nprotocol: number, 4 (for IPv4) or 6 (for IPv6)\n"\
                        "port: port number\n"\
                        "path: string path to root web directory\n\n ");
        exit(EXIT_FAILURE);
    }

    // get protocol
    if (strcmp(argv[1], "4") == 0) {
        // using IPv4: AF_INET
        protocol = AF_INET;
    }
    else if (strcmp(argv[1], "6") == 0) {
        // using IPv6: AF_INET6
        protocol = AF_INET6;
    }
    else {
        // unrecognised protocol given
        fprintf(stderr, "Error: unrecognised protocol (%s)", argv[1]);
        exit(EXIT_FAILURE);
    }

    // get port and path
    port = argv[2];
    path = argv[3];

    printf("%d %s %s",protocol, port, path);
}