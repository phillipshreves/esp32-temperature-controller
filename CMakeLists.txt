# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
# "Trim" the build. Include the minimal set of components, main, and anything it depends on.
idf_build_set_property(MINIMAL_BUILD ON)

# Set WiFi environment variables if defined
if(DEFINED ENV{ENV_WIFI_SSID})
	add_compile_definitions(ENV_WIFI_SSID=$ENV{ENV_WIFI_SSID})
    message(STATUS "SETTING ENV_WIFI_SSID TO : $ENV{ENV_WIFI_SSID}")
	add_compile_definitions(ENV_WIFI_PASSWORD=$ENV{ENV_WIFI_PASSWORD})
    message(STATUS "SETTING ENV_WIFI_PASSWORD TO : $ENV{ENV_WIFI_PASSWORD}")
endif()


project(esp32_temperature_controller)
