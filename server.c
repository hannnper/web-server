// File:    server.c
// Author:  Hannah Jean Perry <hperry1@student.unimelb.edu.au>
// Subject: COMP30023 Computer Systems
// Project: Serving the Web (project 2)
// Purpose: Contains the main program of server, which is a basic multiplexed
//          HTTP server that responds to a limited set of requests

// Note: server ip
//       IPv4: 172.26.130.61
//       IPv6: fe80::f816:3eff:fe12:e19c

#include "utils.h"
#include "request.h"
#include "response.h"

// IPv6 compatible
#define IMPLEMENTS_IPV6
// actually uses epoll rather than multithreading to implement the multiplexing
#define MULTITHREADED

int main(int argc, char** argv) {
    int protocol, s, n, sockfd, status_code, i;
    char *port;
    char *server_path, *file_path;
    struct addrinfo hints, *res;
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
    protocol = get_protocol(argv[1]);
    port = argv[2];
    server_path = argv[3];

    printf("%d %s %s\n",protocol, port, server_path);

	// check server_path exists
	struct stat stat_buf;
	if (stat(server_path, &stat_buf) < 0) {
		perror("server path stats");
		exit(EXIT_FAILURE);
	}
	else if (S_ISDIR(stat_buf.st_mode) == 0) {
		// server path is not a directory
		printf("server path (root directory) is not a directory\n");
		exit(EXIT_FAILURE);
	}

    // code source: server.c from week 9 (sockets) prac comp30023 (modified)
    // create address for listening (with given port number)
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = protocol;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
	// node (NULL means any interface), service (port), hints, res
	s = getaddrinfo(NULL, port, &hints, &res);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}

	printf("creating socket...\n");
	// Create socket
	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sockfd < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}
	printf("created socket: %d\n", sockfd);

	// reuse port (code from project specifications)
	int enable = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	// Bind address to the socket
	if (bind(sockfd, res->ai_addr, res->ai_addrlen) < 0) {
		perror("bind");
		exit(EXIT_FAILURE);
	}
	freeaddrinfo(res);

	// Listen on socket - means we're ready to accept connections,
	// incoming connection requests will be queued, man 3 listen
	if (listen(sockfd, 5) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}
	printf("now listening\n");

	// create file descriptor for the epoll instance
	int epollfd = epoll_create1(0);
	int event_count = 0;
	// struct epoll_event: events (bit set), data (epoll_data_t: has fd)
	struct epoll_event events[MAX_EVENTS];
	struct epoll_event event;
	event.data.fd = sockfd;
	event.events = EPOLLIN;
	// add listening socket to epoll
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &event) < 0) {
		printf("failed to add listening socket to epoll\n");
		exit(EXIT_FAILURE);
	}

	while (true) {
		// wait for epoll (-1 for no timeout)
		event_count = epoll_wait(epollfd, events, MAX_EVENTS, -1);
		printf("There are %d epoll events\n", event_count);

		// iterate over ready events
		for (i = 0; i < event_count; i++) {
			if ((events[i].events & EPOLLIN) && (events[i].data.fd == sockfd)) {
				// accept connection
				accept_connection(sockfd, epollfd);
			}
			else if (events[i].events & EPOLLRDHUP || events[i].events & EPOLLHUP) {
				// connection was closed by client so deregister the fd of
				// this socket and don't do any further reading
				epoll_ctl(epollfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
				printf("disconnect on socket: %d", events[i].data.fd);
			}
			else {
				// there is something to read from the socket (and it is not
				// the listener socket)
				char buffer[BUFFER_SIZE + 1];
				n = read(events[i].data.fd, buffer, BUFFER_SIZE);
				if (n < 0) {
					perror("read");
					continue;
				}
				else if (n == 0) {
					// nothing to read from socket
					epoll_ctl(epollfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
					printf("disconnect on socket: %d", events[i].data.fd);
					continue;
				}
				// Null-terminate string
				buffer[n] = '\0';

				// print the request
				printf("Here is the request: %s\n", buffer);
				printf("length: %ld\n", strlen(buffer));

				// process the request
				request = process_request(buffer);
				printf("method: %d, path: %s\n", request->method, request->path);

				// get the full path to the requested file
				// (including server path)
				file_path = get_full_path(server_path, request);
				printf("full path to requested file: %s\n", file_path);

				// get the status code for the response to the request
				status_code = get_status_code(request, file_path);
				printf("status code: %d\n", status_code);
				send_status_line(events[i].data.fd, request, file_path);
				if (status_code == OK) {
					// send headers and content if OK status code
					send_http_headers(events[i].data.fd, request);
					// send the requested file contents
					send_contents(events[i].data.fd, file_path);
					
				}
				// clean up
				free(file_path);
				// close connection after sending response
				epoll_ctl(epollfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
				printf("disconnect socket (after sending response): %d", 
						events[i].data.fd);
			}
		}

		//TODO: check request formatting (400 error if malformed)
		//TODO: check there's no ../ in path (404 error)
		//TODO: check file access allowed (file can be opened for reading) (403 error)
		//TODO: check requested file exists (404 error)
		//TODO: determine http status (200 if all good)

		//TODO: format response
		//          - status line: 
		//              - http version (HTTP/1.0)
		//              - status code (200/403/404/400...)
		//              - reason phrase (OK/Forbidden/Not Found/Bad Request...)
		//          - http headers:
		//              - Date
		//              - Server
		//              - Content-type
		//          - message body
		//TODO: copy requested file to response (if okay)

		//TODO: (if time) implement BREW request

		//TODO: fix the seg fault that occurs when empty request received
		//TODO: properly implement IPv6
	}

	// it won't actually get to this part
	close(epollfd);
	close(sockfd);
	return 0;
}