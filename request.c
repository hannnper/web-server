// File:    request.c
// Author:  Hannah Jean Perry <hperry1@student.unimelb.edu.au>
// Subject: COMP30023 Computer Systems
// Project: Serving the Web (project 2)
// Purpose: Contains the functions and type definitions relating to the
//          requests to the server

#include "utils.h"
#include "request.h"

// function definitions

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

