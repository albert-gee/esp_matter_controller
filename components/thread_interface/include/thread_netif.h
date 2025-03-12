#ifndef THREAD_NETIF_H
#define THREAD_NETIF_H

#ifdef __cplusplus
extern "C" {
#endif

esp_netif_t *thread_netif_init(const esp_openthread_platform_config_t *ot_platform_config);

void thread_netif_deinit(esp_netif_t *thread_netif);

#ifdef __cplusplus
}
#endif

#endif //THREAD_NETIF_H
