#include "server.h"

static void client_comm(void *pvParameters) {
  struct client_info **ci = (struct client_info **)pvParameters;

  // Extracting variables from the pointer as the pointer updates every time a
  // new client registers. To prevent losing all data, we store a copy of the
  // memory address of the client_info structure.
  int s = (*ci)->socket;
  int client_id = (*ci)->id;
  struct client_info *current_cli = *ci;

  char buff[BUFF_SIZE] = "";
  ssize_t input_len = 0;
  // Client Communication Logic: All we are doing is listening to what the
  // client has sent and echoing it
  for (;;) {
    if ((input_len = read(s, buff, sizeof(buff))) > 0) {
      if (strncmp(buff, "exit", 4) == 0)
        break;
      printf("Client %d on %d Sent: %s (length %zd)\n", client_id, s, buff,
             input_len - 1);
      bzero(buff, sizeof(buff));
    }
  }

  // as client has exited, peforming cleanup operations
  printf("Client %d Connection Closed\n", client_id);
  closesocket(s);
  free(current_cli);
  vTaskDelete(NULL);
}

void server_init() {
  // configuring server
  int sockd = socket(AF_INET, SOCK_STREAM, 0);
  int total_clients = 0;

  struct sockaddr_in server;
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  server.sin_family = AF_INET;
  server.sin_port = htons(PORT);

  int err = bind(sockd, (const struct sockaddr *)&server, sizeof(server));
  if (err < 0)
    ESP_LOGE("BIND", "Socket unable to bind %d", err);

  err = listen(sockd, 1);
  if (err < 0)
    ESP_LOGE("LISTEN", "Socket unable to listen %d", err);

  // For debugging purposes we store the Client Comm X task name
  // as X is dynamic, we have to dynamically allocate it.
  char taskname_prefix[] = "Client Comm";
  char *process_name = (char *)malloc(sizeof(taskname_prefix));

  for (;;) {
    // waiting for client connections, and when client connects, we create a
    // FreeRTOS task `client_comm` to keep the communication with the client
    // presistent.
    struct client_info *client =
        (struct client_info *)malloc(sizeof(struct client_info));
    client->task_handle = NULL;
    client->id = ++total_clients;
    client->addr_len = sizeof(client->addr);

    client->socket =
        accept(sockd, (struct sockaddr *)&(client->addr), &(client->addr_len));

    if (client->socket < 0)
      ESP_LOGE("ACCEPT", "Unable to accept client connection %d",
               client->socket);

    // prefix + space + no. of digits + /0 (already accounted for in prefix)
    unsigned long process_name_size =
        sizeof(taskname_prefix) + 1 + sizeof(char) * (int)log10(client->id);
    process_name = (char *)realloc(process_name, process_name_size);
    sprintf(process_name, "%s %d", taskname_prefix, client->id);
    xTaskCreate(&client_comm, process_name, CLIENT_STACK_SIZE, (void *)&client,
                CLIENT_PRIORITY, &(client->task_handle));
  }
  // code should never run till here
  free(process_name);
}
