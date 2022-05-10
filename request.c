// File:    request.c
// Author:  Hannah Jean Perry <hperry1@student.unimelb.edu.au>
// Subject: COMP30023 Computer Systems
// Project: Serving the Web (project 2)
// Purpose: Contains the functions and type definitions relating to the
//          requests to the server

#include "utils.h"
#include "request.h"

// constants

#define SEPS " "  // field separator in request header

// function definitions

// process_request() takes a string request and returns a pointer to a request_t
// containing the information for the request
request_t* process_request(char* req_string) {
    request_t* request = malloc(sizeof(request_t));
    assert(request != NULL);
    request->method = INVALID;
    request->path[0] = '\0';

    // split the request string header into parts
    char req_copy[BUFFER_SIZE + 1];
    strncpy(req_copy, req_string, BUFFER_SIZE);
    char* next_tok = NULL;
    char* req_type = strtok_r(req_copy, SEPS, &next_tok);
    char* path = strtok_r(next_tok, SEPS, &next_tok);
    if (path != NULL) {
        strcpy(request->path, path);
    }
    char* version = strtok_r(next_tok, SEPS, &next_tok);
    printf("req: %s, path: %s, ver: %s\n", req_type, path, version);
    
    // check if the request method is a supported type
    if (strcmp(req_type, GET_STR) == 0) {
        // this is a GET request
        request->method = GET;
    }
    return request;
}

