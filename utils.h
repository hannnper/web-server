// File:    utils.h
// Author:  Hannah Jean Perry <hperry1@student.unimelb.edu.au>
// Subject: COMP30023 Computer Systems
// Project: Serving the Web (project 2)
// Purpose: Header file for the utility functions of server

#ifndef UTILS_H
#define UTILS_H

// include headers for shared libraries
#define _POSIX_C_SOURCE 200112L
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// function prototypes

// `get_protocol()` takes a string of "4" or "6" and returns the appropriate 
// protocol AF_INET/AF_INET6, or exits with failure if non-recognised protocol 
int get_protocol(char*);


#endif