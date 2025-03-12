#ifndef MATTER_PAIRING_H
#define MATTER_PAIRING_H

#include <esp_err.h>
#include <stdint.h>

esp_err_t pair_ble_thread(uint64_t node_id, uint32_t pincode, uint16_t disc, uint8_t *dataset_tlvs,
                             uint8_t dataset_len);

#endif //MATTER_PAIRING_H
