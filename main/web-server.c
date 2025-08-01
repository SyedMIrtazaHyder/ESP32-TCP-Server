#include <stdio.h>
#include <strings.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lwip/sockets.h"
#include "wifi.h"

#define PORT 3000

struct client_info {
  int id;
  int *socket;
};
//-------------------- Socket Code --------------------
void client_comm(void *pvParameters) {
  struct client_info *ci = (struct client_info *)pvParameters;

  int *s = ci->socket;
  int client_id = ci->id;
  char buff[128] = "";
  ssize_t input_len = 0;
  for (;;) {
    if ((input_len = read(*s, buff, sizeof(buff))) > 0) {
      if (strncmp(buff, "exit", 4) == 0)
        break;
      printf("Client %d Sent: %s (length %zd)\n", client_id, buff, input_len - 1);
      bzero(buff, sizeof(buff));
    }
  }
  printf("Client Connection Closed\n");
  closesocket(*s);
  vTaskDelete(NULL);
}

void server_setup() {
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

  for (;;) {
    struct sockaddr_in client;
    socklen_t client_addr_len = sizeof(client);
    int connfd = accept(sockd, (struct sockaddr *)&client, &client_addr_len);
    struct client_info ci = {.id = (++total_clients), .socket = &connfd};
    if (connfd < 0)
      ESP_LOGE("ACCEPT", "Unable to accept client connection %d", connfd);
    // TODO: Change the name of the created task
    TaskHandle_t client_handle = NULL;
    xTaskCreate(&client_comm, "Client Communication 1", 2048, (void *)&ci, 2,
                &client_handle);
  }
}

void app_main(void) {
  wifi_init();
  server_setup();
}
