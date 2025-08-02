#include "wifi.h"

// Design based on Wifi General AP Sceanrio:
// https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-guides/wifi.html#esp32-wi-fi-station-general-scenario

EventGroupHandle_t event_group;

static void event_debugging(void *event_handler_arg,
                            esp_event_base_t event_base, int32_t event_id,
                            void *event_data) {
  if (event_base == WIFI_EVENT) {
    if (event_id == WIFI_EVENT_STA_START) {
      // Phase 4
      ESP_LOGI("STA", "Configured STA");
      ESP_ERROR_CHECK(esp_wifi_connect()); // as recommended by docs
    } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
      xEventGroupSetBits(event_group, WIFI_DISCONNECTED_BIT);
      ESP_LOGI("STA", "WiFi Disconnected... Retrying to connect");
      ESP_ERROR_CHECK(esp_wifi_connect()); // as recommended by docs
    }
  } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
    xEventGroupSetBits(event_group, WIFI_CONNECTED_BIT);
    ESP_LOGI("DHCP", "Received IP from DHCP");
    // Opening relevant TCP_sockets to listen to incoming traffic over here or
    // after WIFI_CONNECTED_BIT is set to 1
    // We prefer to open it after calling wifi_init() in app_main function
    // completes for better readability, as we are already waiting on
    // WIFI_CONNECTED_BIT event
  }
}

void wifi_init() {
  // Phase 1: Init
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NOT_INITIALIZED) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }

  ESP_ERROR_CHECK(ret);

  ESP_ERROR_CHECK(esp_netif_init());

  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_create_default_wifi_sta();

  esp_event_handler_instance_t instance_any_id;
  esp_event_handler_instance_t instance_got_ip;

  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      WIFI_EVENT, ESP_EVENT_ANY_ID, &event_debugging, NULL, &instance_any_id));
  ESP_ERROR_CHECK(esp_event_handler_instance_register(
      IP_EVENT, IP_EVENT_STA_GOT_IP, &event_debugging, NULL, &instance_got_ip));

  // Phase 2: Configure Wifi as station
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  esp_wifi_set_mode(WIFI_MODE_STA);
  wifi_config_t sta_config = {.sta = {
                                  .ssid = SSID,
                                  .password = PASSWORD,
                              }};
  esp_wifi_set_config(WIFI_IF_AP, &sta_config);

  // Phase 3: Start
  ESP_ERROR_CHECK(esp_wifi_start());

  // Logging (Optional)
  event_group = xEventGroupCreate();
  EventBits_t bits = xEventGroupWaitBits(
      event_group, WIFI_CONNECTED_BIT | WIFI_DISCONNECTED_BIT, pdTRUE, pdFALSE,
      portMAX_DELAY); // ensuring that the code continues only when the Wifi is
                      // connected

  if (bits & WIFI_CONNECTED_BIT)
    ESP_LOGI("C", "Connected to %s", sta_config.sta.ssid);
  else if (bits & WIFI_DISCONNECTED_BIT)
    ESP_LOGI("DC", "Lost Connection");
  else
    ESP_LOGE("WTF",
             "Unexpected Error, both Connected and Disconnected bit are 1!!!");
}
