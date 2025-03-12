#include <cstring>
#include <esp_event.h>
#include <esp_log.h>
// #include <esp_netif.h>
#include <esp_wifi.h>
#include <esp_wifi_default.h>

#include "wi_fi_sta_interface.h"

#include <esp_check.h>

static char* TAG = "WI_FI_STA_INTERFACE";

// Event group to signal Wi-Fi connection status
static EventGroupHandle_t s_wifi_event_group;
// Number of retry attempts
// static int s_retry_num = 0;
// Network Interface for default STA
// static esp_netif_t *netif;

/**
 * @brief Wi-Fi event handler.
 *
 * Handles Wi-Fi events such as connection attempts, disconnections, and IP address acquisition.
 *
 * @param arg        Unused parameter.
 * @param event_base The event base (WIFI_EVENT or IP_EVENT).
 * @param event_id   The specific event ID being handled.
 * @param event_data Data associated with the event.
 */
static void event_handler(void *arg, const esp_event_base_t event_base, const int32_t event_id, void *event_data) {

    if (event_base == WIFI_EVENT) {

        if (event_id == WIFI_EVENT_STA_START) {
            ESP_LOGI(TAG, "Wi-Fi Started: connecting...");
            esp_err_t ret = esp_wifi_connect();
            if (ret == ESP_OK) {
                ESP_LOGI(TAG, "Wi-Fi Connected");
            } else {
                xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
                if (ret == ESP_ERR_WIFI_NOT_INIT) {
                    ESP_LOGE(TAG, "Wi-Fi not initialized. Call esp_wifi_init() first.");
                } else if (ret == ESP_ERR_WIFI_NOT_STARTED) {
                    ESP_LOGE(TAG, "Wi-Fi not started. Call esp_wifi_start() before connecting.");
                } else if (ret == ESP_ERR_WIFI_MODE) {
                    ESP_LOGE(TAG, "Wi-Fi mode error. Ensure Wi-Fi is in station mode (WIFI_MODE_STA).");
                } else if (ret == ESP_ERR_WIFI_CONN) {
                    ESP_LOGE(TAG, "Wi-Fi connection failed due to internal error.");
                } else if (ret == ESP_ERR_WIFI_SSID) {
                    ESP_LOGE(TAG, "Invalid SSID. Ensure the SSID is correctly set before connecting.");
                } else {
                    ESP_LOGE(TAG, "Wi-Fi connection failed with unknown error: 0x%x", ret);
                }
            }

        } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
            ESP_LOGI(TAG, "Wi-Fi Disconnected");
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        const ip_event_got_ip_t *event = static_cast<ip_event_got_ip_t *>(event_data);
        ESP_LOGI(TAG, "Connected! Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        // s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

esp_err_t wifi_sta_init(const char *ssid, const char *password) {
    // ESP_ERROR_CHECK(esp_event_loop_create_default());
    s_wifi_event_group = xEventGroupCreate();

    // // Create network interface for Wi-Fi STA (esp_netif_new)
    // ESP_LOGI(TAG, "Create network interface for Wi-Fi STA...");
    // netif = esp_netif_create_default_wifi_sta();

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, nullptr, &instance_any_id));
    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, nullptr, &instance_got_ip));

    // Initialize Wi-Fi
    ESP_LOGI(TAG, "Initializing Wi-Fi...");
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_RETURN_ON_ERROR(esp_wifi_init(&cfg), TAG, "Failed to initialize WiFi");
    ESP_RETURN_ON_ERROR(esp_wifi_stop(), TAG, "Failed to stop WiFi");
    ESP_RETURN_ON_ERROR(esp_wifi_set_mode(WIFI_MODE_STA), TAG, "Failed to set WiFi mode");

    // Set Wi-Fi config
    wifi_config_t wifi_config = {};
    memcpy(wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    memcpy(wifi_config.sta.password, password, sizeof(wifi_config.sta.password));
    ESP_RETURN_ON_ERROR(esp_wifi_set_config(WIFI_IF_STA, &wifi_config), TAG, "Failed to set WiFi configuration");
    ESP_RETURN_ON_ERROR(esp_wifi_start(), TAG, "Failed to start WiFi");

    // Start Wi-Fi
    ESP_RETURN_ON_ERROR(esp_wifi_start(), TAG, "Failed to start WiFi");

    xEventGroupWaitBits(s_wifi_event_group,
        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY);

    return ESP_OK;
}

// void wifi_sta_deinit() {
//     ESP_LOGI(TAG, "Deinitializing Wi-Fi...");
//
//     ESP_ERROR_CHECK(esp_wifi_stop());
//     ESP_ERROR_CHECK(esp_wifi_deinit());
//     esp_netif_destroy_default_wifi(netif);
//
//     vEventGroupDelete(s_wifi_event_group);
//
//     ESP_LOGI(TAG, "Wi-Fi deinitialized successfully.");
// }

void wifi_sta_deinit()
{
    ESP_LOGI(TAG, "Deinitializing Wi-Fi...");

    const esp_err_t err = esp_wifi_stop();
    if (err == ESP_ERR_WIFI_NOT_INIT) {
        return;
    }
    ESP_ERROR_CHECK(err);

    ESP_ERROR_CHECK(esp_wifi_deinit());

    // esp_netif_destroy_default_wifi(netif);
    //
    // netif = nullptr;

    vEventGroupDelete(s_wifi_event_group);

    // esp_event_loop_delete_default();

    ESP_LOGI(TAG, "Wi-Fi deinitialized successfully.");
}

