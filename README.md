# Pico Samples

This repository contains a collection of libraries and code samples for the **Raspberry Pi Pico (RP2040)**. The goal is to provide easy-to-use drivers for various common peripherals, including OLED and TFT displays, 7-segment displays, servos, and more.

## 🚀 Project Content

### Drivers
The project includes C++ implementations for the following components:

- **OLED Displays:**
  - `SSD1306`: Driver for I2C OLED displays (128x64 and 128x32).
- **TFT Displays:**
  - `ILI9341`: Driver for 2.4" SPI TFT displays or similar.
  - `ST7735`: Driver for 1.8" SPI TFT displays or similar.
- **Peripherals:**
  - `Display7Segments`: Control for 7-segment displays using shift registers.
  - `ShiftRegister`: Generic driver for shift registers (e.g., 74HC595).
  - `Servo`: Servomotor control via PWM.
  - `PWM`: Library to simplify PWM usage on the Pico.
  - `Button` and `LED`: Simple classes for digital input and output handling.
- **Utilities:**
  - `SimpleSerial`: Simplified serial communication.

### Examples
In the `example/` folder, you will find ready-to-use implementations:
- `ili9341-sample`: Usage example for the ILI9341 display.
- `st7735-sample`: Usage example for the ST7735 display.
- `main.cpp`: An integrated example combining SSD1306, 7-segment display, buttons, and JSON communication.

## 🛠️ How to Build

This project uses **CMake** and the **Pico SDK**.

### Prerequisites
- Raspberry Pi Pico SDK installed.
- CMake and a compiler (such as GCC ARM Embedded).

### Building on Windows
You can use the provided `build.bat` script. **Note:** Make sure to adjust the `PICO_SDK_PATH` and `Python3_EXECUTABLE` paths in the file if necessary.

```batch
build.bat
```

Or manually:
1. Create a `build` folder.
2. Run CMake pointing to the SDK.
3. Compile with `nmake` or `make`.

## 📂 Repository Structure

- `include/`: Header files (.h).
- `libs/`: Driver implementations (.cpp).
- `example/`: Example code for specific targets.
- `src/`: Additional source code and external libraries.
- `serial/`: Serial communication utilities.

## 🔗 References

- [ST7735-Pico](https://github.com/bablokb/pic-st7735)
- [ILI9341-Pico](https://github.com/tvlad1234/pico-displayDrivs)

