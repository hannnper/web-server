// File:    utils.c
// Author:  Hannah Jean Perry <hperry1@student.unimelb.edu.au>
// Subject: COMP30023 Computer Systems
// Project: Serving the Web (project 2)
// Purpose: Contains the utility functions of server

#include "utils.h"


// function definitions

// `get_protocol()` takes a string of "4" or "6" and returns the appropriate 
// protocol AF_INET/AF_INET6, or exits with failure if non-recognised protocol 
int get_protocol(char* protocol_string) {
    int protocol;
    if (strcmp(protocol_string, "4") == 0) {
        // using IPv4: AF_INET
        protocol = AF_INET;
    }
    else if (strcmp(protocol_string, "6") == 0) {
        // using IPv6: AF_INET6
        protocol = AF_INET6;
    }
    else {
        // unrecognised protocol given
        fprintf(stderr, "Error: unrecognised protocol (%s)\n", protocol_string);
        exit(EXIT_FAILURE);
    }
    return protocol;
}


// accept_connection() accepts a new connection and adds it to the epoll, 
// returns the new socket file descriptor or FAIL (-1) if an error occurred
int accept_connection(int sockfd, int epollfd) {
    int newsockfd;
    struct sockaddr_storage client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    struct epoll_event event;

    // accept the connection
    newsockfd = accept(sockfd, &client_addr, &client_addr_size);
    if (newsockfd < 0) {
        // error occurred in attempt to accept
        perror("accept");
        return FAIL;
    }

    // add this new file descriptor to the epoll
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, newsockfd, &event) < 0) {
        // error occurred
        perror("epoll_ctl");
        return FAIL;
    }
    return newsockfd;
}

