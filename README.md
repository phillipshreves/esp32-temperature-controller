# esp32-temperature-controller
An ESP32 microcontroller temperature controller 

## ESP32 Special clangd Configuration

To use `clangd` with the ESP32 toolchain, you need to generate a `compile_commands.json` file that `clangd` can use for code completion and navigation. You can do this by reconfiguring your ESP-IDF project with the following command:

```
idf.py -B build -D IDF_TOOLCHAIN=clang reconfigure
```
