deps_config := \
	/home/zxd/esp32-A1S/esp-adf/esp-idf/components/app_trace/Kconfig \
	/home/zxd/esp32-A1S/esp-adf/esp-idf/components/aws_iot/Kconfig \
	/home/zxd/esp32-A1S/esp-adf/esp-idf/components/bt/Kconfig \
	/home/zxd/esp32-A1S/esp-adf/esp-idf/components/esp32/Kconfig \
	/home/zxd/esp32-A1S/esp-adf/esp-idf/components/esp_adc_cal/Kconfig \
	/home/zxd/esp32-A1S/esp-adf/components/esp_http_client/Kconfig \
	/home/zxd/esp32-A1S/esp-adf/esp-idf/components/ethernet/Kconfig \
	/home/zxd/esp32-A1S/esp-adf/esp-idf/components/fatfs/Kconfig \
	/home/zxd/esp32-A1S/esp-adf/esp-idf/components/freertos/Kconfig \
	/home/zxd/esp32-A1S/esp-adf/esp-idf/components/heap/Kconfig \
	/home/zxd/esp32-A1S/esp-adf/esp-idf/components/libsodium/Kconfig \
	/home/zxd/esp32-A1S/esp-adf/esp-idf/components/log/Kconfig \
	/home/zxd/esp32-A1S/esp-adf/esp-idf/components/lwip/Kconfig \
	/home/zxd/esp32-A1S/esp-adf/esp-idf/components/mbedtls/Kconfig \
	/home/zxd/esp32-A1S/esp-adf/esp-idf/components/openssl/Kconfig \
	/home/zxd/esp32-A1S/esp-adf/esp-idf/components/pthread/Kconfig \
	/home/zxd/esp32-A1S/esp-adf/esp-idf/components/spi_flash/Kconfig \
	/home/zxd/esp32-A1S/esp-adf/esp-idf/components/spiffs/Kconfig \
	/home/zxd/esp32-A1S/esp-adf/esp-idf/components/tcpip_adapter/Kconfig \
	/home/zxd/esp32-A1S/esp-adf/esp-idf/components/wear_levelling/Kconfig \
	/home/zxd/esp32-A1S/esp-adf/components/audio_hal/Kconfig.projbuild \
	/home/zxd/esp32-A1S/esp-adf/esp-idf/components/bootloader/Kconfig.projbuild \
	/home/zxd/esp32-A1S/esp-adf/esp-idf/components/esptool_py/Kconfig.projbuild \
	/home/zxd/esp32-A1S/esp-adf/esp-idf/components/partition_table/Kconfig.projbuild \
	/home/zxd/esp32-A1S/esp-adf/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
