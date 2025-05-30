// File:    response.c
// Author:  Han Perry
// Subject: COMP30023 Computer Systems
// Project: Serving the Web (project 2)
// Purpose: Contains the functions and type definitions relating to the
//          response of the server

#define _GNU_SOURCE
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include "response.h"


// function definitions

// takes request_t `request` and char* `full_path` (full path to requested file)
// and returns an integer http status code:
// OK 200
// BAD_REQUEST 400
// FORBIDDEN 403
// NOT_FOUND 404
// TEAPOT 418
int get_status_code(request_t* request, char* full_path) {
    int ret;
    struct stat buf;
    // check if method is invalid
    if (request->method == INVALID) {
        return BAD_REQUEST;
    }
    else if (request->method == BREW) {
        return TEAPOT;
    }

    // check if filepath contains ".."
    if (strstr(request->path, "/../") != NULL) {
        // return NOT_FOUND 404
        return NOT_FOUND;
    }

    // handle if filepath is not absolute (doesn't begin with '/')
    if (request->path[0] != '/') {
        return BAD_REQUEST;
    }
    
    // check file exists and is regular
    ret = stat(full_path, &buf);
    if (ret != 0) {
        // error in stat() call, e.g. file doesn't exist
        return NOT_FOUND;
    }
    if (S_ISREG(buf.st_mode) == 0) {
        // not a regular file, may be directory/socket/symlink/etc
        return NOT_FOUND;
    }

    // check permission for reading file
    if (access(full_path, R_OK) < 0) {
        return FORBIDDEN;
    }

    // otherwise all okay
    return OK;
}


// formats and sends the status line of the response to the given socketfd
void send_status_line(int socketfd, request_t* request, char* full_path) {
    char* status_line;
    char* reason;
    int status_code = get_status_code(request, full_path);
    int ret;
    // get reason
    if (status_code == OK) {
        reason = OK_STR;
    }
    else if (status_code == BAD_REQUEST) {
        reason = BAD_REQUEST_STR;
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
    if (request->method == BREW) {
        ret = asprintf(&status_line, "%s %3.3d %s%s", HTCPCP, status_code,
                reason, CRLF);
    }
    else {
        ret = asprintf(&status_line, "%s %3.3d %s%s", HTTP_VER, status_code,
                reason, CRLF);
    }
    if (ret < 0) {
        // error occurred during formatting or memory allocation
        perror("format");
        return;
    }
    // send the status line
    if (write(socketfd, status_line, strlen(status_line)) < 0) {
        // error occurred during writing to socket
        perror("write status line");
    }
    free(status_line);
}


// takes `socketfd` file descriptor and the request_t `request` and sends the
// appropriate mime type in the "Content-Type" http header
void send_mimetype(int socketfd, request_t *request) {
    char *header, *extension;

    // set default mime type
    char mime_type[HEADER_MAX] = "application/octet-stream";


    // get extension (will be NULL if no extension)
    extension = strrchr(request->path, (int)'.');

    // find appropriate mime type for other extensions
    if (extension != NULL) {
        if (strcmp(extension, ".html") == 0) {
            strcpy(mime_type, "text/html");
        }
        else if (strcmp(extension, ".jpg") == 0) {
            strcpy(mime_type, "image/jpeg");
        }
        else if (strcmp(extension, ".css") == 0) {
            strcpy(mime_type, "text/css");
        }
        else if (strcmp(extension, ".js") == 0) {
            strcpy(mime_type, "text/javascript");
        }
    }

    // format the content-type header
    if (asprintf(&header, "Content-type: %s%s", mime_type, CRLF) < 0) {
        // error occurred during formatting or memory allocation
        perror("asprintf mime type formatting");
        return;
    }

    if (write(socketfd, header, strlen(header)) < 0) {
        // error occurred during writing to socket
        perror("write mime type");
    }
    free(header);
}


// sends the "Date" header line of the http headers in the response
void send_datetime(int socketfd) {
    char header[HEADER_MAX];
    time_t _time;
    time(&_time);
    struct tm *time = gmtime(&_time);

    // format the Date header
    if (strftime(header, HEADER_MAX, "Date: %a, %d %b %Y %H:%M:%S %Z\r\n",
         time) < 0) {
        // error occurred during formatting
        perror("format datetime");
        return;
    }
    if (write(socketfd, header, strlen(header)) < 0) {
        // error occurred during writing to socket
        perror("write datetime");
        return;
    }
}

// formats and sends the http headers
void send_http_headers(int socketfd, request_t *request) {
    // headers
    send_mimetype(socketfd, request);
    send_datetime(socketfd);

    // send final CRLF
    if (write(socketfd, CRLF, 2) < 0) {
        // error occurred during writing to socket
        perror("write final CRLF");
        return;
    }
}


// sends the file at the given `path` to the socket `socketfd`
void send_contents(int socketfd, char *path) {
    // open the file and store a file descriptor for it
    int filed = open(path, O_RDONLY);
    if (filed < 0) {
        // something went wrong in attempt to open file
        perror("file open");
        return;
    }

    // get file size
    struct stat buf;
    if (fstat(filed, &buf) < 0) {
        // something went wrong when attempting to find file stats
        perror("file stats");
        return;
    }

    // using sendfile because it is more efficient as it copies the files
    // within the kernel space, so it avoids having to read the file in to 
    // user-space and then write the file back out with multiple system calls.
    // Note from manual: Note that a successful call to sendfile() may write
    // fewer bytes than requested; the caller should be prepared to retry the
    // call if there were unsent bytes
    ssize_t bytes_sent = 0;
    ssize_t total_bytes_sent = 0;
    off_t *offset = NULL;
    do {
        // loop until whole file contents sent or error occurs
        bytes_sent = sendfile(socketfd, filed, offset, buf.st_size);
        if (bytes_sent < 0) {
            // something went wrong when attempting to send the file contents
            perror("sendfile");
            return;
        }
        total_bytes_sent = total_bytes_sent + bytes_sent;
    } while (total_bytes_sent < buf.st_size);
}
