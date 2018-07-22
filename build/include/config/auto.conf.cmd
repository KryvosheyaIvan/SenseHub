deps_config := \
	/esp/esp-idf/components/app_trace/Kconfig \
	/esp/esp-idf/components/aws_iot/Kconfig \
	/esp/esp-idf/components/bt/Kconfig \
	/esp/esp-idf/components/driver/Kconfig \
	/esp/esp-idf/components/esp32/Kconfig \
	/esp/esp-idf/components/esp_adc_cal/Kconfig \
	/esp/esp-idf/components/esp_http_client/Kconfig \
	/esp/esp-idf/components/ethernet/Kconfig \
	/esp/esp-idf/components/fatfs/Kconfig \
	/esp/esp-idf/components/freertos/Kconfig \
	/esp/esp-idf/components/heap/Kconfig \
	/esp/esp-idf/components/libsodium/Kconfig \
	/esp/esp-idf/components/log/Kconfig \
	/esp/esp-idf/components/lwip/Kconfig \
	/esp/esp-idf/components/mbedtls/Kconfig \
	/esp/esp-idf/components/mdns/Kconfig \
	/esp/esp-idf/components/openssl/Kconfig \
	/esp/esp-idf/components/pthread/Kconfig \
	/esp/esp-idf/components/spi_flash/Kconfig \
	/esp/esp-idf/components/spiffs/Kconfig \
	/esp/esp-idf/components/tcpip_adapter/Kconfig \
	/esp/esp-idf/components/vfs/Kconfig \
	/esp/esp-idf/components/wear_levelling/Kconfig \
	/esp/esp-idf/Kconfig.compiler \
	/esp/esp-idf/components/bootloader/Kconfig.projbuild \
	/esp/esp-idf/components/esptool_py/Kconfig.projbuild \
	/esp/esp-idf/components/partition_table/Kconfig.projbuild \
	/esp/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
