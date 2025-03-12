#include <config.h>
#include <esp_compiler.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_netif_types.h>
#include <esp_openthread.h>
#include <esp_openthread_lock.h>
#include <esp_openthread_netif_glue.h>

#include "thread_netif.h"

static const char *TAG = "THREAD_NETIF";

esp_netif_t *thread_netif_init(const esp_openthread_platform_config_t *ot_platform_config) {

    // Create the OpenThread Network Interface
    ESP_LOGI(TAG, "Creating OpenThread Network Interface...");
    esp_netif_inherent_config_t netif_inherent_config = {
        .flags = static_cast<esp_netif_flags_t>(0),
        ESP_COMPILER_DESIGNATED_INIT_AGGREGATE_TYPE_EMPTY(mac)
        ESP_COMPILER_DESIGNATED_INIT_AGGREGATE_TYPE_EMPTY(ip_info)
        .get_ip_event = 0,
        .lost_ip_event = 0,
        .if_key = "OT_DEF",
        .if_desc = "openthread",
        .route_prio = 15
    };
    esp_netif_config_t netif_config = {
        .base = &netif_inherent_config,
        .stack = &g_esp_netif_netstack_default_openthread,
    };
    esp_netif_t *thread_netif = esp_netif_new(&netif_config);
    if (!thread_netif) {
        ESP_LOGE(TAG, "Failed to create OpenThread network interface!");
        return nullptr;
    }
    ESP_LOGI(TAG, "OpenThread Network Interface created.");

    // Create the OpenThread interface handlers and attach them to the OpenThread Network Interface
    ESP_LOGI(TAG, "Attaching OpenThread network interface...");
    if (esp_netif_attach(thread_netif, esp_openthread_netif_glue_init(ot_platform_config)) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to attach OpenThread network interface!");
        esp_netif_destroy(thread_netif);
        return nullptr;
    }
    ESP_LOGI(TAG, "OpenThread network interface attached.");

    return thread_netif;
}

void thread_netif_deinit(esp_netif_t *thread_netif) {
    if (!thread_netif) {
        ESP_LOGW(TAG, "thread_netif is NULL. Skipping deinitialization.");
        return;
    }

    // Deinitialize the OpenThread network interface
    ESP_LOGI(TAG, "Deinitializing OpenThread network interface...");
    esp_openthread_netif_glue_deinit();
    esp_netif_destroy(thread_netif);
}
