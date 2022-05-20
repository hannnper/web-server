// File:    request.h
// Author:  Hannah Jean Perry <hperry1@student.unimelb.edu.au>
// Subject: COMP30023 Computer Systems
// Project: Serving the Web (project 2)
// Purpose: Header file for the functions and type definitions relating to the
//          requests to the server

#ifndef REQUEST_H
#define REQUEST_H

#include "utils.h"

// constants
#define MAX_VER_LEN 20
// request method types
#define INVALID 0
#define GET 1
#define BREW 2
#define GET_STR "GET"
#define BREW_STR "BREW"


// type definitions

typedef struct message_t {
    bool ready;                    // indicates if it is ready to be processed
    int fd;                        // source file desc
    int n_read;                    // how many chars have been read in
    char buffer[BUFFER_SIZE + 1];  // buffer for storing partial message
    struct message_t *next;        // pointer to next message
} message_t;

typedef struct request_t {
    int method;                      // request method (e.g. GET)
    char path[BUFFER_SIZE + 1];      // string path to requested file
                                     //     from web root
    char http_ver[MAX_VER_LEN + 1];  // http version
} request_t;


// function prototypes
// see request.c for documentation
message_t* add_message(int, message_t *);
message_t* delete_message(int, message_t *);
message_t* find_message(int, message_t *);
void update_message_status(message_t *);
request_t* process_request(char *);
char* get_full_path(char *, request_t *);
message_t *close_connection(int, int, message_t *);

#endif
