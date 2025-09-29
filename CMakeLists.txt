cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(pico-widgets)

# Initialize the SDK
pico_sdk_init()

include_directories(include)

add_executable(${PROJECT_NAME}
        src/shift_register.cpp
        src/ssd1306.cpp
        src/pwm_lib.cpp
        src/servo.cpp
        src/display7segments.cpp
        src/json11.cpp
        pantalla.cpp
        )

pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

pico_add_extra_outputs(${PROJECT_NAME})

# pull in common dependencies
target_link_libraries(${PROJECT_NAME} pico_stdlib hardware_i2c hardware_pwm hardware_adc pico_multicore)