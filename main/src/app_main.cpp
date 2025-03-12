#include <esp_log.h>
#include <nvs_flash.h>

#if CONFIG_OPENTHREAD_BORDER_ROUTER
#include <esp_ot_config.h>
#include <platform/ESP32/OpenthreadLauncher.h>
#endif // CONFIG_OPENTHREAD_BORDER_ROUTER

#include <cstring>
#include <matter_interface.h>
#include <wi_fi_sta_interface.h>

#include <thread_util.h>

#include <matter_pairing.h>

#define MATTER_CONTROLLER_NODE_ID 1234
#define MATTER_CONTROLLER_FABRIC_ID 1
#define MATTER_CONTROLLER_LISTEN_PORT 5580

static const char *TAG = "app_main";

void hex_string_to_bytes(const char *hex_string, uint8_t *byte_array, size_t byte_array_len) {
    size_t hex_len = strlen(hex_string);
    for (size_t i = 0; i < hex_len / 2 && i < byte_array_len; i++) {
        sscanf(&hex_string[i * 2], "%2hhx", &byte_array[i]);
    }
}

extern "C" void app_main()
{
    // Initialize the ESP NVS layer
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

#ifdef CONFIG_OPENTHREAD_BORDER_ROUTER
#ifdef CONFIG_AUTO_UPDATE_RCP
    esp_vfs_spiffs_conf_t rcp_fw_conf = {
        .base_path = "/rcp_fw", .partition_label = "rcp_fw", .max_files = 10, .format_if_mount_failed = false};
    if (ESP_OK != esp_vfs_spiffs_register(&rcp_fw_conf)) {
        ESP_LOGE(TAG, "Failed to mount rcp firmware storage");
        return;
    }
    esp_rcp_update_config_t rcp_update_config = ESP_OPENTHREAD_RCP_UPDATE_CONFIG();
    openthread_init_br_rcp(&rcp_update_config);
#endif
    /* Set OpenThread platform config */
    esp_openthread_platform_config_t config = ESP_OPENTHREAD_DEFAULT_CONFIG();
    set_openthread_platform_config(&config);
#endif // CONFIG_OPENTHREAD_BORDER_ROUTER

    // Start Matter stack
    start_matter_interface();

    initialize_matter_controller(MATTER_CONTROLLER_NODE_ID, MATTER_CONTROLLER_FABRIC_ID, MATTER_CONTROLLER_LISTEN_PORT);

    // Connect to Wi-Fi
    wifi_sta_init("SkyNet_Guest", "password147");

    // Add a delay of 5000 milliseconds (5 seconds)
    vTaskDelay(pdMS_TO_TICKS(5000));
    //
    // wifi_sta_deinit();

    // Initialize Thread network
    // vTaskDelay(pdMS_TO_TICKS(10000));
    const char *dataset = thread_dataset_init_new();
    if (dataset) {
        printf("Generated Dataset TLVs: %s\n", dataset);
    } else {
        printf("Failed to generate dataset TLVs.\n");
    }
    vTaskDelay(pdMS_TO_TICKS(3000));

    ifconfig_up();
    vTaskDelay(pdMS_TO_TICKS(3000));

    thread_start();
    vTaskDelay(pdMS_TO_TICKS(3000));

    // Commission Thread device
    uint64_t relay_node_id = 0x1122;
    uint32_t relay_pincode = 20202021;
    uint16_t relay_disc = 3840;

    // Convert dataset TLVs hex string to byte array
    size_t dataset_len = strlen(dataset) / 2;
    uint8_t dataset_tlvs[dataset_len];
    hex_string_to_bytes(dataset, dataset_tlvs, dataset_len);

    // Call the function
    esp_err_t result = pair_ble_thread(relay_node_id, relay_pincode, relay_disc, dataset_tlvs, dataset_len);
    if (result == ESP_OK) {
        printf("Pairing successful\n");
    } else {
        printf("Pairing failed with error: %d\n", result);
    }

    // Commission Thread devices through Thread Border Router

    // Control Matter Relay device
}
