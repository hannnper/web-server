// File:    utils.h
// Author:  Hannah Jean Perry <hperry1@student.unimelb.edu.au>
// Subject: COMP30023 Computer Systems
// Project: Serving the Web (project 2)
// Purpose: Header file for the utility functions of server

#ifndef UTILS_H
#define UTILS_H

// include headers for shared libraries
// #define _POSIX_C_SOURCE 200809L
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
// using sendfile because it is more efficient as it copies the files within 
// the kernel space, so it avoids having to read the file in to user-space and 
// then write the file back out with multiple system calls and multiple lines
// of code
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/epoll.h>

// constants
#define FAIL -1
#define SUCCESS 0

// can assume that GET requests will be no larger than 2kB in size
#define BUFFER_SIZE 2048

// set maximum number of epoll events
#define MAX_EVENTS 128
// set maximum number of queued connections
#define BACKLOG 128

// end of line chars (CRLF)
#define CRLF "\r\n"

// function prototypes
// see utils.c for documentation
int get_protocol(char *);
int accept_connection(int, int);


#endif