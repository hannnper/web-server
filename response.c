// File:    response.c
// Author:  Hannah Jean Perry <hperry1@student.unimelb.edu.au>
// Subject: COMP30023 Computer Systems
// Project: Serving the Web (project 2)
// Purpose: Contains the functions and type definitions relating to the
//          response of the server

#define _GNU_SOURCE
#include <errno.h>
#include <time.h>
#include "utils.h"
#include "request.h"
#include "response.h"


// function definitions

// takes request_t `request` and returns an integer http status code:
// OK 200
// BAD_REQUEST 400
// FORBIDDEN 403
// NOT_FOUND 404
// TEAPOT 418
int get_status_code(request_t* request, char* server_path) {
    int ret, total_len;
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
    total_len = strlen(server_path) + strlen(request->path);
    full_path = malloc(sizeof(char) * total_len);
    strcpy(full_path, server_path);
    full_path = strcat(full_path, request->path);
    printf("full path: %s\n", full_path);
    ret = stat(full_path, &buf);
    free(full_path);
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


// formats and sends the status line of the response to the given socketfd
void send_status_line(int socketfd, request_t* request, char* server_path) {
    char* status_line;
    char* reason;
    int status_code = get_status_code(request, server_path);
    int ret;
    // get reason
    if (status_code == OK) {
        reason = OK_STR;
    }
    else if (status_code == NOT_FOUND) {
        reason = NOT_FOUND_STR;
    }
    else if (status_code == FORBIDDEN) {
        reason = FORBIDDEN_STR;
    }
    else if (status_code == TEAPOT) {
        reason = TEAPOT_STR;
    }
    // format the status line
    ret = asprintf(&status_line, "%s %3.3d %s%s", HTTP_VER, status_code,
             reason, CRLF);
    if (ret < 0) {
        // error occurred during formatting
        perror("format:");
        exit(EXIT_FAILURE);
    }
    // send the status line
    send(socketfd, status_line, strlen(status_line), 0);
}


// formats and sends the http headers
void send_http_headers(int socketfd, char* mime_type) {
    int ret;
    char* headers;
    time_t _time;
    time(&_time);
    struct tm *time = localtime(&_time);
    // format headers
    // TODO: format the date as specified by the standard for http/1.0
    ret = asprintf(&headers, "Date: %sContent-type: %s%s%s", asctime(time),
                   mime_type, CRLF, CRLF);
    if (ret < 0) {
        // error occurred during formatting
        perror("format:");
        exit(EXIT_FAILURE);
    }
    send(socketfd, headers, strlen(headers), 0);
}


// sends the file to the socket
void send_contents(int socketfd, char *path) {
    ;
}