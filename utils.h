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
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
// consider using sendfile() for efficiency (TODO: add required 2+ line comments here!)
// sendfile is more efficient because it copies the files in kernel, so avoids 
// having to read the file in to user-space and the write the file out
#include <sys/sendfile.h>
#include <sys/stat.h>

// constants

// can assume that GET requests will be no larger than 2kB in size
#define BUFFER_SIZE 2048


// function prototypes
// see utils.c for documentation
int get_protocol(char*);

#endif