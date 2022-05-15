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
	if (listen(sockfd, MAX_CONNECTIONS) < 0) {
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
				int newfd = accept_connection(sockfd, epollfd);
				if (newfd < 0) {
					// failure in accept_connection
					printf("failed to accept new connection\n");
					continue;
				}
				messages = add_message(newfd, messages);
			}
			else if (events[i].events & EPOLLRDHUP || events[i].events & EPOLLHUP) {
				// connection was closed by client so deregister the fd of
				// this socket and don't do any further reading
				epoll_ctl(epollfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
				close(events[i].data.fd);
				messages = delete_message(events[i].data.fd, messages);
				printf("disconnect on socket: %d\n", events[i].data.fd);
			}
			else {
				// there is something to read from the socket (and it is not
				// the listener socket)
				message_t *message = find_message(events[i].data.fd, messages);
				printf("message pointer: %p, message: %s, n_read: %d\n", message, message->buffer, message->n_read);
				n = read(events[i].data.fd, &message->buffer[message->n_read], 
						 BUFFER_SIZE - message->n_read);
				// update n_read for this message
				message->n_read = message->n_read + n;
				printf("message pointer: %p, message: %s, n_read: %d\n", message, message->buffer, message->n_read);
				if (n < 0) {
					perror("read");
					continue;
				}
				else if (n == 0) {
					// nothing to read from socket (but there was an epoll event)
					epoll_ctl(epollfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
					close(events[i].data.fd);
					messages = delete_message(events[i].data.fd, messages);
					printf("disconnect on socket: %d\n", events[i].data.fd);
					continue;
				}
				// Null-terminate string
				message->buffer[message->n_read] = '\0';

				// print the request
				printf("Here is the request: %s\n", message->buffer);
				printf("length: %ld\n", strlen(message->buffer));

				// update request readiness status
				update_message_status(message);

				// process request only if it is ready
				if (message->ready) {
					// process the request
					request = process_request(message->buffer);
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
					close(events[i].data.fd);
					messages = delete_message(events[i].data.fd, messages);
					printf("disconnect socket (after sending response): %d\n", 
							events[i].data.fd);
				}

			}
		}

		//TODO: find out why it seg faults sometimes
		//TODO: properly implement IPv6
		//TODO: make sure checking permissions works (it doesn't seem to)
		//TODO: respond with 404 to requests for a directory
		//TODO: implement http version checking
		//TODO: check valgrind
	}

	// it won't actually get to this part
	close(epollfd);
	close(sockfd);
	return 0;
}