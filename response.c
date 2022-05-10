// File:    response.c
// Author:  Hannah Jean Perry <hperry1@student.unimelb.edu.au>
// Subject: COMP30023 Computer Systems
// Project: Serving the Web (project 2)
// Purpose: Contains the functions and type definitions relating to the
//          response of the server


#include <errno.h>
#include "response.h"


// function definitions

// takes request_t `request` and returns an integer http status code:
// OK 200
// BAD_REQUEST 400
// FORBIDDEN 403
// NOT_FOUND 404
// TEAPOT 418
int get_status_code(request_t* request, char* server_path) {
    int ret;
    char* full_path;
    struct stat buf;
    // check if method is invalid
    if (request->method == INVALID) {
        return BAD_REQUEST;
    }
    else if (request->method == BREW) {
        return TEAPOT;
    }
    // check if filepath contains "../"
    if (strstr(request->path, "../") != NULL) {
        // return NOT_FOUND 404
        return NOT_FOUND;
    }
    // check file exists and permissions
    // this part may not be threadsafe!!! 
    // TODO: make threadsafe
    full_path = strcat(server_path, request->path);
    printf("full path: %s\n", full_path);
    ret = stat(full_path, &buf);
    if (ret != 0) {
        if (errno == EACCES) {
            // permission denied
            return FORBIDDEN;
        }
        return NOT_FOUND;
    }
    // otherwise all okay
    return OK;
}



