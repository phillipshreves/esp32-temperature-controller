# esp32-temperature-controller
An ESP32 microcontroller temperature controller 

## Development

### Environment Setup

Setup a function in your shell called `get_esp_idf`. This function should then call `. YOUR-PATH-TO/esp-idf/export.sh`.

```
get_esp_idf
```

This must be done before running any `idf.py` commands.

### ESP32 Special clangd Configuration

To use `clangd` with the ESP32 toolchain, you need to generate a `compile_commands.json` file that `clangd` can use for code completion and navigation. You can do this by reconfiguring your ESP-IDF project with the following command(also needs to be used whenever running reconfigure):

```
idf.py -D IDF_TOOLCHAIN=clang set-target esp32
```

Old method to configure with `clangd`, here for reference only:

```
idf.py -B build -D IDF_TOOLCHAIN=clang reconfigure
```

### To Build

Run `idf.py build` to build the project.

### To Flash

Run `idf.py flash` to flash the firmware to your ESP32 device, replacing `PORT` with the appropriate serial port (e.g., `/dev/ttyUSB0` or `/dev/cu.usbserial-0001` or `COM3`).

### To Monitor

Run `idf.py monitor` to view the serial output from the ESP32 device.


