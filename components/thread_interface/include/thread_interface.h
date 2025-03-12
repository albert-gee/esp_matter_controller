#ifndef THREAD_INTERFACE_H
#define THREAD_INTERFACE_H

#include <openthread/thread.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize and start the Thread interface.
 *
 * This function initializes the ESP‑netif (lwIP) stack, creates the necessary synchronization semaphores,
 * registers event handlers, creates the OpenThread task, and then waits for critical events
 * (network attachment and DNS configuration) to ensure the Thread interface is fully initialized.
 *
 * @return ESP_OK if the initialization was successful; otherwise, an appropriate error code.
 */
esp_err_t thread_interface_init();

/**
 * @brief Shutdown the Thread interface.
 *
 * This function cleans up and releases all resources associated with the Thread interface,
 * including stopping the OpenThread task, destroying the network interface, unregistering
 * event file descriptors, and deleting synchronization semaphores.
 */
void thread_interface_deinit();

/**
 * @brief Get the current role of the Thread device.
 *
 * @return otThreadState The current role of the Thread device.
 */
otDeviceRole get_role();

/**
 * @brief Get the name of the current Thread role.
 *
 * @return const char* The name of the current Thread role.
 */
const char* get_role_name();

/**
 * @brief Retrieve the active operational dataset.
 *
 * @param[out] dataset Pointer to a otOperationalDatasetTlvs structure to hold the dataset.
 * @return otError OT_ERROR_NONE if successful; otherwise, an appropriate error.
 */
otError get_active_dataset(otOperationalDatasetTlvs *dataset);

/** * @brief Retrieve the active operational dataset data.
 *
 * @param[out] buffer Pointer to a buffer to hold the dataset data.
 * @param[in] buffer_size Size of the buffer.
 * @return otError OT_ERROR_NONE if successful; otherwise, an appropriate error.
 */
otError get_active_dataset_data(char *buffer, size_t buffer_size);

/**
 * @brief Set a new operational dataset.
 *
 * This function applies a new dataset to the Thread stack. The dataset typically includes
 * network parameters such as the network name, channel, PAN ID, extended PAN ID, network key, etc.
 *
 * @param[in] dataset Pointer to the dataset to apply.
 * @return otError OT_ERROR_NONE if successful; otherwise, an appropriate error.
 */
otError set_active_dataset(const otOperationalDatasetTlvs *dataset);

/**
 * @brief Set the active operational dataset data.
 *
 * Apply a new dataset to the Thread stack. The dataset typically includes network parameters such as the network name,
 * channel, PAN ID, extended PAN ID, network key, etc.
 *
 * @param[in] data Pointer to the dataset data to apply.
 * @param[in] length Length of the dataset data.
 * @return otError OT_ERROR_NONE if successful; otherwise, an appropriate error.
 */
otError set_active_dataset_data(const uint8_t *data, uint8_t length);


#ifdef __cplusplus
}
#endif

#endif //THREAD_INTERFACE_H
