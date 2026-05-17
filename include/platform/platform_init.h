#ifndef PLATFORM_INIT_H
#define PLATFORM_INIT_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Initialize platform: Pico SDK, GPIOs, target in reset.
 * @return true if successful, false if failed
 */
typedef enum {
    ERR_NONE = 0,
    CLOCK_FAIL,
    RUN_FAIL,
    LED_FAIL,
} dolos_fault_code_t;

typedef struct {
    dolos_fault_code_t code;
    bool is_fatal;
} dolos_fault_t;

// Category B - Boot
#define ERR_NONE 0x0000
#define B001 0x0101 // Clock configuration failed
#define B002 0x0102 // GPIO initialization failed
#define B003 0x0103 // Timer initialization failed
#define B004 0x0104 // Buffer allocation failed

// Category S - SWD
#define S000 0x0200 // SWD mode activated
#define S001 0x0201 // Line reset failed
#define S002 0x0202 // JTAG-to-SWD sequence failed
#define S003 0x0203 // Target did not respond
#define S004 0x0204 // Invalid IDCODE
#define S101 0x0211 // Persistent ACK WAIT
#define S102 0x0212 // ACK FAULT
#define S103 0x0213 // Invalid ACK
#define S104 0x0214 // Parity error
#define S105 0x0215 // Transaction timeout
#define S106 0x0216 // Turnaround failed
#define S107 0x0217 // Protocol out of sync

// Category F - Fatal
#define F001 0x0901 // Internal assert
#define F002 0x0902 // Invalid FSM state
#define F003 0x0903 // Watchdog triggered
#define F004 0x0904 // Memory corruption detected
#define F005 0x0905 // Unrecoverable failure

// Category U - USB
#define U000 0x0300 // USB initialized
#define U001 0x0301 // USB controller did not initialize
#define U002 0x0302 // Enumeration failed
#define U003 0x0303 // Host connection timeout
#define U004 0x0304 // Endpoint not configured
#define U005 0x0305 // Transmission error
#define U006 0x0306 // Reception error

// Category T - Target Control
#define T000 0x0400 // Target in reset (RUN LOW)
#define T001 0x0401 // Target released (RUN HIGH)
#define T002 0x0402 // CPU halt executed
#define T003 0x0403 // Halt failed - CPU still running
#define T004 0x0404 // Target watchdog reset

// Category Q - QSPI/XIP
#define Q000 0x0500 // QSPI/XIP initialized
#define Q001 0x0501 // Failed to configure XIP registers
#define Q002 0x0502 // External flash did not respond
#define Q003 0x0503 // QSPI initialization timeout

// Category M - Memory
#define M000 0x0600 // Read started
#define M001 0x0601 // Invalid address
#define M002 0x0602 // Read failed
#define M003 0x0603 // Read outside permitted region
#define M004 0x0604 // Buffer overflow (FATAL)
#define M005 0x0605 // Invalid block size

// Category H - Hash/Integrity
#define H000 0x0700 // SHA-256 calculation started
#define H001 0x0701 // SHA-256 calculation failed
#define H002 0x0702 // Hash mismatch (FATAL)
#define H003 0x0703 // Data corrupted (FATAL)
#define H004 0x0704 // Intermediate checksum divergent

// Category C - Host Communication
#define C000 0x0800 // Communication channel ready
#define C001 0x0801 // Invalid command
#define C002 0x0802 // Invalid argument
#define C003 0x0803 // Unsupported command
#define C004 0x0804 // Invalid packet checksum
#define C005 0x0805 // Command timeout

// Category I - Operational States
#define I000 0x0A00 // Idle
#define I001 0x0A01 // Initializing
#define I002 0x0A02 // Awaiting host
#define I003 0x0A03 // Connecting to target
#define I004 0x0A04 // Reading IDCODE
#define I005 0x0A05 // Extracting firmware
#define I006 0x0A06 // Calculating hash
#define I007 0x0A07 // Sending data
#define I008 0x0A08 // Complete

/** @brief Turn on onboard LED. */
void platform_led_on(void);

/** @brief Turn off onboard LED. */
void platform_led_off(void);

/** @brief Blink the LED n times (status/error signaling).
 *  @param n number of blinks
 *  @param period_ms period in ms
 */
void platform_led_blink(uint8_t n, uint32_t period_ms);

#endif /* PLATFORM_INIT_H */