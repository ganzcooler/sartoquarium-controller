# Project: sartoquarium-controller

## Project Overview

This project is an Arduino-based controller for "Sartoquarium". It is designed to regulate a heating mat using a Solid State Relay (SSR). The temperature is monitored by two DS18B20 temperature sensors. The power output to the heating mat can be controlled by sending a value from 0-100 over the serial interface.

The controller operates on a time-proportional basis to modulate the power to the SSR. It reads temperatures from the sensors in a non-blocking manner.

**Key Technologies:**
*   Arduino / C++
*   OneWire protocol
*   DallasTemperature library

**Hardware Components:**
*   Arduino-compatible microcontroller
*   Solid State Relay (SSR)
*   2x DS18B20 temperature sensors

## Building and Running

This is an Arduino project. To build and upload the code to a microcontroller, you will need:

1.  **Arduino IDE:**
    *   Open the `ssr-heizmatte-ds18b20.ino` file in the Arduino IDE.
    *   Install the required libraries:
        *   `OneWire` by Paul Stoffregen
        *   `DallasTemperature` by Miles Burton and others
    *   Select your board and port from the `Tools` menu.
    *   Click the "Upload" button.

2.  **PlatformIO (alternative):**
    *   If you prefer a command-line or VS Code-based workflow, you can set up a `platformio.ini` file for this project.
    *   A basic `platformio.ini` would look like this:
        ```ini
        [env:uno]
        platform = atmelavr
        board = uno
        framework = arduino
        lib_deps =
            paulstoffregen/OneWire
            milesburton/DallasTemperature
        ```
    *   You could then build and upload using the PlatformIO CLI or the VS Code extension.

**Usage:**
*   After uploading, open the Serial Monitor at 9600 baud.
*   Send a number between 0 and 100 (followed by a newline) to set the power percentage for the SSR.
*   The controller will print the current temperature readings from the two sensors.

## Development Conventions

*   The code is written in C++ for the Arduino framework.
*   Pin definitions are clearly laid out at the top of the file.
*   The main loop (`loop()`) is written in a non-blocking style to handle serial input and temperature readings concurrently.
*   Error handling is present for sensor connection issues.
