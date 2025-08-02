#include "server.h"
#include "wifi.h"

void app_main(void) {
  wifi_init();
  server_init();
}
