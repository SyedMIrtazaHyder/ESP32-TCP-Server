#include "wifi.h"
#include "server.h"

void app_main(void) {
  wifi_init();
  server_init();
}
