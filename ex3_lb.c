#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define LISTEN_BACKLOG 50
#define NUM_SERVERS 3
#define BUF_SIZE 1024
#define SERVER_PORT_FILENAME "server_port"
#define HTTP_PORT_FILENAME "http_port"
#define DOUBLE_NEWLINE "\r\n\r\n"
#define LOWEST_PORT 1024
#define HIGHEST_PORT 62976

int get_rand_port_number() { return LOWEST_PORT + (rand() % HIGHEST_PORT); }

int create_and_bind_socket(int *socket_fd) {
  int port_number = 0, bind_result = 0, enable = 1;
  struct sockaddr_in address;

  *socket_fd = socket(AF_INET, SOCK_STREAM, 0);

  setsockopt(*socket_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_ANY);

  do {
    port_number = get_rand_port_number();
    address.sin_port = htons(port_number);
    bind_result =
        bind(*socket_fd, (const struct sockaddr *)&address, sizeof(address));
  } while (bind_result < 0);

  return port_number;
}

void write_num_to_file(int num, const char *filename) {
  FILE *file_handle = fopen(filename, "w");
  fprintf(file_handle, "%d", num);
  fclose(file_handle);
}

void init_sockets(int *servers_fd, int *client_fd) {
  int server_port = 0, http_port = 0;

  server_port = create_and_bind_socket(servers_fd);
  write_num_to_file(server_port, SERVER_PORT_FILENAME);

  http_port = create_and_bind_socket(client_fd);
  write_num_to_file(http_port, HTTP_PORT_FILENAME);

  if (listen(*servers_fd, LISTEN_BACKLOG) < 0)
    printf("Error on listen() to servers");

  if (listen(*client_fd, LISTEN_BACKLOG) < 0)
    printf("Error on listen() to clients");
}

void accept_servers_connections(int socket_fd, int *server_handles) {
  int current_server = 0;
  for (current_server = 0; current_server < NUM_SERVERS; current_server++) {
    server_handles[current_server] = accept(socket_fd, NULL, NULL);
  }
}

char *recieve_http(int connection, bool is_response) {
  int bytes_recieved = 0;
  char *buffer;
  bool done = false;

  buffer = calloc(BUF_SIZE, sizeof(char));
  do {
    bytes_recieved +=
        recv(connection, buffer + bytes_recieved, BUF_SIZE - 1, 0);
    if (is_response)
      done = strstr(strstr(buffer, DOUBLE_NEWLINE), DOUBLE_NEWLINE) != NULL;
    else
      done = strstr(buffer, DOUBLE_NEWLINE) != NULL;
  } while (!done);

  buffer[bytes_recieved] = '\0';

  return buffer;
}

int send_string_to_socket(char *string, int connection) {
  int msg_len = strlen(string), bytes_sent = 0;

  while (bytes_sent < msg_len) {
    bytes_sent += write(connection, string, msg_len);
  }
  return bytes_sent;
}

void handle_clients(int client_fd, int *server_handles) {
  int client_handle = 0;
  char *request = NULL, *response = NULL;
  int current_server = 0;

  while (true) {
    client_handle = accept(client_fd, NULL, NULL);
    request = recieve_http(client_handle, false);
    if (current_server == (NUM_SERVERS - 1))
      current_server = 0;
    else
      current_server++;

    send_string_to_socket(request, server_handles[current_server]);
    response = recieve_http(server_handles[current_server], true);
    send_string_to_socket(response, client_handle);

    free(request);
    free(response);
  }
}

int main() {
  int servers_fd = 0, client_fd = 0;
  int server_handles[NUM_SERVERS];

  srandom(time(NULL));
  init_sockets(&servers_fd, &client_fd);
  accept_servers_connections(servers_fd, server_handles);
  handle_clients(client_fd, server_handles);

  close(servers_fd);
  close(client_fd);
  return 0;
}
