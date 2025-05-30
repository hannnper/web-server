// File:    server.c
// Author:  Han Perry
// Subject: COMP30023 Computer Systems
// Project: Serving the Web (project 2)
// Purpose: Contains the main program of server, which is a basic multiplexed
//          HTTP server that responds to a limited set of requests


#include "utils.h"
#include "request.h"
#include "response.h"

// IPv6 compatible
#define IMPLEMENTS_IPV6
// actually uses epoll rather than multithreading to implement the multiplexing
#define MULTITHREADED

int main(int argc, char** argv) {
    int protocol, sockfd, status_code, i, s, n;
    char *port;
    char *server_path, *file_path, *protocol_string;
    struct addrinfo hints, *result, *p_addr;
    request_t* request;

    // print usage information if incorrect number command line arguments given
    if (argc != 4) {
        fprintf(stderr, "Usage:\n\t./server <protocol> <port> <path>\n\n"\
                        "protocol: number, 4 (for IPv4) or 6 (for IPv6)\n"\
                        "port: port number\n"\
                        "server path: string path to root web directory\n\n");
        exit(EXIT_FAILURE);
    }

    // get protocol, port and server path from command line arguments
    protocol_string = argv[1];
    protocol = get_protocol(protocol_string);
    port = argv[2];
    server_path = argv[3];

    printf("Initialising server with IPv%s on port %s\nwith root %s\n",
            protocol_string, port, server_path);

    // check server_path exists
    struct stat stat_buf;
    if (stat(server_path, &stat_buf) < 0) {
        perror("server path stats");
        exit(EXIT_FAILURE);
    }
    else if (S_ISDIR(stat_buf.st_mode) == 0) {
        // server path is not a directory
        printf("server path (web root) is not a directory\n");
        exit(EXIT_FAILURE);
    }

    // code source for initialising getaddrinfo: 
    // server.c from week 9 (sockets) prac comp30023 (modified)
    // create address for listening (with given port number)
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = protocol;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    // node (NULL means any interface), service (port), hints, result
    s = getaddrinfo(NULL, port, &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }
    
    printf("creating socket and binding address to it...\n");
    // getaddrinfo() returns a linked list of address structures,
    // trying them in the order returned
    for (p_addr = result; p_addr != NULL; p_addr = p_addr->ai_next) {
        // create socket
        sockfd = socket(p_addr->ai_family, p_addr->ai_socktype, 
                        p_addr->ai_protocol);
        if (sockfd < 0) {
            // error occurred in attempt to create socket
            perror("socket");
            continue;
        }
        // reuse port (code from project specifications)
        int enable = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
            perror("setsockopt");
            // try next address with a new socket
            continue;
        }
        // bind address to the socket
        if (bind(sockfd, p_addr->ai_addr, p_addr->ai_addrlen) == 0) {
            // successfully bound the address to the socket

            // set listening socket to non-blocking
            int flags = fcntl(sockfd, F_GETFL, 0);
            if (flags < 0 || fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0) {
                perror("fcntl (non-blocking sockfd)");
                exit(EXIT_FAILURE);
            }

            break;
        }
        else {
            perror("bind");
            continue;
        }
    }
    if (p_addr == NULL) {
        // exhausted all addresses without successfully being able to create
        // the socket and bind
        fprintf(stderr, "unable to create socket or bind address\n");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(result);
    printf("succesfully created socket and bound address to it\n");

    // listen on socket `sockfd` with maximum `BACKLOG` connections queued
    if (listen(sockfd, BACKLOG) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    printf("now listening\n");

    // partial message storage
    message_t *messages = NULL;

    // create file descriptor for the epoll instance
    int epollfd = epoll_create1(0);
    int event_count = 0;
    // struct epoll_event: events (bit set), data (epoll_data_t: has fd)
    struct epoll_event events[MAX_EVENTS];
    struct epoll_event event;
    memset(events, 0, sizeof(events));
    memset(&event, 0, sizeof(event));
    event.data.fd = sockfd;
    event.events = EPOLLIN;
    // add listening socket to epoll
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &event) < 0) {
        fprintf(stderr, "failed to add listening socket to epoll\n");
        exit(EXIT_FAILURE);
    }

    while (true) {
        // wait for epoll (-1 for no timeout)
        event_count = epoll_wait(epollfd, events, MAX_EVENTS, -1);

        // iterate over ready events
        for (i = 0; i < event_count; i++) {
            if ((events[i].events & EPOLLIN) && (events[i].data.fd == sockfd)) {
                // accept connection
                int newfd = accept_connection(sockfd, epollfd);
                if (newfd < 0) {
                    // failure in accept_connection
                    fprintf(stderr, "failed to accept new connection\n");
                    continue;
                }
                message_t *new_messages = add_message(newfd, messages);
                if (new_messages == NULL) {
                    // unsuccessful at creating new message (likely to be a
                    // malloc fail) - handle by closing connection
                    fprintf(stderr, "unable to allocate memory for message\n");
                    close_connection(newfd, epollfd, NULL);
                }
                else {
                    // successfully added new message so replace old messages
                    // list with new messages list 
                    messages = new_messages;
                }
            }
            else if (events[i].events & EPOLLRDHUP
                     || events[i].events & EPOLLHUP) {
                // connection was closed by client so deregister the fd of
                // this socket and don't do any further reading
                messages = close_connection(events[i].data.fd, epollfd,
                                             messages);
            }
            else {
                // there is something to read from the socket (and it is not
                // the listener socket)
                message_t *message = find_message(events[i].data.fd, messages);
                if (message == NULL) {
                    fprintf(stderr, "message not found for fd %d\n", events[i].data.fd);
                    messages = close_connection(events[i].data.fd, epollfd, messages);
                    continue;
                }
                n = read(events[i].data.fd, &message->buffer[message->n_read], 
                         BUFFER_SIZE - message->n_read);
                // update n_read for this message
                message->n_read = message->n_read + n;
                if (n < 0) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        // nothing to read right now
                        continue;
                    } else {
                        perror("read");
                        messages = close_connection(events[i].data.fd, epollfd, messages);
                        continue;
                    }
                }
                else if (n == 0) {
                    // nothing to read (but there was an epoll event)
                    messages = close_connection(events[i].data.fd, epollfd,
                                                messages);
                    continue;
                }
                // Null-terminate string
                message->buffer[message->n_read] = '\0';

                // update request readiness status
                update_message_status(message);

                // process request only if it is ready
                if (message->ready) {
                    // process the request
                    request = process_request(message->buffer);
                    if (request == NULL) {
                        // unable to allocate memory for request - handle by
                        // closing connection and continuing
                        messages = close_connection(events[i].data.fd, epollfd,
                                                messages);
                        continue;
                    }

                    // get the full path to the requested file
                    // (including server path)
                    file_path = get_full_path(server_path, request);

                    // get the status code for the response to the request
                    status_code = get_status_code(request, file_path);
                    send_status_line(events[i].data.fd, request, file_path);
                    printf("response status code: %d\n", status_code);
                    if (status_code == OK) {
                        // send headers and content if OK status code
                        send_http_headers(events[i].data.fd, request);
                        // send the requested file contents
                        send_contents(events[i].data.fd, file_path);
                        printf("sent http headers and file contents\n");
                    }
                    // clean up
                    free(file_path);
                    free(request);
                    // close connection after sending response
                    messages = close_connection(events[i].data.fd, epollfd, 
                                                messages);
                }
            }
        }
    }

    // it won't actually get to this part
    close(epollfd);
    close(sockfd);
    return 0;
}
