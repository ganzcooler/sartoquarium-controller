# Project: sartoquarium-controller

## Project Overview

This project is an Arduino-based temperature controller for "Sartoquarium". It uses a PID algorithm to precisely regulate the temperature of a water tank by controlling a heating mat via a Solid State Relay (SSR).

The system utilizes two DS18B20 temperature sensors:
1.  **Control Sensor (Index 0):** Measures the water temperature, which is used as the input for the PID controller.
2.  **Safety Sensor (Index 1):** Is attached directly to the heating mat to prevent overheating. If its temperature exceeds 60°C, the controller shuts down the heating mat as a safety measure.

The target temperature (setpoint) is initially set to 20°C and can be adjusted in real-time via the serial interface.

**Key Technologies:**
*   Arduino / C++
*   PID Control Algorithm
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
    *   Make sure the `PID_v1.h` file is in the same directory as your `.ino` file. The IDE will include it as a local library.
    *   Install the required external libraries from the Library Manager:
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
        lib_ldf_mode = chain+
        lib_dir = .
        ```
    *   You could then build and upload using the PlatformIO CLI or the VS Code extension.

## Usage

*   After uploading, open the Serial Monitor at 9600 baud.
*   The controller will automatically start regulating the temperature to the default setpoint of 20°C.
*   **To change the setpoint:** Send the letter 'S' followed by the desired temperature. For example, to set the temperature to 25.5°C, send `S25.5`.
*   **Monitoring:** Every 2 seconds, the controller will print a status line to the serial monitor, for example:
    `Soll: 25.50 C, Ist: 24.87 C, T_sicher: 45.12 C, Leistung: 75.34 %`

## Key Features & Conventions

*   **PID Control:** The core of the project is a PID controller that calculates the required heating power based on the difference between the setpoint and the current temperature. The PID parameters (`Kp`, `Ki`, `Kd`) have been tuned for a system with high thermal inertia (like 10L of water).
*   **Non-Blocking Code:** The main loop (`loop()`) is written in a non-blocking style to handle serial input and temperature readings concurrently without delays.
*   **Time-Proportional SSR Control:** The power to the SSR is modulated using a time-proportional approach within a 2-second cycle, providing finer control than a simple on/off logic.
*   **Safety First:** A dedicated safety sensor and logic prevent the heating mat from overheating. The controller also halts heating if the main control sensor fails.