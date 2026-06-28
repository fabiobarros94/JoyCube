# JoyCube Hardware Design & Netlist

This document serves as a guide to help the community design a Printed Circuit Board (PCB) (e.g., ordering from JLCPCB or PCBWay) for the JoyCube project. It contains the complete wiring table (Netlist) and layout best practices.

If you use software like **KiCad** or **EasyEDA**, you only need to add the components below to your schematic and connect them exactly as described in the tables. The routing (drawing the copper traces) will be up to your physical design choices.

## 1. Required Components (BOM for PCB)
- **1x Microcontroller:** Raspberry Pi Pico (Standard) or *Waveshare RP2040-Zero* (if you want to solder it as an SMD module).
- **1x Multiplexer IC:** CD74HC4067 (Can be a breakout board module or the bare SMD chip `SSOP-24` / `SOIC-24` soldered directly).
- **Buttons (12x):** Mechanical or tactile switches (e.g., Cherry MX, Kailh, Omron).
- **Analog Sensors (6x axes):** 
  - 2x 3D Joystick modules (X and Y axes).
  - 2x Linear slide potentiometers (For the L and R analog triggers).
- *(Optional)*: JST connectors or pin headers to connect the analog components modularly.

## 2. Logical Schematic (Netlist)

Below is the exact connection map. In your CAD software, simply connect **Column A** to **Column B**.

### Power & GND
| Column A (Source) | Column B (Destination) | Notes |
| :--- | :--- | :--- |
| RP2040 `3V3 (Out)` | CD74HC4067 `VCC` | Powers the multiplexer IC. NEVER use 5V (VBUS). |
| RP2040 `3V3 (Out)` | Potentiometers (Top Pin) | Powers the analog sensors. |
| RP2040 `GND` | CD74HC4067 `GND` and `EN` | The EN (Enable) pin of the MUX must be grounded (active LOW). |
| RP2040 `GND` | All 12 Buttons (Pin 2) | Completes the circuit for digital buttons. |
| RP2040 `GND` | Potentiometers (Bottom Pin) | Ground reference for the analog sensors. |

### Digital Logic (Buttons - Close to GND)
*The buttons do not require external resistors in the schematic; the RP2040 will use internal Pull-ups.*
| Button (Pin 1) | RP2040 Pin |
| :--- | :--- |
| D-Pad Up | GP2 |
| D-Pad Down | GP3 |
| D-Pad Left | GP4 |
| D-Pad Right| GP5 |
| Button A | GP6 |
| Button B | GP7 |
| Button X | GP8 |
| Button Y | GP9 |
| Button Z | GP10 |
| Start | GP11 |
| L-Click (Trigger Bottom) | GP12 |
| R-Click (Trigger Bottom) | GP13 |

### Multiplexer Logic (Control and Signal)
| CD74HC4067 Pin | RP2040 Pin |
| :--- | :--- |
| `S0` | GP14 |
| `S1` | GP15 |
| `S2` | GP16 |
| `S3` | GP17 |
| `SIG` (Z) | GP26 (ADC0) |

### Analog Inputs (From Potentiometers)
The center pin (Wiper) of each potentiometer must go to the following ports on the CD74HC4067:
| Analog Signal (Center Pin) | CD74HC4067 Pin |
| :--- | :--- |
| Main Stick - X Axis | `C0` |
| Main Stick - Y Axis | `C1` |
| C-Stick - X Axis | `C2` |
| C-Stick - Y Axis | `C3` |
| L-Trigger Analog | `C4` |
| R-Trigger Analog | `C5` |

*(Ports C6 to C15 on the CD74HC4067 should remain unconnected).*

---

## 3. Quick Guide for EasyEDA (JLCPCB)

1. Create a new project on the [EasyEDA](https://easyeda.com/) website.
2. In the schematic editor, search for the parts: "Raspberry Pi Pico" and "CD74HC4067" (prefer breakout modules if you lack experience with tiny SMD soldering).
3. Draw wires connecting the pins according to the Netlist tables above.
4. Click on "Design -> Update PCB".
5. In the PCB view, draw the board outline for your controller.
6. Position the buttons where your fingers will comfortably reach (you can measure or use Joycon templates from the internet).
7. Route the traces with a width of at least `0.254mm` for signals and `0.5mm` for VCC/GND.
8. Generate the Gerber files and submit them to a manufacturer's website (JLCPCB, PCBWay, etc.).
