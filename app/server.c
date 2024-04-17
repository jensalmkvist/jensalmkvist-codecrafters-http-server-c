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

	printf("Printing client buffer\n%s\n", client_buffer);

	char responseStr[1024];

	// State machine for handling different requests

	if (strstr(client_buffer, "GET / ") != NULL) // check if path after "GET" is only "/"
	{
		sprintf(responseStr, "%s%s", HTTP_status_codes.HTTP_OK, CRLF);
		send(client_fd, responseStr, sizeof(responseStr), 0); // send response to client
		printf("Response:\n%s\n", responseStr);
	}
	else if (strstr(client_buffer, "/echo/") != NULL)
	{

		char *posStart = strstr(client_buffer, "/echo/") + strlen("/echo/");
		char *posEnd = strstr(posStart, " HTTP/1.1");
		size_t len = posEnd - posStart;
		char body[len];
		printf("len: %zu\n", len);

		strncpy(body, posStart, len);
		//body[len] = '\0';
		printf("body: %s\n", body);

	/*	sprintf(responseStr, "%sContent-Type: %s %sContent-Length: %s%s%s%s%s%s",
				HTTP_status_codes.HTTP_OK,
				"text/plain",
				CRLF,
				len, // content length
				CRLF, CRLF,
				body, // content body
				CRLF, CRLF, CRLF);*/

		strcat(responseStr, HTTP_status_codes.HTTP_OK);
		strcat(responseStr, "Content-Type: text/plain\r\n");
		strcat(responseStr, CRLF);
		strcat(responseStr, "Content-Length: ");
		strcat(responseStr, len);
		strcat(responseStr, CRLF);
		strcat(responseStr, CRLF);
		strcat(responseStr, body);
		strcat(responseStr, CRLF);
		strcat(responseStr, CRLF);
		strcat(responseStr, CRLF);

		printf("Response:\n%s\n", responseStr);
		// send(client_fd, responseStr, sizeof(responseStr), 0); // send response to client
	}
	else
	{
		sprintf(responseStr, "%s%s", HTTP_status_codes.HTTP_NOT_FOUND, CRLF);
		send(client_fd, responseStr, sizeof(responseStr), 0); // send response to client
		printf("Response:\n%s\n", responseStr);
	}

	/*if (strcmp(request.path, "/") == 0) // check if string is only /
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

		// char content_length_buffer[strlen(response.body)];
		sprintf(response.content_length, "%zu", strlen(response.body));
		//		printf("Content length: %s \n", response.content_length);
		//		printf("Response body: %s \n", response.body);
		//		printf("strlen sixe of body: %zu \n", strlen(response.body));

		sprintf(responseStr, "%sContent-Type: %s %sContent-Length: %s%s%s%s%s",
				response.status_code,
				response.content_type,
				CRLF,
				response.content_length,
				CRLF, CRLF, response.body,
				CRLF, CRLF);

		strcat(responseStr, CRLF);

		send(client_fd, responseStr, sizeof(responseStr), 0); // send response to client

		// Debugging prints
		//		printf("Status Code: %s\n", response.status_code);
		//		printf("Status Message: %s\n", response.status_message);
		//		printf("Content Type: %s\n", response.content_type);
		//		printf("Body: %s\n", response.body);
		//		printf("Content Length: %s\n", response.content_length);
		printf("Response:\n%s\n", responseStr);
	}
	else if (strstr(client_buffer, "User-Agent: ") != NULL)
	{
		strcpy(response.status_code, HTTP_status_codes.HTTP_OK);
		strcpy(response.status_message, HTTP_status_messages.OK);
		strcpy(response.content_type, "text/plain");
		char *position = strstr(client_buffer, "User-Agent: ");
		// position = position - client_buffer;
		strcpy(response.body, position);

		sprintf(response.content_length, "%zu", strlen(response.body));

		sprintf(responseStr, "%sContent-Type: %s %sContent-Length: %s%s%s%s%s",
				response.status_code,
				response.content_type,
				CRLF,
				response.content_length,
				CRLF, CRLF, response.body,
				CRLF, CRLF);

		strcat(responseStr, CRLF);

		// Debugging prints
		// printf("Status Code: %s\n", response.status_code);
		// printf("Status Message: %s\n", response.status_message);
		// printf("Content Type: %s\n", response.content_type);
		// printf("Body: %s\n", response.body);
		// printf("Content Length: %s\n", response.content_length);
		// printf("Response:\n%s\n", responseStr);

		send(client_fd, responseStr, sizeof(responseStr), 0); // send response to client
	}

	else
	{
		strcpy(response.status_code, HTTP_status_codes.HTTP_NOT_FOUND);
		sprintf(responseStr, "%s", response.status_code);
		strcat(responseStr, CRLF);

		send(client_fd, responseStr, sizeof(responseStr), 0); // send response to client
	}*/

	close(server_fd);

	return 0;
}
