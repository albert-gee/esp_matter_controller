idf_component_register(SRCS "src/wi_fi_sta_interface.cpp"
        INCLUDE_DIRS "include"
        REQUIRES esp_event esp_netif esp_wifi
)
