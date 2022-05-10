// File:    request.h
// Author:  Hannah Jean Perry <hperry1@student.unimelb.edu.au>
// Subject: COMP30023 Computer Systems
// Project: Serving the Web (project 2)
// Purpose: Header file for the functions and type definitions relating to the
//          requests to the server

#ifndef REQUEST_H
#define REQUEST_H

// constants
// request method types
#define INVALID 0
#define GET 1
#define BREW 2
#define GET_STR "GET"
#define BREW_STR "BREW"
// chars that end the request header line (CRLF)
#define LINE_END "\r\n"

// type definitions

typedef struct request_t {
    int method;                   // request method (e.g. GET)
    char path[BUFFER_SIZE + 1];   // string path
} request_t;


// function prototypes
// see request.c for documentation
request_t* process_request(char*);

#endif