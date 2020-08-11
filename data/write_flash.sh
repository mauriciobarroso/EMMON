./mkspiffs -c spiffs/ -b 4096 -p 256 -s 0x200000 spiffs_image.bin
esptool.py -p /dev/ttyUSB0 -b 921600 write_flash 0xbf000 spiffs_image.bin
