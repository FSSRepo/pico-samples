cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(pico-samples)

# Initialize the SDK
pico_sdk_init()

include_directories(include)

add_executable(${PROJECT_NAME}
	libs/display7segments.cpp
	# libs/ili9341.cpp
	libs/servo.cpp
	libs/ssd1306.cpp
	libs/shift_register.cpp
	libs/st7735.cpp
	ili9341_sample.cpp)

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

pico_add_extra_outputs(${PROJECT_NAME})

# pull in common dependencies
target_link_libraries(${PROJECT_NAME} pico_stdlib hardware_i2c hardware_spi hardware_pwm hardware_adc pico_multicore)