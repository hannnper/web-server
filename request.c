// File:    request.c
// Author:  Han Perry
// Subject: COMP30023 Computer Systems
// Project: Serving the Web (project 2)
// Purpose: Contains the functions and type definitions relating to the
//          requests to the server

#include "request.h"

#define SEPS " "  // field separator in request header


// function definitions

// use add_message() to add an element in the messages linked list to store 
// the incoming message from this socket, returns pointer to linked list of
// messages or NULL is malloc fails
message_t* add_message(int socketfd, message_t *head) {
    message_t *new_message = malloc(sizeof(message_t));
    if (new_message == NULL) {
        // malloc failed, return NULL immediately. Note, this doesn't return
        // the head of the existing list which may still contains messages
        return NULL;
    }
    new_message->ready = false;
    new_message->fd = socketfd;
    new_message->buffer[0] = '\0';
    new_message->n_read = 0;
    new_message->next = head;
    return new_message;
}


// use delete_message() to delete message from the linked list, returns a
// pointer to linked list of messages
message_t* delete_message(int socketfd, message_t *head) {
    if (head == NULL) {
        // no message to delete
        return NULL;
    }
    else if (head->fd == socketfd) {
        // the message to delete is at the front of the linked list
        message_t *rest = head->next;
        free(head);
        return rest;
    }
    else {
        head->next = delete_message(socketfd, head->next);
        return head;
    }
}


// returns a pointer to the message_t which contains the message data for the
// specified socketfd
message_t* find_message(int socketfd, message_t *head) {
    if (head == NULL) {
        // message not in list
        return NULL;
    }
    else if (head->fd == socketfd) {
        return head;
    }
    else {
        return find_message(socketfd, head->next);
    }
}


// update the ready flag in message struct
void update_message_status(message_t *message) {
    // request is ready to be processed once it ends in two CRLF 
    if (strstr(message->buffer, CRLF CRLF) != NULL) {
        message->ready = true;
        return;
    }
}


// process_request() takes a string request and returns a pointer to a request_t
// containing the information for the request, or NULL if malloc fails
request_t* process_request(char* req_string) {
    request_t *request = malloc(sizeof(request_t));
    if (request == NULL) {
        // malloc failed, return NULL
        return NULL;
    }
    request->method = INVALID;
    request->path[0] = '\0';

    // split the request string header into parts
    char req_copy[BUFFER_SIZE + 1];
    // take only request header line
    char *rest = NULL;
    strncpy(req_copy, req_string, BUFFER_SIZE);
    // get everything before the CRLF (just ignore body of request if it exists)
    char *req_line = strtok_r(req_copy, CRLF, &rest);
    // check if req_line is NULL (the case for empty or all-delim string input)
    if (req_line == NULL) {
        // request is INVALID, and must avoid calls to strtok_r()
        return request;
    }
    char *next_tok = NULL;
    char *req_type = strtok_r(req_line, SEPS, &next_tok);
    char *path = strtok_r(next_tok, SEPS, &next_tok);
    char *version = strtok_r(next_tok, SEPS, &next_tok);

    // copy path and version info to request if they are non-NULL
    if (path != NULL) {
        strcpy(request->path, path);
    }
    if (version != NULL) {
        strncpy(request->http_ver, version, MAX_VER_LEN);
    }
    printf("req: %s, path: %s, ver: %s\n", req_type, path, version);
    // check if the request method is a supported type
    if (req_type == NULL) {
        // avoid using strcmp on NULL
        return request;
    }
    else if (strcmp(req_type, GET_STR) == 0) {
        // this is a GET request
        request->method = GET;
    }
    else if (strcmp(req_type, BREW_STR) == 0) {
        request->method = BREW;
    }
    return request;
}


// combines the path to the root of the web server with the requested path
// note: this uses malloc so returned string needs to be freed after use
char* get_full_path(char *server_path, request_t *request) {
    int total_len = strlen(server_path) + strlen(request->path);
    char *full_path = malloc(sizeof(char) * (total_len + 1));
    if (full_path == NULL) {
        // malloc failed, return NULL
        return NULL;
    }
    strcpy(full_path, server_path);
    full_path = strcat(full_path, request->path);
    return full_path;
}


// closes the connection: close socket, delete record from epoll and remove
// from messages list. returns new head of messages list
message_t *close_connection(int socketfd, int epollfd, message_t *messages) {
    epoll_ctl(epollfd, EPOLL_CTL_DEL, socketfd, NULL);
    close(socketfd);
    messages = delete_message(socketfd, messages);
    printf("disconnecting socket: %d\n", socketfd);
    return messages;
}

