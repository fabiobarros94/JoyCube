#ifndef USB_DESCRIPTORS_H_
#define USB_DESCRIPTORS_H_

#include <stdint.h>

// Nintendo WUP-028 GameCube Adapter Constants
#define VENDOR_ID_NINTENDO  0x057e
#define PRODUCT_ID_WUP028   0x0337

// Controller connection states
#define GC_STATUS_DISCONNECTED 0x00
#define GC_STATUS_CONNECTED    0x10 // Standard wired controller

// GameCube Controller Report (1 port = 9 bytes)
typedef struct __attribute__ ((packed)) {
    uint8_t status;      // 0x10 = connected, 0x00 = disconnected
    uint8_t buttons1;    // Bit 0: A, 1: B, 2: X, 3: Y, 4: D-Left, 5: D-Right, 6: D-Down, 7: D-Up
    uint8_t buttons2;    // Bit 0: Start, 1: Z, 2: R, 3: L, 4: Unused, 5: Unused, 6: Unused, 7: Unused
    uint8_t stick_x;     // 0 - 255 (Center = 128)
    uint8_t stick_y;     // 0 - 255 (Center = 128)
    uint8_t c_stick_x;   // 0 - 255 (Center = 128)
    uint8_t c_stick_y;   // 0 - 255 (Center = 128)
    uint8_t l_analog;    // 0 - 255 (0 = released, 255 = fully pressed)
    uint8_t r_analog;    // 0 - 255 (0 = released, 255 = fully pressed)
} wup_port_status_t;

// Full WUP-028 USB Report (37 bytes)
typedef struct __attribute__ ((packed)) {
    uint8_t command;             // Always 0x21 when reporting buttons
    wup_port_status_t port[4];   // Status for all 4 adapter ports
} wup_028_report_t;

#endif /* USB_DESCRIPTORS_H_ */
