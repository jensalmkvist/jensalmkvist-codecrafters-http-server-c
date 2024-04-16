extern int server_port;
extern char* server_root;
extern int server_threads;
extern int server_queue_size;

extern struct HTTP_status_codes {
    /* data */
    const char HTTP_NOT_FOUND[30] = "HTTP/1.1 404 Not Found\r\n\r\n";
    const char HTTP_OK[30] = "HTTP/1.1 200 OK\r\n\r\n";
};


