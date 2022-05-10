// File:    response.c
// Author:  Hannah Jean Perry <hperry1@student.unimelb.edu.au>
// Subject: COMP30023 Computer Systems
// Project: Serving the Web (project 2)
// Purpose: Contains the functions and type definitions relating to the
//          response of the server

#include "response.h"

// function definitions

// takes request_t `request` and returns an integer http status code:
// OK 200
// BAD_REQUEST 400
// FORBIDDEN 403
// NOT_FOUND 404
// TEAPOT 418
int get_status_code(request_t* request) {
    // check if method is invalid
    if (request->method == INVALID) {
        return BAD_REQUEST;
    }
    // check file 
}



