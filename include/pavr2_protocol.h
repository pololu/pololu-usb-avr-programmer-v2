// This file defines the constants needed to communicate with the native
// USB interfaces of these devices:
//
// - Pololu USB AVR Programmer v2 (pgm04a)
// - Pololu USB AVR Programmer v2.1 (pgm04b)

#ifndef _PAVR2_NATIVE_USB_PROTOCOL_H
#define _PAVR2_NATIVE_USB_PROTOCOL_H

#define PAVR2_USB_VENDOR_ID 0x1FFB

// USB product ID for pgm04a, Pololu USB AVR Programmer v2
#define PAVR2_USB_PRODUCT_ID_V2 0x00B0

// USB product ID for pgm04b, Pololu USB AVR Programmer v2.1
#define PAVR2_USB_PRODUCT_ID_V2_1 0x00BB

/* To get the value of a setting from the device, make a request like this:
 *   bRequestType = 0xC0
 *   bRequest = PAVR2_REQUEST_GET_SETTING
 *   wValue = 0
 *   wIndex = one of the PAVR2_SETTING_* defines below
 *   wLength = length of the setting, in bytes */
#define PAVR2_REQUEST_GET_SETTING 0x81

/* To set the value of a setting from the device, make a request like this:
 *   bRequestType = 0x40
 *   bRequest = REQUEST_SET_SETTING
 *   wValue = value of setting
 *   wIndex = one of the PAVR2_SETTING_* defines below
 *   wLength = 0 */
#define PAVR2_REQUEST_SET_SETTING 0x82

#define PAVR2_SETTING_NOT_INITIALIZED 0
#define PAVR2_SETTING_SCK_DURATION 1
#define PAVR2_SETTING_ISP_FASTEST_PERIOD 2
#define PAVR2_SETTING_REGULATOR_MODE 3
#define PAVR2_SETTING_VCC_OUTPUT_ENABLED 4
#define PAVR2_SETTING_VCC_OUTPUT_INDICATOR 5
#define PAVR2_SETTING_LINE_A_FUNCTION 6
#define PAVR2_SETTING_LINE_B_FUNCTION 7
#define PAVR2_SETTING_SOFTWARE_VERSION_MAJOR 8
#define PAVR2_SETTING_SOFTWARE_VERSION_MINOR 9
#define PAVR2_SETTING_HARDWARE_VERSION 10
#define PAVR2_SETTING_RESET_POLARITY 11
#define PAVR2_SETTING_VCC_VDD_MAX_RANGE 12
#define PAVR2_SETTING_VCC_3V3_MIN 13
#define PAVR2_SETTING_VCC_3V3_MAX 14
#define PAVR2_SETTING_VCC_5V_MIN 15
#define PAVR2_SETTING_VCC_5V_MAX 16

/* To get the value of a variable from the device, make a request like this:
 *   bRequestType = 0xC0
 *   bRequest = PAVR2_REQUEST_GET_VARIABLE
 *   wValue = 0
 *   wIndex = one of the PAVR2_SETTING_* defines below
 *   wLength = length of the variable, in bytes */
#define PAVR2_REQUEST_GET_VARIABLE 0x83

#define PAVR2_VARIABLE_LAST_DEVICE_RESET 1
#define PAVR2_VARIABLE_PROGRAMMING_ERROR 2
#define PAVR2_VARIABLE_TARGET_VCC_MEASURED_MIN 3
#define PAVR2_VARIABLE_TARGET_VCC_MEASURED_MAX 4
#define PAVR2_VARIABLE_PROGRAMMER_VDD_MEASURED_MIN 5
#define PAVR2_VARIABLE_PROGRAMMER_VDD_MEASURED_MAX 6
#define PAVR2_VARIABLE_TARGET_VCC 7
#define PAVR2_VARIABLE_PROGRAMMER_VDD 8
#define PAVR2_VARIABLE_REGULATOR_LEVEL 9
#define PAVR2_VARIABLE_IN_PROGRAMMING_MODE 10

#define PAVR2_REQUEST_DIGITAL_READ 0x84

// The minimum allowed value for the ISP_FASTEST_PERIOD setting.
// Values faster/lower behave the same as 2 (6000 MHz).
#define PAVR2_ISP_FASTEST_PERIOD_MIN 2

// The largest allowed value for the ISP_FASTEST_PERIOD setting.  Values
// slower/higher than this are not needed, because they are slower than the
// SCK_DURATION=1 (444 kHz) and allowing the user to set them would make the
// name "fastest period" and "max frequency" be invalid.
#define PAVR2_ISP_FASTEST_PERIOD_MAX 26

/* The conversion factor between all the voltage variables above and
 * millivolts.  */
#define PAVR2_VOLTAGE_UNITS 32

/* VCC output indicator modes. */
#define PAVR2_VCC_OUTPUT_INDICATOR_BLINKING 0
#define PAVR2_VCC_OUTPUT_INDICATOR_STEADY 1

/* Line functions: */
#define PAVR2_LINE_IS_NOTHING 0  // Line is not used for anything.
#define PAVR2_LINE_IS_DTR 1      // DB9 Pin 4 - OUT - Data Terminal Ready
#define PAVR2_LINE_IS_RTS 2      // DB9 Pin 7 - OUT - Request to Send
#define PAVR2_LINE_IS_CD 3       // DB9 Pin 1 - IN - Carrier Detect
#define PAVR2_LINE_IS_DSR 4      // DB9 Pin 6 - IN - Data Set Ready
#define PAVR2_LINE_IS_CLOCK 5    // Clock output (line B only)
#define PAVR2_LINE_IS_DTR_RESET 6  // Open drain output, pulses low when DTR gets asserted.

/* To enter bootloader mode, send this request:
 *   bRequestType = 0x40
 *   bRequest = REQUEST_START_BOOTLOADER
 *   wValue = 0
 *   wIndex = 0
 *   wLength = 0
 */
#define PAVR2_REQUEST_START_BOOTLOADER 0xFF

/* Causes of programmer resets. */
#define PAVR2_RESET_POWER_UP 0
#define PAVR2_RESET_BROWNOUT 1
#define PAVR2_RESET_RESET_LINE 2
#define PAVR2_RESET_WATCHDOG 4
#define PAVR2_RESET_SOFTWARE 8
#define PAVR2_RESET_STACK_OVERFLOW 16
#define PAVR2_RESET_STACK_UNDERFLOW 32

/* Programming errors. */
#define PAVR2_PROGRAMMING_ERROR_TARGET_POWER_BAD 1
#define PAVR2_PROGRAMMING_ERROR_SYNCH 2
#define PAVR2_PROGRAMMING_ERROR_IDLE_FOR_TOO_LONG 3
#define PAVR2_PROGRAMMING_ERROR_USB_NOT_CONFIGURED 4
#define PAVR2_PROGRAMMING_ERROR_USB_SUSPEND 5
#define PAVR2_PROGRAMMING_ERROR_PROGRAMMER_POWER_BAD 6

/* Regulator levels (current setting of the regulator). */
#define PAVR2_REGULATOR_LEVEL_3V3 3
#define PAVR2_REGULATOR_LEVEL_5V 5

/* Regulator modes (policy that decides the regulator level). */
#define PAVR2_REGULATOR_MODE_AUTO 0
#define PAVR2_REGULATOR_MODE_3V3 3
#define PAVR2_REGULATOR_MODE_5V 5

#endif
