// File:    server.c
// Author:  Hannah Jean Perry <hperry1@student.unimelb.edu.au>
// Subject: COMP30023 Computer Systems
// Project: Serving the Web (project 2)
// Purpose: Contains the main program of server, which is a basic
//          multithreaded HTTP server that responds to a limited set of 
//          GET requests


#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {

    // print usage information if incorrect number command line arguments given
    if (argc != 3) {
        fprintf(stderr, "./server usage:\n\t./server <protocol> <port> <path>"\
                        "\n\nprotocol: number, 4 (for IPv4) or 6 (for IPv6)\n"\
                        "port: port number\n"\
                        "path: string path to root web directory\n\n ");
        exit(EXIT_FAILURE);
    }

    int protocol = argv[1];
    int port = argv[2];
    char* path = argv[3];

    printf("%d %d %s",protocol, port, path);
}