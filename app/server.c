#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
	// parse main arguments
	char *directory = NULL;
	if (argc > 1)
	{
		for (int i = 0; i < argc; i++)
		{
			if (strcmp(argv[i], "--directory") == 0)
			{
				directory = argv[i + 1];
			}
		}
		printf("Directory: %s\n", directory);
	}

	char CRLF[4] = "\r\n"; // carriage return line feed
	char doubleCRLF[8] = "\r\n\r\n";
	// variable declarations

	struct HTTP_STATUS_CODES
	{
		char HTTP_NOT_FOUND[30];
		char HTTP_OK[30];
		char HTTP_CREATED[30];
	};

	struct HTTP_STATUS_CODES HTTP_status_codes = {
		"HTTP/1.1 404 Not Found\r\n",
		"HTTP/1.1 200 OK\r\n",
		"HTTP/1.1 201 Created\r\n"};

	struct HTTP_STATUS_MESSAGES
	{
		char NOT_FOUND[30];
		char OK[30];
		char CREATED[30];
	};

	struct HTTP_STATUS_MESSAGES HTTP_status_messages = {
		"404 Not Found",
		"200 OK",
		"201 Created"};

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

	// while loop for handling concurrent connections
	while (1)
	{
		int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len); // create file descriptor for client
		printf("Client connected\n");

		if (0 == fork()) // create fork for handling multiple clients concurrently
		{
			close(server_fd);

			// declare buffer for client message
			char client_buffer[2048];

			if (read(client_fd, client_buffer, sizeof(client_buffer)) < 0)
			{ // read from client
				printf("Error reading from client: %s\n", strerror(errno));
				return -1;
			}

			// printf("Printing client buffer\n%s\n", client_buffer);

			char responseStr[1024];

			// If case for handling the different stages

			if (strstr(client_buffer, "GET / ") != NULL) // GET nothing
			{
				sprintf(responseStr, "%s%s", HTTP_status_codes.HTTP_OK, CRLF);
				send(client_fd, responseStr, sizeof(responseStr), 0); // send response to client
				printf("Response:\n%s\n", responseStr);
			}
			else if (strstr(client_buffer, "/echo/") != NULL) // GET Echo the message
			{
				// extract the text after /ecjo/ and before HTTP/1.1
				char *posStart = strstr(client_buffer, "/echo/") + strlen("/echo/");
				char *posEnd = strstr(posStart, " HTTP/1.1");
				size_t len = posEnd - posStart;
				char body[len];
				printf("len: %zu\n", len);

				strncpy(body, posStart, len);
				body[len] = '\0';
				printf("body: %s\n", body);

				// create response string
				sprintf(responseStr, "%sContent-Type: %s%sContent-Length: %zu%s%s%s%s%s%S",
						HTTP_status_codes.HTTP_OK,
						"text/plain",
						CRLF,
						len, // content length
						CRLF, CRLF,
						body, // content body
						CRLF, CRLF, CRLF);

				// send response to client
				printf("Response:\n%s\n", responseStr);				  // debug print
				send(client_fd, responseStr, sizeof(responseStr), 0); // send response to client
			}
			else if (strstr(client_buffer, "GET /user-agent") != NULL) // Get user agent
			{
				// extract the text after /ecjo/ and before HTTP/1.1
				char *posStart = strstr(client_buffer, "User-Agent: ") + strlen("User-Agent: ");
				char *posEnd = strstr(posStart, CRLF);
				size_t len = posEnd - posStart;
				char body[len];
				printf("len: %zu\n", len);

				strncpy(body, posStart, len);
				body[len] = '\0';
				printf("body: %s\n", body);

				// create response string
				sprintf(responseStr, "%sContent-Type: %s%sContent-Length: %zu%s%s%s%s%s%S",
						HTTP_status_codes.HTTP_OK,
						"text/plain",
						CRLF,
						len, // content length
						CRLF, CRLF,
						body, // content body
						CRLF, CRLF, CRLF);

				// send response to client
				printf("Response:\n%s\n", responseStr);				  // debug print
				send(client_fd, responseStr, sizeof(responseStr), 0); // send response to client
			}
			else if (strstr(client_buffer, "GET /files/") != NULL) // Get a file
			{
				// extract file path from client buffer
				char *posStart = strstr(client_buffer, "GET /files/") + strlen("GET /files/");
				char *posEnd = strstr(posStart, " HTTP/1.1");
				size_t len = posEnd - posStart;
				char fileName[len];
				strncpy(fileName, posStart, len);
				fileName[len] = '\0';

				fflush(stdout);
				printf("File name: %s\n", fileName);

				char completeFilePath[strlen(directory) + strlen(fileName)];
				strcpy(completeFilePath, directory);
				strncpy(completeFilePath + strlen(directory), fileName, strlen(fileName));
				completeFilePath[strlen(directory) + strlen(fileName)] = '\0';
				printf("Complete file path: %s\n", completeFilePath);

				FILE *file = fopen(completeFilePath, "r");

				// construct the full file path
				if (file != NULL) // check if file exists in directory
				{
					printf("File exists\n");
					fseek(file, 0, SEEK_END);
					long fileSize = ftell(file);
					rewind(file);

					char body[fileSize];
					size_t bodyLen = fread(body, 1, fileSize, file);
					body[bodyLen] = '\0';

					fclose(file);

					// create response string
					sprintf(responseStr, "%sContent-Type: %s%sContent-Length: %zu%s%s%s%s%s%S",
							HTTP_status_codes.HTTP_OK,
							"application/octet-stream",
							CRLF,
							bodyLen, // content length
							CRLF, CRLF,
							body, // content body
							CRLF, CRLF, CRLF);

					// send response to client
					printf("Response:\n%s\n", responseStr);				  // debug print
					send(client_fd, responseStr, sizeof(responseStr), 0); // send response to client
				}

				else
				{
					sprintf(responseStr, "%s%s", HTTP_status_codes.HTTP_NOT_FOUND, CRLF);
					send(client_fd, responseStr, sizeof(responseStr), 0); // send response to client
					printf("Response:\n%s\n", responseStr);
				}
			}
			else if (strstr(client_buffer, "POST ") != NULL) // POST a file
			{
				printf("POST request\n");
				// extract file path from client buffer
				char *posStart = strstr(client_buffer, "POST /files/") + strlen("POST /files/");
				char *posEnd = strstr(posStart, " HTTP/1.1");
				size_t len = posEnd - posStart;
				char fileName[len];
				strncpy(fileName, posStart, len);
				fileName[len] = '\0';

				fflush(stdout);
				//printf("File name: %s\n", fileName);

				char completeFilePath[strlen(directory) + strlen(fileName)];
				strcpy(completeFilePath, directory);
				strncpy(completeFilePath + strlen(directory), fileName, strlen(fileName));
				completeFilePath[strlen(directory) + strlen(fileName)] = '\0';
				//printf("Complete file path: %s\n", completeFilePath);
				fflush(stdout);
				//printf(client_buffer);

				FILE *file = fopen(completeFilePath, "wb");

				if (file != NULL) // check if file exists in directory
				{
					//printf("File exists\n");

					char *fileContent = strstr(client_buffer, "\r\n\r\n") + strlen("\r\n\r\n");
					fwrite(fileContent, 1, strlen(fileContent), file);
					

					fclose(file);

					// create response string
					sprintf(responseStr, "%s%s", HTTP_status_codes.HTTP_CREATED, CRLF);
					send(client_fd, responseStr, sizeof(responseStr), 0); // send response to client
					printf("Response:\n%s\n", responseStr);
				}

				else
				{
					sprintf(responseStr, "%s%s", HTTP_status_codes.HTTP_NOT_FOUND, CRLF);
					send(client_fd, responseStr, sizeof(responseStr), 0); // send response to client
					printf("Response:\n%s\n", responseStr);
				}
			}
			else
			{
				sprintf(responseStr, "%s%s", HTTP_status_codes.HTTP_NOT_FOUND, CRLF);
				send(client_fd, responseStr, sizeof(responseStr), 0); // send response to client
				printf("Response:\n%s\n", responseStr);
			}

			exit(0);
		}
		else
		{
			close(client_fd);
		}
	}

	close(server_fd);

	return 0;
}
