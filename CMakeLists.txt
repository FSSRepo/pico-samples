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
	libs/st7735.cpp)

add_executable(ili9341-sample
	example/ili9341_sample.cpp)

add_executable(st7735-sample
	example/st7735_sample.cpp)

pico_enable_stdio_usb(ili9341-sample 1)
pico_enable_stdio_uart(ili9341-sample 0)

pico_add_extra_outputs(ili9341-sample)

pico_enable_stdio_usb(st7735-sample 1)
pico_enable_stdio_uart(st7735-sample 0)

pico_add_extra_outputs(st7735-sample)

target_link_libraries(pico-libs pico_stdlib hardware_i2c hardware_spi hardware_pwm)
target_link_libraries(ili9341-sample pico-libs pico_stdlib)
target_link_libraries(st7735-sample pico-libs pico_stdlib)