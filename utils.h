// File:    utils.h
// Author:  Hannah Jean Perry <hperry1@student.unimelb.edu.au>
// Subject: COMP30023 Computer Systems
// Project: Serving the Web (project 2)
// Purpose: Header file for the utility functions of server

#ifndef UTILS_H
#define UTILS_H

// include headers for shared libraries
// #define _POSIX_C_SOURCE 200112L
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
// consider using sendfile() for efficiency (TODO: add required 2+ line comments here!)
// sendfile is more efficient because it copies the files in kernel, so avoids 
// having to read the file in to user-space and the write the file out
#include <sys/sendfile.h>

// constants

// can assume that GET requests will be no larger than 2kB in size
#define BUFFER_SIZE 2048

// request type
#define INVALID 0
#define GET 1
#define GET_STR "GET"

// type definitions

typedef struct request_t {
    int method;               // request method (e.g. GET)
    char path[BUFFER_SIZE];   // string path
} request_t;

// function prototypes
// see utils.c for documentation
int get_protocol(char*);
request_t* process_request(char*);


#endif