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


// process_request() takes a string request and returns a pointer to a request_t
// containing the information for the request
request_t* process_request(char* req_string) {
    request_t* request = malloc(sizeof(request_t));
    assert(request != NULL);
    request->method = INVALID;
    request->path[0] = '\0';
    if (strncmp(req_string, GET_STR, 3) == 0) {
        // this is a GET request
        request->method = GET;
        strncpy(request->path, &req_string[strlen(GET_STR) + 1], BUFFER_SIZE);
    }
    return request;
}

