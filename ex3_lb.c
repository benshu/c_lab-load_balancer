#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <stdbool.h>

#define LISTEN_BACKLOG 50
#define NUM_SERVERS 3
#define BUF_SIZE 1024
#define SERVER_PORT_FILENAME "server_port"
#define HTTP_PORT_FILENAME "http_port"
#define DOUBLE_NEWLINE "\r\n\r\n"

int get_rand_port_number()
{
    return 1024 + (rand() % 62976);
}

int create_and_bind_socket(int* socket_fd)
{
    int port_number = 0,  bind_result = 0;
    struct sockaddr_in addr;

    *socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    int enable=1;

    setsockopt(*socket_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    do {
        port_number = get_rand_port_number();
        addr.sin_port = htons(port_number);
        bind_result = bind(*socket_fd, (const struct sockaddr *) &addr, sizeof(addr));
    } while (bind_result < 0);

    return port_number;
}

void write_num_to_file(int num, const char *filename)
{
    FILE *f = fopen(filename, "w");
    fprintf(f, "%d", num); 
    fclose(f);
}

void init_sockets(int *servers_fd, int* client_fd){
    int  server_port = 0, http_port =0;

    server_port = create_and_bind_socket(servers_fd);
    /*printf("Servers port: %d\n", server_port);*/
    write_num_to_file(server_port, SERVER_PORT_FILENAME);

    http_port = create_and_bind_socket(client_fd);
    write_num_to_file(http_port, HTTP_PORT_FILENAME);
    /*printf("http port: %d\n", http_port);*/

    if(listen(*servers_fd, LISTEN_BACKLOG) < 0 )
        printf("Error on listen() to servers");

    if(listen(*client_fd, LISTEN_BACKLOG) < 0 )
        printf("Error on listen() to clients");
}

void accept_servers_connections(int socket_fd, int *server_handles)
{
	int i=0;
    for (i = 0; i < NUM_SERVERS; i++) {
        /*printf("Waiting for connection\n");*/
        server_handles[i] = accept(socket_fd , NULL, NULL);
        /*printf("server connected\n");*/
    }
}

char *recieve_http(int connection, bool is_response)
{
    int bytes_recieved =0;
    char *buffer;
    bool done = false;
    buffer = calloc(BUF_SIZE, sizeof(char));
    do {
        /*printf("Recieving from client\n");*/
        bytes_recieved += recv(connection, buffer + bytes_recieved, BUF_SIZE - 1, 0);
        /*printf("Data: %s\n", buffer);*/
        if (is_response) 
            done = strstr(strstr(buffer, DOUBLE_NEWLINE), DOUBLE_NEWLINE) != NULL ; 
        else
            done = strstr(buffer, DOUBLE_NEWLINE) != NULL; 
        
    } while(!done);

    buffer[bytes_recieved] = '\0';

    return buffer;
}

int send_string_to_socket(char *string, int connection)
{
    int msg_len = strlen(string), sent = 0;
    while (sent < msg_len) {

        sent += write(connection, string, strlen(string));
    }
    return sent;
}
void handle_clients(int client_fd, int *server_handles)
{
    int client_handle =0;
    char *request = NULL, *response = NULL;
    int current_server = 0;

    while (1) {
        /*printf("Waiting for client\n");*/
        client_handle = accept(client_fd , NULL, NULL);
        /*printf("Client connected\n");*/

        request = recieve_http(client_handle, false);
        /*printf("Recieved: %s", request);*/

        if (current_server == (NUM_SERVERS -1))
            current_server =0;
        else current_server++;

        send_string_to_socket(request, server_handles[current_server]);
        /*printf("Wrote %d bytes to server %d\n", res, current_server);*/

        response = recieve_http(server_handles[current_server], true);

        send_string_to_socket(response, client_handle);
         /*printf("Wrote %d bytes to client\n", res);*/

        free(request);
        free(response);
    } 
}


int main()
{
    int servers_fd = 0, client_fd = 0;
    int server_handles[NUM_SERVERS];

    init_sockets(&servers_fd, &client_fd);
    accept_servers_connections(servers_fd, server_handles);
    handle_clients(client_fd, server_handles);


    close(servers_fd);
    return 0;
}
