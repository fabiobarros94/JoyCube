#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "bsp/board.h"
#include "tusb.h"

#include "usb_descriptors.h"

// Pin Mapping - Digital Buttons
#define BTN_UP      2
#define BTN_DOWN    3
#define BTN_LEFT    4
#define BTN_RIGHT   5
#define BTN_A       6
#define BTN_B       7
#define BTN_X       8
#define BTN_Y       9
#define BTN_Z       10
#define BTN_START   11
#define BTN_L_CLICK 12
#define BTN_R_CLICK 13

// Pin Mapping - Multiplexer (CD74HC4067)
#define MUX_S0      14
#define MUX_S1      15
#define MUX_S2      16
#define MUX_S3      17
#define MUX_SIG     26  // ADC0

// Button array for easy initialization
const uint8_t buttons[] = {
    BTN_UP, BTN_DOWN, BTN_LEFT, BTN_RIGHT,
    BTN_A, BTN_B, BTN_X, BTN_Y, BTN_Z,
    BTN_START, BTN_L_CLICK, BTN_R_CLICK
};
#define NUM_BUTTONS (sizeof(buttons)/sizeof(buttons[0]))

// Global State Variables for WUP-028 Protocol
static bool wup_polling_enabled = false;
static uint32_t last_report_time_ms = 0;

// Function to initialize hardware pins
void board_hardware_init(void) {
    // Configure digital buttons as input with pull-up resistors
    for (int i = 0; i < NUM_BUTTONS; i++) {
        gpio_init(buttons[i]);
        gpio_set_dir(buttons[i], GPIO_IN);
        gpio_pull_up(buttons[i]);
    }

    // Configure multiplexer select pins
    uint8_t mux_pins[] = {MUX_S0, MUX_S1, MUX_S2, MUX_S3};
    for (int i = 0; i < 4; i++) {
        gpio_init(mux_pins[i]);
        gpio_set_dir(mux_pins[i], GPIO_OUT);
        gpio_put(mux_pins[i], 0);
    }

    // Configure the ADC
    adc_init();
    adc_gpio_init(MUX_SIG);
    adc_select_input(0); // Select ADC0 (GP26)
}

// Function to select the multiplexer channel (0 to 15)
void select_mux_channel(uint8_t channel) {
    gpio_put(MUX_S0, (channel & 0x01) ? 1 : 0);
    gpio_put(MUX_S1, (channel & 0x02) ? 1 : 0);
    gpio_put(MUX_S2, (channel & 0x04) ? 1 : 0);
    gpio_put(MUX_S3, (channel & 0x08) ? 1 : 0);
    
    // Small delay to allow the analog signal to settle
    sleep_us(2);
}

// Read the ADC and scale the 12-bit resolution (0-4095) down to 8-bits (0-255)
// The native GameCube protocol strictly uses 8-bits per axis.
uint8_t read_analog_8bit(void) {
    uint16_t raw_adc = adc_read();
    return (uint8_t)(raw_adc >> 4); // Divide by 16
}

// Process data received from Switch/PC (Adapter Commands)
void process_vendor_commands(void) {
    if (tud_vendor_available()) {
        uint8_t buf[37];
        uint32_t count = tud_vendor_read(buf, sizeof(buf));
        
        if (count > 0) {
            uint8_t cmd = buf[0];
            if (cmd == 0x13) {
                // Command 0x13: Start polling
                wup_polling_enabled = true;
            }
            if (cmd == 0x11) {
                // Command 0x11: Rumble command
                // For 4 controllers, the following bytes control the rumble motors.
                // Since we don't have rumble hardware, we can silently ignore this.
            }
        }
    }
}

// Send the GameCube controller status
void wup_task(void) {
    // 1000Hz polling -> Send every 1ms
    if ( board_millis() - last_report_time_ms < 1) return; 

    // If the console/host hasn't sent the start polling command yet, do not send status.
    if (!wup_polling_enabled) return;

    if (!tud_vendor_write_available()) return;

    last_report_time_ms = board_millis();

    wup_028_report_t report;
    memset(&report, 0, sizeof(report));

    report.command = 0x21; // Controller status response

    // Initialize all 4 ports (Ports 2, 3 and 4 remain disconnected - 0x00)
    for (int i = 0; i < 4; i++) {
        report.port[i].status = GC_STATUS_DISCONNECTED;
        report.port[i].stick_x = 128;   // Neutral position
        report.port[i].stick_y = 128;
        report.port[i].c_stick_x = 128;
        report.port[i].c_stick_y = 128;
    }

    // Configure only Controller 1 (Port 0)
    report.port[0].status = GC_STATUS_CONNECTED;

    // Read Buttons
    if (!gpio_get(BTN_A))       report.port[0].buttons1 |= (1 << 0);
    if (!gpio_get(BTN_B))       report.port[0].buttons1 |= (1 << 1);
    if (!gpio_get(BTN_X))       report.port[0].buttons1 |= (1 << 2);
    if (!gpio_get(BTN_Y))       report.port[0].buttons1 |= (1 << 3);
    if (!gpio_get(BTN_LEFT))    report.port[0].buttons1 |= (1 << 4);
    if (!gpio_get(BTN_RIGHT))   report.port[0].buttons1 |= (1 << 5);
    if (!gpio_get(BTN_DOWN))    report.port[0].buttons1 |= (1 << 6);
    if (!gpio_get(BTN_UP))      report.port[0].buttons1 |= (1 << 7);

    if (!gpio_get(BTN_START))   report.port[0].buttons2 |= (1 << 0);
    if (!gpio_get(BTN_Z))       report.port[0].buttons2 |= (1 << 1);
    if (!gpio_get(BTN_R_CLICK)) report.port[0].buttons2 |= (1 << 2);
    if (!gpio_get(BTN_L_CLICK)) report.port[0].buttons2 |= (1 << 3);

    // Read Analog Axes via Multiplexer
    select_mux_channel(0); report.port[0].stick_x = read_analog_8bit();
    select_mux_channel(1); report.port[0].stick_y = read_analog_8bit();
    select_mux_channel(2); report.port[0].c_stick_x = read_analog_8bit();
    select_mux_channel(3); report.port[0].c_stick_y = read_analog_8bit();
    select_mux_channel(4); report.port[0].l_analog = read_analog_8bit();
    select_mux_channel(5); report.port[0].r_analog = read_analog_8bit();

    // Send the 37-byte report via the IN endpoint (Vendor interface)
    tud_vendor_write(&report, sizeof(report));
    tud_vendor_flush(); // Ensure immediate dispatch
}

int main(void) {
    board_init();
    board_hardware_init();
    tusb_init();

    while (1) {
        tud_task(); // TinyUSB device task
        process_vendor_commands();
        wup_task(); // Main polling and reporting logic
    }

    return 0;
}
