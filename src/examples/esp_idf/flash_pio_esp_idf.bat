REM This requires python and the esp idf set up correctly
set PORT="COM3"
set FILE_DIR=./.pio/build/az-delivery-devkit-v4-esp-idf
python %IDF_PATH%/components/esptool_py/esptool/esptool.py --chip esp32 --port %PORT% --baud 921600 --before "default_reset" --after "hard_reset" write_flash -z --flash_mode "dio" --flash_freq "40m" --flash_size detect 0x1000 %FILE_DIR%/bootloader.bin 0x10000 %FILE_DIR%/firmware.bin 0x8000 %FILE_DIR%/partitions.bin 