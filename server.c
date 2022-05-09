// File:    server.c
// Author:  Hannah Jean Perry <hperry1@student.unimelb.edu.au>
// Subject: COMP30023 Computer Systems
// Project: Serving the Web (project 2)
// Purpose: Contains the main program of server, which is a basic
//          multithreaded HTTP server that responds to a limited set of 
//          GET requests

// Note: server ip
//       IPv4: 172.26.130.61
//       IPv6: fe80::f816:3eff:fe12:e19c


#include "utils.h"
#include "response.h"

int main(int argc, char** argv) {
    int protocol, s, re, n, sockfd, newsockfd;
    char* port;
    char* path;
    char buffer[BUFFER_SIZE + 1];
    struct addrinfo hints, *res;
	struct sockaddr_storage client_addr;
	socklen_t client_addr_size;
    request_t* request;

    // print usage information if incorrect number command line arguments given
    if (argc != 4) {
        fprintf(stderr, "Usage:\n\t./server <protocol> <port> <path>\n\n"\
                        "protocol: number, 4 (for IPv4) or 6 (for IPv6)\n"\
                        "port: port number\n"\
                        "path: string path to root web directory\n\n");
        exit(EXIT_FAILURE);
    }

    // get protocol, port and path from command line arguments
    protocol = get_protocol(argv[1]);
    port = argv[2];
    path = argv[3];

    printf("%d %s %s\n",protocol, port, path);

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

	// Create socket
	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sockfd < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	// Reuse port if possible
	re = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &re, sizeof(int)) < 0) {
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

	// Accept a connection - blocks until a connection is ready to be accepted
	// Get back a new file descriptor to communicate on
	client_addr_size = sizeof client_addr;
	newsockfd =
		accept(sockfd, (struct sockaddr*)&client_addr, &client_addr_size);
	if (newsockfd < 0) {
		perror("accept");
		exit(EXIT_FAILURE);
	}

	// Read characters from the connection, then process
	n = read(newsockfd, buffer, BUFFER_SIZE); // n is number of characters read
	if (n < 0) {
		perror("read");
		exit(EXIT_FAILURE);
	}
	// Null-terminate string
	buffer[n] = '\0';

	// print the request
	printf("Here is the request: %s\n", buffer);

    //TODO: check the request is GET
    printf("%d\n", strncmp(buffer, "GET", 3));
    request = process_request(buffer);
    printf("method: %d, path: %s\n", request->method, request->path);

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

    // response
	n = write(newsockfd, "I got your message", 18);
	if (n < 0) {
		perror("write");
		exit(EXIT_FAILURE);
	}

	close(sockfd);
	close(newsockfd);
	return 0;
}