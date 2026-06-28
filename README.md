# JoyCube (WUP-028 Edition)

JoyCube is an ultra-low latency custom GameCube controller firmware built on the **RP2040** microcontroller using the **Pico SDK** and the **TinyUSB** stack.

Designed specifically for the Nintendo Switch (and backward/forward compatibility), this project uses a Vendor-Specific USB implementation to mimic the official **Nintendo Wii U GameCube Controller Adapter (WUP-028)**. This allows the Switch to natively recognize the controller and utilize the **Analog Triggers**, which is absolutely vital for racing games like Grid Autosport.

## 🚀 Features

- **Native Switch GameCube Support**: Disguises itself as the official WUP-028 adapter (VID: `0x057E`, PID: `0x0337`).
- **Analog Triggers on Switch**: Perfectly reads and sends the L and R analog states so you can control your throttle and brake precisely in supported games.
- **1000Hz Hardware Polling**: Relies on the RP2040's native physical USB PHY for a zero-jitter, 1ms latency connection (no bit-banging!).
- **Unified Joycon Setup**: Simulates "Port 1" of the adapter.
- **6 Analog Axes & 12 Digital Buttons**: Handled by a single CD74HC4067 multiplexer and standard GPIO pins.

## 📦 Bill of Materials (BOM)

To build your own JoyCube, you will need the following hardware:

1. **Microcontroller**: An RP2040-based board.
   - *Recommended*: YD-RP2040, Waveshare RP2040-Zero, or the standard Raspberry Pi Pico. Boards with native USB-C are strongly preferred.
2. **Analog Multiplexer**: CD74HC4067 (16-Channel Analog Multiplexer Breakout Board). This is mandatory since the RP2040 only exposes 3 ADC pins and we need 6 analog inputs.
3. **Potentiometers/Triggers**:
   - 2x 2-Axis Joystick modules (for Main Stick and C-Stick).
   - 2x Linear Potentiometers (for L and R Analog triggers).
4. **Buttons/Switches**: 12x Tactile switches or mechanical keyboard switches for the digital inputs.
5. **Misc**: Wires, soldering iron, breadboard, and a USB-C data cable.

## 🔌 Wiring Guide

All digital buttons should be wired connecting the specified RP2040 GPIO pin directly to the switch, and the other side of the switch to **GND**. Internal Pull-Up resistors are enabled in the firmware.

### Digital Buttons

| Function | RP2040 Pin | Details |
| :--- | :--- | :--- |
| D-Pad Up | GP2 | Connects to GND when pressed |
| D-Pad Down | GP3 | Connects to GND when pressed |
| D-Pad Left | GP4 | Connects to GND when pressed |
| D-Pad Right | GP5 | Connects to GND when pressed |
| Button A | GP6 | Connects to GND when pressed |
| Button B | GP7 | Connects to GND when pressed |
| Button X | GP8 | Connects to GND when pressed |
| Button Y | GP9 | Connects to GND when pressed |
| Button Z | GP10 | Connects to GND when pressed |
| Start | GP11 | Connects to GND when pressed |
| L-Click (Digital) | GP12 | Connects to GND when pressed |
| R-Click (Digital) | GP13 | Connects to GND when pressed |

### CD74HC4067 Multiplexer

The multiplexer enables reading all 6 analog axes using just one ADC pin on the RP2040.

| MUX Pin | RP2040 Pin | Function |
| :--- | :--- | :--- |
| S0 | GP14 | Digital Output (Channel Select) |
| S1 | GP15 | Digital Output (Channel Select) |
| S2 | GP16 | Digital Output (Channel Select) |
| S3 | GP17 | Digital Output (Channel Select) |
| SIG / Z | GP26 (ADC0)| Analog Input |
| EN | GND | Always Enable (Active Low) |
| VCC | 3.3V | Power (Do NOT use 5V) |
| GND | GND | Ground |

**Analog Input Mapping on the Multiplexer:**
- **C0**: Main Stick X-Axis
- **C1**: Main Stick Y-Axis
- **C2**: C-Stick X-Axis
- **C3**: C-Stick Y-Axis
- **C4**: L-Trigger Analog
- **C5**: R-Trigger Analog

*Note: Connect the outer pins of the potentiometers to 3.3V and GND, and the middle wiper pin to the respective `C` channel on the multiplexer.*

## 🛠️ Build Instructions

### Prerequisites
- Install the [Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk).
- Install `CMake` and an ARM cross-compiler toolchain (e.g., `gcc-arm-none-eabi`).

### Compiling
1. Clone this repository or download the source files.
2. Ensure the `PICO_SDK_PATH` environment variable is set to your Pico SDK installation path.
3. Open a terminal in the project root and run:
   ```bash
   mkdir build
   cd build
   cmake ..
   make -j4
   ```
4. This will generate the `joycube.uf2` binary file inside the `build` directory.

## 🚀 Flashing the Firmware

1. Press and hold the **BOOT** (or BOOTSEL) button on your RP2040 board.
2. While holding the button, connect the board to your PC via USB.
3. The board will appear as a mass storage device named `RPI-RP2`.
4. Drag and drop the compiled `joycube.uf2` file into the `RPI-RP2` drive.
5. The board will automatically reboot.

## 🎮 Verification and Compatibility

### Nintendo Switch
Simply plug the controller via USB into the Switch Dock. It will immediately be recognized as a USB GameCube Controller, and the analog triggers will function natively in supported games.

### PC / Dolphin Emulator
Dolphin Emulator has native support for the WUP-028 adapter. Simply go to Controller settings -> GameCube Adapters -> "Wii U Adapter" and you're good to go.
For general Steam PC gaming, you will need a driver wrapper like **Zadig** + **Delfinovin** to translate the WUP-028 protocol into XInput.

## 🏗️ Architecture Overview

Unlike generic gamepads, this firmware ditches standard `HID` classes to implement a `Vendor-Specific` interface matching Nintendo's specifications. 
The device exposes two endpoints (IN and OUT) with a `wMaxPacketSize` of 37 bytes. 

The main loop polls `tud_vendor_read` to catch the `0x13` initialization command sent by the Switch. Once received, it fires 37-byte payload structures at 1000Hz via `tud_vendor_write` containing the state of all buttons and raw 8-bit analog values perfectly scaled and packed for the proprietary driver.
