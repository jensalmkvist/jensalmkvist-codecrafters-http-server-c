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

	char CRLF[4] = "\r\n"; // carriage return line feed
	// variable declarations
	struct HTTP_STATUS_CODES
	{
		char HTTP_NOT_FOUND[30];
		char HTTP_OK[30];
	};

	struct HTTP_STATUS_CODES HTTP_status_codes = {
		"HTTP/1.1 404 Not Found\r\n",
		"HTTP/1.1 200 OK\r\n"};

	struct HTTP_STATUS_MESSAGES
	{
		char NOT_FOUND[30];
		char OK[30];
	};

	struct HTTP_STATUS_MESSAGES HTTP_status_messages = {
		"404 Not Found",
		"200 OK"};

	struct Request
	{
		char http_method[10];
		char path[100];
		char http_protocol[10];
	};

	struct Request request;

	struct Response
	{
		char http_protocol[10];
		char status_code[30];
		char status_message[30];
		char content_type[30];
		char content_length[10];
		char body[1000];
	};

	struct Response response = {
		.http_protocol = "HTTP/1.1"};

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
	char client_buffer[2048];

	if (read(client_fd, client_buffer, sizeof(client_buffer)) < 0)
	{ // read from client
		printf("Error reading from client: %s\n", strerror(errno));
		return -1;
	}

	// This line is not used for stage 3 and after
	// send(client_fd, HTTP_status_codes.HTTP_OK, sizeof(HTTP_status_codes.HTTP_OK), 0); // send response to client

	// stage 3 code
	sscanf(client_buffer, "%s %s %s", request.http_method, request.path, request.http_protocol); // parse client message

	// Print request for debugging purposes
	//	printf("HTTP Method: %s\n", request.http_method);
	//	printf("Path: %s\n", request.path);
	//	printf("HTTP Protocol: %s\n", request.http_protocol);

	char responseStr[2048];

	if (strcmp(request.path, "/") == 0) // check if string is only /
	{
		strcpy(response.status_code, HTTP_status_codes.HTTP_OK);
		sprintf(responseStr, "%s", response.status_code);
		strcat(responseStr, CRLF);

		send(client_fd, responseStr, sizeof(responseStr), 0); // send response to client
	}
	else if (strstr(request.path, "/echo/") != NULL)
	{

		strcpy(response.status_code, HTTP_status_codes.HTTP_OK);
		strcpy(response.status_message, HTTP_status_messages.OK);
		strcpy(response.content_type, "text/plain");
		strcpy(response.body, request.path + strlen("/echo/"));

		char content_length_buffer[strlen(response.body)];
		sprintf(content_length_buffer, "%d", response.content_length);

		sprintf(responseStr, "%sContent-Type: %s %sContent-Length: %u %s %s%s%s",
				response.status_code,
				response.content_type,
				CRLF,
				content_length_buffer,
				CRLF, response.body,
				CRLF);

		strcat(responseStr, CRLF);
		send(client_fd, responseStr, sizeof(responseStr), 0); // send response to client

		// Debugging prints
//		printf("Status Code: %s\n", response.status_code);
//		printf("Status Message: %s\n", response.status_message);
//		printf("Content Type: %s\n", response.content_type);
//		printf("Body: %s\n", response.body);
//		printf("Content Length: %d\n", response.content_length);
//		printf("content_length_buffer: %s\n", content_length_buffer);
		printf("Response:\n %s\n", responseStr);
	}
	else
	{
		strcpy(response.status_code, HTTP_status_codes.HTTP_NOT_FOUND);
		sprintf(responseStr, "%s", response.status_code);
		strcat(responseStr, CRLF);

		send(client_fd, responseStr, sizeof(responseStr), 0); // send response to client
	}

	// printf(client_buffer); // print client message for seeing structure

	close(server_fd);

	return 0;
}
