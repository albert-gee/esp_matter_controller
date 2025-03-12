#include "esp_err.h"
#include "esp_event.h"
#include "esp_event_base.h"
#include "esp_vfs_eventfd.h"
#include "esp_log.h"
#include <cstring>
#include <config.h>
#include <esp_openthread_lock.h>
#include <esp_openthread_types.h>
#include <esp_openthread.h>
#include <openthread/dataset.h>
#include "esp_netif.h"
#include "esp_openthread_cli.h"
#include "esp_openthread_netif_glue.h"
#include "openthread/logging.h"

#include "thread_interface.h"
#include "thread_events.h"
#include "thread_netif.h"

// Task handle for the OpenThread main task
static TaskHandle_t s_ot_task_handle = nullptr;

static esp_netif_t *thread_netif = nullptr;

// Binary semaphores for network attachment and DNS configuration
SemaphoreHandle_t s_sem_thread_attached = nullptr;
SemaphoreHandle_t s_sem_thread_set_dns = nullptr;

static auto TAG = "THREAD_INTERFACE";

void ot_task_worker(void* aContext) {

    ESP_LOGI(TAG, "Initializing OpenThread stack...");
    esp_openthread_platform_config_t ot_platform_config = ESP_OPENTHREAD_DEFAULT_CONFIG();
    if (esp_openthread_init(&ot_platform_config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize OpenThread stack!");
        esp_netif_destroy(thread_netif);
        return;
    }
    ESP_LOGI(TAG, "OpenThread stack initialized.");

#if CONFIG_OPENTHREAD_LOG_LEVEL_DYNAMIC
    // Set OpenThread log level to match ESP log level
    (void)otLoggingSetLevel(CONFIG_LOG_DEFAULT_LEVEL);
#endif

    // Initialize OpenThread Network Interface
    thread_netif = thread_netif_init(&ot_platform_config);
    if (thread_netif == nullptr) {
        ESP_LOGE(TAG, "Failed to create Thread Network Interface!");
        esp_openthread_deinit();
        return;
    }

#if CONFIG_OPENTHREAD_AUTOSTART
    ESP_LOGI(TAG, "Fetching OpenThread active dataset...");
    otOperationalDatasetTlvs dataset;
    otError error = otDatasetGetActiveTlvs(esp_openthread_get_instance(), &dataset);
    ESP_ERROR_CHECK(esp_openthread_auto_start((error == OT_ERROR_NONE ? &dataset : nullptr));
#endif

    // Start the OpenThread main loop
    // Node that this is a busy loop and does not return until the OpenThread stack is terminated
    ESP_LOGI(TAG, "Launching OpenThread main loop...");
    if (esp_openthread_launch_mainloop() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to launch OpenThread main loop!");
        return;
    }
    ESP_LOGI(TAG, "OpenThread main loop exited, cleaning up.");

    // Clean up
    esp_openthread_netif_glue_deinit();
    esp_netif_destroy(thread_netif);
    vTaskDelete(nullptr);
}

esp_err_t thread_interface_init() {
    ESP_LOGI(TAG, "Initializing Thread Interface ...");

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    constexpr esp_vfs_eventfd_config_t eventfd_config = {.max_fds = 3};
    ESP_ERROR_CHECK(esp_vfs_eventfd_register(&eventfd_config));

    // Create the task to initialize and run the OpenThread stack
    ESP_LOGI(TAG, "Creating OpenThread task worker");
    if (xTaskCreate(ot_task_worker, "ot_thread_main", CONFIG_THREAD_TASK_STACK_SIZE, nullptr, 5, &s_ot_task_handle) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create OpenThread task worker");
        return ESP_FAIL;
    }

    return ESP_OK;
}

void thread_interface_deinit() {

    // Deinitialize the Thread network interface
    if (thread_netif != nullptr) {
        ESP_LOGI(TAG, "Deinitializing Thread network interface...");
        thread_netif_deinit(thread_netif);
        thread_netif = nullptr;
    }

    // Stop the OpenThread task worker if it is running
    if (s_ot_task_handle != nullptr) {
        ESP_LOGI(TAG, "Deleting OpenThread task worker...");
        vTaskDelete(s_ot_task_handle);
        s_ot_task_handle = nullptr;
    }

    // Deinitialize the OpenThread stack
    ESP_LOGI(TAG, "Deinitializing OpenThread stack...");
    esp_openthread_deinit();

    esp_vfs_eventfd_unregister();
    esp_event_loop_delete_default();

    ESP_LOGI(TAG, "Thread Interface deinitialized successfully.");
}

// Note: The OpenThread APIs are not thread-safe. When calling OpenThread APIs from other tasks, make sure to hold the lock with :cpp:func:`esp_openthread_lock_acquire` and release the lock with :cpp:func:`esp_openthread_lock_release` afterwards.

otDeviceRole get_role() {
    return otThreadGetDeviceRole(esp_openthread_get_instance());
}

const char* get_role_name() {
    switch (get_role()) {
    case OT_DEVICE_ROLE_DISABLED:
        return "DISABLED";
    case OT_DEVICE_ROLE_DETACHED:
        return "DETACHED";
    case OT_DEVICE_ROLE_CHILD:
        return "CHILD";
    case OT_DEVICE_ROLE_ROUTER:
        return "ROUTER";
    case OT_DEVICE_ROLE_LEADER:
        return "LEADER";
    default:
        return "UNKNOWN";
    }
}

otError get_active_dataset(otOperationalDatasetTlvs *dataset) {
    return otDatasetGetActiveTlvs(esp_openthread_get_instance(), dataset);
}

otError get_active_dataset_data(char *buffer, size_t buffer_size) {
    otOperationalDatasetTlvs dataset;
    otError error = otDatasetGetActiveTlvs(esp_openthread_get_instance(), &dataset);
    if (error != OT_ERROR_NONE) {
        return error;
    }

    // Convert dataset bytes to hex string
    if (buffer_size < (2 * dataset.mLength + 1)) {
        return OT_ERROR_INVALID_ARGS;
    }

    for (uint8_t i = 0; i < dataset.mLength; i++) {
        sprintf(&buffer[i * 2], "%02X", dataset.mTlvs[i]);
    }
    buffer[dataset.mLength * 2] = '\0';

    return OT_ERROR_NONE;
}

otError set_active_dataset(const otOperationalDatasetTlvs *dataset) {
    return otDatasetSetActiveTlvs(esp_openthread_get_instance(), dataset);
}

otError set_active_dataset_data(const uint8_t *data, const uint8_t length) {
    if (length > OT_OPERATIONAL_DATASET_MAX_LENGTH) {
        return OT_ERROR_INVALID_ARGS;
    }

    // Copy dataset data
    otOperationalDatasetTlvs dataset;
    memcpy(dataset.mTlvs, data, length);
    dataset.mLength = length;

    return set_active_dataset(&dataset);
}
