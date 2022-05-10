// File:    response.h
// Author:  Hannah Jean Perry <hperry1@student.unimelb.edu.au>
// Subject: COMP30023 Computer Systems
// Project: Serving the Web (project 2)
// Purpose: Header file for the functions and type definitions relating to the
//          response of the server

#ifndef RESPONSE_H
#define RESPONSE_H

#include "utils.h"
#include "request.h"

// constants
// status codes
#define OK 200
#define BAD_REQUEST 400
#define FORBIDDEN 403
#define NOT_FOUND 404
#define TEAPOT 418


    //format response
    //          - status line: 
    //              - http version (HTTP/1.0)
    //              - status code (200/403/404/400...) 
    //              - reason phrase (OK/Forbidden/Not Found/Bad Request...)
    //          - http headers:
    //              - Date
    //              - Server
    //              - Content-type
    //          - message body

// function prototypes
int get_status_code(request_t*);

#endif

