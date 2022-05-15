// File:    request.c
// Author:  Hannah Jean Perry <hperry1@student.unimelb.edu.au>
// Subject: COMP30023 Computer Systems
// Project: Serving the Web (project 2)
// Purpose: Contains the functions and type definitions relating to the
//          requests to the server

#include "utils.h"
#include "request.h"

#define SEPS " "  // field separator in request header


// function definitions

// use add_message() to add an element in the messages linked list to store 
// the incoming message from this socket, returns pointer to linked list of
// messages
message_t* add_message(int socketfd, message_t *head) {
    message_t *new_message = malloc(sizeof(message_t));
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
    // request is ready to be processed once it contains a CRLF
    if (strstr(message->buffer, CRLF) != NULL) {
        message->ready = true;
        return;
    }
}


// process_request() takes a string request and returns a pointer to a request_t
// containing the information for the request
request_t* process_request(char* req_string) {
    request_t *request = malloc(sizeof(request_t));
    assert(request != NULL);
    request->method = INVALID;
    request->path[0] = '\0';

    // check if req_string is empty or just \n or \r\n
    if (strlen(req_string) == 0 || strcmp(req_string, CRLF) == 0 ||
        strcmp(req_string, "\n") == 0) {
        // empty request is INVALID, so just return defaults
        return request;
    }

    // split the request string header into parts
    char req_copy[BUFFER_SIZE + 1];
    // take only request header line
    char *rest = NULL;
    strncpy(req_copy, req_string, BUFFER_SIZE);
    strcpy(req_copy, strtok_r(req_copy, CRLF, &rest));
    char *next_tok = NULL;
    char *req_type = strtok_r(req_copy, SEPS, &next_tok);
    char *path = strtok_r(next_tok, SEPS, &next_tok);
    if (path != NULL) {
        strcpy(request->path, path);
    }
    char *version = strtok_r(next_tok, SEPS, &next_tok);
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
    strcpy(full_path, server_path);
    full_path = strcat(full_path, request->path);
    return full_path;
}





