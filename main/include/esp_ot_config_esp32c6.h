#pragma once

#include "esp_openthread_types.h"
#include <driver/uart.h>
#include <driver/gpio.h>
#include <esp_netif.h>

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
// Network Interface Configuration for OpenThread
// -----------------------------------------------------------------------------
#define OT_NETIF_INHERENT_CONFIG() {                         \
    .flags         = (esp_netif_flags_t)0,                   \
    .mac           = { 0 },                                  \
    .ip_info       = { 0 },                                  \
    .get_ip_event  = 0,                                      \
    .lost_ip_event = 0,                                      \
    .if_key        = "OT_DEF",                              \
    .if_desc       = "THREAD_NETIF",                         \
    .route_prio    = 15                                     \
}

/* Declare a static inherent configuration instance */
static const esp_netif_inherent_config_t ot_netif_inherent_config_instance = OT_NETIF_INHERENT_CONFIG();

#define OT_NETIF_CONFIG() {                                  \
    .base  = &ot_netif_inherent_config_instance,             \
    .stack = &g_esp_netif_netstack_default_openthread         \
}

// -----------------------------------------------------------------------------
// OpenThread Radio Default Configuration
// -----------------------------------------------------------------------------
#define ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG()              \
{                                                      \
.radio_mode = RADIO_MODE_NATIVE                    \
}

// -----------------------------------------------------------------------------
// OpenThread Host Default Configuration
// -----------------------------------------------------------------------------
#if CONFIG_OPENTHREAD_CONSOLE_TYPE_UART
#define ESP_OPENTHREAD_DEFAULT_HOST_CONFIG()                    \
    {                                                           \
        .host_connection_mode = HOST_CONNECTION_MODE_NONE,  \
    }
#elif CONFIG_OPENTHREAD_CONSOLE_TYPE_USB_SERIAL_JTAG
#define ESP_OPENTHREAD_DEFAULT_HOST_CONFIG()                        \
    {                                                               \
        .host_connection_mode = HOST_CONNECTION_MODE_CLI_USB,       \
        .host_usb_config = USB_SERIAL_JTAG_DRIVER_CONFIG_DEFAULT()    \
    }
#endif

// -----------------------------------------------------------------------------
// OpenThread Port Default Configuration
// -----------------------------------------------------------------------------
#define ESP_OPENTHREAD_DEFAULT_PORT_CONFIG()    \
    {                                           \
        .storage_partition_name = "nvs",        \
        .netif_queue_size = 10,                 \
        .task_queue_size = 10                   \
    }

// -----------------------------------------------------------------------------
// OpenThread Default Configuration
// -----------------------------------------------------------------------------
#define ESP_OPENTHREAD_DEFAULT_CONFIG()    \
    {                                           \
    .radio_config = ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG(),        \
    .host_config = ESP_OPENTHREAD_DEFAULT_HOST_CONFIG(),                 \
    .port_config = ESP_OPENTHREAD_DEFAULT_PORT_CONFIG()                  \
    }

#ifdef __cplusplus
}
#endif
