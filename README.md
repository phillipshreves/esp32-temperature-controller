# esp32-temperature-controller
An ESP32 microcontroller temperature controller 

## Development

### Environment Setup

Run the `x-env-setup` script to set up the development environment. The `get_esp_idf` function it calls should be set(in .zshrc/your shell) to call `. YOUR-PATH-TO/esp-idf/export.sh`.

```
. ./x-env-setup.sh
```

This must be done before running any `idf.py` commands.

### ESP32 Special clangd Configuration

To use `clangd` with the ESP32 toolchain, you need to generate a `compile_commands.json` file that `clangd` can use for code completion and navigation. You can do this by reconfiguring your ESP-IDF project with the following command(also needs to be used whenever running reconfigure):

```
idf.py -B build -D IDF_TOOLCHAIN=clang reconfigure
```

### To Build

To build the project, run the `x-build.sh` script.

```
. ./x-build.sh
```

### To Flash

Run `idf.py -p PORT flash` to flash the firmware to your ESP32 device, replacing `PORT` with the appropriate serial port (e.g., `/dev/ttyUSB0` or `/dev/cu.usbserial-0001` or `COM3`).

