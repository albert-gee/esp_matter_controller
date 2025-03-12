#include <esp_err.h>
#include <esp_log.h>
#include <esp_matter.h>
#include <esp_matter_console.h>
#include <esp_matter_controller_client.h>
#include <esp_matter_controller_console.h>
#include <esp_matter_controller_utils.h>
#include <esp_matter_ota.h>

#if CONFIG_OPENTHREAD_BORDER_ROUTER
#include <esp_openthread_border_router.h>
#include <esp_openthread_lock.h>
#include <esp_spiffs.h>
#include <platform/ESP32/OpenthreadLauncher.h>
#endif // CONFIG_OPENTHREAD_BORDER_ROUTER

#include <portmacro.h>

uint16_t switch_endpoint_id = 0;

using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;

static const char *TAG = "Matter Interface";


static void app_event_cb(const ChipDeviceEvent *event, intptr_t arg)
{
    switch (event->Type) {
        case chip::DeviceLayer::DeviceEventType::PublicEventTypes::kInterfaceIpAddressChanged:
            ESP_LOGI(TAG, "Interface IP Address changed");
        break;
        case chip::DeviceLayer::DeviceEventType::kESPSystemEvent:
            if (event->Platform.ESPSystemEvent.Base == IP_EVENT &&
                event->Platform.ESPSystemEvent.Id == IP_EVENT_STA_GOT_IP) {
#if CONFIG_OPENTHREAD_BORDER_ROUTER
                static bool sThreadBRInitialized = false;
                if (!sThreadBRInitialized) {
                    esp_openthread_set_backbone_netif(esp_netif_get_handle_from_ifkey("WIFI_STA_DEF"));
                    esp_openthread_lock_acquire(portMAX_DELAY);
                    esp_openthread_border_router_init();
                    esp_openthread_lock_release();
                    sThreadBRInitialized = true;
                }
#endif
                }
        break;
        default:
            break;
    }
}

esp_err_t identification_callback(esp_matter::identification::callback_type_t const type, uint16_t const endpoint_id,
                                  uint8_t const effect_id, uint8_t const effect_variant, void *priv_data) {
    ESP_LOGI(TAG, "Identification Callback Invoked: type=%d, endpoint_id=%u, effect_id=%u, effect_variant=%u",
             type, (unsigned int)endpoint_id, (unsigned int)effect_id, (unsigned int)effect_variant);
    return ESP_OK;
}

esp_err_t attribute_update_callback(const esp_matter::attribute::callback_type_t type, const uint16_t endpoint_id,
                                    const uint32_t cluster_id, const uint32_t attribute_id, esp_matter_attr_val_t *val,
                                    void *priv_data) {
    ESP_LOGI(TAG, "Attribute update: type=%d, endpoint_id=%u, cluster_id=%u, attribute_id=%u",
             type, (unsigned int)endpoint_id, (unsigned int)cluster_id, (unsigned int)attribute_id);
    return ESP_OK;
}

void start_matter_interface() {
#if CONFIG_ENABLE_CHIP_SHELL
    esp_matter::console::diagnostics_register_commands();
    esp_matter::console::wifi_register_commands();
    esp_matter::console::factoryreset_register_commands();
    esp_matter::console::init();
#if CONFIG_ESP_MATTER_CONTROLLER_ENABLE
    esp_matter::console::controller_register_commands();
#endif // CONFIG_ESP_MATTER_CONTROLLER_ENABLE
#ifdef CONFIG_OPENTHREAD_BORDER_ROUTER
    esp_matter::console::otcli_register_commands();
#endif // CONFIG_OPENTHREAD_BORDER_ROUTER
#endif // CONFIG_ENABLE_CHIP_SHELL


    // Start Matter stack
    esp_err_t err = esp_matter::start(app_event_cb);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start Matter, err:%d", err);
        return;
    }
    ESP_LOGI(TAG, "Matter stack started");
}

void initialize_matter_controller(const uint64_t node_id, const uint64_t fabric_id, const uint16_t listen_port) {
    esp_matter::lock::chip_stack_lock(portMAX_DELAY);

    // Initialize the Matter controller client
    esp_err_t err = esp_matter::controller::matter_controller_client::get_instance().init(
        node_id, fabric_id, listen_port);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize Matter controller client");
    } else {
        ESP_LOGI(TAG, "Matter controller client initialized successfully");
    }

    // Set up commissioner
#ifdef CONFIG_ESP_MATTER_COMMISSIONER_ENABLE
    err = esp_matter::controller::matter_controller_client::get_instance().setup_commissioner();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to setup commissioner");
    } else {
        ESP_LOGI(TAG, "Commissioner setup successfully");
    }
#endif

    esp_matter::lock::chip_stack_unlock();
}
