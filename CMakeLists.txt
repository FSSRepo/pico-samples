cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(pico-samples)

# Initialize the SDK
pico_sdk_init()

include_directories(include)

add_library(pico-libs
	STATIC
	libs/display7segments.cpp
	libs/ili9341.cpp
	libs/servo.cpp
	libs/ssd1306.cpp
	libs/shift_register.cpp
	libs/st7735.cpp
	libs/SHT31.cpp
	src/pwm_lib.cpp)

add_executable(ili9341-sample
	example/ili9341_sample.cpp)

add_executable(st7735-sample
	example/st7735_sample.cpp)

add_executable(hc05-bluetooth-sample
	example/hc05_bluetooth_sample.cpp)

add_executable(esp12f-wifi-sample
	example/esp12f_wifi_sample.cpp)

add_executable(water-pump-sht31-st7735-sample
	example/water_pump_sht31_st7735_sample.cpp)

add_executable(water-pump-st7735-bluetooth-state-sample
	example/water_pump_st7735_bluetooth_state_sample.cpp)

add_executable(sht31-sample
	example/sht31_sample.cpp)

add_executable(blink-led-sample
	example/blink_led_sample.cpp)

pico_enable_stdio_usb(ili9341-sample 1)
pico_enable_stdio_uart(ili9341-sample 0)

pico_add_extra_outputs(ili9341-sample)

pico_enable_stdio_usb(st7735-sample 1)
pico_enable_stdio_uart(st7735-sample 0)

pico_add_extra_outputs(st7735-sample)

pico_enable_stdio_usb(hc05-bluetooth-sample 1)
pico_enable_stdio_uart(hc05-bluetooth-sample 0)

pico_add_extra_outputs(hc05-bluetooth-sample)

pico_enable_stdio_usb(esp12f-wifi-sample 1)
pico_enable_stdio_uart(esp12f-wifi-sample 0)

pico_add_extra_outputs(esp12f-wifi-sample)

pico_enable_stdio_usb(water-pump-sht31-st7735-sample 1)
pico_enable_stdio_uart(water-pump-sht31-st7735-sample 0)

pico_add_extra_outputs(water-pump-sht31-st7735-sample)

pico_enable_stdio_usb(water-pump-st7735-bluetooth-state-sample 1)
pico_enable_stdio_uart(water-pump-st7735-bluetooth-state-sample 0)

pico_add_extra_outputs(water-pump-st7735-bluetooth-state-sample)

pico_enable_stdio_usb(sht31-sample 1)
pico_enable_stdio_uart(sht31-sample 0)

pico_add_extra_outputs(sht31-sample)

pico_enable_stdio_usb(blink-led-sample 1)
pico_enable_stdio_uart(blink-led-sample 0)

pico_add_extra_outputs(blink-led-sample)

target_link_libraries(pico-libs pico_stdlib hardware_i2c hardware_spi hardware_pwm)
target_link_libraries(ili9341-sample pico-libs pico_stdlib)
target_link_libraries(st7735-sample pico-libs pico_stdlib)
target_link_libraries(hc05-bluetooth-sample pico_stdlib hardware_uart)
target_link_libraries(esp12f-wifi-sample pico_stdlib hardware_uart)
target_link_libraries(water-pump-sht31-st7735-sample pico-libs pico_stdlib hardware_uart)
target_link_libraries(water-pump-st7735-bluetooth-state-sample pico-libs pico_stdlib hardware_uart)
target_link_libraries(sht31-sample pico-libs pico_stdlib hardware_i2c)
target_link_libraries(blink-led-sample pico_stdlib)