#ifndef WI_FI_STA_INTERFACE_H
#define WI_FI_STA_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Maximum number of Wi-Fi reconnection attempts. */
#define MAX_WIFI_CONNECT_RETRY 5

/** @brief Bit flag for successful Wi-Fi connection. */
#define WIFI_CONNECTED_BIT BIT0

/** @brief Bit flag for Wi-Fi connection failure. */
#define WIFI_FAIL_BIT BIT1


/**
 * @brief Initializes the Wi-Fi station mode.
 *
 * Configures the ESP32 as a Wi-Fi station (client), connects to the specified
 * Wi-Fi network, and handles connection events.
 *
 * @param ssid     The SSID of the Wi-Fi network.
 * @param password The password of the Wi-Fi network.
 */
void wifi_sta_init(const char *ssid, const char *password);

/**
 * @brief Deinitialize and clean up Wi-Fi resources.
 *
 * Stops the Wi-Fi driver, deinitializes the Wi-Fi stack, and releases allocated resources.
 */
void wifi_sta_deinit();

#ifdef __cplusplus
}
#endif

#endif // WI_FI_STA_INTERFACE_H


