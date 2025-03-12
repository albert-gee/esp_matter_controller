#include <esp_err.h>
#include <esp_matter_controller_pairing_command.h>

#include <matter_pairing.h>

esp_err_t pair_ble_thread(uint64_t node_id, uint32_t pincode, uint16_t disc, uint8_t *dataset_tlvs,
                             uint8_t dataset_len) {

    return esp_matter::controller::pairing_ble_thread(node_id, pincode, disc, dataset_tlvs, dataset_len);
}
