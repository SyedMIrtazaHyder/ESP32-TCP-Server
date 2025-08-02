#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "lwip/sockets.h"

#define PORT 3000
#define BUFF_SIZE 128
#define CLIENT_PRIORITY 3
#define CLIENT_STACK_SIZE 2048

struct client_info {
  struct sockaddr_in addr;
  socklen_t addr_len;
  TaskHandle_t task_handle;
  int id;
  int socket;
};

void server_init();
