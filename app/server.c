#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

int main()
{
	// variable declarations
	struct HTTP_STATUS_CODES
	{
		char HTTP_NOT_FOUND[30];
		char HTTP_OK[30];
	};

	struct HTTP_STATUS_CODES HTTP_status_codes = {
		"HTTP/1.1 404 Not Found\r\n\r\n",
		"HTTP/1.1 200 OK\r\n\r\n"};

	struct Request
	{
		char http_method[10];
		char path[100];
		char http_protocol[10];
	};

	struct Request request;

	// Disable output buffering
	setbuf(stdout, NULL);

	// You can use print statements as follows for debugging, they'll be visible when running tests.
	printf("Logs from your program will appear here!\n");

	// Uncomment this block to pass the first stage

	int server_fd, client_addr_len;
	struct sockaddr_in client_addr;

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1)
	{
		printf("Socket creation failed: %s...\n", strerror(errno));
		return 1;
	}

	// Since the tester restarts your program quite often, setting REUSE_PORT
	// ensures that we don't run into 'Address already in use' errors
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0)
	{
		printf("SO_REUSEPORT failed: %s \n", strerror(errno));
		return 1;
	}

	struct sockaddr_in serv_addr = {
		.sin_family = AF_INET,
		.sin_port = htons(4221),
		.sin_addr = {htonl(INADDR_ANY)},
	};

	if (bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0)
	{
		printf("Bind failed: %s \n", strerror(errno));
		return 1;
	}

	int connection_backlog = 5;
	if (listen(server_fd, connection_backlog) != 0)
	{
		printf("Listen failed: %s \n", strerror(errno));
		return 1;
	}

	printf("Waiting for a client to connect...\n");
	client_addr_len = sizeof(client_addr);

	// Stage 2 code
	int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len); // create file descriptor for client
	printf("Client connected\n");

	// declare buffer for client message
	char client_buffer[1024];

	if (read(client_fd, client_buffer, sizeof(client_buffer)) < 0)
	{ // read from client
		printf("Error reading from client: %s\n", strerror(errno));
		return -1;
	}

	//This line is not used for stage 3 and after
	//send(client_fd, HTTP_status_codes.HTTP_OK, sizeof(HTTP_status_codes.HTTP_OK), 0); // send response to client

	// stage 3 code
	sscanf(client_buffer, "%s %s %s", request.http_method, request.path, request.http_protocol); // parse client message

	// Print request for debugging purposes
	printf("HTTP Method: %s\n", request.http_method);
	printf("Path: %s\n", request.path);
	printf("HTTP Protocol: %s\n", request.http_protocol);

	if (strcmp(request.path, "/") == 0) //check if string is only /
	{
		send(client_fd, HTTP_status_codes.HTTP_OK, sizeof(HTTP_status_codes.HTTP_OK), 0); // send response to client
	}
	else
	{
		send(client_fd, HTTP_status_codes.HTTP_NOT_FOUND, sizeof(HTTP_status_codes.HTTP_NOT_FOUND), 0); // send response to client
	}

	printf(client_buffer); // print client message for seeing structure

	close(server_fd);

	return 0;
}
